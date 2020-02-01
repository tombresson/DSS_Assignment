//Using SDL and standard IO
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <stdbool.h>

// Screen dimension constants
const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;

// The window we'll be rendering to
SDL_Window* g_window = NULL;

// The surface contained by the window
SDL_Surface* g_screen_surface = NULL;

// The image we will load and show on the screen
SDL_Surface* g_background_surface = NULL;
SDL_Surface* g_loading_surface = NULL;

bool init()
{
	// Initialization flag
	bool success = true;

	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		// load support for the JPG and PNG image formats
		int flags = IMG_INIT_JPG | IMG_INIT_PNG;
		int initted = IMG_Init(flags);
		if ((initted & flags) != flags) {
			printf("IMG_Init: Failed to init required jpg and png support!\n");
			printf("IMG_Init: %s\n", IMG_GetError());
			// handle error
		}

		// Create window
		g_window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (g_window == NULL)
		{
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			// Get window surface
			g_screen_surface = SDL_GetWindowSurface(g_window);
		}
	}

	return success;
}

bool loadMedia()
{
	// Loading success flag
	bool success = true;

	// Load splash image
	g_background_surface = IMG_Load("1.jpg");
	if (g_background_surface == NULL)
	{
		printf("Unable to load image %s! SDL Error: %s\n", "1.bmp", SDL_GetError());
		success = false;
	}

	g_loading_surface = IMG_Load("loading.png");
	if (g_loading_surface == NULL)
	{
		printf("Unable to load image %s! SDL Error: %s\n", "loading.png", SDL_GetError());
		success = false;
	}


	return success;
}

void close()
{
	// Deallocate surface
	SDL_FreeSurface(g_background_surface);
	g_background_surface = NULL;
	SDL_FreeSurface(g_loading_surface);
	g_loading_surface = NULL;

	// Destroy window
	SDL_DestroyWindow(g_window);
	g_window = NULL;

	// Quit SDL subsystems
	SDL_Quit();
}

int display()
{
	// Start up SDL and create window
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		// Load media
		if (!loadMedia())
		{
			printf("Failed to load media!\n");
		}
		else
		{
			// Apply the image
			SDL_BlitSurface(g_background_surface, NULL, g_screen_surface, NULL);
			SDL_BlitSurface(g_loading_surface, NULL, g_screen_surface, NULL);

			// Update the surface
			SDL_UpdateWindowSurface(g_window);
			SDL_Delay(3000);

			// Done loading.
			SDL_BlitSurface(g_background_surface, NULL, g_screen_surface, NULL);
			SDL_UpdateWindowSurface(g_window);
			SDL_Delay(2000);
		}
	}

	// Free resources and close SDL
	close();

	return 0;
}
