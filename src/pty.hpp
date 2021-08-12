#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace ccterm::pty {
/// A wrapper around a POSIX PTY.
class PseudoTerm final {
   public:
    /// Creates the PTY
    /// and spawns the /bin/sh process in it.
    /// This function does not block.
    PseudoTerm();
    PseudoTerm(PseudoTerm&) = delete;
    PseudoTerm& operator=(PseudoTerm) = delete;
    PseudoTerm(PseudoTerm&&) = delete;
    PseudoTerm& operator=(PseudoTerm&&) = delete;
    ~PseudoTerm();
    /// Try to read from the PTY.
    /// Does not block.
    std::optional<std::string> read();
    /// Write to the PTY.
    void write(std::string_view text);

   private:
    const int m_master_fd;
    const int m_slave_fd;
};
}  // namespace ccterm::pty
