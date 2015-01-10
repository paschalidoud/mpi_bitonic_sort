#include "utils.h"

/*
 *@brief: Function that generates random numbers
 *@param: dataset [int*]: pointer to array of numbers
 *@param: size [int]: length of sequence to be created
 *@return: void 
*/ 
void generateDataSet(int *dataset, int size, int processId, int *all_data)
{
  printf("Creating dataset of size %d....\n", size);
  srand(time(NULL)+processId);
  int i;
  for(i=0; i<size; ++i){
    int r=rand()%((size-1)+15);
    dataset[i] = r;
    all_data[processId*size+i] =r;
  }
}

/*
 *@brief: Function that compares two elements
 *@param: a [const void *]: first parameter to be compared
 *@param: b [const void *]: second parameter to be compared
 *@param: type [bool]: if true sorts in ascending order, if false
 *  in descending order.
 *@return: [int]
*/ 
int comparator(const void *a, const void *b)
{
   return (*(int*)a - *(int*)b);
}

/*
 *@brief: Function that compares two elements in descending order
 *@param: a [const void *]: first parameter to be compared
 *@param: b [const void *]: second parameter to be compared
 *@return: [int]
*/ 
int inverseComparator(const void *a, const void *b)
{
   return (*(int*)b - *(int*)a);
}

/*
 *@brief: Function that merges two sorted lists in one
 *@param: a [int *]: pointer to first array to be merged
 *@param: b [int *]: pointer to second array to be merged
 *@param: size [int ]: sizes of both arrays to be merged
 *@param: sorted [int *]: pointer to final merged-sorted array
*/ 
void merge(int *a, int *b, int size, int *sorted) 
{
  int i, j, k;
  j = k = 0;
  for (i = 0; i < 2*size;) {
    if (j < size && k < size) {
      if (a[j] < b[k]) {
        sorted[i] = a[j];
        j++;
      }
      else {
        sorted[i] = b[k];
        k++;
      }
      i++;
    }
    else if (j == size) {
      for (; i < 2*size;) {
        sorted[i] = b[k];
        k++;
        i++;
      }
    }
    else {
      for (; i < 2*size;) {
        sorted[i] = a[j];
        j++;
        i++;
      }
    }
  }
}

enum bool verify_bitonic_sort_small(int *all_data, int *data, int size, int N)
{
  int i;
  enum bool test = true;
  qsort(all_data, N, sizeof(int), comparator);
  for(i=0; i< size; i++)
    if(data[i] != all_data[i])
        test = false;
  
  if(test == false)
    printf("Small test failed...Looser!!!\n");
  else
    printf("Small test passed...\n");
  
  return test;  
}

enum bool verify_bitonic_sort_massive(int *all_data, int *sorted_data, int N)
{
  int i;
  enum bool test1 = true;
  enum bool test2 = true;
  for(i=0; i<N-1; i++){
    if(sorted_data[i] > sorted_data[i+1])
      test1 = false;
  }
  qsort(all_data, N, sizeof(int), comparator);
  for(i=0; i< N; i++)
    if(sorted_data[i] != all_data[i]){
      test2 = false;
      printf("Wrong index is %d\n", i);
    }
        
  
  if(test1 == false || test2 == false)
    printf("Massive test failed...Looser!!!\n");
  else
    printf("Massive test passed...\n");
    
  return test1&&test2;
}
