#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <microtime.h>

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
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

void PMerge(int* arr, int left, int right, int rank, int size) {
    int n = right - left + 1;
    int chunk = n / size;
    int rem = n % size;
    int local_n = chunk + (rank < rem ? 1 : 0);
    int *local_arr = (int*)malloc(local_n * sizeof(int));
    int *sizes = (int*)malloc(size * sizeof(int));
    int *displs = (int*)malloc(size * sizeof(int));
    int displ = 0;
    for (int i = 0; i < size; i++) {
        sizes[i] = chunk + (i < rem ? 1 : 0);
        displs[i] = displ;
        displ += sizes[i];
    }

    MPI_Scatterv(arr + left, sizes, displs, MPI_INT, local_arr, local_n, MPI_INT, 0, MPI_COMM_WORLD);
    mergeSort(local_arr, 0, local_n - 1);
    int *sorted_arr = (int*)malloc(n * sizeof(int));
    MPI_Allgatherv(local_arr, local_n, MPI_INT, sorted_arr, sizes, displs, MPI_INT, MPI_COMM_WORLD);
    if (rank == 0) {
        int* temp_arr = (int*)malloc(n * sizeof(int));
        int start1 = 0;
        int size1 = sizes[0];

        for (int i = 1; i < size; i++) {
            int start2 = displs[i];
            int size2 = sizes[i];
            int k = 0, left = start1, right = start2;
            while (left < start1 + size1 && right < start2 + size2) {
                if (sorted_arr[left] <= sorted_arr[right]) {
                    temp_arr[k++] = sorted_arr[left++];
                } else {
                    temp_arr[k++] = sorted_arr[right++];
                }
            }
            while (left < start1 + size1) {
                temp_arr[k++] = sorted_arr[left++];
            }
            while (right < start2 + size2) {
                temp_arr[k++] = sorted_arr[right++];
            }
            memcpy(sorted_arr + start1, temp_arr, (size1 + size2) * sizeof(int));
            size1 += size2;
        }
        memcpy(arr, sorted_arr, n * sizeof(int));
        free(temp_arr);
    }
    free(local_arr);
    free(sizes);
    free(displs);
    free(sorted_arr);
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

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int loaded_length;
    int* loaded_array = NULL;

    if (rank == 0) {
        loaded_array = load_test_data("testdata.txt", &loaded_length);
    }
    MPI_Bcast(&loaded_length, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0)
        loaded_array = (int*)malloc(loaded_length * sizeof(int));
    
    double time1 = microtime();
    PMerge(loaded_array, 0, loaded_length - 1, rank, size);
    double time2 = microtime();

    if (rank == 0) {
        printf("\nTime = %g seconds\n", time2 - time1);
        for (int i = 0; i < loaded_length - 1; i++) {
            if (loaded_array[i] > loaded_array[i + 1]) {
                printf("Test data arrays not sorted: %i > %i\n", loaded_array[i], loaded_array[i + 1]);
            }
        }
        free(loaded_array);
    }
    MPI_Finalize();
    return 0;
}
