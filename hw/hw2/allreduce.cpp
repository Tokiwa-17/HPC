#include <chrono>
#include <iostream>
#include <mpi.h>
#include <time.h>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <tuple>
#include <vector>
#include <utility>
#define EPS 1e-8

namespace ch = std::chrono;

std::tuple<int, int> Src_Dest_Rank(int my_rank, int comm_sz) {
    /**
     \brief Compute the index of ranks that the current rank would send to and receive to.
     */
    int srcRank = (my_rank - 1 + comm_sz) % comm_sz;
    int destRank = (my_rank + 1) % comm_sz;
    return std::make_tuple(srcRank, destRank);
}

void printSendBuf(void* sendbuf, int n) {
    std::cout << "----------------------------------------" << std::endl;
    for (int i = 0; i < n; i++) {
        std::cout << *static_cast<float*>(sendbuf + i * 4) << ' ';
    }
    std::cout << std::endl;
    std::cout << "----------------------------------------" << std::endl;
}

std::vector<std::pair<int, int>>* splitArray(int n, int comm_sz) {
    /**
    \brief an array whose length is l is divided into n splits, the size of l % n splits would
    be l // n + 1, and the rest is l // n
    */
    std::vector<std::pair<int, int>>* split = new std::vector<std::pair<int, int>>(comm_sz, std::make_pair(0, 0));
    int splitNum = comm_sz;
    int curPos = 0;
    for (int i = 0; i < comm_sz; i++) {
        int chunkSize;
        if (i < n % splitNum)
            chunkSize = n / splitNum + 1;
        else chunkSize = n / splitNum;
        (*split)[i].first = curPos;
        (*split)[i].second = (*split)[i].first + chunkSize;
        curPos = (*split)[i].second;
    }
    return split;
}

void Ring_Allreduce(void* sendbuf, void* recvbuf, int n, MPI_Comm comm, int comm_sz, int my_rank)
{
   /**
    \brief Ring algorithm, which could be divided into Reduce-scatter and allgather
    */
    int blockNum = comm_sz;
    MPI_Request sendReq, recvReq;
    auto ranks = Src_Dest_Rank(my_rank, comm_sz);
    // std::cout << "ttttt" << std::endl;
    auto split = splitArray(n, comm_sz);
    // #ifdef DEBUG
    // if (my_rank == 0) {
    //     auto split_value = *split;
    //     for (auto it: split_value)  {
    //         std::cout << it.first << ' ' << it.second << std::endl;
    //     }
    // }
    // #endif // DEBUG
    int srcRank = std::get<0>(ranks), destRank = std::get<1>(ranks);
    // Step 1: Reduce-scatter
    for (int k = 0; k < comm_sz - 1; k++) {
        int blockIdx = (my_rank - k + comm_sz) % comm_sz;
        int recvBlockIdx = (srcRank - k + comm_sz) % comm_sz;
        //std::cout << my_rank << ' ' << blockIdx << ' ' << recvBlockIdx << std::endl;
        auto interval = (*split)[blockIdx];
        auto blockSize = (interval.second - interval.first);
        MPI_Isend(sendbuf + interval.first * 4, blockSize, MPI_FLOAT, destRank, 0, MPI_COMM_WORLD, &sendReq);
        auto recvInterval = (*split)[recvBlockIdx];
        blockSize = (recvInterval.second - recvInterval.first);
        MPI_Irecv(recvbuf + recvInterval.first * 4, blockSize, MPI_FLOAT, srcRank, 0, MPI_COMM_WORLD, &recvReq);
        MPI_Wait(&sendReq, nullptr);    
        MPI_Wait(&recvReq, nullptr);
        int l = recvInterval.first * 4;
        int r = recvInterval.second * 4;
        while(l < r) {
            *static_cast<float*>(sendbuf + l) += *static_cast<float*>(recvbuf + l);
            l += 4;
        }
    }
    // Step 2: Allgather
    for (int k = 0; k < comm_sz - 1; k++) {
        int blockIdx = (my_rank + 1 - k + comm_sz) % comm_sz;
        int recvBlockIdx = (srcRank + 1 - k + comm_sz) % comm_sz;
        auto interval = (*split)[blockIdx];
        auto blockSize = (interval.second - interval.first);
        MPI_Isend(sendbuf + interval.first * 4, blockSize, MPI_FLOAT, destRank, 0, MPI_COMM_WORLD, &sendReq);
        auto recvInterval = (*split)[recvBlockIdx];
        blockSize = (recvInterval.second - recvInterval.first); 
        MPI_Irecv(recvbuf + recvInterval.first * 4, blockSize, MPI_FLOAT, srcRank, 0, MPI_COMM_WORLD, &recvReq);
        MPI_Wait(&sendReq, nullptr);
        MPI_Wait(&recvReq, nullptr);
        int l = recvInterval.first * 4;
        int r = recvInterval.second * 4;
        while (l < r) {
            *static_cast<float*>(sendbuf + l) = *static_cast<float*>(recvbuf + l);
            l += 4;
        }
    }
    // Finally, Update recvbuf
    if (my_rank == 0) {
        for (int i = 0; i < n; i++) {
            *static_cast<float*>(recvbuf + i * 4) = *static_cast<float*>(sendbuf + i * 4);
        }
    }
}


// reduce + bcast
void Naive_Allreduce(void* sendbuf, void* recvbuf, int n, MPI_Comm comm, int comm_sz, int my_rank)
{
    MPI_Reduce(sendbuf, recvbuf, n, MPI_FLOAT, MPI_SUM, 0, comm);
    MPI_Bcast(recvbuf, n, MPI_FLOAT, 0, comm);
}

int main(int argc, char *argv[])
{
    int ITER = atoi(argv[1]); // 10
    int n = atoi(argv[2]);    // 100000000
    float* mpi_sendbuf = new float[n];
    float* mpi_recvbuf = new float[n];
    float* naive_sendbuf = new float[n];
    float* naive_recvbuf = new float[n];
    float* ring_sendbuf = new float[n];
    float* ring_recvbuf = new float[n];

    MPI_Init(nullptr, nullptr);
    int comm_sz;
    int my_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    
    srand(time(NULL) + my_rank);
    for (int i = 0; i < n; ++i)
        mpi_sendbuf[i] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    memcpy(naive_sendbuf, mpi_sendbuf, n * sizeof(float));
    memcpy(ring_sendbuf, mpi_sendbuf, n * sizeof(float));

    //warmup and check
    MPI_Allreduce(mpi_sendbuf, mpi_recvbuf, n, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
    Naive_Allreduce(naive_sendbuf, naive_recvbuf, n, MPI_COMM_WORLD, comm_sz, my_rank);
    Ring_Allreduce(ring_sendbuf, ring_recvbuf, n, MPI_COMM_WORLD, comm_sz, my_rank);
    bool correct = true;
    for (int i = 0; i < n; ++i)
        if (abs(mpi_recvbuf[i] - ring_recvbuf[i]) > EPS)
        {
            correct = false;
            break;
        }
    #ifdef DEBUG
    if (my_rank == 0) {
        std::cout << "correct." << std::endl;
    }
    #endif // DEBUG
    if (correct)
    {
        auto beg = ch::high_resolution_clock::now();
        for (int iter = 0; iter < ITER; ++iter)
            MPI_Allreduce(mpi_sendbuf, mpi_recvbuf, n, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
        auto end = ch::high_resolution_clock::now();
        double mpi_dur = ch::duration_cast<ch::duration<double>>(end - beg).count() * 1000; //ms

        beg = ch::high_resolution_clock::now();
        for (int iter = 0; iter < ITER; ++iter)
            Naive_Allreduce(naive_sendbuf, naive_recvbuf, n, MPI_COMM_WORLD, comm_sz, my_rank);
        end = ch::high_resolution_clock::now();
        double naive_dur = ch::duration_cast<ch::duration<double>>(end - beg).count() * 1000; //ms

        beg = ch::high_resolution_clock::now();
        for (int iter = 0; iter < ITER; ++iter)
            Ring_Allreduce(ring_sendbuf, ring_recvbuf, n, MPI_COMM_WORLD, comm_sz, my_rank);
        end = ch::high_resolution_clock::now();
        double ring_dur = ch::duration_cast<ch::duration<double>>(end - beg).count() * 1000; //ms
        
        if (my_rank == 0)
        {
            std::cout << "Correct." << std::endl;
            std::cout << "MPI_Allreduce:   " << mpi_dur << " ms." << std::endl;
            std::cout << "Naive_Allreduce: " << naive_dur << " ms." << std::endl;
            std::cout << "Ring_Allreduce:  " << ring_dur << " ms." << std::endl;
        }
    }
    else
        if (my_rank == 0)
            std::cout << "Wrong!" << std::endl;

    delete[] mpi_sendbuf;
    delete[] mpi_recvbuf;
    delete[] naive_sendbuf;
    delete[] naive_recvbuf;
    delete[] ring_sendbuf;
    delete[] ring_recvbuf;
    MPI_Finalize();
    return 0;
}
