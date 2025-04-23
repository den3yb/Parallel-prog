#include <iostream>
#include <vector>
#include <fstream>
#include <ctime>
#include <omp.h>

using namespace std;


namespace constants {
    string first_file = "matrix_in\\first_matrix.txt";
    string second_file = "matrix_in\\second_matrix.txt";
    string result_file = "matrix_out\\result_matrix.txt";
    string statistic_file = "matrix_out\\matrix_statistic.txt";
}

namespace algoritms {

    bool createRandomMatrixFile(int row, int colum, string file_name) {
        // Функция создаёт файл с матрицой рандомного содержания, по заданным размерам
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
        // Функция считывает из файла матрицу рандомного содержания, по заданным размерам
        std::vector<std::vector<int>> matrix(row, std::vector<int>(colum));
        std::ifstream file(file_name);
        int number;

        for (int i = 0; i < row; ++i) {
            for (int j = 0; j < colum; ++j) {
                if (file >> number) {
                    matrix[i][j] = number;
                } else {
                    matrix[i][j] = 0;
                }
            }
        }

        file.close();
        return matrix;
    }

    vector<vector<int>> multiplySaveMatrixes(vector<vector<int>>& matrix1, vector<vector<int>>& matrix2) {
        // выполняет произведение матриц и записывает результат в файл
        if (matrix1[0].size() != matrix2.size()) {
            cerr << "Матрицы не могут быть умноженны" << endl;
            return {};
        }

        int row = matrix1.size();
        int colum = matrix2[0].size();

        vector<vector<int>> result(row, vector<int>(colum, 0));

        // Распараллеливаем внешний цикл по строкам результирующей матрицы
        omp_set_num_threads(2);
        #pragma omp parallel for schedule(auto) 
        for (int i = 0; i < row; ++i) {
            for (int j = 0; j < colum; ++j) {
                for (int k = 0; k < matrix1[0].size(); ++k) {
                    result[i][j] += matrix1[i][k] * matrix2[k][j];
                }
            }
        }

        ofstream file(constants::result_file);
        for (int i = 0; i < row; ++i) {
            for (int j = 0; j < colum; ++j) {
                file << result[i][j] << " ";
            }
            file << endl;
        }
        file.close();

        return result;
    }

    void printMatrix(const vector<vector<int>>& matrix) {
        // функция вывода матрицы в консоль
        for (const auto& row : matrix) {
            for (int val : row) {
                cout << val << " ";
            }
            cout << endl;
        }
    }

}

using namespace algoritms;

void oneCycle(int row_1, int colum_1, int row_2, int colum_2){
    //Функция делает прогон программы один раз, создавая рандомные значения для обеих матриц
    // и перемножая их


    createRandomMatrixFile(row_1, colum_1, constants::first_file);
    vector<vector<int>> matrix_1 = readMatrixFromFile(row_1, colum_1, constants::first_file);

    createRandomMatrixFile(row_2, colum_2, constants::second_file);
    vector<vector<int>> matrix_2 = readMatrixFromFile(row_2, colum_2, constants::second_file);

    vector<vector<int>> result = multiplySaveMatrixes(matrix_1, matrix_2);
}

void getMatrixStatistic(vector<int> cycyles){
    //функция замеряет время выполнения в зависимости от количества эллементов и записывает в файл
    //размеры пердаются в векторе
    ofstream file(constants::statistic_file);
    for (size_t i = 0; i < cycyles.size(); ++i) { 
        clock_t start = clock();
        oneCycle(cycyles[i], cycyles[i], cycyles[i], cycyles[i]);
        clock_t end = clock();
        double duration = double(end - start) / CLOCKS_PER_SEC;
        int count = cycyles[i];
        file << count << " " << duration << endl;
    }
    file.close();
}

int main() {
    vector<int> count = {2, 5, 10, 25, 50, 100, 150, 250, 350, 500, 750, 1000};
    getMatrixStatistic(count);
    oneCycle(2,3,3,3);
    return 0;
}
