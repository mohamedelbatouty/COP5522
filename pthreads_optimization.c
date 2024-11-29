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

int max_threads = 0;
int current_threads = 0;
pthread_mutex_t thread_count_mutex = PTHREAD_MUTEX_INITIALIZER;

void* mergeSortHelper(void* args);

void merge(int arr[], int left, int mid, int right) {
    int i, j, k;
    int n1 = mid - left + 1;
    int n2 = right - mid;

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
        
        if(right - left  > MIN){
            pthread_t leftThread, rightThread;

            pthread_mutex_lock(&thread_count_mutex);
            current_threads += 2;
            if (current_threads > max_threads) {
                max_threads = current_threads;
            }
            pthread_mutex_unlock(&thread_count_mutex);

            ArrayArgs leftArgs = {arr, left, mid};
            pthread_create(&leftThread, NULL, mergeSortHelper, (void*)&leftArgs);

            ArrayArgs rightArgs = {arr, mid+1, right};
            pthread_create(&rightThread, NULL, mergeSortHelper, (void*)&rightArgs);
            
            pthread_join(leftThread, NULL);
            pthread_join(rightThread, NULL);

            pthread_mutex_lock(&thread_count_mutex);
            current_threads -= 2;
            pthread_mutex_unlock(&thread_count_mutex);

            merge(arr, left, mid, right);
        }else{
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
    fclose(file);
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
    double time1 = microtime();    
    mergeSort(loaded_array, 0, n - 1);
    double time2 = microtime();

    double t = time2 - time1;
    printf("\nTime = %g us\n", t);

    for (int i = 0; i < n-1; i++) {
        if (loaded_array[i] > loaded_array[i+1]) {
            printf("Test data arrays not sorted: %i > %i", loaded_array[i], loaded_array[i+1]);
        }
    }

    printf("Max threads used = %d\n", max_threads);

    return 0;
}

