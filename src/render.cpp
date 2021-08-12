#include "render.hpp"

#include <SDL_render.h>
#include <SDL_ttf.h>
#include <SDL_video.h>
#include <fmt/core.h>

#include <string>
#include <string_view>

/// White
static constexpr SDL_Color TEXT_COLOR{0xFF, 0xFF, 0xFF};
/// Black
static constexpr SDL_Color BG_COLOR{0x00, 0x00, 0x00};
/// Go monospaced font
static constexpr std::string_view FONT_PATH = "../Go-Mono.ttf";
static constexpr int FONT_SIZE_PT = 16;

namespace ccterm::render {
Renderer::Renderer()
    : m_window(nullptr),
      m_renderer(nullptr),
      m_font(nullptr),
      m_text_color(TEXT_COLOR),
      m_bg_color(BG_COLOR),
      m_contents("") {
    // Initialize video subsystem
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error{
            fmt::format("Renderer::Renderer(): Failed to initialize SDL2 video subsystem: {}", SDL_GetError())};
    }

    // Initialize TTF library
    if (TTF_Init() < 0) {
        throw std::runtime_error{
            fmt::format("Renderer::Renderer(): Failed to initialize SDL2_TTF: {}", TTF_GetError())};
    }

    // Create window
    // TODO: Determine size dynamically
    const int width = 640;
    const int height = 480;
    m_window = SDL_CreateWindow("ccterm", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
                                SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (m_window == nullptr) {
        throw std::runtime_error{
            fmt::format("Renderer::Renderer(): Failed to initialize SDL2 window: {}", SDL_GetError())};
    }

    // Create renderer
    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (m_renderer == nullptr) {
        throw std::runtime_error{
            fmt::format("Renderer::Renderer(): Failed to initialize SDL2 renderer: {}", SDL_GetError())};
    }

    // Open font
    m_font = TTF_OpenFont(std::string(FONT_PATH).c_str(), FONT_SIZE_PT);
    if (m_font == nullptr) {
        throw std::runtime_error{
            fmt::format("Renderer::Renderer(): Failed to load TTF font from path '{}': {}", FONT_PATH, TTF_GetError())};
    }

    fmt::print("Renderer::Renderer(): Initialization complete\n");
}

void Renderer::append_text(std::string_view new_text) { m_contents.append(new_text); }

void Renderer::redraw() {
    SDL_SetRenderDrawColor(m_renderer, m_bg_color.r, m_bg_color.g, m_bg_color.b, m_bg_color.a);
    SDL_RenderClear(m_renderer);

    // Having no text to draw causes SDL_TTF to error
    if (m_contents.size() == 0) {
        return;
    }
    // Render text to surface
    int window_width;
    SDL_GetWindowSize(m_window, &window_width, NULL);
    SDL_Surface* text_surf = TTF_RenderText_Blended_Wrapped(m_font, m_contents.c_str(), m_text_color, window_width);
    if (text_surf == nullptr) {
        throw std::runtime_error{
            fmt::format("Renderer::redraw(): Failed to initialize SDL2_TTF surface: {}", TTF_GetError())};
    }

    // Convert surface to texture
    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, text_surf);
    if (texture == nullptr) {
        throw std::runtime_error{
            fmt::format("Renderer::redraw(): Failed to convert text surface to texture: {}", SDL_GetError())};
    }

    // Render texture
    int texture_width, texture_height;
    SDL_QueryTexture(texture, NULL, NULL, &texture_width, &texture_height);
    const SDL_Rect dstrect{
        .x = 0,
        .y = 0,
        .w = texture_width,
        .h = texture_height,
    };
    SDL_RenderCopy(m_renderer, texture, NULL, &dstrect);
    SDL_RenderPresent(m_renderer);
    SDL_FreeSurface(text_surf);
    SDL_DestroyTexture(texture);
}

Renderer::~Renderer() {
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    TTF_CloseFont(m_font);
    m_window = nullptr;
    m_renderer = nullptr;
    m_font = nullptr;
    TTF_Quit();
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    fmt::print("Renderer::~Renderer(): Destroyed\n");
}
}  // namespace ccterm::render
