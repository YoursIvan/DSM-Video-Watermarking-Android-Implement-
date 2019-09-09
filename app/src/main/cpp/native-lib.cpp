#include <jni.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <math.h>
#include <cstdlib>
#include <vector>
#include "Wavelet.cpp"
#include "svd.cpp"
#include "BCH.cpp"
#include "Tools.cpp"

using namespace std;

#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"jason",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"jason",FORMAT,##__VA_ARGS__);

extern "C"
{
#include <android/log.h>
#include <libavutil/opt.h>
//封装格式
#include "libavformat/avformat.h"
//解码
#include "libavcodec/avcodec.h"
//缩放
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}

extern "C"
JNIEXPORT void JNICALL
Java_com_xmu_lxq_hello_MainActivity_robust_1embedding(JNIEnv *env, jobject instance, jstring input_,
                                                  jstring output_, jstring message_,jdouble embrate,jdouble qStep) {
    const char *input = env->GetStringUTFChars(input_, 0);
    const char *output = env->GetStringUTFChars(output_, 0);
    const char *messagefile = env->GetStringUTFChars(message_, 0);


    //BCH parameter(15,5,3)
    int ret=0;
    int m = 4;
    int length = 15;
    int t = 3;
    int k = 5;
    string GenPolynom = "10100110111";


    /********** Reading Messagefile ***********/
    ifstream file;
    file.open(messagefile,ios::in);
    string message;
    file>>message;
    int len = message.length();
    int len_bits = len * 8 ;
    string messagebit(len_bits,'0');
    for(int i = 0; i < len; i++)
    {
        int a_ascii = int(message[i]);
        string temp = to_string(decToBin(a_ascii));
        string pad="";
        ostringstream oss;
        temp = padstring(temp,8);

        for (int k = 0; k < 8; k++) {
            messagebit[i * 8 + k] = temp[k];
        }
    }

    //bch_encoding
    int len_bits_pad = ceil(double(len_bits/k)) *k;
    string messagebit_pad(len_bits_pad,'0');
    for (int i = 0; i < messagebit.size(); ++i) {
        messagebit_pad[i] = messagebit[i];
    }

    string messagebitbch(len_bits_pad/k*length,'0');
    for (int i = 0; i < len_bits_pad/k; ++i) {
        string group_bits(k,'0');
        for (int j = 0; j < k; ++j) {
            group_bits[j] = messagebit_pad[i*k + j];
        }
        string group_codes = Bch_Encoding(group_bits,GenPolynom);
        group_codes=padstring(group_codes,length);

        for (int n = 0; n < length; ++n) {
            messagebitbch[i*length + n] = group_codes[n];
        }
    }


    //Variables

    AVRational frame_rate;
    int FrameNum = 0;
    int FrameHeight = 0;
    int FrameWidth = 0;
    int BlockSize = 32;

    /*************** Decoder ***************/
    //1.注册组件
    av_register_all();
    avcodec_register_all();

    //封装格式上下文
    AVFormatContext *pFormatCoctx = avformat_alloc_context();

    //2.打开输入视频文件
    int err_code=avformat_open_input(&pFormatCoctx,input,NULL,NULL);
    if(err_code!= 0){
        LOGE("%s","打开输入视频文件失败");
        return;
    }
    //3.获取视频信息
    if(avformat_find_stream_info(pFormatCoctx,NULL) < 0){
        LOGE("%s","获取视频信息失败");
        return;
    }

    //视频解码，需要找到视频对应的AVStream所在pFormatCoctx->streams的索引位置
    int video_stream_idx = -1;
    int i = 0;
    for(; i < pFormatCoctx->nb_streams;i++){
        //根据类型判断，是否是视频流
        if( pFormatCoctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            video_stream_idx = i;
            break;
        }
    }


    //4.获取视频解码器
    AVCodec *Decodec =  avcodec_find_decoder(pFormatCoctx->streams[video_stream_idx]->codecpar->codec_id);
    AVCodecContext *pDecodeCtx = avcodec_alloc_context3(Decodec);
    avcodec_parameters_to_context(pDecodeCtx,  pFormatCoctx->streams[video_stream_idx]->codecpar);
    AVCodec *pDecodec = avcodec_find_decoder(pDecodeCtx->codec_id);
    if(pDecodec == NULL){
        LOGE("%s","无法解码");
        return;
    }
    //5.打开解码器
    if(avcodec_open2(pDecodeCtx,pDecodec,NULL) < 0){
        LOGE("%s","解码器无法打开");
        return;
    }
    //编码数据
    AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));

    //像素数据（解码数据）
    AVFrame *frame = av_frame_alloc();
    AVFrame *yuvFrame = av_frame_alloc();

    //只有指定了AVFrame的像素格式、画面大小才能真正分配内存
    //缓冲区分配内存
    uint8_t *out_buffer = (uint8_t *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pDecodeCtx->width, pDecodeCtx->height,1));
    //初始化缓冲区
    av_image_fill_arrays(yuvFrame->data, yuvFrame->linesize, out_buffer, AV_PIX_FMT_YUV420P, pDecodeCtx->width, pDecodeCtx->height,1);

    FrameHeight = pDecodeCtx->height;
    FrameWidth = pDecodeCtx->width;
    //用于像素格式转换或者缩放
    struct SwsContext *sws_ctx = sws_getContext(
            pDecodeCtx->width, pDecodeCtx->height, pDecodeCtx->pix_fmt,
            pDecodeCtx->width, pDecodeCtx->height, AV_PIX_FMT_YUV420P,
            SWS_BILINEAR, NULL, NULL, NULL);



    /*************** Encoder ***************/
    AVFormatContext *pFormatDectx = nullptr;
    const char *H264_output = "/sdcard/videokit/temp.264";
    avformat_alloc_output_context2(&pFormatDectx,NULL,NULL,H264_output);
    if (avio_open(&pFormatDectx->pb, H264_output, AVIO_FLAG_READ_WRITE) < 0){
        printf("Failed to open output file! \n");
        return;
    }

    AVStream* video_st = avformat_new_stream(pFormatDectx, 0);

    FrameNum = int(pFormatCoctx->streams[0]->nb_frames);
    video_st->time_base.num = pFormatCoctx->streams[0]->avg_frame_rate.den;
    video_st->time_base.den = pFormatCoctx->streams[0]->avg_frame_rate.num;
    av_dump_format(pFormatDectx, 0, H264_output, 1);

    AVCodec *Codec = avcodec_find_encoder(AV_CODEC_ID_H264);//查找编解码器
    if(NULL == Codec)
    {
        LOGE("%s","找不到编码器");
        return ;
    }

    //分配AVCodecContex实例
    AVCodecContext *pCodeCtx = avcodec_alloc_context3(Codec);
    if(NULL == pCodeCtx)
    {
        LOGE("%s","找不到编码器上下文");
        return ;
    }

    //设置编码器的参数
    pCodeCtx->width = pDecodeCtx->width;//帧高
    pCodeCtx->height = pDecodeCtx->height;
    pCodeCtx->bit_rate = pDecodeCtx ->bit_rate;//比特率

    pCodeCtx->time_base.num=pFormatCoctx->streams[0]->avg_frame_rate.den;
    pCodeCtx->time_base.den=pFormatCoctx->streams[0]->avg_frame_rate.num;
    pCodeCtx->gop_size = pDecodeCtx->gop_size;
    pCodeCtx->max_b_frames = 0;//实时编码
    pCodeCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    //H264
    pCodeCtx->me_range = 16;
    pCodeCtx->max_qdiff = pDecodeCtx->max_qdiff;
    pCodeCtx->qcompress = pDecodeCtx->qcompress;
    pCodeCtx->qmin = 10;
    pCodeCtx->qmax = 51;
    av_opt_set(pCodeCtx->priv_data, "preset", "ultrafast", 0);
    av_opt_set(pCodeCtx->priv_data, "tune", "zerolatency", 0);
    av_opt_set(pCodeCtx->priv_data, "qp", "0", 0);
    AVDictionary *param = 0;

    //打开编码器
    if(avcodec_open2(pCodeCtx, Codec, &param) < 0)
    {
        LOGE("%d",avcodec_open2(pCodeCtx, Codec, &param));
        LOGE("%s","打不开编码器");
        return;
    }
    avcodec_parameters_from_context(video_st->codecpar,pCodeCtx);
    av_dump_format(pFormatDectx,0,H264_output,1);

    //Prepare for embedding message
    int FrameFullCapacity = FrameHeight/BlockSize *FrameWidth/BlockSize;
    int FrameCapacity = int(FrameFullCapacity * embrate);
    int VideoCapacity = FrameCapacity * FrameNum;
    int EncodeMessageLength = len_bits_pad/k * length + 100;
    if(EncodeMessageLength > VideoCapacity)
    {
        LOGE("%s","嵌入失败：消息序列太长");
        return;
    }


    int EmbFrameCount = int(ceil(EncodeMessageLength/FrameCapacity));
    if(EmbFrameCount < FrameNum)
        EmbFrameCount = EmbFrameCount + 1;

    int len_message_pad = EmbFrameCount * FrameCapacity;
    string message_pad(len_message_pad,'0');
    for (int i = 0; i < messagebitbch.length(); i++) {
        message_pad[i] = messagebitbch[i];
    }


    //Write File Header
    avformat_write_header(pFormatDectx,NULL);
    AVPacket pkt;

    int got_frame, framecount = 0;
    string message_block(FrameCapacity,'0');
    //6.一帧一帧读取压缩的视频数据AVPacket
    while(av_read_frame(pFormatCoctx,packet) >= 0)
    {
        if(packet->stream_index==video_stream_idx) {
            //解码AVPacket->AVFrame
            avcodec_send_packet(pDecodeCtx, packet);
            got_frame = avcodec_receive_frame(pDecodeCtx, frame);
            //非零，正在解码
            if (!got_frame) {
                //frame->yuvFrame (YUV420P)
                //转为指定的YUV420P像素帧
                //LOGI("解码%d帧", framecount);
                sws_scale(sws_ctx,
                          frame->data, frame->linesize, 0, frame->height,
                          yuvFrame->data, yuvFrame->linesize);


                //向YUV文件保存解码之后的帧数据
                int y_size = pDecodeCtx->width * pDecodeCtx->height;
                //fwrite(yuvFrame->data[0], 1, y_size, fp_yuv);
                //fwrite(yuvFrame->data[1], 1, y_size/4, fp_yuv);
                //fwrite(yuvFrame->data[2], 1, y_size/4, fp_yuv);



                /******* Embeding yuvFrame->data[0] Here *******/
                if(framecount < EmbFrameCount) {
                    LOGI("Embedding:%d帧", framecount);
                    //Initialize embedded message
                    for (int i = 0; i < FrameCapacity; i++) {
                        message_block[i] = message_pad[framecount * FrameCapacity + i];
                    }
                    uint8_t *y_data = yuvFrame->data[0];

                    //De-block
                    int BlockRows = FrameHeight / BlockSize;
                    int BlockCols = FrameWidth / BlockSize;

                    vector<vector<double> > BlockData(BlockSize, vector<double>(BlockSize));
                    vector<vector<double> > ReconData(BlockSize, vector<double>(BlockSize));
                    vector<vector<double> > LL(BlockSize / 2, vector<double>(BlockSize / 2));
                    vector<vector<double> > LH(BlockSize / 2, vector<double>(BlockSize / 2));
                    vector<vector<double> > HL(BlockSize / 2, vector<double>(BlockSize / 2));
                    vector<vector<double> > HH(BlockSize / 2, vector<double>(BlockSize / 2));
                    vector<vector<double> > U(BlockSize / 2, vector<double>(BlockSize / 2));
                    vector<vector<double> > S_Mat(BlockSize / 2, vector<double>(BlockSize / 2));
                    vector<double> S(BlockSize / 2);
                    vector<vector<double> > V(BlockSize / 2, vector<double>(BlockSize / 2));
                    vector<vector<double> > Temp(BlockSize / 2, vector<double>(BlockSize / 2));
                    vector<vector<double> > ReconLL(BlockSize / 2, vector<double>(BlockSize / 2));
                    vector<int> randnum = randperm(FrameFullCapacity, framecount);

                    for (int n = 0; n < FrameCapacity; n++)
                    {
                        int row_index = randnum[n]/ BlockCols;
                        int col_index = randnum[n]% BlockCols;
                        for (int x = 0; x < BlockSize; x++)
                        {
                            for (int y = 0; y < BlockSize; y++)
                            {
                                BlockData[x][y] = double(y_data[(row_index * BlockSize + x) * FrameWidth + col_index * BlockSize + y]);
                            }
                        }
                        //DWT-SVD Modulation
                        Wavelet* Wl = new Wavelet;
                        Filter* db = new Filter(1);
                        Wl->dwt2(BlockData,BlockSize, *db, LL, LH, HL, HH);
                        svd(LL, BlockSize / 2, U, S, V);
                        int MessageBit = message_block[n] - '0';
                        S[0] = floor(S[0] / qStep) * qStep +
                               (int(floor(S[0] / qStep + MessageBit)) % 2) * qStep;
                        //Reconstruct
                        for (int x = 0; x < BlockSize / 2; x++)
                            S_Mat[x][x] = S[x];

                        MatMul(MatInv(U),S_Mat,Temp);
                        MatMul(Temp,V,ReconLL);
                        ReconData = Wl->idwt2(ReconLL, LH, HL, HH, BlockSize,*db);

                        for (int x = 0; x < BlockSize; x++)
                        {
                            for (int y = 0; y < BlockSize; y++)
                            {
                                y_data[(row_index * BlockSize + x) * FrameWidth + col_index * BlockSize + y] = uint8_t(ReconData[x][y]);
                            }
                        }

                    }
                    yuvFrame->data[0] = y_data;
                }


                /******* Encoder After Embedding *******/

                //Deframe->data[0] = yuvFrame->data[0];
                //Deframe->data[1] = yuvFrame->data[1];
                //Deframe->data[2] = yuvFrame->data[2];
                //Deframe->pts = framecount;

                yuvFrame->pts = framecount;
                //Deframe-> pts = framecount * (video_st->time_base.den) / ((video_st->time_base.num) * 25);
                av_init_packet(&pkt);
                avcodec_send_frame(pCodeCtx, (const AVFrame *) yuvFrame);
                int got_packet = avcodec_receive_packet(pCodeCtx, &pkt);
                if (!got_packet) {
                    //LOGI("编码输出%d帧", framecount);
                    framecount++;
                    //size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
                    pkt.stream_index = video_st->index;
                    av_packet_rescale_ts(&pkt, pCodeCtx->time_base, video_st->time_base);
                    frame_rate.den = pCodeCtx->time_base.num;
                    frame_rate.num = pCodeCtx->time_base.den;
                    //av_write_frame(pFormatDectx, &pkt);
                    pkt.pos = -1;
                    av_interleaved_write_frame(pFormatDectx,&pkt);
                    av_packet_unref(&pkt);
                }
            }
        }
        av_packet_unref(packet);
    }

    av_write_trailer(pFormatDectx);
    avio_close(pFormatDectx->pb);


    /*************** encapsulate h264 to mp4 ***************/
    AVOutputFormat *ofmt = NULL;
    //Input AVFormatContext and Output AVFormatContext
    AVFormatContext *ifmt_ctx_v = NULL, *ifmt_ctx_a = NULL,*ofmt_ctx = NULL;
    int videoindex_v=0,videoindex_out=0;
    int frame_index=0;
    int64_t cur_pts_v=0,cur_pts_a=0;

    av_register_all();
    //Input
    if ((ret = avformat_open_input(&ifmt_ctx_v, H264_output, 0, 0)) < 0) {
        LOGI( "Could not open input file.");
        goto end;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx_v, 0)) < 0) {
        LOGI( "Failed to retrieve input stream information");
        goto end;
    }

    av_dump_format(ifmt_ctx_v, 0, H264_output, 0);
    //Output
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, output);
    if (!ofmt_ctx) {
        LOGI( "Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    ofmt = ofmt_ctx->oformat;
    LOGI("ifmt_ctx_v->nb_streams=%d\n",ifmt_ctx_v->nb_streams);
    for (i = 0; i < ifmt_ctx_v->nb_streams; i++) {
        //Create output AVStream according to input AVStream
        //if(ifmt_ctx_v->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            AVStream *in_stream = ifmt_ctx_v->streams[i];
            AVCodecContext* pCodecCtx = avcodec_alloc_context3(NULL);
            avcodec_parameters_to_context(pCodecCtx,in_stream->codecpar);
            AVCodec *Mcodec = avcodec_find_decoder(pCodecCtx->codec_id);
            AVStream *out_stream = avformat_new_stream(ofmt_ctx, Mcodec);
            videoindex_v=i;
            videoindex_out=out_stream->index;
            //Copy the settings of AVCodecContext
            if (avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar) < 0) {
                LOGI( "Failed to copy context from input to output stream codec context\n");
                goto end;
            }
            out_stream->codecpar->codec_tag = 0;
            //if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            //    out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
            //break;
        }
    }

    av_dump_format(ofmt_ctx, 0, output, 1);
    //Open output file
    if (!(ofmt->flags & AVFMT_NOFILE)) {
        if (avio_open(&ofmt_ctx->pb, output, AVIO_FLAG_WRITE) < 0) {
            LOGI( "Could not open output file '%s'", output);
            goto end;
        }
    }
    //Write file header
    if (avformat_write_header(ofmt_ctx, NULL) < 0) {
        LOGI( "Error occurred when opening output file\n");
        goto end;
    }
    //FIX
#if USE_H264BSF
    AVBitStreamFilterContext* h264bsfc =  av_bitstream_filter_init("h264_mp4toannexb");
#endif
#if USE_AACBSF
    AVBitStreamFilterContext* aacbsfc =  av_bitstream_filter_init("aac_adtstoasc");
#endif
    while (1) {
        AVFormatContext *ifmt_ctx;
        int stream_index=0;
        AVStream *in_stream, *out_stream;
        //Get an AVPacket
        //if(av_compare_ts(cur_pts_v,ifmt_ctx_v->streams[videoindex_v]->time_base,cur_pts_a,ifmt_ctx_a->streams[audioindex_a]->time_base) <= 0)
        {

            ifmt_ctx=ifmt_ctx_v;
            stream_index=videoindex_out;
            if(av_read_frame(ifmt_ctx, &pkt) >= 0){
                do{
                    in_stream  = ifmt_ctx->streams[pkt.stream_index];
                    out_stream = ofmt_ctx->streams[stream_index];
                    if(pkt.stream_index==videoindex_v){
                        //FIX：No PTS (Example: Raw H.264)
                        //Simple Write PTS
                        if(pkt.pts==AV_NOPTS_VALUE){
                            LOGI("frame_index==%d\n",frame_index);
                            //Write PTS
                            in_stream->r_frame_rate = frame_rate;
                            AVRational time_base1=in_stream->time_base;
                            //Duration between 2 frames (us)
                            int64_t calc_duration=(double)AV_TIME_BASE/av_q2d(in_stream->r_frame_rate);
                            //Parameters
                            pkt.pts=(double)(frame_index*calc_duration)/(double)(av_q2d(time_base1)*AV_TIME_BASE);
                            pkt.dts=pkt.pts;
                            pkt.duration=(double)calc_duration/(double)(av_q2d(time_base1)*AV_TIME_BASE);
                            frame_index++;
                        }
                        cur_pts_v=pkt.pts;
                        break;
                    }
                }while(av_read_frame(ifmt_ctx, &pkt) >= 0);
            }else{
                break;
            }
        }
#if USE_H264BSF
        av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif
#if USE_AACBSF
        av_bitstream_filter_filter(aacbsfc, out_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif
        //Convert PTS/DTS
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        pkt.stream_index=stream_index;
        LOGI("Write 1 Packet. size:%5d\tpts:%ld\n",pkt.size,pkt.pts);
        //Write
        if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
            printf( "Error muxing packet\n");
            break;
        }
        av_packet_unref(&pkt);
    }
    //Write file trailer
    av_write_trailer(ofmt_ctx);
#if USE_H264BSF
    av_bitstream_filter_close(h264bsfc);
#endif
#if USE_AACBSF
    av_bitstream_filter_close(aacbsfc);
#endif
    end:
    avformat_close_input(&ifmt_ctx_v);
    //avformat_close_input(&ifmt_ctx_a);
    /* close output */
    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
        avio_close(ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
    if (ret < 0 && ret != AVERROR_EOF) {
        printf( "Error occurred.\n");
        return;
    }


    avcodec_flush_buffers(pCodeCtx);
    avcodec_flush_buffers(pDecodeCtx);
    av_frame_free(&frame);
    av_frame_free(&yuvFrame);
    avcodec_close(pDecodeCtx);
    avcodec_close(pCodeCtx);
    avformat_free_context(pFormatCoctx);
    avformat_free_context(pFormatDectx);
    // TODO
    av_free(pDecodeCtx);
    av_free(pCodeCtx);
    //av_freep(&Deframe->data);
    //av_frame_free(&Deframe);

    env->ReleaseStringUTFChars(input_, input);
    env->ReleaseStringUTFChars(output_, output);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_xmu_lxq_hello_MainActivity_robust_1extracting(JNIEnv *env, jobject instance, jstring input_,
                                                      jstring messageoutput_,jdouble embrate,jdouble qStep) {

    const char *input = env->GetStringUTFChars(input_, 0);
    const char *messagefile = env->GetStringUTFChars(messageoutput_, 0);

    //Variables
    int FrameNum = 0;
    int FrameHeight = 0;
    int FrameWidth = 0;
    int BlockSize = 32;
    string GenPolynom = "10100110111";

    /*************** Decoder ***************/
    //1.注册组件
    av_register_all();
    avcodec_register_all();

    //封装格式上下文
    AVFormatContext *pFormatCoctx = avformat_alloc_context();

    //2.打开输入视频文件
    int err_code=avformat_open_input(&pFormatCoctx,input,NULL,NULL);
    if(err_code!= 0){
        LOGE("%s","打开输入视频文件失败");
        return;
    }
    //3.获取视频信息
    if(avformat_find_stream_info(pFormatCoctx,NULL) < 0){
        LOGE("%s","获取视频信息失败");
        return;
    }

    //视频解码，需要找到视频对应的AVStream所在pFormatCoctx->streams的索引位置
    int video_stream_idx = -1;
    int i = 0;
    for(; i < pFormatCoctx->nb_streams;i++){
        //根据类型判断，是否是视频流
        if( pFormatCoctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            video_stream_idx = i;
            break;
        }
    }

    //4.获取视频解码器
    AVCodec *Decodec =  avcodec_find_decoder(pFormatCoctx->streams[video_stream_idx]->codecpar->codec_id);
    AVCodecContext *pDecodeCtx = avcodec_alloc_context3(Decodec);
    avcodec_parameters_to_context(pDecodeCtx,  pFormatCoctx->streams[video_stream_idx]->codecpar);
    AVCodec *pDecodec = avcodec_find_decoder(pDecodeCtx->codec_id);
    if(pDecodec == NULL){
        LOGE("%s","无法解码");
        return;
    }
    //5.打开解码器
    if(avcodec_open2(pDecodeCtx,pDecodec,NULL) < 0){
        LOGE("%s","解码器无法打开");
        return;
    }
    //编码数据
    AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    //像素数据（解码数据）
    AVFrame *frame = av_frame_alloc();
    AVFrame *yuvFrame = av_frame_alloc();

    //只有指定了AVFrame的像素格式、画面大小才能真正分配内存
    //缓冲区分配内存
    uint8_t *out_buffer = (uint8_t *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pDecodeCtx->width, pDecodeCtx->height,1));
    //初始化缓冲区
    av_image_fill_arrays(yuvFrame->data, yuvFrame->linesize, out_buffer, AV_PIX_FMT_YUV420P, pDecodeCtx->width, pDecodeCtx->height,1);

    FrameHeight = pDecodeCtx->height;
    FrameWidth = pDecodeCtx->width;
    //用于像素格式转换或者缩放
    struct SwsContext *sws_ctx = sws_getContext(
            pDecodeCtx->width, pDecodeCtx->height, pDecodeCtx->pix_fmt,
            pDecodeCtx->width, pDecodeCtx->height, AV_PIX_FMT_YUV420P,
            SWS_BILINEAR, NULL, NULL, NULL);

    //Prepare for embedding message
    int FrameFullCapacity = FrameHeight/BlockSize *FrameWidth/BlockSize;
    int FrameCapacity = int(FrameFullCapacity * embrate);
    int VideoCapacity = FrameCapacity * FrameNum;

    //Write File Header
    AVPacket pkt;
    int got_frame, framecount = 0;
    vector<char> bit_array;
    //6.一帧一帧读取压缩的视频数据AVPacket
    while(av_read_frame(pFormatCoctx,packet) >= 0)
    {
        if(packet->stream_index==video_stream_idx) {
            //解码AVPacket->AVFrame
            avcodec_send_packet(pDecodeCtx, packet);
            got_frame = avcodec_receive_frame(pDecodeCtx, frame);
            //非零，正在解码
            if (!got_frame) {
                //LOGI("解码%d帧", framecount);
                sws_scale(sws_ctx,
                          frame->data, frame->linesize, 0, frame->height,
                          yuvFrame->data, yuvFrame->linesize);


                //向YUV文件保存解码之后的帧数据
                int y_size = pDecodeCtx->width * pDecodeCtx->height;

                uint8_t *y_data = yuvFrame->data[0];

                //De-block
                int BlockRows = FrameHeight / BlockSize;
                int BlockCols = FrameWidth / BlockSize;

                vector<vector<double> > BlockData(BlockSize, vector<double>(BlockSize));
                vector<vector<double> > LL(BlockSize / 2, vector<double>(BlockSize / 2));
                vector<vector<double> > LH(BlockSize / 2, vector<double>(BlockSize / 2));
                vector<vector<double> > HL(BlockSize / 2, vector<double>(BlockSize / 2));
                vector<vector<double> > HH(BlockSize / 2, vector<double>(BlockSize / 2));
                vector<vector<double> > U(BlockSize / 2, vector<double>(BlockSize / 2));
                vector<double> S(BlockSize / 2);
                vector<vector<double> > V(BlockSize / 2, vector<double>(BlockSize / 2));
                vector<int> randnum = randperm(FrameFullCapacity, framecount);

                LOGI("Extracting:%d帧", framecount++);

                for (int n = 0; n < FrameCapacity; n++)
                {
                    int row_index = randnum[n]/ BlockCols;
                    int col_index = randnum[n]% BlockCols;
                    for (int x = 0; x < BlockSize; x++)
                    {
                        for (int y = 0; y < BlockSize; y++)
                        {
                            BlockData[x][y] = double(y_data[(row_index * BlockSize + x) * FrameWidth + col_index * BlockSize + y]);
                        }
                    }
                    //DWT-SVD Modulation
                    Wavelet* Wl = new Wavelet;
                    Filter* db = new Filter(1);
                    Wl->dwt2(BlockData,BlockSize, *db, LL, LH, HL, HH);
                    svd(LL, BlockSize / 2, U, S, V);
                    bit_array.push_back(char('0' + int(floor(S[0] / qStep + 0.5))%2)) ;
                }
                int ZeroCount = 0;
                if (bit_array.size() > 100)
                {
                    for (int l = bit_array.size() - 1; l >= bit_array.size() - 100; l--)
                    {
                        if (bit_array[l] == '0')
                            ZeroCount++;
                    }
                    if (framecount > 30)
                        break;
                }
            }
        }
        av_packet_unref(packet);
    }

    //bch_decoding
    int m = 4;
    int length = 15;
    int t = 3;
    int k = 5;
    int padlen = ceil(double(bit_array.size())/length) * length;
    string bit_array_pad(padlen,'0');
    string messagebits(padlen/length*k, '0');
    for (int i = 0; i < bit_array.size(); ++i) {
        bit_array_pad[i] = bit_array[i];
    }

    for (int i = 0; i < padlen/length; ++i) {
        string group_codes(length,'0');
        for (int j = 0; j < length; ++j) {
            group_codes[j] = bit_array_pad[i*length + j];
        }
        int flag = 0;
        string group_bits = Bch_Correcting(group_codes,GenPolynom,flag);
        group_bits = padstring(group_bits,15);
        if (flag == 1)
            group_bits = Bch_Decoding(group_codes);
        else
            group_bits = Bch_Decoding(group_bits);


        for (int n = 0; n < k; ++n) {
            messagebits[i*k + n] = group_bits[n];
        }
    }

    string messagebits_pad(ceil(double(messagebits.size())/8)*8, '0');
    for (int i = 0; i < messagebits.size(); ++i) {
        messagebits_pad[i] = messagebits[i];
    }

    string ExtMessage = "";
    for (int i = 0; i < int(messagebits_pad.size())/8 ; ++i) {
        vector<int> ascii_bits(8,0);
        for (int j = 0; j < 8; ++j) {
            ascii_bits[j] = messagebits_pad[i*8+j] - '0';
        }
        ExtMessage += BinTodec(ascii_bits);
    }

    //coding


    //writing files
    ofstream writefile;
    writefile.open(messagefile);
    writefile << ExtMessage;
    writefile.close();

    double ber = BER("/sdcard/videokit/message.txt",messagefile);
    LOGI("BER:%f", ber);

    avcodec_flush_buffers(pDecodeCtx);
    av_frame_free(&frame);
    av_frame_free(&yuvFrame);
    avcodec_close(pDecodeCtx);
    avformat_free_context(pFormatCoctx);
    // TODO
    av_free(pDecodeCtx);


    env->ReleaseStringUTFChars(input_, input);
    env->ReleaseStringUTFChars(messageoutput_, messagefile);
}