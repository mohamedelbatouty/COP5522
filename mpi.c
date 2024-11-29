#include <microtime.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>


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

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    double time1 =0;

    int procs, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int* loaded_array = NULL;
    int loaded_length = 0;

    if (rank == 0) {
        loaded_array = load_test_data("testdata.txt", &loaded_length);
    }

    MPI_Bcast(&loaded_length, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int* lArray = (int*)malloc(loaded_length * sizeof(int));
    int local_size = loaded_length / procs;
    int remainder = loaded_length % procs;

    int* sCounts = (int*)malloc(procs * sizeof(int));
    int* displacement = (int*)malloc(procs * sizeof(int));

    for (int i = 0; i < procs; i++) {
        sCounts[i] = local_size + (i < remainder ? 1 : 0);
        displacement[i] = (i > 0) ? (displacement[i-1] + sCounts[i-1]) : 0;
    }

    MPI_Scatterv(loaded_array, sCounts, displacement, MPI_INT, lArray, sCounts[rank], MPI_INT, 0, MPI_COMM_WORLD);
    if (rank == 0) {
        time1 = microtime();
    }
    mergeSort(lArray, 0, sCounts[rank] - 1);

    int* gArray = NULL;
    if (rank == 0) {
        gArray = (int*)malloc(loaded_length * sizeof(int));
    }

    MPI_Gatherv(lArray, sCounts[rank], MPI_INT, gArray, sCounts, displacement, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        mergeSort(gArray, 0, loaded_length - 1);
        double time2 = microtime();

        for (int i = 0; i < loaded_length - 1; i++) {
            if (gArray[i] > gArray[i+1]) {
                printf("Test data arrays not sorted: %i > %i\n",
                       gArray[i], gArray[i+1]);
                break;
            }
        }

        double t = time2 - time1;
        printf("\nTime = %g us\n", t);
        free(gArray);
        free(loaded_array);
    }

    free(lArray);
    free(sCounts);
    free(displacement);

    MPI_Finalize();

    return 0;
}