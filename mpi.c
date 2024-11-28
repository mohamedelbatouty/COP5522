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
    double start_time;
    
    int procs, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int* loaded_array = NULL;
    int loaded_length = 0;

    if (rank == 0) {
        loaded_array = load_test_data("testdata.txt", &loaded_length);
    }

    MPI_Bcast(&loaded_length, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int* local_array = (int*)malloc(loaded_length * sizeof(int));
    int local_size = loaded_length / procs;
    int remainder = loaded_length % procs;

    int* sendcounts = (int*)malloc(procs * sizeof(int));
    int* displacement = (int*)malloc(procs * sizeof(int));

    for (int i = 0; i < procs; i++) {
        sendcounts[i] = local_size + (i < remainder ? 1 : 0);
        displacement[i] = (i > 0) ? (displacement[i-1] + sendcounts[i-1]) : 0;
    }

    MPI_Scatterv(
        loaded_array,
        sendcounts,
        displacement,
        MPI_INT,
        local_array,
        sendcounts[rank],
        MPI_INT,
        0,
        MPI_COMM_WORLD
    );
    if (rank==0){
    start_time = microtime();
    }
    mergeSort(local_array, 0, sendcounts[rank] - 1);

    int* gathered_array = NULL;
    if (rank == 0) {
        gathered_array = (int*)malloc(loaded_length * sizeof(int));
    }

    MPI_Gatherv(
        local_array,        // send buffer
        sendcounts[rank],   // send count
        MPI_INT,            // send type
        gathered_array,     // receive buffer
        sendcounts,         // receive counts
        displacement,             // displacements
        MPI_INT,            // receive type
        0,                  // root
        MPI_COMM_WORLD      // communicator
    );

    if (rank == 0) {

        mergeSort(gathered_array, 0, loaded_length - 1);
        double end_time = microtime();

        for (int i = 0; i < loaded_length - 1; i++) {
            if (gathered_array[i] > gathered_array[i+1]) {
                printf("Test data arrays not sorted: %i > %i\n",
                       gathered_array[i], gathered_array[i+1]);
                break;
            }
        }

        printf("\nExecution Time = %g seconds\n", end_time - start_time);

        free(gathered_array);
        free(loaded_array);
    }

    free(local_array);
    free(sendcounts);
    free(displacement);

    MPI_Finalize();

    return 0;
}