#include "H264Decoder.h"

H264Decoder::H264Decoder(int dstWidth, int dstHeight){
	pCodecCtx = NULL;
	pCodecParserCtx = NULL;
	codec_id = AV_CODEC_ID_H264;
	width = dstWidth;
	height = dstHeight;

	avcodec_register_all();

	pCodec = avcodec_find_decoder(codec_id);
	if (!pCodec) {
		//printf("Codec not found\n");
		throw "未找到解码器";
	}

	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (!pCodecCtx){
		//printf("Could not allocate video codec context\n");
		throw "不能分配视频编解码器上下文";
	}

	//初始化参数，下面的参数应该由具体的业务决定  
	//pCodecCtx->time_base.num = 1;
	//pCodecCtx->frame_number = 1; //每包一个视频帧  
	pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	pCodecCtx->bit_rate = 0;
	pCodecCtx->time_base.den = 25;//帧率  
	pCodecCtx->width = 1280;//视频宽  
	pCodecCtx->height = 720;//视频高  

	pCodecParserCtx = av_parser_init(codec_id);
	if (!pCodecParserCtx){
		//printf("Could not allocate video parser context\n");
		throw "不能分配视频解析器上下文";
	}

	//if(pCodec->capabilities&CODEC_CAP_TRUNCATED)
	//    pCodecCtx->flags|= CODEC_FLAG_TRUNCATED; /* we do not send complete frames */

	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		//printf("Could not open codec\n");
		throw "不能打开解码器";
	}

	pFrame = av_frame_alloc();

	//printf("\nCodec Full Name:%s\n", pCodecCtx->codec->long_name);
	//printf("width:%d\nheight:%d\n\n", pCodecCtx->width, pCodecCtx->height);
	//SwsContext
	img_convert_ctx = sws_getContext(1280, 720, AV_PIX_FMT_YUV420P,
		width, height, AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);

	pConvertedFrame = av_frame_alloc();
	out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_BGR24, width, height));
	avpicture_fill((AVPicture *)pConvertedFrame, out_buffer, AV_PIX_FMT_BGR24, width, height);
}
H264Decoder::~H264Decoder() {
	sws_freeContext(img_convert_ctx);
	av_parser_close(pCodecParserCtx);

	av_frame_free(&pConvertedFrame);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	av_free(pCodecCtx);
}

AVFrame* H264Decoder::Decoder(int buf_size, char* buf_data) {
	av_init_packet(&packet);

	int ret, got_picture;
	int cur_size = buf_size;
	uint8_t* cur_ptr = (uint8_t *)buf_data;

	while (cur_size > 0){
		int len = av_parser_parse2(
			pCodecParserCtx, pCodecCtx,
			&packet.data, &packet.size,
			cur_ptr, cur_size,
			AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);

		cur_ptr += len;
		cur_size -= len;

		//printf("Size:%d\n", buf_size);

		if (packet.size == 0)
			continue;

		ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);
		if (ret >= 0 && got_picture) {
			//反转图像
			pFrame->data[0] += pFrame->linesize[0] * (pCodecCtx->height - 1);
			pFrame->linesize[0] *= -1;
			pFrame->data[1] += pFrame->linesize[1] * (pCodecCtx->height / 2 - 1);
			pFrame->linesize[1] *= -1;
			pFrame->data[2] += pFrame->linesize[2] * (pCodecCtx->height / 2 - 1);
			pFrame->linesize[2] *= -1;

			//printf("Succeed to decode 1 frame!\n");
			sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
				pConvertedFrame->data, pConvertedFrame->linesize);

			av_free(buf_data);
			av_free_packet(&packet);

			return pConvertedFrame;
		}
	}
	return NULL;
};