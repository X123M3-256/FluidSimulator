#include <stdio.h>
#include <stdlib.h>
#include<math.h>
#include <SDL/SDL.h>
#include "grid.h"
#include "particle.h"


void compute_simulation_step(particle_system_t* particles,grid_t* grid)
{
//Determine which grid cells contain fluid
particle_system_mark_grid_cells(particles,grid);
//Compute velocities on grid from particle velocities
particle_system_transfer_to_grid(particles,grid);
//Save the current grid velocities
grid_save_velocities(grid);
//Apply gravity force
grid_apply_gravity(grid,0.05);
//Here we solve for the pressure
grid_project(grid,0.05);
//Compute velocity delta
grid_calculate_velocity_delta(grid);
//Update particle velocities using new grid velocities
grid_transfer_to_particle_system(grid,particles);
//Advect particles
particle_system_advect(particles,grid,0.05);
}

void put_pixel(SDL_Surface* screen,int x,int y,char r,char g,char b)
{
    if(x<0||x>=screen->w||y<0||y>=screen->h)return;
char* pixel=screen->pixels+y*screen->pitch+4*x;
pixel[0]=b;
pixel[1]=g;
pixel[2]=r;
}

void render_cell(SDL_Surface* screen,grid_t* grid,int x,int y)
{
int sx=x*8;
int sy=y*8;
char r,g,b;
float u;
    switch(GRID_CELL(grid,x,y).type)
    {
    case SOLID:
    r=100,g=100,b=100;
    break;
    case FLUID:
    u=GRID_CELL(grid,x,y).pressure;///200000.0;
        if(u>1)u=1;
        else if(u<-1)u=-1;
        if(u>=0)r=0,g=255*u,b=255*(1.0-u);
        else r=-u*255,g=0,b=255*(1.0+u);
    break;
    case EMPTY:
    r=0,g=0,b=0;
    break;
    }
int i;
    for(i=0;i<8;i++)
    {
    put_pixel(screen,sx+i,sy,r,g,b);
    put_pixel(screen,sx+i,sy+7,r,g,b);
    put_pixel(screen,sx,sy+i,r,g,b);
    put_pixel(screen,sx+7,sy+i,r,g,b);
    }
int x_vel_mag=(int)floor(GRID_VELOCITY_X(grid,MINUS_HALF(x),y));
int y_vel_mag=(int)floor(GRID_VELOCITY_Y(grid,x,MINUS_HALF(y)));
    if(x_vel_mag>=0) for(i=0;i<x_vel_mag;i++)put_pixel(screen,sx+i,sy+4,255,0,0);
    else for(i=0;i>x_vel_mag;i--)put_pixel(screen,sx+i,sy+4,255,0,0);

    if(y_vel_mag>=0) for(i=0;i<y_vel_mag;i++)put_pixel(screen,sx+4,sy+i,255,0,0);
    else for(i=0;i>y_vel_mag;i--)put_pixel(screen,sx+4,sy+i,255,0,0);
}

void render(SDL_Surface* screen,grid_t* grid,particle_system_t* particles)
{
SDL_Rect all;
all.x=0;
all.y=0;
all.w=screen->w;
all.h=screen->h;
SDL_FillRect(screen,&all,0);
SDL_LockSurface(screen);

int i,x,y;
    for(y=0;y<grid->height;y++)
    for(x=0;x<grid->width;x++)
    {
    //render_cell(screen,grid,x,y);
        /*
        if(GRID_CELL(grid,x,y).type==SOLID)
        {
            for(i=0;i<4;i++)
            {
            put_pixel(screen,x*4+i,y*4,80,80,80);
            put_pixel(screen,x*4+i,y*4+3,80,80,80);
            put_pixel(screen,x*4,y*4+i,80,80,80);
            put_pixel(screen,x*4+3,y*4+i,80,80,80);
            }
        }
    */
    }
    for(i=0;i<particles->num_particles;i++)
    {
    float u=0.1*sqrt(particles->particles[i].velocity_x*particles->particles[i].velocity_x+particles->particles[i].velocity_y*particles->particles[i].velocity_y);
        if(u>1)u=1;
    put_pixel(screen,(int)((particles->particles[i].position_x+0.5)*8.0),(int)((particles->particles[i].position_y+0.5)*8.0),255*u,255*u,255);
    }
SDL_UnlockSurface(screen);
SDL_Flip(screen);
}

int main(int argc,char* argv[])
{
SDL_Init(SDL_INIT_VIDEO);
freopen( "CON", "w", stdout );
atexit(SDL_Quit);



grid_t* grid=grid_new(300,150);

particle_system_t* particles=particle_system_new(16192);

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
//particle_system_populate_rectangle(particles,1,40,299,49);
//particle_system_populate_rectangle(particles,0,10,30,40);

particle_system_populate_rectangle(particles,1,140,299,149);
particle_system_populate_rectangle(particles,145,5,155,15);

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
particle_system_populate_rectangle(particles,0,40,90,49);
particle_system_populate_rectangle(particles,90,40,100,47);
particle_system_populate_rectangle(particles,100,40,110,45);
particle_system_populate_rectangle(particles,110,40,199,43);
particle_system_populate_rectangle(particles,0,20,20,40);
particle_system_populate_rectangle(particles,20,30,30,40);
*/

SDL_Surface* screen=SDL_SetVideoMode(grid->width*8,grid->height*8,32,SDL_DOUBLEBUF);

int milliseconds=SDL_GetTicks();
int frames=0;
    while(!SDL_GetKeyState(NULL)[SDLK_SPACE])
    {
    frames++;
    SDL_PumpEvents();
    compute_simulation_step(particles,grid);
        //if(timer++%5==0)particle_system_populate_rectangle(particles,1,2,4,4);
    render(screen,grid,particles);
    }
printf("FPS %f\n",1000.0*(float)frames/(float)(SDL_GetTicks()-milliseconds));
return 0;
}
