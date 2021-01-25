/*This source code copyrighted by Lazy Foo' Productions (2004-2020)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, standard IO, and strings
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string>

//Screen dimension constants
const int SCREEN_WIDTH = 1440;
const int SCREEN_HEIGHT = 1080;

//Texture wrapper class
class LTexture {
	public:
		static const size_t SCALE_FACTOR = 10;
		LTexture();
		~LTexture();
		bool loadFromFile(std::string path);
		void free();
		void setColor(Uint8 red, Uint8 green, Uint8 blue);
		void setBlendMode(SDL_BlendMode blending);
		void setAlpha(Uint8 alpha);
		void render(int x, int y, SDL_Rect* clip = NULL, SDL_RendererFlip flipType = SDL_FLIP_NONE);
		size_t getWidth();
		size_t getHeight();

	private:
		SDL_Texture* texture;
		size_t width;
		size_t height;
};

class Dwarf {
    public:
		static const int DWARF_VEL = 7;
		static const int WALKING_ANIMATION_FRAMES = 8;
		Dwarf();
		void handleEvent(SDL_Event& e);
		void move();
		void render(size_t frame);

    private:
		int posX, posY;
		size_t width, height;
		int velX, velY;
		SDL_RendererFlip flipType;
};

bool init();
bool loadMedia();
void close();
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_Rect gSpriteClips[Dwarf::WALKING_ANIMATION_FRAMES];
LTexture gDwarfTexture;

LTexture::LTexture() {
	//Initialize
	texture = NULL;
	width = 0;
	height = 0;
}

LTexture::~LTexture() {
	//Deallocate
	free();
}

bool LTexture::loadFromFile(std::string path) {
	free();
	SDL_Texture* newTexture = NULL;
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL) {
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	} else {
		// SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));
        SDL_Surface* stretchedSurface = SDL_CreateRGBSurface(
			loadedSurface->flags,
			loadedSurface->w * SCALE_FACTOR,
			loadedSurface->h * SCALE_FACTOR,
			32,
			loadedSurface->format->Rmask,
			loadedSurface->format->Gmask,
			loadedSurface->format->Bmask,
			loadedSurface->format->Amask);
		auto stretchRect = SDL_Rect{0, 0, loadedSurface->w * SCALE_FACTOR, loadedSurface->h * SCALE_FACTOR};
		SDL_BlitScaled(loadedSurface, NULL, stretchedSurface, &stretchRect);
		// SDL_FreeSurface(loadedSurface);
		newTexture = SDL_CreateTextureFromSurface(gRenderer, stretchedSurface);
		// newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
		if (newTexture == NULL) {
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		} else {
			width = loadedSurface->w;
			height = loadedSurface->h;
		}
		SDL_FreeSurface(loadedSurface);
	}
	texture = newTexture;
	return texture != NULL;
}

void LTexture::free() {
	if (texture != NULL) {
		SDL_DestroyTexture(texture);
		texture = NULL;
		width = 0;
		height = 0;
	}
}

void LTexture::setColor(Uint8 red, Uint8 green, Uint8 blue) {
	SDL_SetTextureColorMod(texture, red, green, blue);
}

void LTexture::setBlendMode(SDL_BlendMode blending) {
	SDL_SetTextureBlendMode(texture, blending);
}
		
void LTexture::setAlpha(Uint8 alpha) {
	SDL_SetTextureAlphaMod(texture, alpha);
}

void LTexture::render(int x, int y, SDL_Rect* clip, SDL_RendererFlip flipType) {
	SDL_Rect renderQuad = { x, y, width, height };
	if (clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}
	SDL_RenderCopyEx(gRenderer, texture, clip, &renderQuad, NULL, NULL, flipType);
}

size_t LTexture::getWidth() {
	return width;
}

size_t LTexture::getHeight() {
	return height;
}

Dwarf::Dwarf() {
    posX = 0;
    posY = 0;
    
	velX = 0;
    velY = 0;

	flipType = SDL_FLIP_NONE;
}

void Dwarf::handleEvent(SDL_Event& e) {
	if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
        switch (e.key.keysym.sym) {
            case SDLK_UP: velY -= DWARF_VEL; break;
            case SDLK_DOWN: velY += DWARF_VEL; break;
            case SDLK_LEFT:
				velX -= DWARF_VEL;
				flipType = SDL_FLIP_HORIZONTAL;
				break;
            case SDLK_RIGHT:
				velX += DWARF_VEL;
				flipType = SDL_FLIP_NONE;
				break;
        }
    }
    else if (e.type == SDL_KEYUP && e.key.repeat == 0) {
        switch (e.key.keysym.sym) {
            case SDLK_UP: velY += DWARF_VEL; break;
            case SDLK_DOWN: velY -= DWARF_VEL; break;
            case SDLK_LEFT: velX += DWARF_VEL; break;
            case SDLK_RIGHT: velX -= DWARF_VEL; break;
        }
    }
}

void Dwarf::move() {
    posX += velX;
    if ((posX < 0) || (posX + width > SCREEN_WIDTH)) {
        posX -= velX;
    }
    posY += velY;
    if ((posY < 0) || (posY + height > SCREEN_HEIGHT)) {
        posY -= velY;
    }
}

void Dwarf::render(size_t frame) {
	SDL_Rect* currentClip = &gSpriteClips[frame / 4];
	width = currentClip->w;
	height = currentClip->h;
	gDwarfTexture.render(posX, posY, currentClip, flipType);
}


bool init() {
	bool success = true;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		return false;
	}
	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
		printf("Warning: Linear texture filtering not enabled!");
	}
	gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (gWindow == NULL) {
		printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}
	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (gRenderer == NULL) {
		printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags)) {
		printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
		return false;
	}
	return true;
}

bool loadMedia() {
	if (!gDwarfTexture.loadFromFile("dwarf.png")) {
		printf("Failed to load walking animation texture!\n");
		return false;
	}
	int xAxisCoords [] = {5,  38,  70, 102, 133, 165, 197, 230};
	for (int i = 0; i != 8; i++) {
		gSpriteClips[i].x = xAxisCoords[i] * LTexture::SCALE_FACTOR;
		gSpriteClips[i].y = 42 * LTexture::SCALE_FACTOR;
		gSpriteClips[i].w = 26 * LTexture::SCALE_FACTOR;
		gSpriteClips[i].h = 21 * LTexture::SCALE_FACTOR;
	}
	// int xAxisCoords [] = {40, 304, 560, 816, 1064, 1320, 1576, 1840};
	// for (int i = 0; i != 8; i++) {
	// 	gSpriteClips[i].x = xAxisCoords[i];
	// 	gSpriteClips[i].y = 336;
	// 	gSpriteClips[i].w = 216;
	// 	gSpriteClips[i].h = 176;
	// }
	return true;
}

void close() {
	gDwarfTexture.free();
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;
	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char* args[]) {
	if (!init()) {
		printf("Failed to initialize!\n");
	} else {
		if (!loadMedia()) {
			printf("Failed to load media!\n");
		} else {	
			bool quit = false;
			SDL_Event e;
			Dwarf dwarf;
			int frame = 0;
			while (!quit) {
				while (SDL_PollEvent(&e) != 0) {
					if (e.type == SDL_QUIT) {
						quit = true;
					}
					dwarf.handleEvent(e);
				}
				dwarf.move();
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderClear(gRenderer);
				dwarf.render(frame);
				SDL_RenderPresent(gRenderer);
				++frame;
				if (frame / 4 >= Dwarf::WALKING_ANIMATION_FRAMES) {
					frame = 0;
				}
			}
		}
	}
	close();
	return 0;
}