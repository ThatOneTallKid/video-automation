#include <context.c>


typedef struct Clip{
	VideoContext *context;
	int64_t start_time;
	int64_t end_time;
	int64_t orig_start;
	int64_t orig_end;
	int64_t frame_index;
}Clip;


void init_clip_internal(Clip *clip){
	clip->context = NULL;
	clip->orig_start = 0;
	clip->orig_end = -1;
	clip->start_time = -1;
	clip->end_time = -1;
	clip->frame_index = 0;
}



int main(int argc, char const *argv[])
{	clip->context->fmt_cntx = avformat_alloc_context();
	
	return 0;
}
