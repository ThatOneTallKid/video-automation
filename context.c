#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <libavcodec/avcodec.h>
#include <stdio.h>
#include <sys/stat.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libavutil/avutil.h>


typedef struct VideoContext{

AVFormatContext *fmt_cntx;
AVCodec *video;
AVCodecContext *video_codec_cntx;
int video_stream_id; //index of stream in fmt_cntx->streams[stream_index]
enum PacketStreamType last_decoder_packet_stream;
char *filename;
int clip_cnt;
double fps;
int64_t curr_pts;
int64_t seek_pts;
}VideoContext;


void init_video(VideoContext *vid){
	vid->fmt_cntx = NULL:
	vid->video = NULL;
	vid->video_codec_cntx = NULL:
	vid->video_stream_id = -1;
	vid->last_decoder_packet_stream = DEC_STREAM_NONE;
	vid->filename = NULL:
	vid->clip_cnt = 0;
	vid->fps = 0;
	vid->curr_pts = 0;
	vid->seek_pts 0;
	
}