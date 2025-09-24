#define SDL_MAIN_HANDLED

#include <SDL.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <math.h>
#include <iostream>
#include <random>

const int GRID_WIDTH = 1000;
const int GRID_HEIGHT = 1000;
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
// This may need to be tuned
const int PERCEPTION_RADIUS = 50;

// For anglular maths
constexpr double RADIAN_CONVERTER = 3.14159 / 180.0d;

// Structure represents boid
struct Boid 
{
	Boid(int x_, int y_, double vx_, double vy_)
	{
		x = x_;
		y = y_;
		vx = vx_;
		vy = vy_;
	}

	int x, y;

	double vx = 0.0d;
	double vy = 0.0d;

	double get_distance(int other_x, int other_y)
	{
		double dx = x - other_x;
		double dy = y - other_y;
		return sqrt(dx*dx + dy*dy);
	}

	void avoid_collisions(Boid& other)
	{
		// Calculate dist
		double dx = (x + vx) - other.x;
		double dy = (y + vy) - other.y;
		double dist = sqrt(dx*dx + dy*dy);
		const double MIN_DIST = 20.0d;

		// Magic number 0.5 here is the replusion factor
		if (dist < MIN_DIST)
		{
			vx += (dx / dist) * 0.5d;
			vy += (dy / dist) * 0.5d;
		}
	}

	void align(Boid& other)
	{
		vx += 0.05 * (other.vx - vx);
		vy += 0.05 * (other.vy - vy);
	}

	void move_position_by_direction()
	{
		double speed = sqrt(vx*vx + vy*vy);
		const double MAX_SPEED = 2.0d;
		// Speed limits
		if (speed > MAX_SPEED)
		{
			vx = (vx / speed) * MAX_SPEED;
			vy = (vy / speed) * MAX_SPEED;
		}

		// Bounce off walls
		if (x + vx >= GRID_WIDTH || x + vx <= 0) vx *= -1;
		if (y + vy >= GRID_HEIGHT || y + vy <= 0) vy *= -1;

		x += vx;
		y += vy;

		// Clamp just in case
		if (x >= GRID_WIDTH) x = GRID_WIDTH - 1;
		else if (x <= 0) x = 0;
		if (y >= GRID_HEIGHT) y = GRID_HEIGHT - 1;
		else if (y <= 0) y = 0;
	}

	void cohesion(std::vector<Boid>& boids)
	{
		double total_x = 0.0d;
		double total_y = 0.0d;

		for (auto& b: boids)
		{
			total_x += b.x;
			total_y += b.y;
		}

		double average_x = total_x / boids.size();
		double average_y = total_y / boids.size();

		// Steer towards centre
		vx += 0.01 * (average_x - x);
		vy += 0.01 * (average_y - y);
	}
};

int main()
{
	std::cout << "Initialised" << std::endl;
	if(SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		std::cerr << "SDL_Init error: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("Grid Texture Optimisation",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			WINDOW_WIDTH, WINDOW_HEIGHT, 0);

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	// Create texture buffer
	SDL_Texture* grid_texture = SDL_CreateTexture(renderer,
			SDL_PIXELFORMAT_RGBA8888,
			SDL_TEXTUREACCESS_STREAMING,
			GRID_WIDTH, GRID_HEIGHT);

	std::cout << "Textures created" << std::endl;
	// Initialise a few random boids
	std::vector<Boid> boids;
	std::srand(static_cast<unsigned int>(std::time(nullptr)));
	std::uniform_real_distribution<double> uniform(0, 1);
	std::default_random_engine re;

	// Build boids vector
	for (int i = 0; i < 2000; ++i)
	{
		int x = std::rand() % GRID_WIDTH;
		int y = std::rand() % GRID_HEIGHT;
		double vx = uniform(re);
		double vy = uniform(re);

		boids.push_back({x, y, vx, vy});
	}

	bool is_running = true;
	SDL_Event event;

	while (is_running)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT) is_running = false;
		}

		// Clear texture pixels
		void* pixels;
		int pitch;

		SDL_LockTexture(grid_texture, NULL, &pixels, &pitch);
		
		// Fade previous frame
		uint32_t* pixel_array = static_cast<uint32_t*>(pixels);
		for (int i = 0; i < GRID_WIDTH * GRID_HEIGHT; ++i)
		{
			uint8_t r = (pixel_array[i] >> 24) & 0xFF;
			uint8_t g = (pixel_array[i] >> 16) & 0xFF;
			uint8_t b = (pixel_array[i] >> 8) & 0xFF;

			// Fade each channel towards black
			r = r * 0.9;
			g = g * 0.9;
			b = b * 0.9;

			pixel_array[i] = (r << 24) | (g << 16) | (b << 8) | 0xFF;
		}

		// Update active cells
		for (auto& b : boids)
		{
			int index = b.y * (pitch / 4) + b.x;
			pixel_array[index] = 0xFFFFFFFF;
		}

		SDL_UnlockTexture(grid_texture);

		// Render texture to window
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		SDL_Rect dst_rect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
		SDL_RenderCopy(renderer, grid_texture, NULL, &dst_rect);
		SDL_RenderPresent(renderer);

		// Boids logic here
		// Random boids movement
		std::vector<Boid> updated_positions = boids;

		for (auto& b: updated_positions)
		{
			// Get all boids within perception radius
			std::vector<Boid> neighbours;
			// TODO: This is n^2, optimise it down
			for (auto& d: boids)
			{
				if (b.get_distance(d.x, d.y) < PERCEPTION_RADIUS)
				{
					neighbours.push_back(d);
				}
			}

			// Align, then cohese, then avoid
			for (auto& d: neighbours)
			{
				b.align(d);
			}

			b.cohesion(neighbours);

			for (auto& d: neighbours)
			{
				b.avoid_collisions(d);
			}

			b.move_position_by_direction();
		}
		boids = updated_positions;

		SDL_Delay(16); //~60fps
	}

	SDL_DestroyTexture(grid_texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
