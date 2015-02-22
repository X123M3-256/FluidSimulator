#include <stdio.h>
#include <stdlib.h>
#include "simulation.h"

simulation_t* simulation_new(unsigned int width,unsigned int height,unsigned int max_particles)
{
simulation_t* simulation=malloc(sizeof(simulation_t));
simulation->grid=grid_new(width,height);
simulation->particle_system=particle_system_new(max_particles);
return simulation;
}
void simulation_step(simulation_t* simulation,float delta_t)
{
//Determine which grid cells contain fluid
particle_system_mark_grid_cells(simulation->particle_system,simulation->grid);
//Compute velocities on grid from particle velocities
particle_system_transfer_to_grid(simulation->particle_system,simulation->grid);
//Save the current grid velocities
grid_save_velocities(simulation->grid);
//Apply gravity force
grid_apply_gravity(simulation->grid,delta_t);
//Here we solve for the pressure
grid_project(simulation->grid,delta_t);
//Compute velocity delta
grid_calculate_velocity_delta(simulation->grid);
//Update particle velocities using new grid velocities
grid_transfer_to_particle_system(simulation->grid,simulation->particle_system);
//Advect particles
particle_system_advect(simulation->particle_system,simulation->grid,delta_t);
//Remove any particles that are in invalid postions
particle_system_remove_invalid_particles(simulation->particle_system,simulation->grid);
}

//void simulation_set_cell_type()

void simulation_free(simulation_t* simulation);
