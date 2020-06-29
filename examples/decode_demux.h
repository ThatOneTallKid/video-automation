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

static int refcount = 0;

static int decode_packet()
