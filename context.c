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
AVStream *stream;
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
	vid->stream = NULL;
	vid->video_codec_cntx = NULL;
	vid->video_stream_id = -1;
	//vid->last_decoder_packet_stream = DEC_STREAM_NONE;
	vid->filename = NULL;
	vid->clip_cnt = 0;
	vid->fps = 0;
	vid->curr_pts = 0;
	vid->seek_pts = 0;

}

// getting the info about the video
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
// printing the info
void print_format_context(VideoContext *context,char *filename)
{
	AVFormatContext *vid = context->fmt_cntx;
	printf("format: %s\n", vid->iformat->name);
	printf("duration: %ld\n", vid->duration);
	printf("bit rate: %ld\n", vid->bit_rate);
	printf("start time: %ld\n", vid->start_time);
}



void open_codec_context(int *stream_id,VideoContext *context, enum AVMediaType type, char *filename)
{
	AVFormatContext *vid = context->fmt_cntx;
	int ret ,stream_idx, refcount = 0;
	AVCodecContext *dec_cntx = context->video_codec_cntx;
	AVCodec *dec = context->video;
	AVStream *st = context->stream;
	AVDictionary *opts = NULL;

	ret = av_find_best_stream(vid, type, -1 , -1, NULL , 0);
	if(ret < 0)
	{
		fprintf(stderr, "Could not find %s stream in input file '%s'\n",
						av_get_media_type_string(type), filename );

		return;
	} else {
		//int stream_id = context->video_stream_id ;
		stream_idx = ret;
		st = vid->streams[*stream_id];
		//dec_cntx = st->codec;
		//dec_cntx = avcodec_alloc_context3(dec);
		dec = avcodec_find_decoder(st->codecpar->codec_id);
		if(!dec){
			fprintf(stderr, "Failed to find %s codec \n", av_get_media_type_string(type));
			return;
		}

		//avcodec_parameters_to_context(dec_cntx, vid->streams[ret]->codecpar);
		dec_cntx = avcodec_alloc_context3(dec);
		if(!dec_cntx){
			fprintf(stderr, "Failed to allocate the %s codec context \n", av_get_media_type_string(type));
			return;
		}

		if((ret = avcodec_parameters_to_context(dec_cntx, st->codecpar)) < 0){
			fprintf(stderr, "Failed to copy %s codec parameters to decoder centext \n", av_get_media_type_string(type));
			return;
		}
		av_dict_set(&opts, "refcounted_frames",refcount ? "1" : "0", 0);
		if ((ret = avcodec_open2(dec_cntx, dec , &opts)) < 0){
			fprintf(stderr, "Failed to open %s codec \n",
		 					av_get_media_type_string(type));

			return;
		} /*else {
			dec_cntx->time_base = vid->streams[ret]->time_base;
			context->video = dec;
			context->video_codec_cntx = dec_cntx;
			context->video_stream_id =ret;

			return;
		}*/
		*stream_id = stream_idx;
	}
printf("\n Success");
}



AVStream *get_video_stream(VideoContext *context){
	int index = context->video_stream_id;
	if(index == -1 ){
		return NULL;
	}

	return context->fmt_cntx->streams[index];
}

int main(int argc, char *argv[])
{
	VideoContext *vid = malloc(sizeof(VideoContext));
	enum AVMediaType type = AVMEDIA_TYPE_VIDEO;
	//char *filename ;
//	int *stream_id = vid->video_stream_id;
	//strcpy(filename, argv[1]);
	init_video(vid);
	//AVFormatContext *vid = avformat_alloc_context();
	av_register_all();
	//avcodec_register_all();
	open_format_context(vid, argv[1]);
	print_format_context(vid, argv[1]);
	open_codec_context(&vid->video_stream_id,vid, type, argv[1]);

	return 0;
}
