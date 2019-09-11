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
#include "memory/Memory.h"

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
	typedef int (*t_av_read_frame)(AVFormatContext* s, AVPacket* pkt);
	static t_av_read_frame p_av_read_frame;
	typedef void (*t_av_packet_unref)(AVPacket* pkt);
	static t_av_packet_unref p_av_packet_unref;
    typedef int (*t_avcodec_send_packet)(AVCodecContext* avctx, const AVPacket* avpkt);
    static t_avcodec_send_packet p_avcodec_send_packet;
    typedef int (*t_avcodec_receive_frame)(AVCodecContext* avctx, AVFrame* frame);
    static t_avcodec_receive_frame p_avcodec_receive_frame;
    typedef int (*t_av_seek_frame)(AVFormatContext* s, int stream_index, int64_t timestamp, int flags);
    static t_av_seek_frame p_av_seek_frame;
    
	static bool g_lib_loaded = false;

#if VR_MAC || VR_WINDOWS
	static LIB_HANDLE g_libswresample;
	static LIB_HANDLE g_libavutil;
	static LIB_HANDLE g_libavcodec;
	static LIB_HANDLE g_libavformat;

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
		GET_FUNC(g_libavformat, av_read_frame);
		GET_FUNC(g_libavcodec, av_packet_unref);
        GET_FUNC(g_libavcodec, avcodec_send_packet);
        GET_FUNC(g_libavcodec, avcodec_receive_frame);
        GET_FUNC(g_libavformat, av_seek_frame);

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
		Image m_out_image;
        ByteBuffer m_yuv_buffer;
        int m_video_stream = -1;
        bool m_loop = false;

        ~VideoDecoderPrivate()
        {
            this->Close();
        }
        
        bool OpenFile(const String& path, bool loop)
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
            
            m_video_stream = -1;
            for (uint32_t i = 0; i < m_format_context->nb_streams; ++i)
            {
                if (m_format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
                {
                    m_video_stream = i;
                    break;
                }
            }
            if (m_video_stream < 0)
            {
                return false;
            }
            
            const AVCodecParameters* codec_param = m_format_context->streams[m_video_stream]->codecpar;
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
            assert(m_codec_context->pix_fmt == AV_PIX_FMT_YUV420P);

			m_frame = p_av_frame_alloc();
            m_loop = loop;
            m_yuv_buffer = ByteBuffer(m_codec_context->width * m_codec_context->height * 3 / 2);

            return true;
        }
        
        void Close()
        {
			if (!g_lib_loaded)
			{
				return;
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
        
        static double r2d(AVRational r)
        {
            return r.num == 0 || r.den == 0 ? 0.0 : (double) r.num / (double) r.den;
        }

		const Image& GetFrame(float* present_time)
		{
			m_out_image.Clear();

			if (!g_lib_loaded || m_codec_context == nullptr)
			{
				return m_out_image;
			}
            
        read:
            AVPacket packet;
            int ret = p_av_read_frame(m_format_context, &packet);
            if (ret < 0)
            {
                if (ret == AVERROR_EOF)
                {
                    if (m_loop)
                    {
                        float second = 0;
                        int64_t timestamp = (int64_t) (second * r2d(m_format_context->streams[m_video_stream]->time_base));
                        ret = p_av_seek_frame(m_format_context, m_video_stream, timestamp, AVSEEK_FLAG_BACKWARD);
                        if (ret < 0)
                        {
                            return m_out_image;
                        }
                        else
                        {
                            goto read;
                        }
                    }
                    else
                    {
                        return m_out_image;
                    }
                }
                else
                {
                    return m_out_image;
                }
            }
            
            if (packet.stream_index == m_video_stream)
            {
            send:
                p_avcodec_send_packet(m_codec_context, &packet);
                
                ret = p_avcodec_receive_frame(m_codec_context, m_frame);
                if (ret < 0)
                {
                    if (ret == AVERROR(EAGAIN))
                    {
                        goto send;
                    }
                    else
                    {
                        return m_out_image;
                    }
                }
                
                int w = m_codec_context->width;
                int h = m_codec_context->height;
                int uv_w = w / 2;
                int uv_h = h / 2;
                int u_offset = w * h;
                int v_offset = u_offset + uv_w * uv_h;
                
                for (int i = 0; i < h; i++)
                {
                    int y_src_offset = i * w;
                    int y_dst_offset = i * m_frame->linesize[0];
                    Memory::Copy(&m_yuv_buffer.Bytes()[y_src_offset], &m_frame->data[0][y_dst_offset], w);
                }
                
                for (int i = 0; i < uv_h; i++)
                {
                    int u_src_offset = u_offset + i * uv_w;
                    int u_dst_offset = i * m_frame->linesize[1];
                    Memory::Copy(&m_yuv_buffer.Bytes()[u_src_offset], &m_frame->data[1][u_dst_offset], uv_w);
                    
                    int v_src_offset = v_offset + i * uv_w;
                    int v_dst_offset = i * m_frame->linesize[2];
                    Memory::Copy(&m_yuv_buffer.Bytes()[v_src_offset], &m_frame->data[2][v_dst_offset], uv_w);
                }

                m_out_image.width = w;
                m_out_image.height = h;
                m_out_image.format = ImageFormat::YUV420P;
                m_out_image.data = m_yuv_buffer;
                
                if (present_time != nullptr)
                {
                    *present_time = (float) (packet.pts * r2d(m_format_context->streams[m_video_stream]->time_base));
                }
            }

            p_av_packet_unref(&packet);

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
    
    bool VideoDecoder::OpenFile(const String& path, bool loop)
    {
        return m_private->OpenFile(path, loop);
    }
    
    void VideoDecoder::Close()
    {
        return m_private->Close();
    }

	const Image& VideoDecoder::GetFrame(float* present_time)
	{
		return m_private->GetFrame(present_time);
	}
}
