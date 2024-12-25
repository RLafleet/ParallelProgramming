#include <chrono>
#include <iostream>
#include <vector>

void ReadSquareMatrix(std::istream& input, std::vector<std::vector<int> >& matrix, const unsigned size) {
    matrix.resize(size, std::vector<int>(size));
    for (int i = 0; static_cast<unsigned>(i) < size; ++i) {
        for (int j = 0; static_cast<unsigned>(j) < size; ++j) {
            input >> matrix[i][j];
        }
    }
}

void WriteSquareMatrix(std::ostream& output, const std::vector<std::vector<int> >& matrix) {
    const auto size = matrix.size();
    for (size_t i = 0; static_cast<unsigned>(i) < size; ++i) {
        for (size_t j = 0; static_cast<unsigned>(j) < size; ++j) {
            output << matrix[i][j] << " ";
        }
        output << std::endl;
    }
}

std::vector<std::vector<int> > MultiplyMatrices(
    const std::vector<std::vector<int> >& firstMatrix,
    const std::vector<std::vector<int> >& secondMatrix
) {
    const auto size = firstMatrix.size();
    std::vector resultMatrix(size, std::vector(size, 0));

#pragma omp parallel for
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            for (int k = 0; k < size; ++k) {
                resultMatrix[i][j] += firstMatrix[i][k] * secondMatrix[k][j];
            }
        }
    }

    return resultMatrix;
}

template<typename Func>
void measureTime(Func&& func) {
    const auto start = std::chrono::high_resolution_clock::now();
    std::forward<Func>(func)();
    const auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    std::cout << "Time taken: " << duration.count() << " seconds" << std::endl;
}

std::vector<std::vector<int>> generateRandomMatrix(const unsigned size, const int minValue, const int maxValue) {
    std::vector matrix(size, std::vector<int>(size));

    for (int i = 0; static_cast<unsigned>(i) < size; ++i) {
        for (int j = 0; static_cast<unsigned>(j) < size; ++j) {
            matrix[i][j] = minValue + rand() % (maxValue - minValue + 1);
        }
    }

    return matrix;
}

int main() {
    unsigned size;

    std::cout << "Enter the size of the matrices: ";
    std::cin >> size;
    std::vector<std::vector<int> > firstMatrix, secondMatrix;

    if (false) {
        std::cout << "Enter elements of first matrix:" << std::endl;
        ReadSquareMatrix(std::cin, firstMatrix, size);

        std::cout << "Enter elements of second matrix:" << std::endl;
        ReadSquareMatrix(std::cin, secondMatrix, size);
    }
    if (true) {
        firstMatrix = generateRandomMatrix(size, -100, 100);
        secondMatrix = generateRandomMatrix(size, -100, 100);
    }

    measureTime([&]() {
        const std::vector<std::vector<int> > resultMatrix = MultiplyMatrices(firstMatrix, secondMatrix);
        std::cout << "Resulting matrix:" << std::endl;
        // WriteSquareMatrix(std::cout, resultMatrix);
        });


    return 0;
}

/* Test cases
## FirstCase
first:
4 2
9 0

second:
3 1
-3 4

result:
6 12
27 9


## SecondCase
first:
1 4 3
2 1 5
3 2 1

second:
1 0 0
0 1 0
0 0 1

result:
1 4 3
2 1 5
3 2 1

## ThirdCase
first:
1 4 3
2 1 5
3 2 1

second:
5 2 1
4 3 2
2 1 5

result:
27 17 24
24 12 29
25 13 12
*/