/* -------------------------------------------------------------------------- */
/* Internal Headers                                                           */
/* -------------------------------------------------------------------------- */
#include <iostream>
#include <memory>

/* -------------------------------------------------------------------------- */
/* External Headers                                                           */
/* -------------------------------------------------------------------------- */
#include "SDL.h"

/* -------------------------------------------------------------------------- */
/* Macros                                                                     */
/* -------------------------------------------------------------------------- */
#define WINDOW_WIDTH        800
#define WINDOW_HEIGHT       600

#define UPDATE_INTERVAL     200.0f

/* -------------------------------------------------------------------------- */
/* SDL2 Application                                                           */
/* -------------------------------------------------------------------------- */
class App
{
private:
    std::shared_ptr<SDL_Window> _window = nullptr;
    std::shared_ptr<SDL_Renderer> _render = nullptr;
    std::shared_ptr<SDL_Texture> _texture = nullptr;
    Uint32 _width = WINDOW_WIDTH;
    Uint32 _height = WINDOW_HEIGHT;
    double _deltaTime = 0.0f;

    void _updateTexture(void);
    void _paintTexture(const Uint8 fixed_color);

public:
    explicit App(void);
    explicit App(Uint32 width, Uint32 height);
    explicit App(const App& app);
    explicit App(App&& app);
    App& operator=(const App& app);
    App& operator=(App&& app);
    ~App();

    void run(void);
};

// App Constructor -------------------------------------------------------------
App::App(Uint32 width, Uint32 height) : _width(width), _height(height)
{
    // SDL2 Initialization -----------------------------------------------------
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "Error: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }
    else
        atexit(SDL_Quit);
    
    // Window Creation ---------------------------------------------------------
    _window = std::shared_ptr<SDL_Window>(
        SDL_CreateWindow("Color",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            _width, _height,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE),
        SDL_DestroyWindow
    );

    if (!_window)
    {
        std::cerr << "Error: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }

    // Renderer Creation -------------------------------------------------------
    _render = std::shared_ptr<SDL_Renderer>(
       SDL_CreateRenderer(_window.get(),
            -1,
            SDL_RENDERER_ACCELERATED |
            SDL_RENDERER_TARGETTEXTURE),
        SDL_DestroyRenderer
    );

    if (!_render)
    {
        std::cerr << "Error: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }

    // Texture Creation --------------------------------------------------------
    _texture = std::shared_ptr<SDL_Texture>(
       SDL_CreateTexture(_render.get(),
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_STREAMING,
            _width, _height),
        SDL_DestroyTexture
    );

    if (!_texture)
    {
        std::cerr << "Error: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Application Main Loop -------------------------------------------------------
void
App::run(void)
{
    Uint32 currTime = 0, lastTime = 0;
    double updateInterval = UPDATE_INTERVAL / 1000;
    for (bool run = true;run;)
    {
        // Delta Time Calculation ----------------------------------------------
        lastTime = currTime;
        currTime = SDL_GetTicks64();
        _deltaTime = static_cast<double>(currTime - lastTime) / 1000;

        // Resize Detaction ----------------------------------------------------
        int nw = 0, nh = 0;
        SDL_GetWindowSize(_window.get(), &nw, &nh);
        if (nw != _width || nh != _height)
        {
            _width = nw;
            _height = nh;

            _texture.reset(
                SDL_CreateTexture(_render.get(),
                    SDL_PIXELFORMAT_RGBA8888,
                    SDL_TEXTUREACCESS_STREAMING,
                    _width,
                    _height),
                SDL_DestroyTexture);
            if (!_texture)
            {
                std::cerr << "Error: " << SDL_GetError() << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        // Event Handler -------------------------------------------------------
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch(event.key.keysym.sym)
            {
                case SDL_QUIT:
                case SDLK_ESCAPE:
                    run = false;
            }
        }

        // Render Update -------------------------------------------------------
        SDL_SetRenderDrawColor(_render.get(), 0, 0, 0, 255);
        SDL_RenderClear(_render.get());

        _updateTexture();
        SDL_RenderCopy(_render.get(), _texture.get(), NULL, NULL);
        
        SDL_RenderPresent(_render.get());
    }
}

// Update Texture --------------------------------------------------------------
void
App::_updateTexture(void)
{
    static double bi = 0.0f;
    static enum Direction
    {
        UP,
        DOWN
    } direction = Direction::UP;

    // Speed Color Changing ----------------------------------------------------
    double speed = UPDATE_INTERVAL / 1000 * _deltaTime;
    switch (direction)
    {
        case UP:
            bi += speed;
            break;
        case DOWN:
            bi -= speed;
            break;
    }

    // Set Initial Values ------------------------------------------------------
    if (bi >= 1.0f)
    {
        bi = 1.0f;
        direction = Direction::DOWN;
    }
    else if (bi <= 0.0f)
    {
        bi = 0.0f;
        direction = Direction::UP;
    }

    // Paint Texture With Fixed Color ------------------------------------------
    Uint8 fixed_color = bi * 255.999;
    _paintTexture(fixed_color);
}

// Paint Texture ---------------------------------------------------------------
void
App::_paintTexture(const Uint8 fixed_color)
{
    int pitch = -1;
    void* pixels = nullptr;
    if (SDL_LockTexture(_texture.get(), NULL, &pixels, &pitch) < 0)
    {
        std::cerr << "Error: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }

    for (int y = 0; y < _height; ++y)
    {
        for (int x = 0; x < _width; ++x)
        {
            Uint8 r = x * (static_cast<double>(255) / _width);
            Uint8 g = y * (static_cast<double>(255) / _height);
            Uint8 b = fixed_color;

            int index = y * pitch + x * 4;
            Uint32 color = (r << 24) | (g << 16) | (b << 8) | 0xFF;
            reinterpret_cast<Uint32*>(pixels)[index / 4] = color;
        }
    }

    SDL_UnlockTexture(_texture.get());
}

App::~App() { }

/* -------------------------------------------------------------------------- */
/* Entry Point                                                                */
/* -------------------------------------------------------------------------- */
int
main(
    int const                argc,
    char const* const* const argv,
    char const* const* const envp)
{
    App app(WINDOW_WIDTH, WINDOW_HEIGHT);
    app.run();

    return EXIT_SUCCESS;
}
