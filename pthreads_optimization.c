#include <microtime.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

//Thread struct for each thread
typedef struct{
    int* arr;
    int left;
    int right;
}ArrayArgs;

#define MIN 1500
#define MAX_THREADS 32
int activeThreads = 0;
int max_threads_reached = 0;
pthread_mutex_t threadMutex = PTHREAD_MUTEX_INITIALIZER;

void* mergeSortHelper(void* args);

void merge(int arr[], int left, int mid, int right) {
    int i, j, k;
    int n1 = mid - left + 1;
    int n2 = right - mid;

    // Allocate memory for left and right subarrays
    int *leftA = (int*)malloc(n1 * sizeof(int));
    int *rightA = (int*)malloc(n2 * sizeof(int));

    for (i = 0; i < n1; i++)
        leftA[i] = arr[left + i];
    for (j = 0; j < n2; j++)
        rightA[j] = arr[mid + 1 + j];
    i = 0;
    j = 0;
    k = left;
    while (i < n1 && j < n2) {
        if (leftA[i] <= rightA[j]) {
            arr[k] = leftA[i];
            i++;
        } else {
            arr[k] = rightA[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        arr[k] = leftA[i];
        i++;
        k++;
    }

    while (j < n2) {
        arr[k] = rightA[j];
        j++;
        k++;
    }

    free(leftA);
    free(rightA);
}

void mergeSort(int arr[], int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        
        if(right - left  > MIN && activeThreads < MAX_THREADS){
            pthread_t leftThread, rightThread;

            pthread_mutex_lock(&threadMutex);
            activeThreads += 2;
            if(activeThreads > max_threads_reached){
                max_threads_reached = activeThreads;
            }
            pthread_mutex_unlock(&threadMutex);
            
            ArrayArgs leftArgs = {arr, left, mid};
            ArrayArgs rightArgs = {arr, mid+1, right};                        
            pthread_create(&rightThread, NULL, mergeSortHelper, (void*)&rightArgs);
            pthread_create(&leftThread, NULL, mergeSortHelper, (void*)&leftArgs);

            pthread_join(leftThread, NULL);
            pthread_join(rightThread, NULL);

            merge(arr, left, mid, right);

            pthread_mutex_lock(&threadMutex);
            activeThreads -= 2;
            pthread_mutex_unlock(&threadMutex);
        }else{
            // If small enough, just run serial mergeSort
            mergeSort(arr, left, mid);
            mergeSort(arr, mid + 1, right);
            merge(arr, left, mid, right);
        }
    }
}

int* load_test_data(const char* filename, int* out_length) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening test data file");
        exit(0);
    }

    int length;
    if (fscanf(file, "%d", &length) != 1) {
        printf("Test data file must contain array length on line 0");
        fclose(file);
        exit(0);
    }

    int* arr = (int*)malloc(length * sizeof(int));

    for (int i = 0; i < length; i++) {
        if (fscanf(file, "%d", &arr[i]) != 1) {
            printf("Error reading number on line %i", i);
            free(arr);
            fclose(file);
            exit(0);
        }
    }

    // Close the file
    fclose(file);

    // Set the output length
    *out_length = length;

    return arr;
}

void* mergeSortHelper(void* args) {
    ArrayArgs* arrArgs = (ArrayArgs*)args;
    mergeSort(arrArgs->arr, arrArgs->left, arrArgs->right);
    return NULL;
}

int main() {
    int loaded_length;
    int* loaded_array = load_test_data("testdata.txt", &loaded_length);
    int n = loaded_length;
    int notSorted = 0;

    double time1 = microtime();
    
    mergeSort(loaded_array, 0, n - 1);

    double time2 = microtime();

    double t = time2 - time1;
    // Print results
    printf("\nTime = %g us\n", t);
    printf("Max threads reached: %d\n", max_threads_reached);

    for (int i = 0; i < n-1; i++) {
        if (loaded_array[i] > loaded_array[i+1]) {
            notSorted = 1;
        }
    }
    if(notSorted){
        printf("Sorting algorithm did not work.\n");
    }

    free(loaded_array);
    return 0;
}

