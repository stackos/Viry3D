#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "../../lib/src/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../lib/src/stb/stb_image_write.h"
#include <string>
#include <vector>

struct Image
{
    stbi_uc* data = nullptr;
    int w = 0;
    int h = 0;
    int c = 0;
};

int main(int argc, char* argv[])
{
    // get args
    std::vector<std::string> args;
    for (int i = 1; i < argc; i++)
    {
        args.push_back(argv[i]);
    }

    // get pathes
    std::vector<std::string> pathes(2);
    for (size_t i = 0; i < args.size(); i++)
    {
        if (i < args.size() - 1)
        {
            if (args[i] == "-i")
            {
                pathes[0] = args[i + 1];
            }
            else if (args[i] == "-o")
            {
                pathes[1] = args[i + 1];
            }
        }
    }

    // check pathes
    for (size_t i = 0; i < pathes.size(); i++)
    {
        if (pathes[i].length() == 0)
        {
            printf("Usage: PNGPremulAlpha.exe -i i.png -o o.png\n");
            return 0;
        }
    }

    // load images
    Image image;
    image.data = stbi_load(pathes[0].c_str(), &image.w, &image.h, &image.c, 0);

    if (image.data == nullptr)
    {
        printf("Error: Image %s load failed\n", pathes[0].c_str());
    }
    else if (image.c != 4)
    {
        printf("Error: Not rgba\n");
    }
    else
    {
        int w = image.w;
        int h = image.h;
        int c = 4;
        stbi_uc* pixels = (stbi_uc*) malloc(w * h * 4);
            
        // fill pixels
        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                stbi_uc r = image.data[y * w * c + x * c + 0];
                stbi_uc g = image.data[y * w * c + x * c + 1];
                stbi_uc b = image.data[y * w * c + x * c + 2];
                stbi_uc a = image.data[y * w * c + x * c + 3];

                r = r * a / 255;
                g = g * a / 255;
                b = b * a / 255;

                pixels[y * w * c + x * c + 0] = r;
                pixels[y * w * c + x * c + 1] = g;
                pixels[y * w * c + x * c + 2] = b;
                pixels[y * w * c + x * c + 3] = a;
            }
        }

        int write_success = stbi_write_png(pathes[1].c_str(), w, h, c, pixels, w * c);
        if (write_success)
        {
            printf("Success: converted image saved to %s\n", pathes[1].c_str());
        }
        else
        {
            printf("Failed: write image to %s\n", pathes[1].c_str());
        }

        free(pixels);
    }
    
    // free images
    if (image.data)
    {
        stbi_image_free(image.data);
        image.data = nullptr;
    }

    return 0;
}
