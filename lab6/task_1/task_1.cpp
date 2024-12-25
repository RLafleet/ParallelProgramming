#include <chrono>
#include <complex>
#include <iostream>
#include <cmath>

double calculatePiSync(const unsigned n) {
    double result = 0;
    for (unsigned i = 0; i < n; ++i) {
        result += std::pow(-1, i) / static_cast<double>(2 * i + 1);
    }
    return result * 4;
}

double calculatePiParallel(const unsigned n) {
    double result = 0;
#pragma omp parallel for
    for (unsigned i = 0; i < n; ++i) {
        result += std::pow(-1, i) / static_cast<double>(2 * i + 1);
    }
    return result * 4;
}

double calculatePiParallelAtomic(const unsigned n) {
    double result = 0;
#pragma omp parallel for
    for (unsigned i = 0; i < n; ++i) {
        double term = std::pow(-1, i) / static_cast<double>(2 * i + 1);
#pragma omp atomic
        result += term;
    }
    return result * 4;
}

double calculatePiReduction(const unsigned n) {
    double result = 0;
#pragma omp parallel for reduction(+:result)
    for (unsigned i = 0; i < n; ++i) {
        result += std::pow(-1, i) / static_cast<double>(2 * i + 1);
    }
    return result * 4;
}

template<typename Func>
auto measureTime(Func&& func) {
    const auto start = std::chrono::high_resolution_clock::now();
    auto result = std::forward<Func>(func)();
    const auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    std::cout << "Time taken: " << duration.count() << " seconds" << std::endl;
    return result;
}

int main() {
    unsigned n = 10000000;

    std::cout << "Synchronous result: "
        << measureTime([&]() { return calculatePiSync(n); }) << std::endl;
    std::cout << "Parallel result (incorrect): "
        << measureTime([&]() { return calculatePiParallel(n); }) << std::endl;
    std::cout << "Parallel result (atomic): "
        << measureTime([&]() { return calculatePiParallelAtomic(n); }) << std::endl;
    std::cout << "Parallel result (reduction): "
        << measureTime([&]() { return calculatePiReduction(n); }) << std::endl;

    return 0;
}