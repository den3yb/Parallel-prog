#include <iostream>
#include <vector>
#include <fstream>
#include <ctime>
#include <mpi.h>

using namespace std;

namespace constants {
    string first_file = "C:\\Proganiy\\Parallel-prog\\matrix_in\\first_matrix.txt";
    string second_file = "C:\\Proganiy\\Parallel-prog\\matrix_in\\second_matrix.txt";
    string result_file = "C:\\Proganiy\\Parallel-prog\\matrix_out\\result_matrix.txt";
    string statistic_file = "C:\\Proganiy\\Parallel-prog\\matrix_out\\matrix_statistic.txt";
}

namespace algoritms {
    bool createRandomMatrixFile(int row, int colum, string file_name) {
        srand(time(0));
        ofstream file(file_name);
        for (int i = 0; i < row; ++i) {
            for (int j = 0; j < colum; ++j) {
                file << rand() % 100 << " ";
            }
            file << endl;
        }
        file.close();
        return true;
    }

    vector<vector<int>> readMatrixFromFile(int row, int colum, string file_name) {
        vector<vector<int>> matrix(row, vector<int>(colum));
        ifstream file(file_name);
        for (int i = 0; i < row; ++i)
            for (int j = 0; j < colum; ++j)
                file >> matrix[i][j];
        file.close();
        return matrix;
    }

    void writeMatrixToFile(const vector<vector<int>>& matrix, string filename) {
        ofstream file(filename);
        for (auto& row : matrix) {
            for (auto& val : row)
                file << val << " ";
            file << "\n";
        }
        file.close();
    }
}

void oneCycleMPI(int rowA, int colA, int rowB, int colB) {
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    vector<vector<int>> A, B;
    vector<int> flatA, flatB, flatResult;
    int rowsPerProc, extraRows;

    if (rank == 0) {
        algoritms::createRandomMatrixFile(rowA, colA, constants::first_file);
        algoritms::createRandomMatrixFile(rowB, colB, constants::second_file);
        A = algoritms::readMatrixFromFile(rowA, colA, constants::first_file);
        B = algoritms::readMatrixFromFile(rowB, colB, constants::second_file);

        flatB.resize(rowB * colB);
        for (int i = 0; i < rowB; ++i)
            for (int j = 0; j < colB; ++j)
                flatB[i * colB + j] = B[i][j];
    }

    MPI_Bcast(&colA, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&colB, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&rowB, 1, MPI_INT, 0, MPI_COMM_WORLD);

    flatB.resize(rowB * colB);
    MPI_Bcast(flatB.data(), rowB * colB, MPI_INT, 0, MPI_COMM_WORLD);

    rowsPerProc = rowA / size;
    extraRows = rowA % size;
    vector<int> sendCounts(size), displs(size);

    int offset = 0;
    for (int i = 0; i < size; ++i) {
        sendCounts[i] = (i < extraRows ? rowsPerProc + 1 : rowsPerProc) * colA;
        displs[i] = offset;
        offset += sendCounts[i];
    }

    vector<int> localA(sendCounts[rank]);
    if (rank == 0) {
        flatA.resize(rowA * colA);
        for (int i = 0; i < rowA; ++i)
            for (int j = 0; j < colA; ++j)
                flatA[i * colA + j] = A[i][j];
    }

    MPI_Scatterv(flatA.data(), sendCounts.data(), displs.data(), MPI_INT,
        localA.data(), sendCounts[rank], MPI_INT, 0, MPI_COMM_WORLD);

    int localRows = sendCounts[rank] / colA;
    vector<int> localResult(localRows * colB);

    for (int i = 0; i < localRows; ++i)
        for (int j = 0; j < colB; ++j)
            for (int k = 0; k < colA; ++k)
                localResult[i * colB + j] += localA[i * colA + k] * flatB[k * colB + j];

    vector<int> recvCounts(size), recvDispls(size);
    for (int i = 0; i < size; ++i) {
        int rows = sendCounts[i] / colA;
        recvCounts[i] = rows * colB;
        recvDispls[i] = (i == 0) ? 0 : recvDispls[i - 1] + recvCounts[i - 1];
    }

    if (rank == 0) flatResult.resize(rowA * colB);

    MPI_Gatherv(localResult.data(), recvCounts[rank], MPI_INT,
        flatResult.data(), recvCounts.data(), recvDispls.data(), MPI_INT,
        0, MPI_COMM_WORLD);

    if (rank == 0) {
        vector<vector<int>> result(rowA, vector<int>(colB));
        for (int i = 0; i < rowA; ++i)
            for (int j = 0; j < colB; ++j)
                result[i][j] = flatResult[i * colB + j];

        algoritms::writeMatrixToFile(result, constants::result_file);
    }
}

void getMatrixStatistic(vector<int> cycles) {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        ofstream file(constants::statistic_file);
        for (int size : cycles) {
            double start = MPI_Wtime();
            oneCycleMPI(size, size, size, size);
            double end = MPI_Wtime();
            file << size << " " << (end - start) << endl;
        }
        file.close();
    }
    else {
        for (int size : cycles)
            oneCycleMPI(size, size, size, size);
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    vector<int> count = { 2, 5, 10, 25, 50, 100, 150, 250, 350, 500, 750, 1000 };
    getMatrixStatistic(count);
    oneCycleMPI(2, 3, 3, 3);

    MPI_Finalize();
    return 0;
}
