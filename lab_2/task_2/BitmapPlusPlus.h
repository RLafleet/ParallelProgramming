#ifndef BITMAPPLUSPLUS_H
#define BITMAPPLUSPLUS_H

#include <vector>
#include <stdexcept>
#include <string>
#include <optional>

namespace bmp {

    struct Pixel {
        int r; // Red component
        int g; // Green component
        int b; // Blue component

        Pixel(int red = 0, int green = 0, int blue = 0)
            : r(red), g(green), b(blue) {}
    };

    class Exception : public std::runtime_error {
    public:
        explicit Exception(const std::string& message)
            : std::runtime_error(message) {}
    };

    class Bitmap {
    public:
        Bitmap(const std::string& filename);
        ~Bitmap();

        unsigned width() const;
        unsigned height() const;

        std::optional<Pixel> get(unsigned x, unsigned y) const;
        void set(unsigned x, unsigned y, const Pixel& pixel);
        void save(const std::string& filename) const;

    private:
        unsigned _width;
        unsigned _height;
        std::vector<Pixel> _pixels;

        void loadFromFile(const std::string& filename);
        void allocateMemory();
    };

} // namespace bmp

#endif // BITMAPPLUSPLUS_H