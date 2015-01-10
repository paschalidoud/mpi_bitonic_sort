#ifndef DEFINES_H 
#define DEFINES_H

#define MASTER_ID 0
//~ #define DEBUG_ENABLE
//~ #define DEBUG_SHOW
//~ #define DEBUG
#define TIMER_ENABLE

/*
 *Define wrapper to use boolean variables 
*/ 
enum bool{ 
  false, 
  true 
};

/*
 *Define a struct that corresponds to global variables
 *associated with the mpi enviroment
*/
struct mpi_timers{
  double timer_start;
  double timer_end;
};


#endif //DEFINES_H
