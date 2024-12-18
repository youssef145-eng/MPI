#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// Comparison function for qsort
int compare(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

void binary_search(int *local_array, int local_n, int target, int my_rank, int *result) {
    int low = 0, high = local_n - 1;
    *result = -1; // Default to not found
    while (low <= high) {
        int mid = (low + high) / 2;
        if (local_array[mid] == target) {
            *result = my_rank * local_n + mid; // Global index
            return;
        } else if (local_array[mid] < target) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
}

int main(int argc, char *argv[]) {
    int my_rank, comm_sz, n, target, result = -1;
    int *array = NULL; // Declare globally for all processes

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    if (my_rank == 0) {
        // Input array size and elements
        printf("Enter the size of the array: ");
        fflush(stdout); // Ensure prompt appears before input
        scanf("%d", &n);

        if (n % comm_sz != 0) {
            printf("Array size must be divisible by the number of processes.\n");
            fflush(stdout);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        array = malloc(n * sizeof(int)); // Allocate memory for the array
        printf("Enter %d elements of the array:\n", n);
        fflush(stdout); // Ensure prompt appears before input
        for (int i = 0; i < n; i++) {
            scanf("%d", &array[i]);
        }

        // Sort the array
        qsort(array, n, sizeof(int), compare);

        // Input target value
        printf("Enter the target value to search for: ");
        fflush(stdout); // Ensure prompt appears before input
        scanf("%d", &target);
    }

    // Broadcast the size and target to all processes
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&target, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Allocate memory for the array on other processes
    if (my_rank != 0) {
        array = malloc(n * sizeof(int));
    }

    // Broadcast the array to all processes
    MPI_Bcast(array, n, MPI_INT, 0, MPI_COMM_WORLD);

    // Compute the local array size
    int local_n = n / comm_sz;
    int *local_array = malloc(local_n * sizeof(int));

    // Scatter the array to all processes
    MPI_Scatter(array, local_n, MPI_INT, local_array, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    // Perform local binary search
    binary_search(local_array, local_n, target, my_rank, &result);

    // Collect results at rank 0
    int global_result = -1;
    MPI_Reduce(&result, &global_result, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

    if (my_rank == 0) {
        if (global_result != -1) {
            printf("Target found at index: %d\n", global_result);
        } else {
            printf("Target not found.\n");
        }
    }

    // Free allocated memory
    free(array);
    free(local_array);

    MPI_Finalize();
    return 0;
}
-
