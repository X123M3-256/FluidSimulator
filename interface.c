#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL/SDL.h>
#include "interface.h"

interface_t* interface_new(simulation_t* simulation)
{
interface_t* interface=malloc(sizeof(interface_t));
interface->simulation=simulation;
interface->mouse_x=0.0;
interface->mouse_y=0.0;
interface->mouse_pressed=0;
interface->mode=DRAW_WATER;
return interface;
}


void interface_process_events(interface_t* interface)
{
SDL_Event event;
    while(SDL_PollEvent(&event)!=0)
    {
        if(event.type==SDL_MOUSEMOTION)
        {
        interface->mouse_x=(float)event.motion.x/4.0;
        interface->mouse_y=(float)event.motion.y/4.0;
        }
        else if(event.type==SDL_MOUSEBUTTONDOWN)
        {
        interface->drag_x=interface->mouse_x;
        interface->drag_y=interface->mouse_y;
        interface->mouse_pressed=1;
        }
        else if(event.type==SDL_MOUSEBUTTONUP)
        {
        interface->mouse_pressed=0;
            if(interface->mode==DRAW_WATER)particle_system_populate_rectangle(interface->simulation->particle_system,interface->drag_x,interface->drag_y,interface->mouse_x,interface->mouse_y);
            else if(interface->mode==DRAW_SOLID)grid_set_rectangle(interface->simulation->grid,SOLID,interface->drag_x,interface->drag_y,interface->mouse_x,interface->mouse_y);
        }
        else if(event.type==SDL_KEYDOWN)
        {
            if(event.key.keysym.sym==SDLK_w)interface->mode=DRAW_WATER;
            else if(event.key.keysym.sym==SDLK_s)interface->mode=DRAW_SOLID;
        }
    }
}

void put_pixel(SDL_Surface* screen,int x,int y,char r,char g,char b)
{
    if(x<0||x>=screen->w||y<0||y>=screen->h)return;
char* pixel=screen->pixels+y*screen->pitch+4*x;
pixel[0]=b;
pixel[1]=g;
pixel[2]=r;
}

void interface_render(interface_t* interface,SDL_Surface* screen)
{
SDL_Rect all;
all.x=0;
all.y=0;
all.w=screen->w;
all.h=screen->h;
SDL_FillRect(screen,&all,0);
SDL_LockSurface(screen);
//Render simulation
int i,x,y;
    for(y=0;y<interface->simulation->grid->height;y++)
    for(x=0;x<interface->simulation->grid->width;x++)
    {
        if(GRID_CELL(interface->simulation->grid,x,y).type==SOLID)
        {

            for(i=0;i<4;i++)
            {
            put_pixel(screen,x*4+i,y*4,80,80,80);
            put_pixel(screen,x*4+i,y*4+3,80,80,80);
            put_pixel(screen,x*4,y*4+i,80,80,80);
            put_pixel(screen,x*4+3,y*4+i,80,80,80);
            }
        }
    }
    for(i=0;i<interface->simulation->particle_system->num_particles;i++)
    {
    particle_t* particle=interface->simulation->particle_system->particles+i;

    float u=0.1*sqrt(particle->velocity_x*particle->velocity_x+particle->velocity_y*particle->velocity_y);
        if(u>=1.0)u=1.0;
    put_pixel(screen,(int)((particle->position_x+0.5)*4.0),(int)((particle->position_y+0.5)*4.0),(int)(255.0*u),(int)(255.0*u),255);
    }
//Render drag box
    if(interface->mouse_pressed)
    {
    int x1=(int)((interface->drag_x+0.5)*4.0);
    int y1=(int)((interface->drag_y+0.5)*4.0);
    int w=(int)((interface->mouse_x-interface->drag_x)*4.0);
    int h=(int)((interface->mouse_y-interface->drag_y)*4.0);
        for(y=0;y<h;y++)
        for(x=0;x<w;x++)
        {
        put_pixel(screen,x1+x,y1,255,255,255);
        put_pixel(screen,x1+x,y1+h,255,255,255);
        put_pixel(screen,x1,y1+y,255,255,255);
        put_pixel(screen,x1+w,y1+y,255,255,255);
        }
    }



SDL_UnlockSurface(screen);
}


/*
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
int x_vel_mag=(int)floor(0.1*GRID_VELOCITY_X(grid,MINUS_HALF(x),y));
int y_vel_mag=(int)floor(0.1*GRID_VELOCITY_Y(grid,x,MINUS_HALF(y)));

    if(x_vel_mag>=0)for(i=0;i<x_vel_mag;i++)put_pixel(screen,sx+i,sy+4,255,0,0);
    else for(i=0;i>x_vel_mag;i--)put_pixel(screen,sx+i,sy+4,255,0,0);

    if(y_vel_mag>=0) for(i=0;i<y_vel_mag;i++)put_pixel(screen,sx+4,sy+i,255,0,0);
    else for(i=0;i>y_vel_mag;i--)put_pixel(screen,sx+4,sy+i,255,0,0);

}
*/
