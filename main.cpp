#include <iostream>
#include <vector>
#include <fstream>
#include <ctime>
#include <mpi.h>
#include <omp.h>
#include <windows.h>

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
            for (int j = 0; j < colum; ++j) { file << rand() << " "; }
            file << endl;
        }
        file.close();
        return true;
    }

    std::vector<std::vector<int>> readMatrixFromFile(int row, int colum, string file_name) {
        std::vector<std::vector<int>> matrix(row, std::vector<int>(colum));
        std::ifstream file(file_name);
        int number;

        for (int i = 0; i < row; ++i) {
            for (int j = 0; j < colum; ++j) {
                if (file >> number) {
                    matrix[i][j] = number;
                }
                else {
                    matrix[i][j] = 0;
                }
            }
        }

        file.close();
        return matrix;
    }

    vector<vector<int>> multiplySaveMatrixes(vector<vector<int>>& matrix1, vector<vector<int>>& matrix2) {
        int rank, size;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);

        if (matrix1[0].size() != matrix2.size()) {
            if (rank == 0) cerr << "Matrices cannot be multiplied due to incompatible dimensions." << endl;
            return {};
        }

        int rows = matrix1.size();
        int cols = matrix2[0].size();
        int common_dim = matrix1[0].size();

        vector<vector<int>> result(rows, vector<int>(cols, 0));

        int chunk_size = rows / size;
        int start_row = rank * chunk_size;
        int end_row = (rank == size - 1) ? rows : start_row + chunk_size;

        if (rank == 0) {
            cout << "Starting matrix multiplication..." << endl;
        }

        // Local multiplication
        for (int i = start_row; i < end_row; ++i) {
            for (int j = 0; j < cols; ++j) {
                int sum = 0;
                for (int k = 0; k < common_dim; ++k) {
                    sum += matrix1[i][k] * matrix2[k][j];
                }
                result[i][j] = sum;
            }
        }

        // Gathering results at process 0
        if (rank == 0) {
            MPI_Status status;
            for (int p = 1; p < size; ++p) {
                int p_start = p * chunk_size;
                int p_end = (p == size - 1) ? rows : p_start + chunk_size;
                for (int i = p_start; i < p_end; ++i) {
                    MPI_Recv(result[i].data(), cols, MPI_INT, p, 0, MPI_COMM_WORLD, &status);
                }
            }
            cout << "Multiplication complete. Saving result..." << endl;
        }
        else {
            for (int i = start_row; i < end_row; ++i) {
                MPI_Send(result[i].data(), cols, MPI_INT, 0, 0, MPI_COMM_WORLD);
            }
        }

        // Save result from process 0
        if (rank == 0) {
            ofstream file(constants::result_file);
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    file << result[i][j] << " ";
                }
                file << endl;
            }
            file.close();
            cout << "Result saved to file." << endl;
        }

        return result;
    }

    void printMatrix(const vector<vector<int>>& matrix) {
        for (const auto& row : matrix) {
            for (int val : row) {
                cout << val << " ";
            }
            cout << endl;
        }
    }

}

using namespace algoritms;

void oneCycle(int row_1, int colum_1, int row_2, int colum_2) {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    vector<vector<int>> matrix_1, matrix_2;

    if (rank == 0) {
        cout << "Generating random matrices..." << endl;
        createRandomMatrixFile(row_1, colum_1, constants::first_file);
        matrix_1 = readMatrixFromFile(row_1, colum_1, constants::first_file);

        createRandomMatrixFile(row_2, colum_2, constants::second_file);
        matrix_2 = readMatrixFromFile(row_2, colum_2, constants::second_file);
        cout << "Matrices loaded." << endl;
    }

    // Broadcast dimensions
    int sizes[4] = { row_1, colum_1, row_2, colum_2 };
    MPI_Bcast(sizes, 4, MPI_INT, 0, MPI_COMM_WORLD);

    row_1 = sizes[0]; colum_1 = sizes[1];
    row_2 = sizes[2]; colum_2 = sizes[3];

    if (colum_1 != row_2) {
        if (rank == 0) cerr << "Error: Incompatible matrix sizes" << endl;
        return;
    }

    // Broadcast matrix 1
    if (rank != 0) matrix_1.resize(row_1, vector<int>(colum_1));
    for (int i = 0; i < row_1; ++i) {
        MPI_Bcast(matrix_1[i].data(), colum_1, MPI_INT, 0, MPI_COMM_WORLD);
    }

    // Broadcast matrix 2
    if (rank != 0) matrix_2.resize(row_2, vector<int>(colum_2));
    for (int i = 0; i < row_2; ++i) {
        MPI_Bcast(matrix_2[i].data(), colum_2, MPI_INT, 0, MPI_COMM_WORLD);
    }

    multiplySaveMatrixes(matrix_1, matrix_2);
}

void getMatrixStatistic(vector<int> cycles) {
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    ofstream file;
    if (rank == 0) {
        file.open(constants::statistic_file);
        cout << "Starting statistics collection..." << endl;
    }

    for (size_t i = 0; i < cycles.size(); ++i) {
        if (i % size != rank) continue;

        int current = cycles[i];
        cout << "Process " << rank << ": processing matrix size " << current << endl;

        clock_t start = clock();
        oneCycle(current, current, current, current);
        clock_t end = clock();

        double duration = double(end - start) / CLOCKS_PER_SEC;

        if (rank != 0) {
            MPI_Send(&duration, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        }
        else {
            file << current << " " << duration << endl;
            for (int p = 1; p < size; ++p) {
                if (i + p >= cycles.size()) break;
                double recv_dur;
                MPI_Recv(&recv_dur, 1, MPI_DOUBLE, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                file << cycles[i + p] << " " << recv_dur << endl;
            }
        }
    }

    if (rank == 0) {
        file.close();
        cout << "Statistics saved." << endl;
    }
}

int main(int argc, char** argv) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    setlocale(LC_ALL, "Russian"); // Optional, can remove or change to "en_US.utf8" if needed
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    vector<int> count = { 2, 5, 10, 25, 50, 100, 150, 250, 350, 500, 750, 1000 };

    getMatrixStatistic(count);


    MPI_Finalize();
    return 0;
}
