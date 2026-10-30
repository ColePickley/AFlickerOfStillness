#ifndef AFLICKEROFSTILLNESS_EVENT_HANDLING_H
#define AFLICKEROFSTILLNESS_EVENT_HANDLING_H
#include "screen_logic/screen_logic.h"

typedef struct {
    bool mouse_down;
    uint64_t mouse_down_time;
    uint64_t key_press_time;
    SDL_Keycode current_keycode;
}EventData;

void handleEvents(ScreenData* screen_data, SDL_Renderer* renderer, GameData** game_data, bool* program_running,
    EventData* event_data, UserData* user_data, SDL_Window* window);

static void mouseButtonDown(SDL_Renderer* renderer, EventData* event_data, SDL_Event event,
    const ScreenData* screen_data, GameData* game_data);

static void mouseButtonUp(SDL_Renderer* renderer, EventData* event_data, SDL_Event event, ScreenData* screen_data,
    UserData* user_data, GameData** game_data, bool* program_running, SDL_Window* window);

static void mouseMotion(SDL_Renderer* renderer, const EventData* event_data, SDL_Event event, const ScreenData* screen_data,
    UserData* user_data, const GameData* game_data);

static void keyDown(EventData* event_data, SDL_Event event, const ScreenData* screen_data, const GameData* game_data);

static void keyUp(SDL_Renderer* renderer, const EventData* event_data, SDL_Event event, ScreenData* screen_data,
    const UserData* user_data, GameData* game_data);

#endif //AFLICKEROFSTILLNESS_EVENT_HANDLING_H
