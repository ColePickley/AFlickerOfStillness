#include "event_handling.h"

void handleEvents(ScreenData* screen_data, SDL_Renderer* renderer, GameData** game_data, bool* program_running,
        EventData* event_data, UserData* user_data, SDL_Window* window) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                *program_running = false;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                mouseButtonDown(renderer, event_data, event, screen_data, *game_data);
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                mouseButtonUp(renderer, event_data, event, screen_data, user_data, game_data, program_running, window);
                break;
            case SDL_EVENT_MOUSE_MOTION:
                mouseMotion(renderer, event_data, event, screen_data, user_data, *game_data);
                break;
            case SDL_EVENT_KEY_DOWN:
                keyDown(event_data, event, screen_data, *game_data);
                break;
            case SDL_EVENT_KEY_UP:
                keyUp(renderer, event_data, event, screen_data, user_data, *game_data);
                break;
            default:
                break;
        }
    }
}

static void mouseButtonDown(SDL_Renderer* renderer, EventData* event_data, SDL_Event event,
        const ScreenData* screen_data, GameData* game_data) {
    SDL_ConvertEventToRenderCoordinates(renderer, &event);
    const SDL_FPoint pt = { .x = event.button.x, .y = event.button.y };
    const bool mouse_in_bounds = pt.x >= 0 && pt.y >= 0 && pt.x <= WINDOW_WIDTH && pt.y <= WINDOW_HEIGHT;
    event_data->mouse_down = true;
    if (event.button.button == SDL_BUTTON_LEFT && mouse_in_bounds) {
        mousePressed(screen_data, pt, game_data);
        event_data->mouse_down_time = SDL_GetPerformanceCounter();
    }
}

static void mouseButtonUp(SDL_Renderer* renderer, EventData* event_data, SDL_Event event, ScreenData* screen_data,
        UserData* user_data, GameData** game_data, bool* program_running, SDL_Window* window) {
    SDL_ConvertEventToRenderCoordinates(renderer, &event);
    const SDL_FPoint pt = { .x = event.button.x, .y = event.button.y };
    const bool mouse_in_bounds = pt.x >= 0 && pt.y >= 0 && pt.x <= WINDOW_WIDTH && pt.y <= WINDOW_HEIGHT;
    event_data->mouse_down = false;
    if (event.button.button == SDL_BUTTON_LEFT && mouse_in_bounds) {
        mouseReleased(screen_data, pt, renderer, program_running, game_data, user_data, window);
        const uint64_t mouse_up_time = SDL_GetPerformanceCounter();
        const double time_elapsed = (double) (mouse_up_time - event_data->mouse_down_time) /
            (double) SDL_GetPerformanceFrequency();
        if (time_elapsed < 0.2) {
            mouseClicked(screen_data, pt, *game_data);
        }
    }
}

static void mouseMotion(SDL_Renderer* renderer, const EventData* event_data, SDL_Event event,
        const ScreenData* screen_data, UserData* user_data, const GameData* game_data) {
    SDL_ConvertEventToRenderCoordinates(renderer, &event);
    const SDL_FPoint motion_pt = {.x = event.motion.xrel, .y = event.motion.yrel};
    const bool motion_in_bounds = motion_pt.x >= 0 && motion_pt.y >= 0 &&
        motion_pt.x <= WINDOW_WIDTH && motion_pt.y <= WINDOW_HEIGHT;
    mouseMoved(screen_data, motion_pt, game_data, user_data);
    if (event_data->mouse_down && motion_in_bounds) {
        mouseDragged(screen_data, motion_pt, game_data, user_data);
    }
}

static void keyDown(EventData* event_data, const SDL_Event event, const ScreenData* screen_data,
        const GameData* game_data) {
    event_data->current_keycode = event.key.key;
    event_data->key_press_time = SDL_GetPerformanceCounter();
    keyPressed(screen_data, event.key.key, game_data);
}

static void keyUp(SDL_Renderer* renderer, const EventData* event_data, const SDL_Event event, ScreenData* screen_data,
        const UserData* user_data, GameData* game_data) {
    keyReleased(screen_data, event.key.key, game_data);
    if (event.key.key == event_data->current_keycode) {
        const uint64_t key_up_time = SDL_GetPerformanceCounter();
        const double time_elapsed = (double) (key_up_time - event_data->mouse_down_time) /
            (double) SDL_GetPerformanceFrequency();
        if (time_elapsed < 100.0) {
            keyClicked(screen_data, event_data->current_keycode, renderer, game_data, user_data);
        }
    }
}