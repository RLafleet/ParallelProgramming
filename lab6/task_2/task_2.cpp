#include <chrono>
#include <complex>
#include <omp.h>
#include <cmath>

// private - собственная неинициализированная копия
// firstprivate - собственная копия инициализированная до входа в параллельный регион
// lastprivate - собственная копия инициализированная до входа в параллельный регион, при выходе поток записывает значение обратно откуда взяд

int main() {
    int x = 44;
#pragma omp parallel for private(x)
    for (int i = 0; i <= 10; i++) {
        x = i;
        printf("Thread number: %d x: %d\n", omp_get_thread_num(), x);
    }
    printf("x is %d\n", x);
}