#include <microtime.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IST 32

void insertionSort(int arr[], int left, int right) {
    for (int i = left + 1; i <= right; i++) {
        int key = arr[i];
        int j = i - 1;
        while (j >= left && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

void merge(int arr[], int temp[], int left, int mid, int right) {
    int i = left, j = mid + 1, k = left;

    while (i <= mid && j <= right) {
        if (arr[i] <= arr[j]) {
            temp[k++] = arr[i++];
        } else {
            temp[k++] = arr[j++];
        }
    }

    while (i <= mid) {
        temp[k++] = arr[i++];
    }
    while (j <= right) {
        temp[k++] = arr[j++];
    }

   
    for (i = left; i <= right; i++) {
        arr[i] = temp[i];
    }
}

void mergeSort(int arr[], int n) {
    int* temp = (int*)malloc(n * sizeof(int));
    if (temp == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    } 
    for (int i = 0; i < n; i += IST) {
        int right = (i + IST - 1 < n) ? i + IST - 1 : n - 1;
        insertionSort(arr, i, right);
    }

    for (int size = IST; size < n; size *= 2) {
        for (int left = 0; left < n - size; left += 2 * size) {
            int mid = left + size - 1;
            int right = (left + 2 * size - 1 < n) ? left + 2 * size - 1 : n - 1;
            merge(arr, temp, left, mid, right);
        }
    }

    free(temp);
}

int* load_test_data(const char* filename, int* out_length) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening test data file.\n");
        exit(1);
    }
    int length;
    if (fscanf(file, "%d", &length) != 1) {
        printf("Test data file must contain array length on line 0.\n");
        fclose(file);
        exit(1);
    }

    int* arr = (int*)malloc(length * sizeof(int));
    if (arr == NULL) {
        printf("Memory allocation failed.\n");
        fclose(file);
        exit(1);
    }
    for (int i = 0; i < length; i++) {
        if (fscanf(file, "%d", &arr[i]) != 1) {
            printf("Error reading number on line %d.\n", i);
            free(arr);
            fclose(file);
            exit(1);
        }
    }
    fclose(file);
    *out_length = length;
    return arr;
}

int main() {
    int loaded_length;
    int* loaded_array = load_test_data("testdata.txt", &loaded_length);
    double time1 = microtime();
    mergeSort(loaded_array, loaded_length);
    double time2 = microtime();
    double t = time2 - time1;
    printf("\nTime = %g us\n", t);
    for (int i = 0; i < loaded_length - 1; i++) {
        if (loaded_array[i] > loaded_array[i + 1]) {
            printf("Test data arrays not sorted: %i > %i", loaded_array[i], loaded_array[i+1]);
            free(loaded_array);
            return 1;
        }
    }

    free(loaded_array);
    return 0;
}
