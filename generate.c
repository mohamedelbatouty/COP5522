#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int* gen_arr(int length) {
    srand(time(NULL));
    int* random_array = (int*)malloc(length * sizeof(int));

    for (int i = 0; i < length; i++) {
        random_array[i] = rand();
    }

    return random_array;
}

int save(int* array, int length, const char* filename) {
    FILE* file = fopen(filename, "w");

    if (file == NULL) {
        printf("Error opening file %s", filename);
        return -1;
    }

    fprintf(file, "%d\n", length);

    for (int i = 0; i < length; i++) {
        fprintf(file, "%d\n", array[i]);
    }

    fclose(file);

    return 0;
}

int main(int argc, char *argv[]) {
    int length = 0;
    
    if (argc != 2) {
        fprintf(stderr, "USAGE: %s rand_count\n", argv[0]);
        exit(1);
    }

    length = atoi(argv[1]);
    int* random_array = gen_arr(length);

    if (save(random_array, length, "testdata.txt") != 0) {
        fprintf(stderr, "Failed to save array to file\n");
        free(random_array);
        return 1;
    }

    printf("Saved %d random numbers to %s\n", length, "testdata.txt");
    free(random_array);
    return 0;
}