#include <iostream>
#include <mpi.h>

double f(double x) {
    return x * x;
}

double Trap(double local_a, double local_b, int local_n, double h) {
    double estimate = 0.0, x;
    int i;
    estimate = (f(local_a) + f(local_b)) / 2.0;
    for (i = 1; i <= local_n - 1; i++) {
        x = local_a + i * h;
        estimate += f(x);
    }
    estimate = estimate * h;
    return estimate;
}

int main() {
    int my_rank, comm_size, n = 1024, local_n;
    double a = 0.0, b = 3.0, h, local_a, local_b;
    double local_integral = 0.0, total_integral = 0.0;
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);    
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);  
    // partition tasks
    h = (b - a) / n;
    local_n = n / comm_size;
    local_a = a + my_rank * local_n * h;
    local_b = local_a + local_n * h;
    // implement corresponding tasks
    local_integral = Trap(local_a, local_b, local_n, h);
    // reduce to rank-0
    if (my_rank != 0) {
        MPI_Send(&local_integral, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    } else {
        total_integral = local_integral;
        for (int i = 1; i < comm_size; i++) {
            MPI_Recv(&local_integral, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            total_integral += local_integral;
        }
    }
    // output result
    if (my_rank == 0) {
        printf("integral of trapezoidal is %lf.\n", total_integral);
    }
    MPI_Finalize(); 
    return 0;
}