#ifndef H264Decoder_H
#define H264Decoder_H


#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
//Windows
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
};
#endif


class H264Decoder {
public:
	explicit H264Decoder(int dstWidth, int dstHeight);
	~H264Decoder();
	AVFrame* Decoder(int buf_size, char* buf_data);
	int width;
	int height;
private:
	AVCodec *pCodec;
	AVCodecContext *pCodecCtx;
	AVCodecParserContext *pCodecParserCtx;
	int frame_count;
	AVFrame	*pFrame, *pConvertedFrame;
	uint8_t *out_buffer;
	AVPacket packet;
	AVCodecID codec_id;
	struct SwsContext *img_convert_ctx;
};

#endif