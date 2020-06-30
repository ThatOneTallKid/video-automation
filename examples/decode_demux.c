/*
@author: Aditya Das, Rupanshi Chawda
Copyright (c) 2020 Aditya Das
@Date: June 29, 2020
@Description:



*/
/*
@file
decode_demux.c
*/

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>

static AVFormatContext *fmt_cntx = NULL;
static AVStream *V_stream = NULL, *A_stream = NULL;
static AVPacket pkt;
static AVFrame *frame = NULL;
static AVCodecContext *v_dec_cntx = NULL, *A_dec_cntx = NULL;
static int width, height;
static const char *src_filename = NULL;
static const char *video_dst_filename = NULL;
static FILE *video_dst_file = NULL;

static enum AVPixelFormat pix_format;
static int vframe_cnt = 0;
static uint8_t *video_dst_data[4] = {NULL};
static int      video_dst_linesize[4];
static int video_dst_bufsize;

static int video_stream_idx = -1;


static int refcount = 0;

static int output_vdo_frame(AVFrame *frame)
{
    if (frame->width != width || frame->height != height ||
        frame->format != pix_format){
          /* To handle this change, one could call av_image_alloc again and
           * decode the following frames into another rawvideo file*/
           fprintf(stderr,"Error: Width, height and pixel format have to be "
                "constant in a rawvideo file, but the width, height or "
                "pixel format of the input video changed:\n"
                "old: width = %d, height = %d, format = %s\n"
                "new: width = %d, height = %d, format = %s\n",
                width, height, av_get_pix_fmt_name(pix_format),
                frame->width, frame->height,
                av_get_pix_fmt_name(frame->format));

            return -1;
        }
      printf (" video_frame:%d coded_n:%d\n",
              vframe_cnt++, frame->coded_picture_number);

      /*
       copy decoded frame to destination buffer:
       this is required since rawvideo
        expects non aligned data
      */
      av_image_copy(video_dst_data, video_dst_linesize,
                    (const uint8_t **)(frame->data), frame->linesize,
                  pix_format, width , height);


      // write to rawvideo file
      fwrite(video_dst_data[0], 1, video_dst_bufsize, video_dst_file);
      return 0;

}

static int decode_packet(AVCodecContext *Pack, AVPacket * pkt){
  int response = 0;

//Submit or supply raw packets data as input to the decoder
  response = avcodec_send_packet(Pack,pkt);
  if (response < 0){
      fprintf(stderr, "Error while sending a packet to the decoder: (%s)\n", av_err2str(response));
      return 0;
  }
  //

  // get all the available frames from the decoder
  while(response >= 0){
    response = avcodec_receive_frame(Pack, frame);
    if (response < 0){
      if (response == AVERROR_EOF || response == AVERROR(EAGAIN))
        return 0;
        // those two retuen values are special and mean there is no output
        // frame available, but there were no errors during decoding
      fprintf(stderr, "Error during decoding (%s)\n", av_err2str(response));
      return response;
    }

    // write frame data to the output file
    //if ( Pack->codec->type == AVMEDIA_TYPE_VIDEO)
      response = output_vdo_frame(frame);
    //else
     //response = output_ado_frame(frame);

      av_frame_unref(frame);
      if(response < 0)
        return response;
  }
  return 0;
}

static int open_coded_context(int *stream_idx, AVCodecContext **dec_ctx,
                              AVFormatContext *fmt_cntx,
                            enum AVMediaType type)
{
      int response, stream_index;
      AVStream *st;
      AVCodec *dec = NULL;
      AVDictionary *opts = NULL;

      response = av_find_best_stream(fmt_cntx, type, -1, -1, NULL, 0);
      if (response < 0){
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(type), src_filename);
        return response;

      } else {
        stream_index = response;
        st = fmt_cntx->streams[stream_index];

        /* find decoder for the stream */
        dec = avcodec_find_decoder(st->codecpar->codec_id);
        if(!dec) {
          fprintf(stderr, "Failed to find %s codec\n",
                  av_get_media_type_string(type));

        return AVERROR(EINVAL);
        }

        /* Allocate a codec context for the decoder */
        *dec_ctx = avcodec_alloc_context3(dec);
        if (!*dec_ctx) {
            fprintf(stderr, "Failed to allocate the %s codec context\n",
                    av_get_media_type_string(type));
            return AVERROR(ENOMEM);
        }

        /* Copy codec parameters from input stream to output codec context */
        if ((response = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
            fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                    av_get_media_type_string(type));
            return response;
        }

        /* Init the decoders */
        if ((response = avcodec_open2(*dec_ctx, dec, &opts)) < 0) {
            fprintf(stderr, "Failed to open %s codec\n",
                    av_get_media_type_string(type));
            return response;
        }
        *stream_idx = stream_index;
      }

      return 0;
}
static int get_format_from_sample_fmt(const char **fmt,
                                      enum AVSampleFormat sample_fmt)
{
    int i;
    struct sample_fmt_entry {
        enum AVSampleFormat sample_fmt; const char *fmt_be, *fmt_le;
    } sample_fmt_entries[] = {
        { AV_SAMPLE_FMT_U8,  "u8",    "u8"    },
        { AV_SAMPLE_FMT_S16, "s16be", "s16le" },
        { AV_SAMPLE_FMT_S32, "s32be", "s32le" },
        { AV_SAMPLE_FMT_FLT, "f32be", "f32le" },
        { AV_SAMPLE_FMT_DBL, "f64be", "f64le" },
    };
    *fmt = NULL;

    for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++) {
        struct sample_fmt_entry *entry = &sample_fmt_entries[i];
        if (sample_fmt == entry->sample_fmt) {
            *fmt = AV_NE(entry->fmt_be, entry->fmt_le);
            return 0;
        }
    }

    fprintf(stderr,
            "sample format %s is not supported as output format\n",
            av_get_sample_fmt_name(sample_fmt));
    return -1;
}

int main(int argc, char const *argv[]) {

  return 0;
}
