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
void particle_system_delete_particle(particle_system_t* system,int particle);

float bilinear_kernel(float x,float y);

void particle_system_mark_grid_cells(particle_system_t* particle_system,struct grid_s* grid);
void particle_system_transfer_to_grid(particle_system_t* particle_system,struct grid_s* grid);
void particle_system_advect(particle_system_t* particle_system,struct grid_s* grid,float delta_t);
void particle_system_remove_invalid_particles(particle_system_t* particle_system,struct grid_s* grid);
#endif // PARTICLE_H_INCLUDED
