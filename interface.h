#ifndef INTERFACE_H_INCLUDED
#define INTERFACE_H_INCLUDED
#include "simulation.h"

#define CELL_SCREEN_SIZE 8

typedef enum
{
DRAW_WATER,
DRAW_SOLID
}interface_mode_t;

typedef struct
{
simulation_t* simulation;
float mouse_x;
float mouse_y;
float drag_x;
float drag_y;
unsigned int mouse_pressed;
interface_mode_t mode;
}interface_t;

interface_t* interface_new(simulation_t* simulation);
void interface_process_events(interface_t* interface);
void interface_render(interface_t* interface,SDL_Surface* screen);
#endif // INTERFACE_H_INCLUDED
