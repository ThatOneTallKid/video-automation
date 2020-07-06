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
//enum PacketStreamType last_decoder_packet_stream;
char *filename;
int clip_cnt;
double fps;
int64_t curr_pts;
int64_t seek_pts;


}VideoContext;



void init_video(VideoContext *vid){
	vid->fmt_cntx = avformat_alloc_context();
	vid->video = NULL;
	vid->video_codec_cntx = NULL;
	vid->video_stream_id = -1;
	//vid->last_decoder_packet_stream = DEC_STREAM_NONE;
	vid->filename = NULL;
	vid->clip_cnt = 0;
	vid->fps = 0;
	vid->curr_pts = 0;
	vid->seek_pts = 0;

}

/*void open_format_context(AVFormatContext *vid,char *filename){
	char buf[256];
	int ret = avformat_open_input(&vid,filename,NULL,NULL);
	if(ret != 0)
	{
		fprintf(stderr, "ERROR: Could not open %s\n", filename);
		av_strerror(ret, buf,256);
		printf("%s\n",buf);
		printf("%d\n", ret);
		return ;
	}
	if(avformat_find_stream_info(vid, NULL) != 0)
	{
		fprintf(stderr, "Could not find stream information about the file %s\n", filename);
		return;
	}

}*/
void open_format_context(VideoContext *context,char *filename){
	char buf[256];
	AVFormatContext *vid = context->fmt_cntx;
	int ret = avformat_open_input(&vid,filename,NULL,NULL);
	if(ret != 0)
	{
		fprintf(stderr, "ERROR: Could not open %s\n", filename);
		av_strerror(ret, buf,256);
		printf("%s\n",buf);
		printf("%d\n", ret);
		return ;
	}
	if(avformat_find_stream_info(vid, NULL) != 0)
	{
		fprintf(stderr, "Could not find stream information about the file %s\n", filename);
		return;
	}

}
/*
void print_video_context(AVFormatContext *vid,char *filename)
{
	printf("format: %s\n", vid->iformat->name);
	printf("duration: %ld\n", vid->duration);
	printf("bit rate: %ld\n", vid->bit_rate);
	printf("start time: %ld\n", vid->start_time);
}
*/
void print_video_context(VideoContext *context,char *filename)
{	
	AVFormatContext *vid = context->fmt_cntx;
	printf("format: %s\n", vid->iformat->name);
	printf("duration: %ld\n", vid->duration);
	printf("bit rate: %ld\n", vid->bit_rate);
	printf("start time: %ld\n", vid->start_time);
}


int main(int argc, char *argv[])
{   
	VideoContext *vid = malloc(sizeof(VideoContext));
	//char *filename ;
	
	//strcpy(filename, argv[1]);
	init_video(vid);
	//AVFormatContext *vid = avformat_alloc_context();
	av_register_all();
	open_format_context(vid, argv[1]);
	print_video_context(vid, argv[1]);
	
	return 0;
}