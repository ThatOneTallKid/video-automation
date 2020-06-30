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
      fwrite(video[0], 1, video_dst_bufsize, video_dst_file);
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
      if (response == AVERROR_EOF || AVERREOE(EAGAIN))
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

int int main(int argc, char const *argv[]) {

  return 0;
}
