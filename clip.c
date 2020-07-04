#include <context.c>


typedef struct Clip{
	VideoContext *context;
	int64_t start_time;
	int64_t end_time;
	int64_t orig_start;
	int64_t orig_end;
}Clip;