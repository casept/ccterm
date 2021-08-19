#pragma once

#include <SDL_render.h>
#include <SDL_ttf.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace ccterm::render {
/// A terminal renderer, coupled to an SDL window.
///
/// Features like scrollback probably only work properly for
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
    /// Try to scroll up by 1 line.
    /// Returns a boolean stating whether the operation succeeded.
    bool scroll_up();
    /// Scroll down by 1 line.
    /// Returns a boolean stating whether the operation succeeded.
    bool scroll_down();

   private:
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    TTF_Font* m_font;
    SDL_Color m_text_color;
    SDL_Color m_bg_color;
    /// Contents are represented as collection of lines.
    /// This also serves as a scrollback buffer.
    ///
    /// Note that these lines are as passed to the renderer, so e.g. line breaks introduced
    /// by line wrapping are ignored. This is to enable resizing the window, causing a re-wrap.
    std::vector<std::string> m_lines;
    /// Whether the last line in `m_lines` has been finished,
    /// and new input should therefore start a new line.
    bool m_last_line_complete;
    /// A cache of `m_lines`, with manual line-wraps inserted where appropriate.
    /// Must be updated when line with or `m_lines` changes.
    std::vector<std::string> m_lines_wrapped_cache;
    /// Offset into the wrapped line cache at which to start drawing.
    /// Used to allow for scrolling.
    std::size_t m_lines_wrapped_cache_scroll_offset;

    void update_line_cache();
    std::size_t compute_num_chars_per_line();
};
}  // namespace ccterm::render
