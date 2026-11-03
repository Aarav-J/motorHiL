#include "quadrature.h"

static const quadrature_outputs_t quadrature_table[4] = { 
    {0,0}, 
    {1, 0}, 
    {1,1}, 
    {0,1}
}; 


uint8_t quadrature_advance_step(uint8_t step, quadrature_dir_t dir) { 
    if(dir == QUADRATURE_DIR_FORWARD) { 
        return (step + 1) % 4; 
    } else { 
        return (step + 3) % 4; 
    }
}

quadrature_outputs_t quadrature_get_outputs(uint8_t step) { 
    return quadrature_table[step % 4]; 
}


