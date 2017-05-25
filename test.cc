#include <iostream>
#include <fstream>
#include <cmath>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

template <typename T>
void write(std::ofstream& of, T val, size_t size = sizeof(T))
{
    for (; size; val >>= 8, --size)
    {
        of.put(static_cast<char> (val));
    }
}

const static int samplerate = 48000;
const static int channels = 1;
const static int seconds = 2;

static SDL_Window* window;
static SDL_Renderer* renderer;
static int width = 600;
static float scale = 1.0f;
static double dx;

static TTF_Font* font;
static SDL_Color white = {255, 255, 255};


void drawScale()
{
	SDL_RenderDrawLine(renderer, 0, 260, width, 260);
	char buf[64];
	SDL_Surface* bottom1 = TTF_RenderText_Solid(font, "0", white);
	SDL_Texture* t1 = SDL_CreateTextureFromSurface(renderer, bottom1);

	SDL_Rect r1;
	r1.x = 0;
	r1.y = 260;
	r1.w = 25;
	r1.h = 40;

	sprintf(buf, "%f", (double) (seconds * scale));
	SDL_Surface* bottom2 = TTF_RenderText_Solid(font, buf, white);
	SDL_Texture* t2 = SDL_CreateTextureFromSurface(renderer, bottom2);

	SDL_Rect r2;
	r2.x = width - 100;
	r2.y = 260;
	r2.w = 100;
	r2.h = 40;

	SDL_RenderCopy(renderer, t1, NULL, &r1);
	SDL_RenderCopy(renderer, t2, NULL, &r2);
}

void redraw(double freqi) 
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

	for (int i = 0; i < samplerate * seconds; i++)
	{
		double s = std::sin(2 * M_PI * freqi * scale * i / samplerate);

		SDL_RenderDrawPoint(renderer, (int) (600.0 - (double)(i)*dx), s * 100 + 150);
	}

	drawScale();
}

int WinMain(int argc, char** argv) {

	/* let's get the frequency from the user input */
	double freq;
	std::cout << "Frequenza: ";
	std::cin >> freq;

	/* let's make a window */

	bool quit = false;

	// create a window and renderer (a canvas to draw things on)
	SDL_CreateWindowAndRenderer(width, 300, 0, &window, &renderer);
	// pick black...
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	// ...and fill the entire window
	SDL_RenderClear(renderer);

	// now pick white, we'll use it to draw points
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

	dx = 600.0 / (samplerate * seconds);
	std::cout << dx << std::endl;

	/* let's make a WAV file */

	// create an empty audio file
	std::ofstream of ("out.wav", std::ios::binary);

	// write the first header, begin the file
    of << "RIFF"; 

    // write the size of the entire file, excluding everthing till now
    write(of, 36+samplerate*seconds*channels*16, 4);

    // second header, specify the format
    of << "WAVE";

    // yet another header, begin the format chunk
    of << "fmt ";

    // write the size of this format chunk, excluding everthing till now
    write(of, 16, 4);

    // write the audio format, 1 for PCM
    write(of, 1,  2);

    // write the channel count
    write(of, 1,  2);

    // write the samplerate
    write(of, samplerate, 4);

    // write the byte rate, or how big is the audio data in bytes
    write(of, samplerate*channels*16, 4);

    // write the block align, or (channels * bytes per sample)
    write(of, channels*16, 2);

    // write the bitrate, or how many bits are in each audio sample
    write(of, 16, 2);

    // last header, begin the part containing the actual data
    of << "data";

    // write the size of the data chunk, in bytes
    write(of, samplerate*seconds*channels*16, 4);

	for (int i = 0; i < samplerate * seconds; i++) 
	{
		// y = sin(2 * pi * f * t)
		double s = std::sin(2 * M_PI * freq * i / samplerate);

		// write the raw audio data to the output file
		// multiply the value by the maximum volume
		write(of, (short) (s * 32767.0), 2);

		// render the current sample on the window
		SDL_RenderDrawPoint(renderer, (int) (600.0 - (double)(i)*dx), s * 100 + 150);
	}

	// we're done, close the file
	of.close();

	TTF_Init();
	font = TTF_OpenFont("Ubuntu-R.ttf", 14);

	drawScale();

	SDL_Event e;
	while (!quit)
	{
		SDL_PollEvent(&e);
		if (e.type == SDL_QUIT)
			quit = true;
		if (e.type == SDL_KEYDOWN) {
			if (e.key.keysym.sym == SDLK_PLUS) {
				scale -= dx / 10.0;
				redraw(freq);
			} else if (e.key.keysym.sym == SDLK_MINUS) {
				scale += dx / 10.0;
				redraw(freq);
			} else if (e.key.keysym.sym == SDLK_r) {
				scale = 1.0f;
				redraw(freq);
			} else if (e.key.keysym.sym == SDLK_q) {
				scale -= dx;
				redraw(freq);
			} else if (e.key.keysym.sym == SDLK_a) {
				scale += dx;
				redraw(freq);
			}
		}

		SDL_RenderPresent(renderer);
		SDL_Delay(3);
	}

	return 0;
}