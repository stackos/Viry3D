#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "../../lib/src/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../lib/src/stb/stb_image_write.h"
#include <string>
#include <vector>

enum ImageType
{
    R,
    G,
    B,
    A,
    O,

    Count
};

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
    std::vector<std::string> pathes(ImageType::Count);
    for (size_t i = 0; i < args.size(); i++)
    {
        if (i < args.size() - 1)
        {
            if (args[i] == "-r")
            {
                pathes[ImageType::R] = args[i + 1];
            }
            else if (args[i] == "-g")
            {
                pathes[ImageType::G] = args[i + 1];
            }
            else if (args[i] == "-b")
            {
                pathes[ImageType::B] = args[i + 1];
            }
            else if (args[i] == "-a")
            {
                pathes[ImageType::A] = args[i + 1];
            }
            else if (args[i] == "-o")
            {
                pathes[ImageType::O] = args[i + 1];
            }
        }
    }

    // check pathes
    for (size_t i = 0; i < pathes.size(); i++)
    {
        if (pathes[i].length() == 0)
        {
            printf("Usage: PNGCombiner.exe -r r.png -g g.png -b b.png -a a.png -o o.png\n");
            return 0;
        }
    }

    // load images
    bool miss_input = false;
    std::vector<Image> images(4);
    for (size_t i = 0; i < images.size(); i++)
    {
        images[i].data = stbi_load(pathes[i].c_str(), &images[i].w, &images[i].h, &images[i].c, 0);
        if (images[i].data == nullptr)
        {
            printf("Error: Image %s load failed\n", pathes[i].c_str());
            miss_input = true;
            break;
        }
    }

    if (miss_input)
    {
        printf("Error: Miss one or more input image\n");
    }
    else
    {
        // check size
        bool same_size = true;
        for (size_t i = 1; i < images.size(); i++)
        {
            if ((images[i].w != images[i - 1].w) || (images[i].h != images[i - 1].h))
            {
                same_size = false;
                break;
            }
        }

        if (!same_size)
        {
            printf("Error: input images should have same size\n");
        }
        else
        {
            int w = images[0].w;
            int h = images[0].h;
            int c = 4;
            stbi_uc* pixels = (stbi_uc*) malloc(w * h * 4);
            
            // fill pixels
            for (int i = 0; i < c; i++)
            {
                const Image& image = images[i];

                int offset = 0;
                if (image.c > i)
                {
                    offset = i;
                }

                for (int y = 0; y < h; y++)
                {
                    for (int x = 0; x < w; x++)
                    {
                        pixels[y * w * c + x * c + i] = image.data[y * w * image.c + x * image.c + offset];
                    }
                }
            }

            int write_success = stbi_write_png(pathes[ImageType::O].c_str(), w, h, c, pixels, w * c);
            if (write_success)
            {
                printf("Success: combined image saved to %s\n", pathes[ImageType::O].c_str());
            }

            free(pixels);
        }
    }
    
    // free images
    for (size_t i = 0; i < images.size(); i++)
    {
        if (images[i].data)
        {
            stbi_image_free(images[i].data);
        }
    }
    images.clear();

    return 0;
}
