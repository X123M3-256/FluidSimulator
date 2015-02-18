#ifndef PARTICLE_H_INCLUDED
#define PARTICLE_H_INCLUDED

struct grid_s;

typedef struct
{
float position_x;
float position_y;
float velocity_x;
float velocity_y;
}particle_t;

typedef struct
{
particle_t* particles;
unsigned int num_particles;
unsigned int max_particles;
}particle_system_t;

particle_system_t* particle_system_new(int max_particles);
void particle_system_add_particle(particle_system_t* system,float x,float y);
void particle_system_populate_rectangle(particle_system_t* system,float x1,float y1,float x2,float y2);

float bilinear_kernel(float x,float y);

void particle_system_mark_grid_cells(particle_system_t* particle_system,struct grid_s* grid);
void particle_system_transfer_to_grid(particle_system_t* particle_system,struct grid_s* grid);
void particle_system_advect(particle_system_t* particle_system,struct grid_s* grid,float delta_t);

#endif // PARTICLE_H_INCLUDED
