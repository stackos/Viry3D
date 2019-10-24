#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(int argc, char* argv[])
{
    int w, h, c;
    unsigned char* data = stbi_load("r.png", &w, &h, &c, 0);
    stbi_write_png("o.png", w, h, c, data, w * c);
    stbi_image_free(data);
    return 0;
}
