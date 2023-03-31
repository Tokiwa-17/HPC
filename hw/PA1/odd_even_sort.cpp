#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <mpi.h>

#include "worker.h"



void merge(float *arr_a, int len_a, float *arr_b, int len_b, float *merged, bool is_left, int iter, int rank) {
#ifdef DEBUG
if (iter == 1 && rank == 2) {
    printf("\n-------------------------------------\n");
    for (int i = 0; i < len_a; i++)
      printf("%f ", arr_a[i]);
    printf("\n");
    for (int i = 0; i < len_b; i++)
      printf("%f ", arr_b[i]);
    printf("\n");
    printf("-------------------------------------\n\n");
}

#endif // DEBUG
  int p = 0, q = 0, l, r;
  if (is_left) {
    l = 0, r = len_a;
    for (; l < r; l++) {
      if (p == len_a) merged[l] = arr_b[q++];
      else if (q == len_b) merged[l] = arr_a[p++];
      else merged[l] = arr_a[p] < arr_b[q] ? arr_a[p++] : arr_b[q++];
    }
  } else { // 主线程位于merged右侧
      l = len_b - 1, r = len_a + len_b - 1, p = len_a - 1, q = len_b - 1;
      int cnt = len_a - 1;
      while(cnt >= 0) {
          if (p < 0) merged[cnt--] = arr_b[q--];
          else if (q < 0) merged[cnt--] = arr_a[p--];
          else merged[cnt--] = arr_a[p] < arr_b[q] ? arr_b[q--] : arr_a[p--];
      }
  }
}

void Worker::sort() {
  /** Your code ... */
  // you can use variables in class Worker: n, nprocs, rank, block_len, data
  if (out_of_range) return;
  std::sort(data, data + block_len);
  int iter = 0, offset = 0;
  size_t block_size = ceiling(n, nprocs);
  float *recvData = new float[block_size];
  float *merged = new float[block_size];
  MPI_Status status[2];
  MPI_Request request[2];
  while(iter < nprocs) {
    bool master_rank = (iter % 2) == (rank % 2);
    int neighbor_rank = master_rank ? rank + 1 : rank - 1;
    if (neighbor_rank < 0 || (last_rank && neighbor_rank == rank + 1)) {
      //printf("\nTTTTiter: %d rank: %d neighbor_rank: %d\n", iter, rank, neighbor_rank);
      iter++;
      continue;
    }

    MPI_Isend(data, block_len, MPI_FLOAT, neighbor_rank, iter, MPI_COMM_WORLD, &request[0]);
    MPI_Irecv(recvData, block_size, MPI_FLOAT, neighbor_rank, iter, MPI_COMM_WORLD, &request[1]);
    MPI_Wait(&request[1], &status[1]);
    int recvSize;
    MPI_Get_count(&status[1], MPI_FLOAT, &recvSize);
    // if (data[block_len - 1] <= recvData[0]) break;
    offset = (neighbor_rank < rank) ? recvSize : block_len;
    merge(data, block_len, recvData, recvSize, merged, master_rank, iter, rank);
    float *tmp = data;
    data = merged, merged = tmp;
#ifdef DEBUG
    printf("\n-------------------------------------\n");
    printf("iter: %d offset: %d\n", iter, offset);
    printf("recvData: \n");
    for (int i = 0; i < recvSize; i++) {
      printf("%f ", recvData[i]);
    }
    // printf("\n");
    // printf("merged: \n");
    // for (int i = 0; i < block_len; i++) {
    //   printf("%f ", merged[i]);
    // }
    printf("\n");
    this -> output();
    printf("-------------------------------------\n\n");
#endif // DEBUG
    MPI_Wait(&request[0], &status[0]);
    iter++;
  }
  delete[] recvData;
  delete[] merged;
}
