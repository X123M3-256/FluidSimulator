#include <stdio.h>
#include <stdlib.h>
#include "simulation.h"


float rand_float(float min,float max)
{
float rand_normal=(float)rand()/(float)(RAND_MAX);
return min+(max-min)*rand_normal;
}

simulation_t* simulation_new(unsigned int width,unsigned int height,unsigned int max_particles)
{
simulation_t* simulation=malloc(sizeof(simulation_t));
simulation->grid=grid_new(width,height);
simulation->particle_system=particle_system_new(max_particles);
return simulation;
}

void simulation_handle_sources(simulation_t* simulation)
{
int i,x,y;
//Loop over all inflows, make sure they are fluid
    for(y=0;y<simulation->grid->height;y++)
    for(x=0;x<simulation->grid->width;x++)
    {
        if((GRID_CELL_FLAGS(simulation->grid,x,y)&INFLOW)&&GRID_CELL_TYPE(simulation->grid,x,y)!=FLUID)
        {
        simulation_set_cell(simulation,FLUID,x,y);
        }
    }
//Loop over particles and delete any that lie in an outflow
    for(i=0;i<simulation->particle_system->num_particles;i++)
    {
    particle_t* particle=simulation->particle_system->particles+i;
    //We compute the cell in which the particle lies in the same manner as for particle_system_mark_grid_cells
    int cell_x=(int)floor(particle->position_x+0.5);
    int cell_y=(int)floor(particle->position_y+0.5);
    //Now, check if the cell is out of bounds or solid
        if(GRID_CELL_FLAGS(simulation->grid,cell_x,cell_y)&OUTFLOW)
        {
        //Delete the particle
        particle_system_delete_particle(simulation->particle_system,i);
        /*Since the current particle has been deleted, there is a
        different particle at this location, and that also needs to
        be checked, so for the next loop iteration, use the same i*/
        i--;
        }
    }
}

void simulation_step(simulation_t* simulation,float delta_t)
{
simulation_handle_sources(simulation);
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
//Determine which grid cells contain fluid
particle_system_mark_grid_cells(simulation->particle_system,simulation->grid);
//Compute velocities on grid from particle velocities
particle_system_transfer_to_grid(simulation->particle_system,simulation->grid);
}

void simulation_set_cell(simulation_t* simulation,grid_cell_type_t type,unsigned int x,unsigned int y)
{
    if(x<1||y<1||x>=simulation->grid->width-1||y>=simulation->grid->height-1)return;

//Nothing to be done if the target cell is already the right type
    if(GRID_CELL_TYPE(simulation->grid,x,y)==type)return;
//Assign type
GRID_CELL_SET_TYPE(simulation->grid,x,y,type);
//If the new type is fluid, some particles need to be added as well
    if(type==FLUID)
    {
    float fx=(float)x,fy=(float)y;
    particle_system_add_particle(simulation->particle_system,fx+rand_float(-0.5,-1.0/6.0),fy+rand_float(-0.5,-1.0/6.0));
    particle_system_add_particle(simulation->particle_system,fx+rand_float(-1.0/6.0,1.0/6.0),fy+rand_float(-0.5,-1.0/6.0));
    particle_system_add_particle(simulation->particle_system,fx+rand_float(1.0/6.0,0.5),fy+rand_float(-0.5,-1.0/6.0));
    particle_system_add_particle(simulation->particle_system,fx+rand_float(-0.5,-1.0/6.0),fy+rand_float(-1.0/6.0,1.0/6.0));
    particle_system_add_particle(simulation->particle_system,fx+rand_float(-1.0/6.0,1.0/6.0),fy+rand_float(-1.0/6.0,1.0/6.0));
    particle_system_add_particle(simulation->particle_system,fx+rand_float(1.0/6.0,0.5),fy+rand_float(-1.0/6.0,1.0/6.0));
    particle_system_add_particle(simulation->particle_system,fx+rand_float(-0.5,-1.0/6.0),fy+rand_float(1.0/6.0,0.5));
    particle_system_add_particle(simulation->particle_system,fx+rand_float(-1.0/6.0,1.0/6.0),fy+rand_float(1.0/6.0,0.5));
    particle_system_add_particle(simulation->particle_system,fx+rand_float(1.0/6.0,0.5),fy+rand_float(1.0/6.0,0.5));
    }
}

void simulation_set_rect(simulation_t* simulation,grid_cell_type_t type,unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2)
{
int x,y;
    for(y=y1;y<=y2;y++)
    for(x=x1;x<=x2;x++)
    {
    simulation_set_cell(simulation,type,x,y);
    }
}

void simulation_free(simulation_t* simulation);
