#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MIN_NUMBER -10000
#define MAX_NUMBER 10000
#define ARR_SIZE 100000
#define THRESH_HOLD 10

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
  
  int t = 100;
  while (t--){
    clock_t timer;
    timer = clock();

    int * array = (int*) malloc(sizeof(int) * ARR_SIZE);
    for(int i = 0; i < ARR_SIZE; i++)
        array[i] = get_random_number(MIN_NUMBER, MAX_NUMBER);
    tim_sort(array, ARR_SIZE);
    
    timer = clock() - timer;
    double time = ((double)timer) / CLOCKS_PER_SEC;
    
    printf("%f, ", time);
 
  }
  printf("\n");
  return 0;
}