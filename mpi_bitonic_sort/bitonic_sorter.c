#include <math.h> 
#include "bitonic_sorter.h"

int process_id;
int num_processes;
int size;
int *data;
int *partner_data;
int *total;
int *all_data;
int * sorted_data;
int N;

int main(int argc, char **argv)
{
  /// Check if the command line arguements are correct
  if (argc != 4) {
    printf("Three arguements required...\n");
    printf("p: total number of processes (in power of two) \n");
    printf("q: length of assigned points in each process (in power of two) \n");
    printf("mode: choose 1 for simple mpi bitonic sort and 2 for optimized mpi bitonic sort \n");
    return(1);
  }
  
  struct mpi_timers mpi_timer;
 
  ///Total number of points
  N = 1<<atoi(argv[1]);
  num_processes = 1<<atoi(argv[2]);
  all_data = (int*)malloc(N*sizeof(int));
  
  int mode = atoi(argv[3]);
    
  ///Initializes MPI execution enviroment
  int rc = MPI_Init (&argc,&argv);
  if(rc!=MPI_SUCCESS)
    printf("Error starting MPI program. Terminating.\n");
  
  MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
  MPI_Comm_rank(MPI_COMM_WORLD, &process_id);
  if(process_id == MASTER_ID) 
    printf("Number of Processes spawned: %d\n", num_processes);
  
  int i;
  size = N/num_processes;
  distributeData();
  ///Blocks until all processes have finished generating
  MPI_Barrier(MPI_COMM_WORLD);
  
  #ifdef TIMER_ENABLE
    if(process_id==MASTER_ID)
      mpi_timer.timer_start = MPI_Wtime();
  #endif
  
  ///Choose size of subdatas
  qsort(data, size, sizeof(int), comparator);
  #ifdef DEBUG_ENABLE
    printf("Hi from process %d \n", process_id);
    for(i=0; i< size; i++)
      printf("%d ", data[i]);
    printf("\n");
  #endif
  MPI_Barrier(MPI_COMM_WORLD);
  
  partner_data = (int*)malloc(size*sizeof(int));
  total = (int*)malloc(2*size*sizeof(int));
  
  int stages = (int)(log(num_processes)/log(2));
  bitonic_sort(stages, mode);
  ///Blocks until all processes have finished generating
  MPI_Barrier(MPI_COMM_WORLD);
  
  enum bool test1, test2;
  if(process_id == MASTER_ID){
    #ifdef TIMER_ENABLE
      mpi_timer.timer_end = MPI_Wtime();
      printf("Time elapsed in seconds: %f\n", mpi_timer.timer_end - mpi_timer.timer_start);
    #endif
    /// Run first test, compare only lower data
    test1 = 
      verify_bitonic_sort_small(all_data, data, size, N);
    
    /// Run second test, check if data are in ascending order and if
    /// "homemade" bitonic sort has same results with qsort
    sorted_data = (int*)malloc(N*sizeof(int));
    if(sorted_data == NULL)
      printf("Executing test...\n");
  }    

  int error = MPI_Gather(
    data, 
    size, 
    MPI_INT, 
    sorted_data, 
    size, 
    MPI_INT, 
    MASTER_ID , 
    MPI_COMM_WORLD
  );
  if(error!=0)
    printf("Unable to gather data of all processes\n");
  
  if(process_id == MASTER_ID && sorted_data != NULL ){
    test2 = 
      verify_bitonic_sort_massive(all_data, sorted_data, N);
    if(test1 == true && test2 == true)
      printf("Success... :-)\n");
    else
      printf("Failure.... :-(\n");  
  }
 
  
  free(data);
  free(partner_data);
  free(total);
  MPI_Finalize();
}

/*
 *@brief: Function that distributes data to processes. Master process
 *  creates andsends data to slave processes.
*/ 
void distributeData()
{
  ///Create data of random numbers for every process to sort
  if(process_id == MASTER_ID){
    for(int slave_id=1; slave_id<num_processes; slave_id++){
      data = (int*)malloc(size*sizeof(int));
      generateDataSet(data, size, slave_id, all_data);
      ///Send subdatas to every slave process
      char buffer[MPI_MAX_ERROR_STRING];
      int len;
      int error = MPI_Send(
        data, 
        size, 
        MPI_INT, 
        slave_id, 
        55, 
        MPI_COMM_WORLD
      ); 
      if(error!=0){
        MPI_Error_string(error, buffer, &len);
        printf("Error  [%s]\n", buffer);
      }
       #ifdef DEBUG_SHOW  
        printf("Maste sent data...\n");
      #endif  
    }
    data = (int*)malloc(size*sizeof(int));
    generateDataSet(data, size, MASTER_ID, all_data);
  }
  else{
    data = (int*)malloc(size*sizeof(int));
    MPI_Recv(
      data, 
      size, 
      MPI_INT, 
      MASTER_ID, 
      55, 
      MPI_COMM_WORLD, 
      MPI_STATUS_IGNORE
    );
    #ifdef DEBUG_SHOW
      printf("Slave with slave-id %d received data from master process \n", process_id);
    #endif  
  }
}

/*
 *@brief: Basic function responsible for the construction 
 * of the bitonic sorting network.
 *@param: stages [int]: stages of bitonic sorting network
 *@param: mode [int]: if mode is 1 simple mpi bitonic sort is executed
 * otherwise optimized mpi bitonic sort is executed.
*/ 
void bitonic_sort(int stages, int mode)
{
  int i,j;
  if(mode==1){
    for(i=0; i<stages; i++){
      for(j=i; j>=0; j--) {
        if(((process_id >> (i + 1)) % 2 == 0 && (process_id >> j) % 2 == 0) || 
            ((process_id >> (i + 1)) % 2 != 0 && (process_id >> j) % 2 != 0)
        )
          compare_exchange_max(j);
        else 
          compare_exchange_min(j);
      }
      #ifdef DEBUG_ENABLE
        printf("Proceeding to next stage %d...\n", i);
      #endif  
    }
  }
  else{
    for(i=0; i<stages; i++){
      for(j=i; j>=0; j--) {
        if(((process_id >> (i + 1))%2 == 0 && (process_id >> j) % 2 == 0) || 
            ((process_id >> (i + 1))%2 != 0 && (process_id >> j) % 2!=0)
        )
          compare_exchange_max_optimized(j, i);
        else 
          compare_exchange_min_optimized(j, i);
      }
      #ifdef DEBUG
        printf("Proceeding to next stage %d...\n", i+1);
      #endif
    }
  }  
}

void compare_exchange_max(int mask)
{
  int partner = process_id ^ (1 << mask);
  #ifdef DEBUG_ENABLE
    printf("Paired processes:  my process_id %d, partner_id %d\n", process_id, partner);
  #endif 
   
  char buffer[MPI_MAX_ERROR_STRING];
  int len;
  MPI_Status status;
  ///Send whole buffer to paired process
  int i;
  int error1 = MPI_Send(
    data, 
    size, 
    MPI_INT, 
    partner, 
    0, 
    MPI_COMM_WORLD
  );
  if(error1!=0){
    MPI_Error_string(error1, buffer, &len);
    printf("Error in process id %d while sending [%s]\n", process_id, buffer);
  }

  #ifdef DEBUG_SHOW 
    printf("Process %d sends data to process %d \n", process_id, partner);
    for(i=0; i<size; i++)
      printf("%d ", data[i]);
    printf("\n");  
  #endif
  
  ///Receive buffer from paired process and store it to partner_data
  int error2 = MPI_Recv( 
    partner_data, 
    size, 
    MPI_INT, 
    partner, 
    0, 
    MPI_COMM_WORLD, 
    &status
  );
  if(error2!=0){
    MPI_Error_string(error2, buffer, &len);
    printf("Error in process id %d while receiving [%s]\n", process_id, buffer);
  }
  
  #ifdef DEBUG_SHOW
    printf("Process %d received data from process %d \n", process_id, partner); 
    for(i=0; i<size; i++)
      printf("%d ", partner_data[i]);
    printf("\n");
  #endif
  
  ///Merge two buffers in one and sort them
  merge(data, partner_data, size, total); 
  #ifdef DEBUG_SHOW
    printf("Hi from process %d ,total are: \n", process_id);
    for(i=0; i< 2*size; i++)
      printf("%d\n", total[i]);
  #endif    
  
  ///Copy only half of merged data
  memcpy(data, total, size*sizeof(int)); 
  #ifdef DEBUG_ENABLE
    printf("Hi from process %d ,my data after compare_exchange_max are: \n", process_id);
    for(i=0; i< size; i++)
      printf("Data of process_id %d data[%d]=%d\n ", process_id, i, data[i]);    
  #endif    
}

void compare_exchange_max_optimized(int mask, int stage)
{
  int partner = process_id ^ (1 << mask);
  #ifdef DEBUG_ENABLE
    printf("Paired processes:  my process_id %d, partner_id %d\n", process_id, partner);
  #endif  
  
  MPI_Status status;
  char buffer[MPI_MAX_ERROR_STRING];
  int len;
  
  ///Send maximum element to paired process
  int i;
  int error1 = MPI_Send(
    &data[size-1],
    1,
    MPI_INT,
    partner,
    0, 
    MPI_COMM_WORLD
  );
  if(error1!=0){
    MPI_Error_string(error1, buffer, &len);
    printf("Error in process id %d while sending maximum value [%s]\n", process_id, buffer);
  }

  #ifdef DEBUG_SHOW 
    printf("Process %d sends local maximum %d to process %d \n", process_id, data[size-1], partner);
  #endif
  
  ///Receive new min from paired process
  int min;
  int error2 = MPI_Recv( 
    &min, 
    1, 
    MPI_INT, 
    partner, 
    0, 
    MPI_COMM_WORLD, 
    &status
  );
  if(error2!=0){
    MPI_Error_string(error2, buffer, &len);
    printf("Error in process id %d while receiving minimum value [%s]\n", process_id, buffer);
  }
  
  #ifdef DEBUG_SHOW
    printf("Process %d receives local minimum %d from process %d \n", process_id, min, partner); 
  #endif
  
  ///Check all data that are greater than received min and send them to 
  ///paired process
  int* buffer1 = malloc((size+1)*sizeof(int));
  int cnt = 0;
  for(i=0; i<size; i++){
    if(data[i] > min){
      buffer1[cnt+1] = data[i];
      cnt++;
    }
  }
  buffer1[0] = cnt;
    
  ///Send buffer of data greater than received min from paired process
  int error3 = MPI_Send(
    buffer1,
    cnt+1,
    MPI_INT,
    partner,
    0,
    MPI_COMM_WORLD
  );
  if(error3!=0){
    MPI_Error_string(error3, buffer, &len);
    printf("Error in process id %d while sending [%s]\n", process_id, buffer1);
  }
  #ifdef DEBUG_ENABLE
    printf("Process %d sends data greater than %d to process %d \n", process_id, min, partner);
    for(i=1; i<buffer1[0]+1; i++)
      printf("%d ", buffer1[i]);
    printf("\n");  
  #endif
  
  ///Receive buffer of data that are smaller than maximum element of current process 
  int * buffer2 = malloc((size+1)*sizeof(int));
  int error4 = MPI_Recv(
    buffer2,
    size+1,
    MPI_INT,
    partner, 
    0, 
    MPI_COMM_WORLD, 
    &status
  );
  if(error4!=0){
    MPI_Error_string(error4, buffer, &len);
    printf("Error in process id %d while receiving data smaller than max element [%s]\n", process_id, buffer2);
  }
  
  #ifdef DEBUG_ENABLE
    printf("Process %d receives data of size %d smaller than %d from process %d \n", process_id, buffer2[0], data[size-1], partner);
    for(i=1; i<buffer2[0]+1; i++)
      printf("%d ", buffer2[i]);
    printf("\n");
  #endif
  
  /// Take received buffer of values from H Process which are smaller than current max
  for(i=1; i<buffer2[0]+1; i++){
    if(data[size-1] > buffer2[i]){
      data[size-1] = buffer2[i];
      qsort(data, size, sizeof(int), comparator);
    }
  }
  
  #ifdef DEBUG_ENABLE
    printf("Hi from process %d in stage %d,my data after compare_exchange_max wiht %d are: \n", process_id, stage, partner);
    for(i=0; i< size; i++)
      printf("Data of process_id %d data[%d]=%d\n ", process_id, i, data[i]);    
  #endif
  
  free(buffer1);
  free(buffer2);
}

void compare_exchange_min_optimized(int mask, int stage)
{
  int partner = process_id ^ (1 << mask);
  
  char buffer[MPI_MAX_ERROR_STRING];
  int len;
  MPI_Status status;
  ///Receive buffer of paired process and store in partner_data
  int i;
  int max;
  ///Receive maximum element from paired process
  int error1 = MPI_Recv( 
    &max, 
    1, 
    MPI_INT, 
    partner, 
    0, 
    MPI_COMM_WORLD, 
    &status
  );
  if(error1!=0){
    MPI_Error_string(error1, buffer, &len);
    printf("Error in process id %d while receiving [%s]\n", process_id, buffer);
  }
  
  #ifdef DEBUG_SHOW
    printf("Process %d receives max element %d from process %d \n", process_id, max, partner); 
  #endif
  
  ///Send current mimimum to paired process
  int error2 = MPI_Send( 
    &data[0],
    1,
    MPI_INT,
    partner,
    0,
    MPI_COMM_WORLD
  );
  if(error2!=0){
    MPI_Error_string(error2, buffer, &len);
    printf("Error in process id %d while sending [%s]\n", process_id, buffer);
  }
  #ifdef DEBUG_SHOW
    printf("Process %d sends local minimum %d to process %d \n", process_id, data[0], partner);
  #endif    
  
  ///Check all data that are smaller than received max and send them to 
  ///paired process
  int * buffer1 = malloc((size+1)* sizeof(int));
  int cnt = 0;
  for(i=0; i<size; i++){
    if(data[i] < max){
      buffer1[cnt+1] = data[i];
      cnt++;
    }
  }
  buffer1[0] = cnt;
  
  int * buffer2 = malloc((size+1)* sizeof(int));
  ///Receive blocks greater than current min from paired process
  int error3 = MPI_Recv(
    buffer2,
    size+1,
    MPI_INT,
    partner,
    0,
    MPI_COMM_WORLD,
    &status
  );
  if(error3!=0){
    MPI_Error_string(error3, buffer, &len);
    printf("Error in process id %d while receiving [%s]\n", process_id, buffer);
  }
  #ifdef DEBUG_ENABLE
    printf("Process %d receives from process %d data greater than %d \n", process_id, partner, data[0]);
    for(i=1; i<buffer2[0]+1; i++)
      printf("%d ", buffer2[i]);
    printf("\n");
  #endif
  
  ///Send buffer1, with elements smaller than max of paired process
  int error4 = MPI_Send(
    buffer1,
    cnt+1,
    MPI_INT,
    partner,
    0,
    MPI_COMM_WORLD
  );
  if(error4!=0){
    MPI_Error_string(error4, buffer, &len);
    printf("Error in process id %d while receiving [%s]\n", process_id, buffer);
  } 
  #ifdef DEBUG_ENABLE 
    printf("Process %d sends to process %d data smaller than %d \n", process_id, partner, max);
    for(i=1; i<buffer1[0]+1; i++)
      printf("%d ", buffer1[i]);
    printf("\n");  
  #endif
  
  /// Store values, which are greater than current min
  for(i=1; i<buffer2[0]+1; i++){
    if (buffer2[i] > data[0]) {
      data[0] = buffer2[i];
      qsort(data, size, sizeof(int), comparator);
    }
  }
  
  #ifdef DEBUG_ENABLE
    printf("Hi from process %d in stage %d,my data after compare_exchange_min with %d are: \n", process_id, stage, partner);
    for(i=0; i< size; i++)
      printf("Data of process_id %d data[%d]=%d\n ", process_id, i, data[i]);    
  #endif
  
  free(buffer1);
  free(buffer2);
}

void compare_exchange_min(int mask)
{
  int partner = process_id ^ (1 << mask); 
  MPI_Status status;
  char buffer[MPI_MAX_ERROR_STRING];
  int len;
  
  int i;
  int error1 = MPI_Recv( 
    partner_data, 
    size, 
    MPI_INT, 
    partner, 
    0, 
    MPI_COMM_WORLD, 
    &status
  );
  if(error1!=0){
    MPI_Error_string(error1, buffer, &len);
    printf("Error in process id %d while receiving [%s]\n", process_id, buffer);
  }
  #ifdef DEBUG_SHOW
    printf("Process %d received data from process %d \n", process_id, partner); 
    for(i=0; i<size; i++)
      printf("%d ", partner_data[i]);
    printf("\n");
  #endif
  
  ///Send data to paired process
  int error2 = MPI_Send( 
    data,
    size,
    MPI_INT,
    partner,
    0,
    MPI_COMM_WORLD
  );
  if(error2!=0){
    MPI_Error_string(error2, buffer, &len);
    printf("Error in process id %d while sending [%s]\n", process_id, buffer);
  }
  #ifdef DEBUG_SHOW
    printf("Process %d sends data to process %d \n", process_id, partner);
    for(i=0; i<size; i++)
      printf("%d ", data[i]);
    printf("\n"); 
  #endif    
  
  merge(data, partner_data, size, total); 
  #ifdef DEBUG_SHOW
    printf("Hi from process %d ,my total are: \n", process_id);
    for(i=0; i< 2*size; i++)
      printf("Total of process_id %d total[%d]=%d\n", process_id, i,total[i]);
  #endif
  
  memcpy(data, total+size, size*sizeof(int));  
  #ifdef DEBUG_SHOW
    printf("Hi from process %d ,my data after compare_exchange_min are: \n", process_id);
    for(i=0; i< size; i++)
      printf("Data of process_id %d data[%d]=%d\n ", process_id, i, data[i]);
  #endif
}


