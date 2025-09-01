#include <SDL3/SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

// util macros (should be in in a common util file somewhere)
#define NANOS(x)   (x)                // converts nanoseconds to nanoseconds
#define MILLIS(x)  ((x) * 1000000)    // converts milliseconds to nanoseconds
#define SECONDS(x) ((x) * 1000000000) // converts seconds to nanoseconds

#define NS_TO_MILLIS(x)  ((float)(x)/(float)1000000)    // converts nanoseconds to milliseconds
#define NS_TO_SECONDS(x) ((float)(x)/(float)1000000000) // converts nanoseconds to seconds

// compile switches (can probably be managed by the building system)
#define ENABLE_DIAGNOSTICS

// gameplay stuff
#define ASTEROIDS_COUNT 10

// contains all the "backend" stuff, not related to gameplay
struct SDLContext
{
	SDL_Renderer* renderer;
	int target_framerate_ns;
	float zoom;     // render zoom
	float window_w; // logic window width, after render zoom as been applied
	float window_h; // logic window height, after render zoom as been applied

	float delta;
	
	// NOTE: list of stuff with the same prefix? Looks like it's a good candidate for consolidation
	bool btn_pressed_up;
	bool btn_pressed_down;
	bool btn_pressed_left;
	bool btn_pressed_right;

	bool quit;
};

// contains everything related to gameplay
struct GameState
{
	float player_speed;
	float player_size;
	SDL_FRect player_rect;
};

void init(SDLContext* context, GameState* state)
{
	// init player
	state->player_speed = 2;
	state->player_size = 40;
	state->player_rect.w = state->player_size;
	state->player_rect.h = state->player_size;
	state->player_rect.x = context->window_w / 2 - state->player_size / 2;
	state->player_rect.y = context->window_h / 2 - state->player_size / 2;
}

void update(SDLContext* context, GameState* state)
{
	// update player
	if(context->btn_pressed_up)    state->player_rect.y -= context->delta * state->player_speed;
	if(context->btn_pressed_down)  state->player_rect.y += context->delta * state->player_speed;
	if(context->btn_pressed_left)  state->player_rect.x -= context->delta * state->player_speed;
	if(context->btn_pressed_right) state->player_rect.x += context->delta * state->player_speed;

	// render player
	SDL_SetRenderDrawColor(context->renderer, 0x3C, 0x63, 0xFF, 0XFF);
	SDL_RenderRect(context->renderer, &state->player_rect);
}

int main(void)
{
	GameState state = { 0 };
	SDLContext context = { 0 };
	context.window_w = 600;
	context.window_h = 800;
	context.target_framerate_ns = SECONDS(1) / 60; // 16666666 nanoseconds

	SDL_Window* window = SDL_CreateWindow("E01 - rendering", context.window_w, context.window_h, 0);
	context.renderer = SDL_CreateRenderer(window, NULL);

	// increase the zoom to make debug text more legible
	// (ie, on the class projector, we will usually use 2)
	{
		context.zoom = 1;
		context.window_w /= context.zoom;
		context.window_h /= context.zoom;
		SDL_SetRenderScale(context.renderer, context.zoom, context.zoom);
	}

	init(&context, &state);

	SDL_Time walltime_frame_beg;
	SDL_Time walltime_work_end;
	SDL_Time walltime_frame_end;
	SDL_Time time_elapsed_frame;
	SDL_Time time_elapsed_work;

	SDL_GetCurrentTime(&walltime_frame_beg);
	while(!context.quit)
	{
		// input
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_EVENT_QUIT:
					context.quit = true;
					break;

				// NOTE: when there is no break, both switch cases will execute the same code block below.
				//       These kind of "clever" solutions can become messy very fast.
				//       We will soon move it to a more appropriate function (with a more solid event parsing).
				case SDL_EVENT_KEY_UP:
				case SDL_EVENT_KEY_DOWN:
				{
					if(event.key.key == SDLK_W) context.btn_pressed_up    = event.key.down;
					if(event.key.key == SDLK_S) context.btn_pressed_down  = event.key.down;
					if(event.key.key == SDLK_A) context.btn_pressed_left  = event.key.down;
					if(event.key.key == SDLK_D) context.btn_pressed_right = event.key.down;

					// debug utilities
					// will be added here
					break;
				}
			}
		}

		// clear screen
		// NOTE: `0x` prefix means we are expressing the number in hexadecimal (base 16)
		//       `0b` is another useful prefix, expresses the number in binary
		SDL_SetRenderDrawColor(context.renderer, 0x00, 0x00, 0x00, 0x00);
		SDL_RenderClear(context.renderer);

		// update player position
		update(&context, &state);

		SDL_GetCurrentTime(&walltime_work_end);
		time_elapsed_work = walltime_work_end - walltime_frame_beg;

		if(context.target_framerate_ns > time_elapsed_work)
			SDL_DelayPrecise(context.target_framerate_ns - time_elapsed_work);
		
		SDL_GetCurrentTime(&walltime_frame_end);
		time_elapsed_frame = walltime_frame_end - walltime_frame_beg;
		context.delta = NS_TO_SECONDS(time_elapsed_frame);
		SDL_Log("%f\n", context.delta);
#ifdef ENABLE_DIAGNOSTICS
		SDL_SetRenderDrawColor(context.renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderDebugTextFormat(context.renderer, 10.0f, 10.0f, "elapsed (frame): %9.6f ms", NS_TO_MILLIS(time_elapsed_frame));
		SDL_RenderDebugTextFormat(context.renderer, 10.0f, 20.0f, "elapsed (work) : %9.6f ms", NS_TO_MILLIS(time_elapsed_work));
#endif

		// render
		SDL_RenderPresent(context.renderer);
		
		walltime_frame_beg = walltime_frame_end;
	}

	return 0;
};
