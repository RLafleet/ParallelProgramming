#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>
#include <Windows.h>

// Структура для хранения заголовка BMP файла
#pragma pack(push, 1)
struct BMPHeader {
    uint16_t file_type{ 0x4D42 };          // File type, always 4D42h ("BM")
    uint32_t file_size{ 0 };               // Size of the file (in bytes)
    uint16_t reserved1{ 0 };               // Reserved, always 0
    uint16_t reserved2{ 0 };               // Reserved, always 0
    uint32_t offset_data{ 0 };             // Start position of pixel data (bytes from the beginning of the file)
    uint32_t size{ 0 };                    // Size of this header (in bytes)
    int32_t width{ 0 };                    // Width of the bitmap in pixels
    int32_t height{ 0 };                   // Height of the bitmap in pixels
    uint16_t planes{ 1 };                  // Number of color planes (must be 1)
    uint16_t bit_count{ 0 };               // Number of bits per pixel (24 for RGB)
    uint32_t compression{ 0 };             // Compression type (0 for uncompressed)
    uint32_t size_image{ 0 };              // Image size (in bytes)
    int32_t x_pixels_per_meter{ 0 };
    int32_t y_pixels_per_meter{ 0 };
    uint32_t colors_used{ 0 };             // Number of colors in the color palette (0 means no palette)
    uint32_t colors_important{ 0 };        // Number of important colors used (0 means all colors are important)
};
#pragma pack(pop)

// Функция для размытия одного пикселя (бокс блюр)
uint8_t blur_pixel(const std::vector<uint8_t>& data, int width, int height, int x, int y, int stride) {
    int blur_size = 1;  // Размер размытия
    int r_sum = 0, g_sum = 0, b_sum = 0;
    int count = 0;

    for (int i = -blur_size; i <= blur_size; ++i) {
        for (int j = -blur_size; j <= blur_size; ++j) {
            int nx = x + j;
            int ny = y + i;
            if (nx >= 0 && ny >= 0 && nx < width && ny < height) {
                int pixel_index = (ny * stride) + nx * 3;
                r_sum += data[pixel_index];
                g_sum += data[pixel_index + 1];
                b_sum += data[pixel_index + 2];
                ++count;
            }
        }
    }

    return static_cast<uint8_t>(r_sum / count); // Возвращаем среднее значение для размытого пикселя
}

// Функция для обработки горизонтальной полосы
void process_strip(int start_row, int end_row, int width, int height, int stride, std::vector<uint8_t>& data, std::vector<uint8_t>& output) {
    for (int y = start_row; y < end_row; ++y) {
        for (int x = 0; x < width; ++x) {
            int pixel_index = (y * stride) + x * 3;
            output[pixel_index] = blur_pixel(data, width, height, x, y, stride);
        }
    }
}

// Основная функция программы
int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file> <threads> <cores>\n";
        return 1;
    }

    const char* input_file = argv[1];
    const char* output_file = argv[2];
    int num_threads = std::stoi(argv[3]);
    int num_cores = std::stoi(argv[4]);

    // Установка количества ядер
    DWORD_PTR process_affinity_mask = (1 << num_cores) - 1;
    SetProcessAffinityMask(GetCurrentProcess(), process_affinity_mask);

    // Открытие BMP файла
    std::ifstream file(input_file, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Could not open input file.\n";
        return 1;
    }

    BMPHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));

    // Проверка формата файла
    if (header.file_type != 0x4D42) {
        std::cerr << "Error: Not a BMP file.\n";
        return 1;
    }

    int width = header.width;
    int height = header.height;
    int stride = (width * 3 + 3) & ~3;  // Учет выравнивания по 4 байта

    std::vector<uint8_t> data(header.size_image);
    file.seekg(header.offset_data, std::ios::beg);
    file.read(reinterpret_cast<char*>(data.data()), header.size_image);
    file.close();

    std::vector<uint8_t> output(data);

    // Измерение времени работы программы
    auto start_time = std::chrono::high_resolution_clock::now();

    // Создание потоков для обработки изображения
    std::vector<std::thread> threads;
    int rows_per_thread = height / num_threads;

    for (int i = 0; i < num_threads; ++i) {
        int start_row = i * rows_per_thread;
        int end_row = (i == num_threads - 1) ? height : (i + 1) * rows_per_thread;
        threads.emplace_back(process_strip, start_row, end_row, width, height, stride, std::ref(data), std::ref(output));
    }

    for (auto& t : threads) {
        t.join();
    }

    // Измерение времени выполнения
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed_time = end_time - start_time;

    // Сохранение результирующего изображения
    std::ofstream output_file_stream(output_file, std::ios::binary);
    if (!output_file_stream) {
        std::cerr << "Error: Could not open output file.\n";
        return 1;
    }

    output_file_stream.write(reinterpret_cast<char*>(&header), sizeof(header));
    output_file_stream.write(reinterpret_cast<char*>(output.data()), output.size());
    output_file_stream.close();

    std::cout << "Execution time: " << elapsed_time.count() << " ms\n";
    return 0;
}
