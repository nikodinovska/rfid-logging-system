#include <sys/time.h>

#include "utils.h"

uint64_t get_timestamp_ms()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000LL + (tv.tv_usec / 1000LL));
}