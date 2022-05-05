#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MIN_NUMBER -10000
#define MAX_NUMBER 10000
#define ARR_SIZE 100
#define THRESH_HOLD 10

//helper functions
int get_random_number(int min, int max) {
    return rand() % (max+1-min) + min;
}

void print_array(int* arr, int n) {
  for(int i = 0; i < n; i++)
    printf("%d ", arr[i]);
  printf("\n");
}

int is_sorted(int * arr, int n) {
    for (int i = 1; i < n; i++)
        if (arr[i-1] > arr[i])
            return 0;
    return 1;
}

//Sorting functions
void _insertion_sort(int arr[], int left, int right) {
    for (int i = left + 1; i <= right; i++) {
        int temp = arr[i];
        int j = i - 1;
        while (j >= left && arr[j] > temp) {
            arr[j+1] = arr[j];
            j--;
        }
        arr[j+1] = temp;
    }
}

void merge(int array[], int a1[], int a2[], int n1, int n2)
{
    int i = 0;
    int j = 0;
    int k = 0;
    while (i < n1 && j < n2)
    {
        if (a1[i] <= a2[j])
            array[k] = a1[i++];
        else
            array[k] = a2[j++];
        k++;
    }
    while (i < n1)
        array[k++] = a1[i++];
    while (j < n2)
        array[k++] = a2[j++];
}

void _merge(int arr[], int l, int m, int r) {
    int len1 = m - l + 1, len2 = r - m;
    int left[len1], right[len2];
    for (int i = 0; i < len1; i++)
        left[i] = arr[l + i];
    for (int i = 0; i < len2; i++)
        right[i] = arr[m + 1 + i];

    int i = 0;
    int j = 0;
    int k = l;

    while (i < len1 && j < len2) {
        if (left[i] <= right[j])
            arr[k] = left[i++];
        else
            arr[k] = right[j++];
        k++;
    }

    while (i < len1)
        arr[k++] = left[i++];
    while (j < len2)
        arr[k++] = right[j++];
}

void _tim_sort(int * arr, int left, int right) {
  if (left < right){
    if (right - left < THRESH_HOLD) {
      _insertion_sort(arr, left, right);
    }else{
      int mid = (left+right) / 2;
      _tim_sort(arr, left, mid);
      _tim_sort(arr, mid+1, right);
      _merge(arr, left, mid, right);
    }
  }
}

void tim_sort(int arr[], int n){
  _tim_sort(arr, 0, n-1);
}

int main(int argc, char** argv) {
  srand(time(NULL));

  MPI_Init(&argc, &argv);  
  int rank, world_size;
  int * array = NULL; 
  
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  // if (rank == 0)
    // printf("Parallel program (3 processes) on array of size 1000000\n");

  int t = 100;
  while (t--) {
    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    int left_child = 2*rank + 1, right_child = 2*rank + 2, parent = (rank - 1) / 2;
    int array_size;
    
    if(rank == 0) { // generating random array 
      array = (int*) malloc(sizeof(int) * ARR_SIZE);
      array_size = ARR_SIZE;
      for(int i = 0; i < ARR_SIZE; i++)
        array[i] = get_random_number(MIN_NUMBER, MAX_NUMBER);
    }
    else { //requesting data from parent process
      MPI_Recv(&array_size, 1, MPI_INT, parent, 0, MPI_COMM_WORLD , MPI_STATUS_IGNORE);
      array = (int*) malloc(sizeof(int) * array_size);
      MPI_Recv(array, array_size, MPI_INT, parent, 1, MPI_COMM_WORLD , MPI_STATUS_IGNORE);
    }

    if (array_size > THRESH_HOLD && right_child < world_size){
      int n1 = array_size / 2;
      int rem = array_size % 2;
      int n2 = n1 + rem;
      //left child
      MPI_Send(&n1, 1, MPI_INT, left_child, 0, MPI_COMM_WORLD);
      MPI_Send(array, n1, MPI_INT, left_child, 1, MPI_COMM_WORLD);
      //right child
      MPI_Send(&n2, 1, MPI_INT, right_child, 0, MPI_COMM_WORLD);
      MPI_Send(array + n1, n2, MPI_INT, right_child, 1, MPI_COMM_WORLD);
    
      int * arr_left_child = (int*) malloc(sizeof(int) * n1);
      MPI_Recv(arr_left_child, n1, MPI_INT, left_child, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      int * arr_right_child = (int*) malloc(sizeof(int) * n2);
      MPI_Recv(arr_right_child, n2, MPI_INT, right_child, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      merge(array, arr_left_child, arr_right_child, n1, n2);
    } 
    else if (left_child == world_size - 1){
      int n1 = array_size / 2;
      int rem = array_size % 2;
      int n2 = n1 + rem;
      MPI_Send(&n2, 1, MPI_INT, left_child, 0, MPI_COMM_WORLD);
      MPI_Send(array + n1, n2, MPI_INT, left_child, 1, MPI_COMM_WORLD);

      int * arr_same = (int *) malloc(sizeof(int) * n1);
      for (int i=0; i<=n1; i++)
        arr_same[i] = array[i];
      tim_sort(arr_same, n1);

      int * arr_left_child = (int*) malloc(sizeof(int) * n1);
      MPI_Recv(arr_left_child, n1, MPI_INT, left_child, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      merge(array, arr_same, arr_left_child, n1, n2);
    }
    else {
      tim_sort(array, array_size);
    }

    if(rank != 0){
      MPI_Send(array, array_size, MPI_INT, parent, 2, MPI_COMM_WORLD);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double end_time = MPI_Wtime();

    if(rank == 0) {
      
      //print_array(array, array_size);
      // printf("is array sorted: %d \n", is_sorted(array, array_size));
      //printf("Total time: %fs\n", end_time-start_time);
      
      printf("%f, ", end_time-start_time);     
    }
  }

  if (rank == 0)
    printf("\n");
  MPI_Finalize();
  return 0;
}