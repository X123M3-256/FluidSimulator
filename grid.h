#ifndef GRID_H_INCLUDED
#define GRID_H_INCLUDED
#include "particle.h"

typedef enum
{
SOLID=0x1,
FLUID=0x2,
EMPTY=0x3,
INFLOW=0x10,
OUTFLOW=0x20,
CELL_FLAGS_MASK=0xF0,
CELL_TYPE_MASK=0x0F
}grid_cell_type_t;

typedef struct
{
grid_cell_type_t type;
float divergence;
float neighbours;
float pressure;
//For fluid cells
float fluid_fraction;
//For conjugate gradient
float matrix_conjugate;
float residual;
float conjugate;
}grid_cell_t;


typedef struct grid_s
{
unsigned int width;
unsigned int height;
float* velocity_x;
float* velocity_y;
float* velocity_x_old;
float* velocity_y_old;
float* velocity_delta_x;
float* velocity_delta_y;
float* weight_sum_x;
float* weight_sum_y;
grid_cell_t* cells;
}grid_t;


/*Although velocity values are defined on the edge of the cells, indices must
still be integers. This can cause confusion, as cell[0 and velocity_x[0] refer
to different locations on the grid. These macros only serve to make explicit
when we are referencing values stored on the edges of cells and when we are not.*/
#define MINUS_HALF(x) (x)
#define PLUS_HALF(x) ((x)+1)
/*For efficiency, the grid is stored as a one-dimensional array, so these
macros are provided for referencing*/
#define GRID_CELL(grid,x,y) ((grid)->cells[(x)+(grid)->width*(y)])
#define GRID_CELL_TYPE(grid,x,y) (GRID_CELL(grid,x,y).type&CELL_TYPE_MASK)
#define GRID_CELL_SET_TYPE(grid,x,y,t) (GRID_CELL(grid,x,y).type=(t)|(GRID_CELL(grid,x,y).type&CELL_FLAGS_MASK))
#define GRID_CELL_FLAGS(grid,x,y) (GRID_CELL(grid,x,y).type&CELL_FLAGS_MASK)
#define GRID_VELOCITY_X(grid,x,y) ((grid)->velocity_x[(x)+(grid)->width*(y)])
#define GRID_VELOCITY_Y(grid,x,y) ((grid)->velocity_y[(x)+(grid)->width*(y)])
#define GRID_PREV_VELOCITY_X(grid,x,y) ((grid)->velocity_x_old[(x)+(grid)->width*(y)])
#define GRID_PREV_VELOCITY_Y(grid,x,y) ((grid)->velocity_y_old[(x)+(grid)->width*(y)])
#define GRID_DELTA_VELOCITY_X(grid,x,y) ((grid)->velocity_delta_x[(x)+(grid)->width*(y)])
#define GRID_DELTA_VELOCITY_Y(grid,x,y) ((grid)->velocity_delta_y[(x)+(grid)->width*(y)])
#define GRID_WEIGHT_SUM_X(grid,x,y) ((grid)->weight_sum_x[(x)+(grid)->width*(y)])
#define GRID_WEIGHT_SUM_Y(grid,x,y) ((grid)->weight_sum_y[(x)+(grid)->width*(y)])

grid_t* grid_new(unsigned int width,unsigned int height);
void grid_set_rectangle(grid_t* grid,grid_cell_type_t type,unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2);

void grid_get_x_velocity_box(float x,float y,unsigned int* x_index,unsigned int* y_index,float* disp_x,float* disp_y);
void grid_get_y_velocity_box(float x,float y,unsigned int* x_index,unsigned int* y_index,float* disp_x,float* disp_y);
void grid_get_velocity_at_point(grid_t* grid,float x,float y,float* velocity_x,float* velocity_y);

void grid_save_velocities(grid_t* grid);
void grid_apply_gravity(grid_t* grid,float delta_t);
void grid_project(grid_t* grid,float delta_t);
void grid_transfer_to_particle_system(grid_t* grid,particle_system_t* particle_system);
void grid_calculate_velocity_delta(grid_t* grid);


#endif // GRID_H_INCLUDED
