#ifndef BITONIC_SORTER_H 
#define BITONIC_SORTER_H

#include <stdio.h> 
#include <math.h> 
#include "mpi.h" 
#include "utils.h"
#include "defines.h"

/*
 *@brief: Function that distributes data to processes. Master process
 * creates andsends data to slave processes.
 *@return: void 
*/ 
void distributeData();

/*
 *@brief: Basic function responsible for the construction 
 * of the bitonic sorting network.
 *@param: stages [int]: stages of bitonic sorting network
 *@param: mode [int]: if mode is 1 simple mpi bitonic sort is executed
 * otherwise optimized mpi bitonic sort is executed.
*/ 
void bitonic_sort(int stages, int mode);

void compare_exchange_max(int mask);
void compare_exchange_min(int mask);
void compare_exchange_max_optimized(int mask, int stage);
void compare_exchange_min_optimized(int mask, int stage);

#endif //BITONIC_SORTER_H
