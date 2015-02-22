#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<math.h>
#include "grid.h"

/*Create a new grid with specified width and height and initialize to zero*/
grid_t* grid_new(unsigned int width,unsigned int height)
{
grid_t* grid=malloc(sizeof(grid_t));
grid->width=width;
grid->height=height;
//Velocities are stored on cell edges
int num_x_components=(grid->width+1)*grid->height;
int num_y_components=grid->width*(grid->height+1);
grid->velocity_x=malloc(num_x_components*sizeof(float));
grid->velocity_y=malloc(num_y_components*sizeof(float));
grid->velocity_x_old=malloc(num_x_components*sizeof(float));
grid->velocity_y_old=malloc(num_y_components*sizeof(float));
grid->velocity_delta_x=malloc(num_x_components*sizeof(float));
grid->velocity_delta_y=malloc(num_y_components*sizeof(float));
grid->weight_sum_x=malloc(num_x_components*sizeof(float));
grid->weight_sum_y=malloc(num_y_components*sizeof(float));
grid->cells=malloc(width*height*sizeof(grid_cell_t));


int x,y;
    for(y=0;y<height;y++)
    for(x=0;x<width;x++)
    {
    /*We set the edge cells to solid. This simplifies subsequent code, because the contrapositive
    is that if a cell is not solid then it is not an edge cell, and therefore we can safely access
    its neighbours without risking an out-of-bounds access*/
        if(x==0||y==0||x==width-1||y==height-1)GRID_CELL(grid,x,y).type=SOLID;
        else GRID_CELL(grid,x,y).type=EMPTY;
    GRID_CELL(grid,x,y).residual=0.0;
    //Pressure must be initialized as it is not reset between frames
    GRID_CELL(grid,x,y).pressure=0.0;
    }
int i;
    for(i=0;i<num_x_components;i++)
    {
    grid->velocity_x[i]=0.0;
    grid->weight_sum_x[i]=0.0;
    }
    for(i=0;i<num_y_components;i++)
    {
    grid->velocity_y[i]=0.0;
    grid->weight_sum_y[i]=0.0;
    }
return grid;
}


/*Here we save a copy of the current velocities. This is done because we need to compute
the difference between the old and new velocities when we update the particles at the end
of the pressure solve*/
void grid_save_velocities(grid_t* grid)
{
memcpy(grid->velocity_x_old,grid->velocity_x,(grid->width+1)*grid->height*sizeof(float));
memcpy(grid->velocity_y_old,grid->velocity_y,grid->width*(grid->height+1)*sizeof(float));
}

void grid_calculate_velocity_delta(grid_t* grid)
{
int i;
int num_x_cells=(grid->width+1)*grid->height;
int num_y_cells=grid->width*(grid->height+1);
    for(i=0;i<num_x_cells;i++)grid->velocity_delta_x[i]=grid->velocity_x[i]-grid->velocity_x_old[i];
    for(i=0;i<num_y_cells;i++)grid->velocity_delta_y[i]=grid->velocity_y[i]-grid->velocity_y_old[i];
}

/*This method applies the gravity term to the velocity grid using forward Euler integration*/
void grid_apply_gravity(grid_t* grid,float delta_t)
{
/*Gravity is applied by accelerating each y-component of the velocity downward by g*delta_T
Only those components of the velocity field that border fluid cells are touched; the velocity
field is not meaningful elsewhere.*/
int x,y;
/*Since edge cells are always solid, we don't need to iterate over them- however, we do iterate over the top row
of cells. This is because we are really looking at the velocity component stored on a cells bottom edge- and that
might need to be considered if the cell below the current one is fluid.*/
    for(y=0;y<grid->height-1;y++)
    for(x=1;x<grid->width-1;x++)
    {
    //If this cell, or the one below it, is a fluid then we need to apply gravity to the velocity on the bottom edge
        if(GRID_CELL(grid,x,y).type==FLUID||GRID_CELL(grid,x,y+1).type==FLUID)
        {
        GRID_VELOCITY_Y(grid,x,PLUS_HALF(y))+=9.81*delta_t;
        }
    }
}

inline float bilinear_interpolation(float p1,float p2,float p3, float p4,float ux,float uy)
{
float i1=p1+(p2-p1)*ux;
float i2=p3+(p4-p3)*ux;
return i1+(i2-i1)*uy;
}

/*The four velocity x-component samples nearest the cell form a square around it. This returns
the coordinates of the top left corner of that square in x and y; the other three points can be
found by adding 1 to each of x and y, and then both x and y.*/
void grid_get_x_velocity_box(float x,float y,unsigned int* x_index,unsigned int* y_index,float* disp_x,float* disp_y)
{
float corner_x=floor(x+0.5);
float corner_y=floor(y);
*disp_x=(x+0.5)-corner_x;
*disp_y=y-corner_y;
*x_index=(unsigned int)corner_x;
*y_index=(unsigned int)corner_y;
}

/*Similar to the previous method, but works with y velocity components instead*/
void grid_get_y_velocity_box(float x,float y,unsigned int* x_index,unsigned int* y_index,float* disp_x,float* disp_y)
{
float corner_x=floor(x);
float corner_y=floor(y+0.5);
*disp_x=x-corner_x;
*disp_y=(y+0.5)-corner_y;
*x_index=(unsigned int)corner_x;
*y_index=(unsigned int)corner_y;
}

void grid_get_velocity_at_point(grid_t* grid,float x,float y,float* velocity_x,float* velocity_y)
{
unsigned int box_x,box_y;
float disp_x,disp_y;


grid_get_x_velocity_box(x,y,&box_x,&box_y,&disp_x,&disp_y);
*velocity_x=bilinear_interpolation(GRID_VELOCITY_X(grid,box_x,box_y),GRID_VELOCITY_X(grid,box_x+1,box_y),GRID_VELOCITY_X(grid,box_x,box_y+1),GRID_VELOCITY_X(grid,box_x+1,box_y+1),disp_x,disp_y);

grid_get_y_velocity_box(x,y,&box_x,&box_y,&disp_x,&disp_y);
*velocity_y=bilinear_interpolation(GRID_VELOCITY_Y(grid,box_x,box_y),GRID_VELOCITY_Y(grid,box_x+1,box_y),GRID_VELOCITY_Y(grid,box_x,box_y+1),GRID_VELOCITY_Y(grid,box_x+1,box_y+1),disp_x,disp_y);
}

void grid_get_velocity_delta_at_point(grid_t* grid,float x,float y,float* delta_x,float* delta_y)
{
unsigned int box_x,box_y;
float disp_x,disp_y;

grid_get_x_velocity_box(x,y,&box_x,&box_y,&disp_x,&disp_y);
*delta_x=bilinear_interpolation(GRID_DELTA_VELOCITY_X(grid,box_x,box_y),GRID_DELTA_VELOCITY_X(grid,box_x+1,box_y),GRID_DELTA_VELOCITY_X(grid,box_x,box_y+1),GRID_DELTA_VELOCITY_X(grid,box_x+1,box_y+1),disp_x,disp_y);

grid_get_y_velocity_box(x,y,&box_x,&box_y,&disp_x,&disp_y);
*delta_y=bilinear_interpolation(GRID_DELTA_VELOCITY_Y(grid,box_x,box_y),GRID_DELTA_VELOCITY_Y(grid,box_x+1,box_y),GRID_DELTA_VELOCITY_Y(grid,box_x,box_y+1),GRID_DELTA_VELOCITY_Y(grid,box_x+1,box_y+1),disp_x,disp_y);
}


/*Here, we take the new velocities, calculated on the grid, and transfer them back to the
particle system. There are two methods of doing this- either we interpolate the velocities
directly (PIC), or we interpolate the *change* in velocity and add that to the particles
existing velocity (FLIP). In practice, a linear combination of these is used*/
void grid_transfer_to_particle_system(grid_t* grid,particle_system_t* particle_system)
{
int i;
    for(i=0;i<particle_system->num_particles;i++)
    {
    particle_t* particle=particle_system->particles+i;

    float pic_x,pic_y;
    grid_get_velocity_at_point(grid,particle->position_x,particle->position_y,&pic_x,&pic_y);

    float flip_x,flip_y;
    grid_get_velocity_delta_at_point(grid,particle->position_x,particle->position_y,&flip_x,&flip_y);

    flip_x+=particle->velocity_x;
    flip_y+=particle->velocity_y;

    particle->velocity_x=0.05*pic_x+0.95*flip_x;
    particle->velocity_y=0.05*pic_y+0.95*flip_y;
    }
}

/*This routine is responsible for enforcing boundary conditions on the velocity field. Two such conditions are handled:
    1)For solid cells, there should be no flow into the cell
    2)For other cells, pressure should be zero (free surface condition)
    */
void grid_enforce_boundary(grid_t* grid)
{
int x,y;
    for(y=0;y<grid->height;y++)
    for(x=0;x<grid->width;x++)
    {
        if(GRID_CELL(grid,x,y).type==SOLID)
        {
        /*Enforce boundary conditions on solid cells. All velocity components bordering this cell
        must point out of the cell (technically they should be zero, but this causes fluid to stick
        to the walls*/
            if(x!=0&&GRID_CELL(grid,x-1,y).type==FLUID&&GRID_VELOCITY_X(grid,MINUS_HALF(x),y)>0.0)GRID_VELOCITY_X(grid,MINUS_HALF(x),y)=0.0;
            if(x!=grid->width-1&&GRID_CELL(grid,x+1,y).type==FLUID&&GRID_VELOCITY_X(grid,PLUS_HALF(x),y)<0.0)GRID_VELOCITY_X(grid,PLUS_HALF(x),y)=0.0;
            if(y!=0&&GRID_CELL(grid,x,y-1).type==FLUID&&GRID_VELOCITY_Y(grid,x,MINUS_HALF(y))>0.0)GRID_VELOCITY_Y(grid,x,MINUS_HALF(y))=0.0;
            if(y!=grid->height-1&&GRID_CELL(grid,x,y+1).type==FLUID&&GRID_VELOCITY_Y(grid,x,PLUS_HALF(y))<0.0)GRID_VELOCITY_Y(grid,x,PLUS_HALF(y))=0.0;
        }
        /*Empty cells should have a pressure of zero, because the free surface has constant
        pressure.*/
        else if(GRID_CELL(grid,x,y).type==EMPTY) GRID_CELL(grid,x,y).pressure=0.0;
    }
}

/*
void grid_jacobian_iteration(grid_t* grid)
{
int x,y;
    for(y=1;y<grid->height-1;y++)
    for(x=1;x<grid->width-1;x++)
    {
        if(GRID_CELL(grid,x,y).type==FLUID)
        {
        //Solid and empty cells have pressure 0, so no need to check for them
        float pressure_sum=GRID_CELL(grid,x+1,y).pressure+GRID_CELL(grid,x-1,y).pressure+GRID_CELL(grid,x,y+1).pressure+GRID_CELL(grid,x,y-1).pressure;
        GRID_CELL(grid,x,y).next_pressure=(pressure_sum-GRID_CELL(grid,x,y).divergence)/GRID_CELL(grid,x,y).neighbours;
        }
    }
    for(y=1;y<grid->height-1;y++)
    for(x=1;x<grid->width-1;x++)
    {
        if(GRID_CELL(grid,x,y).type==FLUID)
        {
        GRID_CELL(grid,x,y).pressure=GRID_CELL(grid,x,y).next_pressure;
        }
    }
}
*/


inline float grid_calculate_residual_norm_squared(grid_t* grid)
{
int x,y;
float residual_norm_squared=0.0;
        for(y=1;y<grid->height-1;y++)
        for(x=1;x<grid->width-1;x++)
        {
            if(GRID_CELL(grid,x,y).type==FLUID)
            {
            residual_norm_squared+=GRID_CELL(grid,x,y).residual*GRID_CELL(grid,x,y).residual;
            }
        }
return residual_norm_squared;
}

void grid_conjugate_gradient(grid_t* grid)
{
int x,y;
//Calculate residual vector
    for(y=1;y<grid->height-1;y++)
    for(x=1;x<grid->width-1;x++)
    {
        if(GRID_CELL(grid,x,y).type==FLUID)
        {
        float matrix_pressure=-GRID_CELL(grid,x,y).neighbours*GRID_CELL(grid,x,y).pressure;
        matrix_pressure+=GRID_CELL(grid,x+1,y).pressure;
        matrix_pressure+=GRID_CELL(grid,x-1,y).pressure;
        matrix_pressure+=GRID_CELL(grid,x,y+1).pressure;
        matrix_pressure+=GRID_CELL(grid,x,y-1).pressure;
        GRID_CELL(grid,x,y).residual=GRID_CELL(grid,x,y).divergence-matrix_pressure;
        }else GRID_CELL(grid,x,y).residual=0.0;
    }
//Set conjugate vector equal to residual vector
    for(y=1;y<grid->height-1;y++)
    for(x=1;x<grid->width-1;x++)
    {
    GRID_CELL(grid,x,y).conjugate=GRID_CELL(grid,x,y).residual;
    }
//Calculate current size of residual
float residual_norm_squared=grid_calculate_residual_norm_squared(grid);
//If the residual is zero, we're already close enough to the solution
    if(residual_norm_squared<0.001)return;
//Iterations
//int iter=0;
    while(1)
    {
   // iter++;
    //Calculate matrix*residual
        for(y=1;y<grid->height-1;y++)
        for(x=1;x<grid->width-1;x++)
        {
            if(GRID_CELL(grid,x,y).type==FLUID)
            {
            GRID_CELL(grid,x,y).matrix_conjugate=-GRID_CELL(grid,x,y).neighbours*GRID_CELL(grid,x,y).conjugate;
            GRID_CELL(grid,x,y).matrix_conjugate+=GRID_CELL(grid,x+1,y).conjugate;
            GRID_CELL(grid,x,y).matrix_conjugate+=GRID_CELL(grid,x-1,y).conjugate;
            GRID_CELL(grid,x,y).matrix_conjugate+=GRID_CELL(grid,x,y+1).conjugate;
            GRID_CELL(grid,x,y).matrix_conjugate+=GRID_CELL(grid,x,y-1).conjugate;
            }
        }
    //Calculate alpha
    float alpha_denominator=0.0;
        for(y=1;y<grid->height-1;y++)
        for(x=1;x<grid->width-1;x++)
        {
            if(GRID_CELL(grid,x,y).type==FLUID)
            {
            alpha_denominator+=GRID_CELL(grid,x,y).conjugate*GRID_CELL(grid,x,y).matrix_conjugate;
            }
        }
    float alpha=residual_norm_squared/alpha_denominator;
    //Calculate new pressure and residual
        for(y=1;y<grid->height-1;y++)
        for(x=1;x<grid->width-1;x++)
        {
            if(GRID_CELL(grid,x,y).type==FLUID)
            {
            GRID_CELL(grid,x,y).pressure+=alpha*GRID_CELL(grid,x,y).conjugate;
            GRID_CELL(grid,x,y).residual-=alpha*GRID_CELL(grid,x,y).matrix_conjugate;
            }
        }
    //Check size of residual
    float old_residual_norm_squared=residual_norm_squared;
    residual_norm_squared=grid_calculate_residual_norm_squared(grid);
        if(residual_norm_squared<0.001)break;
    //Calculate beta
    float beta=residual_norm_squared/old_residual_norm_squared;
    //Calculate new conjugate vector
        for(y=1;y<grid->height-1;y++)
        for(x=1;x<grid->width-1;x++)
        {
            if(GRID_CELL(grid,x,y).type==FLUID)
            {
            GRID_CELL(grid,x,y).conjugate=GRID_CELL(grid,x,y).residual+beta*GRID_CELL(grid,x,y).conjugate;
            }
        }
    }
//printf("%d\n",iter);
}

void grid_calculate_divergence(grid_t* grid)
{
int x,y;
    for(y=1;y<grid->height-1;y++)
    for(x=1;x<grid->width-1;x++)
    {
        if(GRID_CELL(grid,x,y).type==FLUID)
        {
        GRID_CELL(grid,x,y).neighbours=0.0;
        GRID_CELL(grid,x,y).divergence=(GRID_VELOCITY_X(grid,PLUS_HALF(x),y)-GRID_VELOCITY_X(grid,MINUS_HALF(x),y))+(GRID_VELOCITY_Y(grid,x,PLUS_HALF(y))-GRID_VELOCITY_Y(grid,x,MINUS_HALF(y)));
        //Solid cells have zero velocity in or out
            if(GRID_CELL(grid,x+1,y).type!=SOLID)GRID_CELL(grid,x,y).neighbours++;
            if(GRID_CELL(grid,x-1,y).type!=SOLID)GRID_CELL(grid,x,y).neighbours++;
            if(GRID_CELL(grid,x,y+1).type!=SOLID)GRID_CELL(grid,x,y).neighbours++;
            if(GRID_CELL(grid,x,y-1).type!=SOLID)GRID_CELL(grid,x,y).neighbours++;
        }
    }
}

void grid_project(grid_t* grid,float delta_t)
{
int x,y;
//Set boundary conditions
grid_enforce_boundary(grid);
//Calculate divergence (and neighbours)
grid_calculate_divergence(grid);
//Solve for pressure
grid_conjugate_gradient(grid);
//Update velocity
    for(y=1;y<grid->height;y++)
    for(x=1;x<grid->width;x++)
    {
        if(GRID_CELL(grid,x,y).type!=SOLID)
        {
            //Apply X pressure
            if(GRID_CELL(grid,x-1,y).type!=SOLID)GRID_VELOCITY_X(grid,MINUS_HALF(x),y)-=(GRID_CELL(grid,x,y).pressure-GRID_CELL(grid,x-1,y).pressure);
            //Apply Y pressure
            if(GRID_CELL(grid,x,y-1).type!=SOLID)GRID_VELOCITY_Y(grid,x,MINUS_HALF(y))-=(GRID_CELL(grid,x,y).pressure-GRID_CELL(grid,x,y-1).pressure);
        }
    }
}
