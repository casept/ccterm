#include <SDL_events.h>
#include <SDL_keyboard.h>
#include <SDL_keycode.h>
#include <SDL_scancode.h>
#include <SDL_video.h>

#include <chrono>
#include <string>
#include <thread>

#include "pty.hpp"
#include "render.hpp"

using namespace ccterm;

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    SDL_StartTextInput();
    render::Renderer renderer{};
    pty::PseudoTerm term{};

    while (true) {
        // Note when to perform next update (crude 60FPS lock for I/O, as rendering is already locked)
        using std::chrono::operator""ms;
        const auto wake{std::chrono::steady_clock::now() + 16ms};

        std::string input{};

        // Handle events on queue
        SDL_Event e = {0};
        while (SDL_PollEvent(&e) != 0) {
            switch (e.type) {
                case SDL_QUIT:
                    SDL_StopTextInput();
                    return EXIT_SUCCESS;
                case SDL_TEXTINPUT:
                    input.append(e.text.text);
                    break;
                case SDL_KEYDOWN:
                    // Apparently return, backspace etc. need to be handled by hand rather than `TextInput`
                    switch (e.key.keysym.scancode) {
                        case SDL_SCANCODE_RETURN:
                        case SDL_SCANCODE_RETURN2:
                            input.append("\n");
                            break;
                        case SDL_SCANCODE_BACKSPACE:
                            input.append("\b");
                            break;
                        case SDL_SCANCODE_TAB:
                            input.append("\t");
                            break;
                        case SDL_SCANCODE_C:
                            if (e.key.keysym.mod == KMOD_LCTRL || e.key.keysym.mod == KMOD_RCTRL) {
                                input.append("\03");
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                case SDL_WINDOWEVENT:
                    renderer.redraw();

                default:
                    continue;
            }
        }

        // Update terminal display
        const auto data_read = term.read();
        if (data_read.has_value()) {
            renderer.append_text(data_read.value());
            renderer.redraw();
        }

        // Send input
        if (input.size() > 0) {
            term.write(input);
        }

        std::this_thread::sleep_until(wake);
    }
}
