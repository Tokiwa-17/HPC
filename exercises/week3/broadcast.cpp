#include <iostream>
#include <mpi.h>

int main() {
    int my_rank, comm_size;
    int value;
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);    
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    do {
        if (my_rank == 0) {
            std::cin >> value; 
        }
        MPI_Bcast(&value, 1, MPI_INT, 0, MPI_COMM_WORLD);
        std::cout << "rank: " << my_rank << " " << "value: " << value << std::endl;
    } while(value != 0);
    MPI_Finalize(); 
    return 0;
}