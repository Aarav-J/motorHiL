#ifndef QUADRATURE_H
#define QUADRATURE_H

#include <stdint.h> 


typedef enum { 
    QUADRATURE_DIR_FORWARD = 0, 
    QUADRATURE_DIR_BACKWARE = 1
} quadrature_dir_t; 


typedef struct { 
    uint8_t a; 
    uint8_t b; 
} quadrature_outputs_t; 


uint8_t quadrature_advance_step(uint8_t step, quadrature_dir_t dir); 
quadrature_outputs_t quadrature_get_outputs(uint8_t step); 



#endif // QUADRATURE_H
