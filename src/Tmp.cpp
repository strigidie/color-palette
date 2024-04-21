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
/* Classes                                                                    */
/* -------------------------------------------------------------------------- */
class App
{
private:
    App(const App& app);
    App(App&& app);
    App& operator=(const App& app);
    App& operator=(App&& app);

public:
    explicit App(void);
    ~App();

    void run(void);
};

App::App(void)
{
    // SDL2 Initialization -----------------------------------------------------
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "Error: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }
    else
        atexit(SDL_Quit);
}

/* -------------------------------------------------------------------------- */
/* Paint Texture                                                              */
/* -------------------------------------------------------------------------- */
void
paintTexture(
    const std::shared_ptr<SDL_Texture>& texture,
    const int width, const int height,
    const Uint8 fixed_color)
{
    void* pixels = nullptr;
    int pitch = -1;
    if (SDL_LockTexture(texture.get(), NULL, &pixels, &pitch) < 0)
    {
        std::cerr << "Error: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            Uint8 r = x * (static_cast<double>(255) / width);
            Uint8 g = y * (static_cast<double>(255) / height);
            Uint8 b = fixed_color;

            int index = y * pitch + x * 4;
            Uint32 color = (r << 24) | (g << 16) | (b << 8) | 0xFF;
            reinterpret_cast<Uint32*>(pixels)[index / 4] = color;
        }
    }

    SDL_UnlockTexture(texture.get());
}

/* -------------------------------------------------------------------------- */
/* Update Texture                                                             */
/* -------------------------------------------------------------------------- */
void
updateTexture(
    const std::shared_ptr<SDL_Texture>& texture,
    const int width, const int height,
    const double deltaTime)
{
    static double bi = 0;
    enum class Direction
    {
        RIGHT,
        LEFT
    };
    static Direction direction = Direction::RIGHT;

    Uint8 fixed_color = bi * 255;
    paintTexture(texture, width, height, fixed_color);

    switch (direction)
    {
        case Direction::RIGHT:
            bi += UPDATE_INTERVAL / 1000 * deltaTime;
            break;
        case Direction::LEFT:
            bi -= UPDATE_INTERVAL / 1000 * deltaTime;
            break;
    }

    if (bi <= 0.0)
        direction = Direction::RIGHT;
    else if (bi >= 1.0)
        direction = Direction::LEFT;
}

/* -------------------------------------------------------------------------- */
/* Entry Point                                                                */
/* -------------------------------------------------------------------------- */
int
main(
    int const                argc,
    char const* const* const argv,
    char const* const* const envp)
{
    // SDL2 Initialization -----------------------------------------------------
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "Error: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }
    else
        atexit(SDL_Quit);

    // Window Creation ---------------------------------------------------------
    auto window = std::shared_ptr<SDL_Window>(
        SDL_CreateWindow("Color",
                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                        WINDOW_WIDTH, WINDOW_HEIGHT,
                        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE),
        SDL_DestroyWindow
    );

    if (!window)
    {
        std::cerr << "Error: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    // Renderer Creation -------------------------------------------------------
    auto render = std::shared_ptr<SDL_Renderer>(
       SDL_CreateRenderer(window.get(),
                        -1,
                        SDL_RENDERER_ACCELERATED |
                        SDL_RENDERER_TARGETTEXTURE),
        SDL_DestroyRenderer
    );

    if (!render)
    {
        std::cerr << "Error: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    // Texture Creation --------------------------------------------------------
    auto texture = std::shared_ptr<SDL_Texture>(
       SDL_CreateTexture(render.get(),
                        SDL_PIXELFORMAT_RGBA8888,
                        SDL_TEXTUREACCESS_STREAMING,
                        WINDOW_WIDTH,
                        WINDOW_HEIGHT),
        SDL_DestroyTexture
    );

    if (!texture)
    {
        std::cerr << "Error: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    // Main Loop ---------------------------------------------------------------
    int width = WINDOW_WIDTH, height = WINDOW_HEIGHT,
        nw = 0, nh = 0;
    
    Uint32 currTime = SDL_GetTicks64(), lastTime = 0;

    double deltaTime = 0.0;
    double updateInterval = UPDATE_INTERVAL / 1000;
    
    for (bool run = true;run;)
    {
        lastTime = currTime;
        currTime = SDL_GetTicks64();
        deltaTime = static_cast<double>(currTime - lastTime) / 1000;

        SDL_GetWindowSize(window.get(), &nw, &nh);
        if (nw != width || nh != height)
        {
            width = nw;
            height = nh;

            texture.reset(SDL_CreateTexture(render.get(),
                                            SDL_PIXELFORMAT_RGBA8888,
                                            SDL_TEXTUREACCESS_STREAMING,
                                            width,
                                            height),
                        SDL_DestroyTexture);
            if (!texture)
            {
                std::cerr << "Error: " << SDL_GetError() << std::endl;
                return EXIT_FAILURE;
            }
        }

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    run = false;
                    break;
                default:
                    break;
            }
        }

        SDL_SetRenderDrawColor(render.get(), 0, 0, 0, 255);
        SDL_RenderClear(render.get());

        updateTexture(texture, width, height, deltaTime);
        SDL_RenderCopy(render.get(), texture.get(), NULL, NULL);
        
        SDL_RenderPresent(render.get());
    }

    return EXIT_SUCCESS;
}
