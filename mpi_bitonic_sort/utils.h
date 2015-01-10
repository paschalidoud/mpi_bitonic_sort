#ifndef UTILS_H
#define UTILS_H

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "defines.h"

/*
 *@brief: Function that generates random numbers
 *@param: dataset [int*]: pointer to array of numbers
 *@param: size [int]: length of sequence to be created
 *@return: void 
*/ 
void generateDataSet(int *dataset, int size, int processId, int *all_data);

/*
 *@brief: Function that compares two elements in ascending order
 *@param: a [const void *]: first parameter to be compared
 *@param: b [const void *]: second parameter to be compared
 *@return: [int]
 */ 
int comparator(const void *a, const void *b);

/*
 *@brief: Function that compares two elements in descending order
 *@param: a [const void *]: first parameter to be compared
 *@param: b [const void *]: second parameter to be compared
 *@return: [int]
 */ 
int inverseComparator(const void *a, const void *b);

enum bool verify_bitonic_sort_small(
  int *all_data, 
  int *data, 
  int size, 
  int N
);
  
enum bool verify_bitonic_sort_massive(
  int *all_data, 
  int *sorted_data, 
  int N
);
#endif //UTILS_H
