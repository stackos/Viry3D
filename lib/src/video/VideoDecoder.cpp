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

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#if VR_MAC
#include <dlfcn.h>
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
    #define LIB_EXT ".dylib"
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
    #define LIB_EXT ".dll"
#endif
    
    static LIB_HANDLE g_libswresample;
    static LIB_HANDLE g_libavutil;
    static LIB_HANDLE g_libavcodec;
    static LIB_HANDLE g_libavformat;
    
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
    typedef int (*t_avcodec_close)(AVCodecContext* avctx);
    static t_avcodec_close p_avcodec_close;
    
    static void LoadFFmpeg()
    {
        String lib_dir = Engine::Instance()->GetDataPath() + "/..";

        String lib_path = lib_dir + "/libswresample.3" + LIB_EXT;
        LOAD_DY_LIB(g_libswresample, lib_path.CString());
        
        lib_path = lib_dir + "/libavutil.56" + LIB_EXT;
        LOAD_DY_LIB(g_libavutil, lib_path.CString());
        
        lib_path = lib_dir + "/libavcodec.58" + LIB_EXT;
        LOAD_DY_LIB(g_libavcodec, lib_path.CString());
        
        lib_path = lib_dir + "/libavformat.58" + LIB_EXT;
        LOAD_DY_LIB(g_libavformat, lib_path.CString());
        
        GET_FUNC(g_libavformat, av_register_all);
        GET_FUNC(g_libavformat, avformat_open_input);
        GET_FUNC(g_libavformat, avformat_close_input);
        GET_FUNC(g_libavformat, avformat_find_stream_info);
        GET_FUNC(g_libavcodec, avcodec_find_decoder);
        GET_FUNC(g_libavcodec, avcodec_alloc_context3);
        GET_FUNC(g_libavcodec, avcodec_parameters_to_context);
        GET_FUNC(g_libavcodec, avcodec_close);
    }
    
    static void FreeFFmpeg()
    {
        FREE_DY_LIB(g_libavformat);
        FREE_DY_LIB(g_libavcodec);
        FREE_DY_LIB(g_libavutil);
        FREE_DY_LIB(g_libswresample);
    }
    
    void VideoDecoder::Init()
    {
        LoadFFmpeg();
        
        p_av_register_all();
        
        // test
        VideoDecoder* decoder = new VideoDecoder();
        decoder->OpenFile("/Users/ccgao/Downloads/video-bg.3e78e808.mp4");
        decoder->Close();
        delete decoder;
    }
    
    void VideoDecoder::Done()
    {
        FreeFFmpeg();
    }
    
    class VideoDecoderPrivate
    {
    public:
        AVFormatContext* m_format_context = nullptr;
        AVCodecContext* m_codec_context = nullptr;
        
        ~VideoDecoderPrivate()
        {
            this->Close();
        }
        
        bool OpenFile(const String& path)
        {
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
            for (int i = 0; i < m_format_context->nb_streams; ++i)
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
            
            return true;
        }
        
        void Close()
        {
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
}
