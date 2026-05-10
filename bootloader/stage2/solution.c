#include "solution.h"
#include "stdio.h"
#include "ctype.h"

#define SCALE 1000L
#define NUM_NODES 50
#define MAX_ITER 10000

long K[NUM_NODES][NUM_NODES];
long F_load[NUM_NODES];
long U_disp[NUM_NODES];

void solve(void) {
    int i, j, iter;
    long sum, new_U, diff, max_diff;
    long k_stiff = 4;
    
    printf("--- 1D FEM: 32-bit ASM Accelerated Solver ---\r\n");
    
    for(i = 0; i < NUM_NODES; i++) {
        F_load[i] = 0;
        U_disp[i] = 0;
        for(j = 0; j < NUM_NODES; j++) {
            K[i][j] = 0;
        }
    }
    
    for(i = 0; i < NUM_NODES - 1; i++) {
        K[i][i] += k_stiff;
        K[i][i+1] -= k_stiff;
        K[i+1][i] -= k_stiff;
        K[i+1][i+1] += k_stiff;
    }
    
    K[0][0] = 1; 
    K[0][1] = 0; 
    K[1][0] = 0;
    
    F_load[NUM_NODES - 1] = 100 * SCALE; 
    
    for(iter = 0; iter < MAX_ITER; iter++) {
        max_diff = 0;
        
        for(i = 1; i < NUM_NODES; i++) {
            sum = 0;
            for(j = 1; j < NUM_NODES; j++) {
                if(i != j) {
                    sum += K[i][j] * U_disp[j];
                }
            }
            
            new_U = (F_load[i] - sum) / K[i][i];
            
            diff = new_U - U_disp[i];
            if (diff < 0) diff = -diff; 
            
            if(diff > max_diff) {
                max_diff = diff;
            }
            
            U_disp[i] = new_U;
        }
        
        if(max_diff == 0) {
            printf("\r\nConverged at iteration %d!\r\n", iter);
            break;
        }
    }
    
    printf("Displacements for last 3 nodes:\r\n");
    for(i = NUM_NODES - 3; i < NUM_NODES; i++) {
        long integer_part = U_disp[i] / SCALE;
        
        long fractional_part = U_disp[i] - (integer_part * SCALE);
        
        if (fractional_part < 0) fractional_part = -fractional_part;
        
        printf("Node %d: %d.%d\r\n", i, (int)integer_part, (int)fractional_part);
    }
}