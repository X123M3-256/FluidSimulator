#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "grid.h"
#include "particle.h"

particle_system_t* particle_system_new(int max_particles)
{
particle_system_t* system=malloc(sizeof(particle_system_t));
system->num_particles=0;
system->max_particles=max_particles;
system->particles=malloc(max_particles*sizeof(particle_t));
return system;
}

void particle_system_add_particle(particle_system_t* system,float x,float y)
{
    if(system->num_particles==system->max_particles)return;
system->particles[system->num_particles].position_x=x;
system->particles[system->num_particles].position_y=y;
system->particles[system->num_particles].velocity_x=0.0;
system->particles[system->num_particles].velocity_y=0.0;
system->num_particles++;
}

/*To delete a particle, we swap it with the particle currently positioned at the end of
the array, and then reduce the particle count.*/
void particle_system_delete_particle(particle_system_t* system,int particle)
{
system->num_particles--;
system->particles[particle]=system->particles[system->num_particles];
}





/*[x,y] is the displacement from the known point to the interpolated point, the
return value is the weight of that point's contribution to the final interpolated value*/
float bilinear_kernel(float x,float y)
{
return (1.0-x)*(1.0-y);
}

/*Here we mark all cells containing a particle as fluid, and all others as empty (or solid).
This is done by first setting all fluid cells to empty, and then calculating the cell in which
each particle falls and marking that cell as fluid. Then, every cell containing at least one
particle is marked as fluid, and everything else is not.*/
void particle_system_mark_grid_cells(particle_system_t* particle_system,grid_t* grid)
{
/*First, we iterate over all grid cells and mark them empty. Solid cells are left alone because they
never change type*/
int x,y;
    for(y=0;y<grid->height;y++)
    for(x=0;x<grid->width;x++)
    {
        if(GRID_CELL_TYPE(grid,x,y)==FLUID)GRID_CELL_SET_TYPE(grid,x,y,EMPTY);
    GRID_CELL(grid,x,y).fluid_fraction=0.0;
    }
/*Now, we iterate over each particle, compute which cell it is in and then mark that cell as fluid*/
int i;
    for(i=0;i<particle_system->num_particles;i++)
    {
    particle_t* particle=particle_system->particles+i;

    /*Here we compute the coordinates of the cell containing the current particle.
    0.5 is added because particle positions are defined with reference to the
    *centre* of a cell, i.e [0,0] is the centre of the first cell, [1,0] the second,
    etc. Thus, to find the cell we're currently in, we add 0.5 to both components
    of the particle position to get the position with reference to the *corner* of the
    cells, and then floor the result to get the coordinates of the centre of the cell we're
    currently in.*/
    unsigned int cell_x=(unsigned int)floor(particle->position_x+0.5);
    unsigned int cell_y=(unsigned int)floor(particle->position_y+0.5);
        assert(cell_x>0&&cell_y>0&&cell_x<grid->width&&cell_y<grid->height);
    /*Ideally, there shouldn't be any particles in solid cells, but if there are, those
    cells shouldn't be changed. Otherwise, mark the current cell as fluid*/
        if(GRID_CELL_TYPE(grid,cell_x,cell_y)!=SOLID)GRID_CELL_SET_TYPE(grid,cell_x,cell_y,FLUID);
    //Compute fluid fraction estimation
    float cell_centre_x=floor(particle->position_x);
    float cell_centre_y=floor(particle->position_y);
    float offset_x=particle->position_x-cell_centre_x;
    float offset_y=particle->position_y-cell_centre_y;
    int cell_centre_index_x=(int)cell_centre_x;
    int cell_centre_index_y=(int)cell_centre_y;
    #define WEIGHT 1.1*1.0/9.0
    GRID_CELL(grid,cell_centre_index_x,cell_centre_index_y).fluid_fraction+=WEIGHT*bilinear_kernel(offset_x,offset_y);
    GRID_CELL(grid,cell_centre_index_x+1,cell_centre_index_y).fluid_fraction+=WEIGHT*bilinear_kernel(1.0-offset_x,offset_y);
    GRID_CELL(grid,cell_centre_index_x,cell_centre_index_y+1).fluid_fraction+=WEIGHT*bilinear_kernel(offset_x,1.0-offset_y);
    GRID_CELL(grid,cell_centre_index_x+1,cell_centre_index_y+1).fluid_fraction+=WEIGHT*bilinear_kernel(1.0-offset_x,1.0-offset_y);
    }
}

/*The interpolated velocity at each grid point is calculated as the weighted average of
the velocities of all particles in a 2x2 box centred on the grid point, with the weights
given by the kernel function above. However, as this would be quadratic time if implemented
directly, so we instead loop over all particles, accumulating both the weighted contributions
and the sum of the weights for each grid point, and then calculate the average in a separate
step. This takes time proportional to the number of particles.*/
void particle_system_transfer_to_grid(particle_system_t* particle_system,grid_t* grid)
{
/*Because we are going to be accumulating particle contributions to each grid cell instead
of calculating them directly, we first need to zero the cells.*/
int i;
int num_x_cells=(grid->width+1)*grid->height;
int num_y_cells=grid->width*(grid->height+1);
    for(i=0;i<num_x_cells;i++)
    {
    grid->velocity_x[i]=0.0f;
    grid->weight_sum_x[i]=0.0f;
    }
    for(i=0;i<num_y_cells;i++)
    {
    grid->velocity_y[i]=0.0f;
    grid->weight_sum_y[i]=0.0f;
    }
/*Now loop over particles, and sum contributions to velocity. We also sum the weights, so that for each cell we
have the sum of the weighted contributions of each particle within it's neighbourhood, and the sum of the weights.
The final weighted average is then the quotient of these*/
    for(i=0;i<particle_system->num_particles;i++)
    {
    particle_t* particle=particle_system->particles+i;
    /*Since we're drawing a box 2 grid cells wide around each grid point, each
    particle will contribute to the four nearest grid points. Since the x and y
    components are located in different places, they must be handled separately*/

    //Displacement of the particle from top left corner of the four cells
    float disp_x,disp_y;
    //Location of the top left corner of the four cells
    unsigned int x,y;

    //Do x coordinates
    grid_get_x_velocity_box(particle->position_x,particle->position_y,&x,&y,&disp_x,&disp_y);

    //Top left corner
    float weight=bilinear_kernel(disp_x,disp_y);
    GRID_VELOCITY_X(grid,x,y)+=particle->velocity_x*weight;
    GRID_WEIGHT_SUM_X(grid,x,y)+=weight;
    //Top right corner
    weight=bilinear_kernel(1.0-disp_x,disp_y);
    GRID_VELOCITY_X(grid,x+1,y)+=particle->velocity_x*weight;
    GRID_WEIGHT_SUM_X(grid,x+1,y)+=weight;
    //Bottom left corner
    weight=bilinear_kernel(disp_x,1.0-disp_y);
    GRID_VELOCITY_X(grid,x,y+1)+=particle->velocity_x*weight;
    GRID_WEIGHT_SUM_X(grid,x,y+1)+=weight;
    //Bottom right corner
    weight=bilinear_kernel(1.0-disp_x,1.0-disp_y);
    GRID_VELOCITY_X(grid,x+1,y+1)+=particle->velocity_x*weight;
    GRID_WEIGHT_SUM_X(grid,x+1,y+1)+=weight;

    //Do y coordinates
    grid_get_y_velocity_box(particle->position_x,particle->position_y,&x,&y,&disp_x,&disp_y);
    //Top left corner
    weight=bilinear_kernel(disp_x,disp_y);
    GRID_VELOCITY_Y(grid,x,y)+=particle->velocity_y*weight;
    GRID_WEIGHT_SUM_Y(grid,x,y)+=weight;
    //Top right corner
    weight=bilinear_kernel(1.0-disp_x,disp_y);
    GRID_VELOCITY_Y(grid,x+1,y)+=particle->velocity_y*weight;
    GRID_WEIGHT_SUM_Y(grid,x+1,y)+=weight;
    //Bottom left corner
    weight=bilinear_kernel(disp_x,1.0-disp_y);
    GRID_VELOCITY_Y(grid,x,y+1)+=particle->velocity_y*weight;
    GRID_WEIGHT_SUM_Y(grid,x,y+1)+=weight;
    //Bottom right corner
    weight=bilinear_kernel(1.0-disp_x,1.0-disp_y);
    GRID_VELOCITY_Y(grid,x+1,y+1)+=particle->velocity_y*weight;
    GRID_WEIGHT_SUM_Y(grid,x+1,y+1)+=weight;
    }

/*Now we loop over each grid cell and calculate the final velocity as the quotient of the sum of each particle
contribution and the sum of the weights*/
    for(i=0;i<num_x_cells;i++)if(grid->weight_sum_x[i]!=0.0)grid->velocity_x[i]/=grid->weight_sum_x[i];
    for(i=0;i<num_y_cells;i++)if(grid->weight_sum_y[i]!=0.0)grid->velocity_y[i]/=grid->weight_sum_y[i];

/*The final task is to extrapolate velocities into neighbouring cells. This is done because particles
may be moved outside the current fluid region during the advection step- if the velocity there is zero,
those particles would be artificially slowed*/

int x,y;
//Iterate over y velocity components not on the edge
    for(y=PLUS_HALF(1);y<MINUS_HALF(grid->height-1);y++)
    for(x=1;x<grid->width-1;x++)
    {
    //If a velocity component has no weight, it hasn't recieved a velocity value from any particle, so we need to extrapolate
        if(GRID_WEIGHT_SUM_Y(grid,x,y)==0.0f)
        {
        float neighbours=0.0;
        float total=0.0;
        //Check neighbours to see if they have recieved a velocity value
            if(GRID_WEIGHT_SUM_Y(grid,x+1,y)!=0.0f)
            {
            neighbours++;
            total+=GRID_VELOCITY_Y(grid,x+1,y);
            }
            if(GRID_WEIGHT_SUM_Y(grid,x-1,y)!=0.0f)
            {
            neighbours++;
            total+=GRID_VELOCITY_Y(grid,x-1,y);
            }
            if(GRID_WEIGHT_SUM_Y(grid,x,y+1)!=0.0f)
            {
            neighbours++;
            total+=GRID_VELOCITY_Y(grid,x,y+1);
            }
            if(GRID_WEIGHT_SUM_Y(grid,x,y-1)!=0.0f)
            {
            neighbours++;
            total+=GRID_VELOCITY_Y(grid,x,y-1);
            }
        //Average the values
            if(neighbours!=0.0)GRID_VELOCITY_Y(grid,x,y)=total/neighbours;
        }
    }
//Same code for x velocities- TODO: reduce repetition
    for(y=1;y<grid->height-1;y++)
    for(x=PLUS_HALF(1);x<MINUS_HALF(grid->width-1);x++)
    {
    //If a velocity component has no weight, it hasn't recieved a velocity value from any particle, so we need to extrapolate
        if(GRID_WEIGHT_SUM_X(grid,x,y)==0.0f)
        {
        float neighbours=0.0;
        float total=0.0;
        //Check neighbours to see if they have recieved a velocity value
            if(GRID_WEIGHT_SUM_X(grid,x+1,y)!=0.0f)
            {
            neighbours++;
            total+=GRID_VELOCITY_X(grid,x+1,y);
            }
            if(GRID_WEIGHT_SUM_X(grid,x-1,y)!=0.0f)
            {
            neighbours++;
            total+=GRID_VELOCITY_X(grid,x-1,y);
            }
            if(GRID_WEIGHT_SUM_X(grid,x,y+1)!=0.0f)
            {
            neighbours++;
            total+=GRID_VELOCITY_X(grid,x,y+1);
            }
            if(GRID_WEIGHT_SUM_X(grid,x,y-1)!=0.0f)
            {
            neighbours++;
            total+=GRID_VELOCITY_X(grid,x,y-1);
            }
        //Average the values
            if(neighbours!=0.0)GRID_VELOCITY_X(grid,x,y)=total/neighbours;
        }
    }
}


/*Remove particles that are outside the domain or embedded in solid cells.
This can happen due to numerical error in the advection step, particularly
when very high pressures are involved. Strictly speaking, it would probably
be better to move such particles back inside the fluid, but this is easier
said than done, and the number of particles ending up outside the domain is
small enough.*/
void particle_system_remove_invalid_particles(particle_system_t* particle_system,grid_t* grid)
{
int i;
    for(i=0;i<particle_system->num_particles;i++)
    {
    particle_t* particle=particle_system->particles+i;
    //We compute the cell in which the particle lies in the same manner as for particle_system_mark_grid_cells
    int cell_x=(int)floor(particle->position_x+0.5);
    int cell_y=(int)floor(particle->position_y+0.5);
    //Now, check if the cell is out of bounds or solid
        if(cell_x<1||cell_y<1||cell_x>=grid->width||cell_y>=grid->height||GRID_CELL_TYPE(grid,cell_x,cell_y)==SOLID)
        {
        //Delete the particle
        particle_system_delete_particle(particle_system,i);
        /*Since the current particle has been deleted, there is a
        different particle at this location, and that also needs to
        be checked, so for the next loop iteration, use the same i*/
        i--;
        }
    }
}

//Advect particles using midpoint method
void particle_system_advect(particle_system_t* particle_system,grid_t* grid,float delta_t)
{
int i;
    for(i=0;i<particle_system->num_particles;i++)
    {
    particle_t* particle=particle_system->particles+i;

    float grid_velocity_x,grid_velocity_y;
    //Get velocity at current particle position
    grid_get_velocity_at_point(grid,particle->position_x,particle->position_y,&grid_velocity_x,&grid_velocity_y);
    //Compute midpoint
    float midpoint_x=particle->position_x+0.5*delta_t*grid_velocity_x;
    float midpoint_y=particle->position_y+0.5*delta_t*grid_velocity_y;
    //Get velocity at midpoint
    grid_get_velocity_at_point(grid,midpoint_x,midpoint_y,&grid_velocity_x,&grid_velocity_y);
    //Compute new particle position
    particle->position_x+=delta_t*grid_velocity_x;
    particle->position_y+=delta_t*grid_velocity_y;
    }
}
