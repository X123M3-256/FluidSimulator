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
void simulation_step(simulation_t* simulation,float delta_t);
void simulation_free(simulation_t* simulation);

#endif // SIMULATION_H_INCLUDED
