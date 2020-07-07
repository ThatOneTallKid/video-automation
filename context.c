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
	context->filename = filename;
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
	int64_t duration = vid->duration;
	printf("%ld\n", (duration/AV_TIME_BASE));
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
{ printf("pass 1\n" );
	AVFormatContext *vid = context->fmt_cntx;
	int ret ,stream_idx, refcount = 0;
	AVCodecContext *dec_cntx ;
	AVCodec *dec = NULL;
	AVStream *st = context->stream;
	AVDictionary *opts = NULL;
	//AVCodecParameters *plocal = NULL;

  printf("pass 2\n" );
	ret = av_find_best_stream(vid, type, -1 , -1, NULL , 0);
	if(ret < 0)
	{
		fprintf(stderr, "Could not find %s stream in input file '%s'\n",
						av_get_media_type_string(type), filename );

		return;
	} else {
		//int stream_id = context->video_stream_id ;
		printf("pass 3\n" );
		stream_idx = ret;
		st = vid->streams[*stream_id];
		//dec_cntx = st->codec;
		//dec_cntx = avcodec_alloc_context3(dec);
		printf("pass 4\n" );
		//plocal = vid->streams[ret]->codecpar;
		//dec = avcodec_find_decoder(plocal->codec_id);
		printf("pass 5\n" );
		//if(!dec){
			//fprintf(stderr, "Failed to find %s codec \n", av_get_media_type_string(type));
			//return;
		//}
		printf("pass 6\n" );
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
		} else {
			dec_cntx->time_base = vid->streams[ret]->time_base;
			context->video = dec;
			context->video_codec_cntx = dec_cntx;
			context->video_stream_id =ret;

			return;
		}
		//*stream_id = stream_idx;
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

void close_video_context(VideoContext *vc) {

        avcodec_free_context(&(vc->video_codec_cntx));
    //    avcodec_free_context(&(vc->audio_codec_ctx));
        avformat_close_input(&(vc->fmt_cntx));
    //    vc->open = false;
        printf("CLOSE VIDEO CONTEXT [%s]\n", vc->filename);

}

/**
 * Free all videoContext data (including url)
 * @param vid_ctx VideoContext
 */
void free_video_context(VideoContext **vc) {
    if(vc == NULL || *vc == NULL) {
        return;
    }
    close_video_context(*vc);
    /*if((*vc)->filename != NULL) {
        free((*vc)->filename);
        (*vc)->filename = NULL;
    }*/
    free(*vc);
    *vc = NULL;

}
void print_codec_context(AVCodecContext *c) {
    char buf[1024], channel_layout[100];
    buf[0] = 0;
    channel_layout[0] = 0;

    // https://www.ffmpeg.org/doxygen/trunk/group__channel__mask__c.html
    av_get_channel_layout_string(channel_layout, 100, av_get_channel_layout_nb_channels(c->channel_layout), c->channel_layout);

    sprintf(buf, "codec_type: %s\ncodec_id: %s\ncodec_tag: %d\nbit_rate: %ld\nbit_rate_tolerance: %d\nglobal_quality: %d\ncompression_level: %d\nflags: %d\nflags2: %d\nextradata_size: %d\ntime_base: %d/%d\nticks_per_frame: %d\ndelay: %d\nwidth: %d\nheight: %d\ncoded_width: %d\ncoded_height: %d\ngop_size: %d\npix_fmt: %s\ncolorspace: %s\ncolor_range: %s\nchroma_sample_location: %s\nslices: %d\nfield_order: %d\nsample_rate: %d\nchannels: %d\nsample_fmt: %s\nframe_size: %d\nframe_number: %d\nblock_align: %d\ncutoff: %d\nchannel_layout: %s\nrequest_channel_layout: %ld\n",
    av_get_media_type_string(c->codec_type), avcodec_get_name(c->codec_id), c->codec_tag, c->bit_rate, c->bit_rate_tolerance, c->global_quality, c->compression_level, c->flags, c->flags2, c->extradata_size, c->time_base.num, c->time_base.den, c->ticks_per_frame, c->delay, c->width, c->height, c->coded_width, c->coded_height, c->gop_size, av_get_pix_fmt_name(c->pix_fmt), av_get_colorspace_name(c->colorspace), av_color_range_name(c->color_range), av_chroma_location_name(c->chroma_sample_location), c->slices, c->field_order, c->sample_rate, c->channels, av_get_sample_fmt_name(c->sample_fmt), c->frame_size, c->frame_number, c->block_align, c->cutoff, channel_layout, c->request_channel_layout);
    char *str = malloc(sizeof(char) * (strlen(buf) + 1));
    strcpy(str, buf);
    printf("%s\n" , str);
}

int main(int argc, char *argv[])
{
	VideoContext *vid = malloc(sizeof(VideoContext));
	enum AVMediaType type = AVMEDIA_TYPE_VIDEO;
	//char *filename ;
//	int *stream_id = vid->video_stream_id;
	//strcpy(filename, argv[1]);
	init_video(vid);
	av_register_all();
	//AVFormatContext *vid = avformat_alloc_context();
   	//avcodec_register_all();
	open_format_context(vid, argv[1]);
	print_format_context(vid, argv[1]);
	//open_codec_context(&vid->video_stream_id,vid, type, argv[1]);
	//close_video_context(vid);
	free_video_context(&vid);
//	print_codec_context(vid->fmt_cntx);
	return 0;
}
