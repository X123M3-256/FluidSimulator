#ifndef SIMULATION_H_INCLUDED
#define SIMULATION_H_INCLUDED
#include "grid.h"
#include "particle.h"

typedef struct
{
grid_t* grid;
particle_system_t* particle_system;
}simulation_t;

simulation_t* simulation_new(unsigned int width,unsigned int height,unsigned int max_particles);
void simulation_set_cell(simulation_t* simulation,grid_cell_type_t type,unsigned int x,unsigned int y);
void simulation_set_rect(simulation_t* simulation,grid_cell_type_t type,unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2);
void simulation_step(simulation_t* simulation,float delta_t);
void simulation_free(simulation_t* simulation);

#endif // SIMULATION_H_INCLUDED
