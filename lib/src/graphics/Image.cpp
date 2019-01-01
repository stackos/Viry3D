/*
* Viry3D
* Copyright 2014-2019 by Stack - stackos@qq.com
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "Image.h"
#include "io/File.h"
#include "memory/Memory.h"
#include "Debug.h"

extern "C"
{
#include "jpeg/jpeglib.h"
#include "png/png.h"
#include "png/pngstruct.h"
}

namespace Viry3D
{
    struct PNGData
    {
        char* buffer;
        int size;
    };

    ByteBuffer Image::LoadJPEG(const ByteBuffer& jpeg, int& width, int& height, int& bpp)
    {
        //use lib jpeg
        jpeg_decompress_struct cinfo;
        jpeg_error_mgr jerr;
        JSAMPARRAY buffer;		/* Output row buffer */
        int row_stride;		/* physical row width in output buffer */

        cinfo.err = jpeg_std_error(&jerr);

        jpeg_create_decompress(&cinfo);
        jpeg_mem_src(&cinfo, jpeg.Bytes(), jpeg.Size());
        jpeg_read_header(&cinfo, TRUE);
        jpeg_start_decompress(&cinfo);
        row_stride = cinfo.output_width * cinfo.output_components;
        /* Make a one-row-high sample array that will go away when done with image */
        buffer = (*cinfo.mem->alloc_sarray)
            ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

        width = cinfo.output_width;
        height = cinfo.output_height;
        bpp = cinfo.output_components * 8;

        ByteBuffer colors(width * height * cinfo.output_components);

        unsigned char* pPixel = colors.Bytes();

        while (cinfo.output_scanline < cinfo.output_height)
        {
            jpeg_read_scanlines(&cinfo, buffer, 1);

            unsigned char *pixel_data = buffer[0];

            for (int j = 0; j < width; ++j)
            {
                memcpy(pPixel, pixel_data, cinfo.output_components);

                pPixel += cinfo.output_components;
                pixel_data += cinfo.output_components;
            }
        }

        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);

        return colors;
    }

    static void PngRead(png_structp png_ptr, png_bytep data, png_size_t length)
    {
        memcpy(data, png_ptr->io_ptr, length);
        png_ptr->io_ptr = (char*) png_ptr->io_ptr + length;
    }

    static void PngWrite(png_structp png_ptr, png_bytep data, png_size_t length)
    {
        PNGData* png = (PNGData*) png_get_io_ptr(png_ptr);

        if (png->buffer == nullptr)
        {
            png->buffer = (char*) malloc(length);
        }
        else
        {
            png->buffer = (char*) realloc(png->buffer, png->size + length);
        }

        memcpy(&png->buffer[png->size], data, length);
        png->size += (int) length;
    }

    static void PngFlush(png_structp png_ptr)
    {

    }

    ByteBuffer Image::LoadPNG(const ByteBuffer& png, int& width, int& height, int& bpp)
    {
        ByteBuffer colors;

        png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
        png_infop info_ptr = png_create_info_struct(png_ptr);
        setjmp(png_jmpbuf(png_ptr));

        png_set_read_fn(png_ptr, png.Bytes(), PngRead);
        png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);

        width = png_get_image_width(png_ptr, info_ptr);
        height = png_get_image_height(png_ptr, info_ptr);

        int color_type = png_get_color_type(png_ptr, info_ptr);
        if (color_type == PNG_COLOR_TYPE_RGBA)
        {
            bpp = png_get_bit_depth(png_ptr, info_ptr) * 4;

            png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);

            colors = ByteBuffer(width * height * 4);

            unsigned char* pPixel = colors.Bytes();

            for (int i = 0; i < height; ++i)
            {
                memcpy(pPixel, row_pointers[i], width * 4);
                pPixel += width * 4;
            }
        }
        else if (color_type == PNG_COLOR_TYPE_RGB)
        {
            bpp = png_get_bit_depth(png_ptr, info_ptr) * 3;

            png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);

            colors = ByteBuffer(width * height * 3);

            unsigned char* pPixel = colors.Bytes();

            for (int i = 0; i < height; ++i)
            {
                memcpy(pPixel, row_pointers[i], width * 3);
                pPixel += width * 3;
            }
        }
        else if (color_type == PNG_COLOR_TYPE_GRAY)
        {
            bpp = png_get_bit_depth(png_ptr, info_ptr) * 1;

            png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);

            colors = ByteBuffer(width * height);

            unsigned char* pPixel = colors.Bytes();

            for (int i = 0; i < height; ++i)
            {
                memcpy(pPixel, row_pointers[i], width * 1);
                pPixel += width * 1;
            }
        }
        else if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        {
            bpp = png_get_bit_depth(png_ptr, info_ptr) * 4;

            png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);

            colors = ByteBuffer(width * height * 4);

            byte* pPixel = colors.Bytes();

            for (int i = 0; i < height; ++i)
            {
                for (int j = 0; j < width; ++j)
                {
                    png_byte g = row_pointers[i][j * 2];
                    png_byte a = row_pointers[i][j * 2 + 1];
                    pPixel[0] = g;
                    pPixel[1] = g;
                    pPixel[2] = g;
                    pPixel[3] = a;

                    pPixel += 4;
                }
            }
        }
        else
        {
            assert(!"color type not support");
        }

        png_destroy_read_struct(&png_ptr, &info_ptr, 0);

        return colors;
    }

    void Image::EncodeToPNG(const String& file, const ByteBuffer& colors, int width, int height, int bpp)
    {
        int color_type = -1;
        switch (bpp)
        {
            case 32:
                color_type = PNG_COLOR_TYPE_RGBA;
                break;
            case 24:
                color_type = PNG_COLOR_TYPE_RGB;
                break;
            case 8:
                color_type = PNG_COLOR_TYPE_GRAY;
                break;
            default:
                return;
        }

        png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
        png_infop info_ptr = png_create_info_struct(png_ptr);
        setjmp(png_jmpbuf(png_ptr));

        PNGData png_data;
        png_data.buffer = 0;
        png_data.size = 0;

        png_set_write_fn(png_ptr, &png_data, PngWrite, PngFlush);

        png_set_IHDR(png_ptr, info_ptr, width, height,
            8, color_type, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
        png_write_info(png_ptr, info_ptr);

        png_bytepp row_pointers = new png_bytep[height];
        for (int i = 0; i < height; ++i)
        {
            row_pointers[i] = (png_bytep) &colors[i * width * bpp / 8];
        }
        png_write_image(png_ptr, row_pointers);
        delete[] row_pointers;

        png_write_end(png_ptr, 0);
        png_destroy_write_struct(&png_ptr, &info_ptr);

        char* data = png_data.buffer;
        int size = png_data.size;

        ByteBuffer file_buffer(size);
        Memory::Copy(file_buffer.Bytes(), data, size);

        File::WriteAllBytes(file, file_buffer);

        free(data);
    }
}
