#pragma once

#include <SDL_render.h>
#include <SDL_ttf.h>

#include <string>
#include <string_view>

namespace ccterm::render {
/// A terminal renderer, coupled to an SDL window.
class Renderer final {
   public:
    Renderer();
    Renderer(Renderer&) = delete;
    Renderer& operator=(Renderer) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;
    ~Renderer();
    /// Append text to the rendered window.
    void append_text(std::string_view new_text);
    /// Redraw the window with updated text.
    void redraw();

   private:
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    TTF_Font* m_font;
    SDL_Color m_text_color;
    SDL_Color m_bg_color;
    std::string m_contents;
};
}  // namespace ccterm::render
