#define RBLXSTB_VORBIS_HEADER_ONLY
#include "rblxstb_vorbis.c"
#include "rblxstb.h"

extern void rblxstb_vorbis_dumpmem(void);

#ifdef VORBIS_TEST
int main(int argc, char **argv)
{
   size_t memlen;
   unsigned char *mem = rblxstb_fileu("../../lib/vorbis/sample/sketch008.ogg", &memlen);
   int chan, samplerate;
   short *output;
   int samples = rblxstb_vorbis_decode_memory(mem, memlen, &chan, &samplerate, &output);
   rblxstb_filewrite("c:/x/sketch008.raw", output, samples*4);
   return 0;
}
#endif
