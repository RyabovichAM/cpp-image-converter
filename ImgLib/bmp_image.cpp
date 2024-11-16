#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {

PACKED_STRUCT_BEGIN BitmapFileHeader {
    char sign1 = 'B';
    char sign2 = 'M';
    uint32_t size;
    int reserve = 0;
    uint start_data_offset;
}
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN BitmapInfoHeader {
    uint info_header_size;
    int img_width;
    int img_height;
    uint16_t number_of_planes = 1;
    uint16_t bits_per_pixel = 24;
    uint compression_type = 0;
    uint bytes_in_data;
    int horisontal_DPI = 11811;
    int vertical_DPI = 11811;
    int number_of_colors = 0;
    int number_of_significant_colors = 0x1000000;
}
PACKED_STRUCT_END

// функция вычисления отступа по ширине
static int GetBMPStride(int w) {
    return 4 * ((w * 3 + 3) / 4);
}

bool SaveBMP(const Path& file, const Image& image) {
    ofstream out(file, ios::binary);

    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;
    const int w = image.GetWidth();
    const int h = image.GetHeight();
    const int stride = GetBMPStride(w);

    const uint16_t header_size = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader);
    file_header.size = header_size + stride * h;
    file_header.start_data_offset = header_size;

    out.write(reinterpret_cast<char*>(&file_header), sizeof(BitmapFileHeader));

    info_header.info_header_size = sizeof(BitmapInfoHeader);
    info_header.img_width = w;
    info_header.img_height = h;
    info_header.bytes_in_data = stride * h;

    out.write(reinterpret_cast<char*>(&info_header), sizeof(BitmapInfoHeader));

    std::vector<char> buff(stride);

    for (int y = h - 1; y >= 0; --y) {
        const Color* line = image.GetLine(y);
        for (int x = 0; x < w; ++x) {
            buff[x * 3 + 0] = static_cast<char>(line[x].b);
            buff[x * 3 + 1] = static_cast<char>(line[x].g);
            buff[x * 3 + 2] = static_cast<char>(line[x].r);
        }

        out.write(buff.data(), stride);
    }

    return out.good();
}

Image LoadBMP(const Path& file) {
    ifstream in(file, ios::binary);

    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;

    in.read(reinterpret_cast<char*>(&file_header),sizeof(file_header));
    in.read(reinterpret_cast<char*>(&info_header),sizeof(info_header));

    const int w = info_header.img_width;
    const int h = info_header.img_height;

    const int stride = GetBMPStride(w);

    Image result(w, h, Color::Black());
    std::vector<char> buff(stride);

    for (int y = h - 1; y >= 0; --y) {
        Color* line = result.GetLine(y);
        in.read(buff.data(), stride);

        for (int x = 0; x < w; ++x) {
            line[x].b = static_cast<byte>(buff[x * 3 + 0]);
            line[x].g = static_cast<byte>(buff[x * 3 + 1]);
            line[x].r = static_cast<byte>(buff[x * 3 + 2]);
        }
    }

    return result;

}

}  // namespace img_lib
