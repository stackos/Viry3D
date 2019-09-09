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

#include "VideoDecoder.h"
#include "Engine.h"
#include "Debug.h"
#include "graphics/Image.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

#if VR_MAC
#include <dlfcn.h>
#endif

#if VR_WINDOWS
#include <Windows.h>
#endif

namespace Viry3D
{
#if VR_MAC
    typedef void* LIB_HANDLE;
    #define LOAD_DY_LIB(handle, name) \
        { \
            handle = dlopen(name, RTLD_LAZY); \
            if (handle == nullptr) \
            { \
                Log("open dynamic lib failed: %s %s", name, dlerror()); \
                return; \
            } \
        }
    #define GET_FUNC(handle, func) p_##func = (t_##func) dlsym(handle, #func)
    #define FREE_DY_LIB(handle) { if (handle) dlclose(handle); handle = nullptr; }
#endif
    
#if VR_WINDOWS
    typedef HMODULE LIB_HANDLE;
    #define LOAD_DY_LIB(handle, name) \
        { \
            handle = LoadLibrary(name); \
            if (handle == nullptr) \
            { \
                Log("open dynamic lib failed: %s", name); \
                return; \
            } \
        }
    #define GET_FUNC(handle, func) p_##func = (t_##func) GetProcAddress(handle, #func)
    #define FREE_DY_LIB(handle) { if (handle) FreeLibrary(handle); handle = nullptr; }
#endif
    
    typedef void (*t_av_register_all)();
    static t_av_register_all p_av_register_all;
    typedef int (*t_avformat_open_input)(AVFormatContext** ps, const char* url, ff_const59 AVInputFormat* fmt, AVDictionary** options);
    static t_avformat_open_input p_avformat_open_input;
    typedef void (*t_avformat_close_input)(AVFormatContext** s);
    static t_avformat_close_input p_avformat_close_input;
    typedef int (*t_avformat_find_stream_info)(AVFormatContext* ic, AVDictionary** options);
    static t_avformat_find_stream_info p_avformat_find_stream_info;
    typedef AVCodec* (*t_avcodec_find_decoder)(enum AVCodecID id);
    static t_avcodec_find_decoder p_avcodec_find_decoder;
    typedef AVCodecContext* (*t_avcodec_alloc_context3)(const AVCodec* codec);
    static t_avcodec_alloc_context3 p_avcodec_alloc_context3;
    typedef int (*t_avcodec_parameters_to_context)(AVCodecContext* codec, const AVCodecParameters* par);
    static t_avcodec_parameters_to_context p_avcodec_parameters_to_context;
	typedef int (*t_avcodec_open2)(AVCodecContext* avctx, const AVCodec* codec, AVDictionary** options);
	static t_avcodec_open2 p_avcodec_open2;
    typedef int (*t_avcodec_close)(AVCodecContext* avctx);
    static t_avcodec_close p_avcodec_close;
	typedef AVFrame* (*t_av_frame_alloc)();
	static t_av_frame_alloc p_av_frame_alloc;
	typedef void (*t_av_frame_free)(AVFrame** frame);
	static t_av_frame_free p_av_frame_free;
	typedef int (*t_av_image_get_buffer_size)(enum AVPixelFormat pix_fmt, int width, int height, int align);
	static t_av_image_get_buffer_size p_av_image_get_buffer_size;
	typedef void* (*t_av_malloc)(size_t size) av_malloc_attrib av_alloc_size(1);
	static t_av_malloc p_av_malloc;
	typedef void (*t_av_free)(void* ptr);
	static t_av_free p_av_free;
	typedef int (*t_av_image_fill_arrays)(uint8_t* dst_data[4], int dst_linesize[4], const uint8_t* src, enum AVPixelFormat pix_fmt, int width, int height, int align);
	static t_av_image_fill_arrays p_av_image_fill_arrays;
	typedef struct SwsContext* (*t_sws_getContext)(int srcW, int srcH, enum AVPixelFormat srcFormat, int dstW, int dstH, enum AVPixelFormat dstFormat, int flags, SwsFilter* srcFilter, SwsFilter* dstFilter, const double* param);
	static t_sws_getContext p_sws_getContext;
	typedef int (*t_av_read_frame)(AVFormatContext* s, AVPacket* pkt);
	static t_av_read_frame p_av_read_frame;
	typedef void (*t_av_packet_unref)(AVPacket* pkt);
	static t_av_packet_unref p_av_packet_unref;

	static bool g_lib_loaded = false;

#if VR_MAC || VR_WINDOWS
	static LIB_HANDLE g_libswresample;
	static LIB_HANDLE g_libavutil;
	static LIB_HANDLE g_libavcodec;
	static LIB_HANDLE g_libavformat;
	static LIB_HANDLE g_libswscale;

    static void LoadFFmpeg()
    {
        String lib_dir = Engine::Instance()->GetDataPath() + "/..";

#if VR_WINDOWS
		String lib_path = lib_dir + "/swresample-3.dll";
#endif
#if VR_MAC
		String lib_path = lib_dir + "/libswresample.3.dylib";
#endif
        LOAD_DY_LIB(g_libswresample, lib_path.CString());
        
#if VR_WINDOWS
		lib_path = lib_dir + "/avutil-56.dll";
#endif
#if VR_MAC
		lib_path = lib_dir + "/libavutil.56.dylib";
#endif
        LOAD_DY_LIB(g_libavutil, lib_path.CString());
        
#if VR_WINDOWS
		lib_path = lib_dir + "/avcodec-58.dll";
#endif
#if VR_MAC
		lib_path = lib_dir + "/libavcodec.58.dylib";
#endif
        LOAD_DY_LIB(g_libavcodec, lib_path.CString());
        
#if VR_WINDOWS
		lib_path = lib_dir + "/avformat-58.dll";
#endif
#if VR_MAC
		lib_path = lib_dir + "/libavformat.58.dylib";
#endif
        LOAD_DY_LIB(g_libavformat, lib_path.CString());

#if VR_WINDOWS
		lib_path = lib_dir + "/swscale-5.dll";
#endif
#if VR_MAC
		lib_path = lib_dir + "/libswscale.5.dylib";
#endif
		LOAD_DY_LIB(g_libswscale, lib_path.CString());
        
        GET_FUNC(g_libavformat, av_register_all);
        GET_FUNC(g_libavformat, avformat_open_input);
        GET_FUNC(g_libavformat, avformat_close_input);
        GET_FUNC(g_libavformat, avformat_find_stream_info);
        GET_FUNC(g_libavcodec, avcodec_find_decoder);
        GET_FUNC(g_libavcodec, avcodec_alloc_context3);
        GET_FUNC(g_libavcodec, avcodec_parameters_to_context);
		GET_FUNC(g_libavcodec, avcodec_open2);
        GET_FUNC(g_libavcodec, avcodec_close);
		GET_FUNC(g_libavutil, av_frame_alloc);
		GET_FUNC(g_libavutil, av_frame_free);
		GET_FUNC(g_libavutil, av_image_get_buffer_size);
		GET_FUNC(g_libavutil, av_malloc);
		GET_FUNC(g_libavutil, av_free);
		GET_FUNC(g_libavutil, av_image_fill_arrays);
		GET_FUNC(g_libswscale, sws_getContext);
		GET_FUNC(g_libavformat, av_read_frame);
		GET_FUNC(g_libavcodec, av_packet_unref);

		g_lib_loaded = true;
    }
    
    static void FreeFFmpeg()
    {
        FREE_DY_LIB(g_libavformat);
        FREE_DY_LIB(g_libavcodec);
        FREE_DY_LIB(g_libavutil);
        FREE_DY_LIB(g_libswresample);
    }
#endif

    void VideoDecoder::Init()
    {
#if VR_MAC || VR_WINDOWS
        LoadFFmpeg();
#endif

		if (g_lib_loaded)
		{
			p_av_register_all();
		}
    }
    
    void VideoDecoder::Done()
    {
#if VR_MAC || VR_WINDOWS
        FreeFFmpeg();
#endif
    }
    
    class VideoDecoderPrivate
    {
    public:
        AVFormatContext* m_format_context = nullptr;
        AVCodecContext* m_codec_context = nullptr;
		AVFrame* m_frame = nullptr;
		AVFrame* m_frame_rgba = nullptr;
		uint8_t* m_rgba_buffer = nullptr;
		int m_rgba_buffer_size = 0;
		SwsContext* m_sws_context = nullptr;
		Image m_out_image;

        ~VideoDecoderPrivate()
        {
            this->Close();
        }
        
        bool OpenFile(const String& path)
        {
			if (!g_lib_loaded)
			{
				return false;
			}

            m_format_context = nullptr;
            if (p_avformat_open_input(&m_format_context, path.CString(), nullptr, nullptr) != 0)
            {
                return false;
            }
            
            if (p_avformat_find_stream_info(m_format_context, nullptr) < 0)
            {
                return false;
            }
            
            int video_stream = -1;
            for (uint32_t i = 0; i < m_format_context->nb_streams; ++i)
            {
                if (m_format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
                {
                    video_stream = i;
                    break;
                }
            }
            if (video_stream < 0)
            {
                return false;
            }
            
            const AVCodecParameters* codec_param = m_format_context->streams[video_stream]->codecpar;
            const AVCodec* codec = p_avcodec_find_decoder(codec_param->codec_id);
            if (codec == nullptr)
            {
                return false;
            }
            
            m_codec_context = p_avcodec_alloc_context3(codec);
            if (p_avcodec_parameters_to_context(m_codec_context, codec_param) != 0)
            {
                return false;
            }

			if (p_avcodec_open2(m_codec_context, codec, nullptr) < 0)
			{
				return false;
			}

			m_frame = p_av_frame_alloc();
			m_frame_rgba = p_av_frame_alloc();

			m_rgba_buffer_size = p_av_image_get_buffer_size(AV_PIX_FMT_RGBA, m_codec_context->width, m_codec_context->height, 1);
			m_rgba_buffer = (uint8_t*) p_av_malloc(m_rgba_buffer_size);
			p_av_image_fill_arrays(m_frame_rgba->data, m_frame_rgba->linesize, m_rgba_buffer, AV_PIX_FMT_RGBA, m_codec_context->width, m_codec_context->height, 1);
			
			m_sws_context = p_sws_getContext(
				m_codec_context->width,
				m_codec_context->height,
				m_codec_context->pix_fmt,
				m_codec_context->width,
				m_codec_context->height,
				AV_PIX_FMT_RGBA,
				SWS_BILINEAR,
				nullptr,
				nullptr,
				nullptr);

            return true;
        }
        
        void Close()
        {
			if (!g_lib_loaded)
			{
				return;
			}

			if (m_rgba_buffer != nullptr)
			{
				p_av_free(m_rgba_buffer);
				m_rgba_buffer = nullptr;
			}
			
			if (m_frame_rgba != nullptr)
			{
				p_av_frame_free(&m_frame_rgba);
				m_frame_rgba = nullptr;
			}

			if (m_frame != nullptr)
			{
				p_av_frame_free(&m_frame);
				m_frame = nullptr;
			}

            if (m_codec_context != nullptr)
            {
                p_avcodec_close(m_codec_context);
                m_codec_context = nullptr;
            }
            
            if (m_format_context != nullptr)
            {
                p_avformat_close_input(&m_format_context);
                m_format_context = nullptr;
            }
        }

		const Image& GetFrame()
		{
			m_out_image.Clear();

			if (!g_lib_loaded || m_codec_context == nullptr)
			{
				return m_out_image;
			}

			m_out_image.width = m_codec_context->width;
			m_out_image.height = m_codec_context->height;
			m_out_image.format = ImageFormat::R8G8B8A8;
			m_out_image.data = ByteBuffer(m_frame_rgba->data[0], m_rgba_buffer_size);

			return m_out_image;
		}
    };
    
    VideoDecoder::VideoDecoder():
        m_private(new VideoDecoderPrivate())
    {
        
    }
    
    VideoDecoder::~VideoDecoder()
    {
        delete m_private;
    }
    
    bool VideoDecoder::OpenFile(const String& path)
    {
        return m_private->OpenFile(path);
    }
    
    void VideoDecoder::Close()
    {
        return m_private->Close();
    }

	const Image& VideoDecoder::GetFrame()
	{
		return m_private->GetFrame();
	}
}
