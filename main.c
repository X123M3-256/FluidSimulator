#include <stdio.h>
#include <stdlib.h>
#include<math.h>
#include <SDL/SDL.h>
#include "simulation.h"
#include "interface.h"






int main(int argc,char* argv[])
{
//particle_system_populate_rectangle(particles,1,20,49,49);
/*
grid_set_rectangle(grid,SOLID,0,35,30,49);
grid_set_rectangle(grid,SOLID,150,40,150,49);
particle_system_populate_rectangle(particles,30,40,149,49);
*/
/*
grid_set_rectangle(grid,SOLID,20,0,20,45);
grid_set_rectangle(grid,SOLID,150,41,150,49);
particle_system_populate_rectangle(particles,1,10,19,49);
*/
/*
grid_set_rectangle(grid,SOLID,200,45,225,49);
grid_set_rectangle(grid,SOLID,225,43,250,49);
grid_set_rectangle(grid,SOLID,250,41,275,49);
grid_set_rectangle(grid,SOLID,275,39,300,49);
grid_set_rectangle(grid,SOLID,300,37,325,49);
grid_set_rectangle(grid,SOLID,325,35,350,49);
grid_set_rectangle(grid,SOLID,350,33,375,49);
grid_set_rectangle(grid,SOLID,375,31,400,49);
particle_system_populate_rectangle(particles,1,5,25,49);
particle_system_populate_rectangle(particles,25,15,50,49);
particle_system_populate_rectangle(particles,50,20,75,49);
particle_system_populate_rectangle(particles,75,25,100,49);
particle_system_populate_rectangle(particles,100,30,200,49);
particle_system_populate_rectangle(particles,200,30,225,44);
particle_system_populate_rectangle(particles,225,30,250,42);
particle_system_populate_rectangle(particles,250,30,275,40);
particle_system_populate_rectangle(particles,275,30,300,38);
particle_system_populate_rectangle(particles,300,30,325,36);
particle_system_populate_rectangle(particles,325,30,350,34);
particle_system_populate_rectangle(particles,350,30,375,32);
particle_system_populate_rectangle(particles,375,30,499,30);
//particle_system_populate_rectangle(particles,1,9,20,29);
*/
/*
particle_system_populate_rectangle(particles,1,40,299,49);
particle_system_populate_rectangle(particles,1,20,11,40);
particle_system_populate_rectangle(particles,289,20,299,40);
*/
/*
grid_set_rectangle(grid,SOLID,50,45,50,49);
particle_system_populate_rectangle(particles,1,10,30,48);
*/
/*
particle_system_populate_rectangle(particles,99,5,101,106);
particle_system_populate_rectangle(particles,1,260,198,299);
*/
/*
grid_set_rectangle(grid,SOLID,30,0,30,95);
grid_set_rectangle(grid,SOLID,30,95,60,95);
grid_set_rectangle(grid,SOLID,60,85,60,95);
grid_set_rectangle(grid,SOLID,65,85,65,99);
particle_system_populate_rectangle(particles,1,5,29,99);
particle_system_populate_rectangle(particles,31,85,59,95);
particle_system_populate_rectangle(particles,66,85,99,99);
*/
/*
grid_set_rectangle(grid,SOLID,0,15,30,49);
grid_set_rectangle(grid,SOLID,30,25,60,49);
grid_set_rectangle(grid,SOLID,60,20,60,49);
particle_system_populate_rectangle(particles,61,30,149,49);
*/
/*
grid_set_rectangle(grid,SOLID,100,45,199,49);
grid_set_rectangle(grid,SOLID,90,47,199,49);
grid_set_rectangle(grid,SOLID,110,43,199,49);
particle_system_populate_rectangle(particles,1,40,90,49);
particle_system_populate_rectangle(particles,90,40,100,47);
particle_system_populate_rectangle(particles,100,40,110,45);
particle_system_populate_rectangle(particles,110,40,199,43);

particle_system_populate_rectangle(particles,1,20,10,40);
particle_system_populate_rectangle(particles,10,25,20,40);
particle_system_populate_rectangle(particles,20,30,30,40);
particle_system_populate_rectangle(particles,30,35,40,40);
*/

SDL_Init(SDL_INIT_VIDEO);
atexit(SDL_Quit);

simulation_t* simulation=simulation_new(200,100,128000);
interface_t* interface=interface_new(simulation);

SDL_Surface* screen=SDL_SetVideoMode(simulation->grid->width*CELL_SCREEN_SIZE,simulation->grid->height*CELL_SCREEN_SIZE,32,SDL_DOUBLEBUF);


float delta_t=0.05;
int milliseconds=SDL_GetTicks();
    while(!SDL_GetKeyState(NULL)[SDLK_SPACE])
    {
    simulation_step(simulation,delta_t);
    interface_process_events(interface);
    interface_render(interface,screen);
    SDL_Flip(screen);
    //Calculate next delta_t
    int prev_milliseconds=milliseconds;
    milliseconds=SDL_GetTicks();
    delta_t=(float)(milliseconds-prev_milliseconds)/1000.0;
    printf("FPS: %f\n",1/delta_t);
    }
return 0;
}
