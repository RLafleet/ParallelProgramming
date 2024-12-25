#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <cmath>
#include <cstring>

// Структуры для хранения BMP заголовков
#pragma pack(push, 1)
struct BMPHeader {
    uint16_t fileType;      // Тип файла (BM)
    uint32_t fileSize;      // Размер файла в байтах
    uint16_t reserved1;     // Зарезервировано
    uint16_t reserved2;     // Зарезервировано
    uint32_t offsetData;    // Смещение до начала данных изображения
};

struct DIBHeader {
    uint32_t size;          // Размер DIB заголовка
    int32_t width;          // Ширина изображения
    int32_t height;         // Высота изображения
    uint16_t planes;        // Количество цветовых плоскостей
    uint16_t bitCount;      // Количество бит на пиксель
    uint32_t compression;    // Метод сжатия
    uint32_t sizeImage;     // Размер изображения
    int32_t xPelsPerMeter;   // Горизонтальное разрешение
    int32_t yPelsPerMeter;   // Вертикальное разрешение
    uint32_t colorsUsed;     // Количество используемых цветов
    uint32_t colorsImportant; // Количество важных цветов
};
#pragma pack(pop)

std::mutex mtx; 

void blurImage(const uint8_t* src, uint8_t* dst, int width, int height, int startX, int startY, int blockSize) {
    int endX = std::min(startX + blockSize, width);
    int endY = std::min(startY + blockSize, height);
    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            int sumR = 0, sumG = 0, sumB = 0;
            int count = 0;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                        sumB += src[(ny * width + nx) * 3];
                        sumG += src[(ny * width + nx) * 3 + 1];
                        sumR += src[(ny * width + nx) * 3 + 2];
                        count++;
                    }
                }
            }
            dst[(y * width + x) * 3] = sumB / count;
            dst[(y * width + x) * 3 + 1] = sumG / count;
            dst[(y * width + x) * 3 + 2] = sumR / count;
        }
    }
}

void processBlocks(const uint8_t* src, uint8_t* dst, int width, int height, int blockSize, int blocksPerThread, int threadID) {
    for (int i = 0; i < blocksPerThread; ++i) {
        int blockX = (threadID * blocksPerThread + i) % (width / blockSize) * blockSize;
        int blockY = (threadID * blocksPerThread + i) / (width / blockSize) * blockSize;
        if (blockY < height) {
            blurImage(src, dst, width, height, blockX, blockY, blockSize);
        } 
    }
}

int main(int argc, char* argv[]) 
{
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <input.bmp> <output.bmp> <num_threads>" << std::endl;
        return 1;
    }

    const char* inputFilename = argv[1];
    const char* outputFilename = argv[2];
    int numThreads = std::stoi(argv[3]);

    std::ifstream inputFile(inputFilename, std::ios::binary);
    if (!inputFile) 
    {
        std::cerr << "Error opening input file." << std::endl;
        return 1;
    }

    BMPHeader bmpHeader;
    DIBHeader dibHeader;
    inputFile.read(reinterpret_cast<char*>(&bmpHeader), sizeof(bmpHeader));
    inputFile.read(reinterpret_cast<char*>(&dibHeader), sizeof(dibHeader));

    if (bmpHeader.fileType != 0x4D42) 
    {
        std::cerr << "Not a valid BMP file." << std::endl;
        return 1;
    }

    int width = dibHeader.width;
    int height = dibHeader.height;
    int imageSize = width * height * 3;
    std::vector<uint8_t> srcImage(imageSize);
    std::vector<uint8_t> dstImage(imageSize);
    inputFile.seekg(bmpHeader.offsetData, std::ios::beg);
    inputFile.read(reinterpret_cast<char*>(srcImage.data()), imageSize);
    inputFile.close();

    int blockSize = 16; 
    int numBlocks = (width * height) / (blockSize * blockSize);
    int blocksPerThread = numBlocks / numThreads;

    std::vector<std::jthread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(processBlocks, srcImage.data(), dstImage.data(), width, height, blockSize, blocksPerThread, i);
    }

    std::ofstream outputFile(outputFilename, std::ios::binary);
    if (!outputFile) {
        std::cerr << "Error opening output file." << std::endl;
        return 1;
    }
    outputFile.write(reinterpret_cast<char*>(&bmpHeader), sizeof(bmpHeader));
    outputFile.write(reinterpret_cast<char*>(&dibHeader), sizeof(dibHeader));
    outputFile.seekp(bmpHeader.offsetData, std::ios::beg);
    outputFile.write(reinterpret_cast<char*>(dstImage.data()), imageSize);
    outputFile.close();

    std::cout << "Blurring complete!" << std::endl;
    return 0;
}
