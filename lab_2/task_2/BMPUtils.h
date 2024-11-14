#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <random>
#include <string>
#include <cstdint>

struct Pixel {
    uint8_t blue, green, red;
};

class BMPImage {
public:
    BMPImage(const std::string& inputFilePath) {
        Load(inputFilePath);
    }

    void Blur(unsigned threadCount) {
        unsigned squareSize = width / threadCount;  // Размер стороны квадрата
        unsigned squaresPerThread = threadCount;    // Количество квадратов на поток
        std::vector<std::thread> threads;

        for (unsigned i = 0; i < threadCount; ++i) {
            threads.emplace_back(&BMPImage::ProcessSquares, this, i, threadCount, squareSize, squaresPerThread);
        }

        for (auto& thread : threads) {
            thread.join();
        }
    }

    void Save(const std::string& outputFilePath) const {
        std::ofstream outFile(outputFilePath, std::ios::binary);
        if (!outFile) throw std::runtime_error("Could not open file for writing.");

        outFile.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
        outFile.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));

        for (const auto& row : pixels) {
            outFile.write(reinterpret_cast<const char*>(row.data()), row.size() * sizeof(Pixel));
        }
    }

private:
    struct BMPFileHeader {
        uint16_t fileType{ 0x4D42 };
        uint32_t fileSize{ 0 };
        uint32_t reserved{ 0 };
        uint32_t dataOffset{ 0 };
    } fileHeader;

    struct BMPInfoHeader {
        uint32_t headerSize{ 0 };
        int32_t width{ 0 };
        int32_t height{ 0 };
        uint16_t planes{ 1 };
        uint16_t bitCount{ 24 };
        uint32_t compression{ 0 };
        uint32_t imageSize{ 0 };
        int32_t xPixelsPerMeter{ 0 };
        int32_t yPixelsPerMeter{ 0 };
        uint32_t colorsUsed{ 0 };
        uint32_t importantColors{ 0 };
    } infoHeader;

    std::vector<std::vector<Pixel>> pixels;
    int width;
    int height;

    void Load(const std::string& filePath) {
        std::ifstream inFile(filePath, std::ios::binary);
        if (!inFile) throw std::runtime_error("Could not open file for reading.");

        inFile.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
        inFile.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));

        if (fileHeader.fileType != 0x4D42) {
            throw std::runtime_error("Not a BMP file.");
        }

        width = infoHeader.width;
        height = infoHeader.height;

        if (width <= 0 || height <= 0) {
            throw std::runtime_error("Invalid BMP dimensions.");
        }

        pixels.resize(height, std::vector<Pixel>(width));

        for (int y = 0; y < height; ++y) {
            inFile.read(reinterpret_cast<char*>(pixels[y].data()), width * sizeof(Pixel));
            int padding = (4 - (width * sizeof(Pixel)) % 4) % 4;
            inFile.ignore(padding);
        }
    }

    void ProcessSquares(unsigned threadIndex, unsigned threadCount, unsigned squareSize, unsigned squaresPerThread) {
        std::default_random_engine generator(threadIndex);
        std::uniform_int_distribution<int> distX(0, width - squareSize);
        std::uniform_int_distribution<int> distY(0, height - squareSize);

        for (unsigned i = 0; i < squaresPerThread; ++i) {
            unsigned startX = distX(generator);
            unsigned startY = distY(generator);
            ApplyBlurToSquare(startX, startY, squareSize);
        }
    }

    void ApplyBlurToSquare(unsigned startX, unsigned startY, unsigned squareSize) {
        for (unsigned y = startY; y < startY + squareSize && y < height; ++y) {
            for (unsigned x = startX; x < startX + squareSize && x < width; ++x) {
                pixels[y][x] = CalculateAverageColor(x, y);
            }
        }
    }

    Pixel CalculateAverageColor(unsigned x, unsigned y) const {
        int red = 0, green = 0, blue = 0, count = 0;
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                unsigned nx = x + dx, ny = y + dy;
                if (nx < width && ny < height) {
                    red += pixels[ny][nx].red;
                    green += pixels[ny][nx].green;
                    blue += pixels[ny][nx].blue;
                    ++count;
                }
            }
        }
        return Pixel{
            static_cast<uint8_t>(blue / count),
            static_cast<uint8_t>(green / count),
            static_cast<uint8_t>(red / count)
        };
    }
};