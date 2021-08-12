#include "pty.hpp"

#include <fcntl.h>
#include <fmt/core.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <unistd.h>

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <optional>
#include <stdexcept>
#include <string>

namespace ccterm::pty {
static int get_master_pty() {
    // Obtain master PTY
    const int master_fd = posix_openpt(O_RDWR | O_NOCTTY);
    if (master_fd == -1) {
        throw std::runtime_error{
            fmt::format("PseudoTerm::PseudoTerm(): Failed to open master PTY device: {}", strerror(errno))};
    }

    // Grant permissions for master PTY
    if (grantpt(master_fd) == -1) {
        throw std::runtime_error{
            fmt::format("PseudoTerm::PseudoTerm(): Failed to grant permissions on PTY device: {}", strerror(errno))};
    }

    // Unlock master PTY
    if (unlockpt(master_fd) == -1) {
        throw std::runtime_error{
            fmt::format("PseudoTerm::PseudoTerm(): Failed to unlock PTY device: {}", strerror(errno))};
    }
    return master_fd;
}

static int get_slave_pty(int master_fd) {
    // Obtain slave's filepath
    char* slave_name_cstr = ptsname(master_fd);
    if (slave_name_cstr == nullptr) {
        throw std::runtime_error{
            fmt::format("PseudoTerm::PseudoTerm(): Failed to obtain slave PTY device: {}", strerror(errno))};
    }

    // Obtain slave FD using a regular open()
    const int slave_fd = open(slave_name_cstr, O_RDWR | O_NOCTTY);
    if (slave_fd == -1) {
        throw std::runtime_error{
            fmt::format("PseudoTerm::PseudoTerm(): Failed to open slave PTY device: {}", strerror(errno))};
    }
    return slave_fd;
}

PseudoTerm::PseudoTerm() : m_master_fd(get_master_pty()), m_slave_fd(get_slave_pty(m_master_fd)) {
    // Fork off process and hook it up
    const pid_t pid = fork();
    if (pid == -1) {
        throw std::runtime_error{fmt::format("PseudoTerm::PseudoTerm(): Failed to fork slave: {}", strerror(errno))};
    }
    if (pid == 0) {
        // Not the master
        close(m_master_fd);

        // Become leader of a new session and process group
        setsid();

        // Declare this terminal as controlling terminal for session.
        if (ioctl(m_slave_fd, TIOCSCTTY, NULL) == -1) {
            throw std::runtime_error{fmt::format(
                "PseudoTerm::PseudoTerm(): Failed to set slave PTY as session's controlling TTY: {}", strerror(errno))};
        }

        // Hook up stdin/stdout/stderr to PTY slave FD
        dup2(m_slave_fd, 0);
        dup2(m_slave_fd, 1);
        dup2(m_slave_fd, 2);
        close(m_slave_fd);

        // Finally, become the shell process
        execle("/bin/sh", "/bin/sh", NULL, environ);
    }
    if (pid > 0) {
        fmt::print("PseudoTerm::PseudoTerm(): Initialization complete\n");
        return;
    }
}

std::optional<std::string> PseudoTerm::read() {
    struct pollfd fd {
        .fd = m_master_fd, .events = POLLIN | POLLPRI, .revents = 0,
    };
    const int have_events = poll(&fd, 1, 0);
    if (have_events == -1) {
        throw std::runtime_error{fmt::format("PseudoTerm::read(): poll() syscall failed: {}", strerror(errno))};
    }
    if (have_events == 0) {
        // We have no events; return None
        return {};
    }

    // Read up to 1024 bytes
    std::vector<char> buf_vec{};
    buf_vec.resize(1024);
    const ssize_t n_read = ::read(m_master_fd, buf_vec.data(), buf_vec.size());
    if (n_read == -1) {
        throw std::runtime_error{fmt::format("PseudoTerm::read(): read() syscall failed: {}", strerror(errno))};
    }
    const auto as_string = std::string(buf_vec.data());
    return {as_string};
}

void PseudoTerm::write(std::string_view text) {
    ssize_t written = 0;
    while (written < text.size()) {
        const ssize_t written_now = ::write(m_master_fd, text.data() + written, text.size() - written);
        if (written_now == -1) {
            throw std::runtime_error{fmt::format("PseudoTerm::write(): write() syscall failed: {}", strerror(errno))};
        }
        written += written_now;
    }
}

PseudoTerm::~PseudoTerm() {
    // TODO: Terminate child process
    close(m_master_fd);
    close(m_slave_fd);

    fmt::print("PseudoTerm::~PseudoTerm(): Destroyed\n");
}
}  // namespace ccterm::pty
