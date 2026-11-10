#include <stdio.h>
#include <stdlib.h>

#define RBLXSTBI_WINDOWS_UTF8

#ifdef _WIN32
#define WIN32 // what rblxstb.h checks
#pragma comment(lib, "advapi32.lib")
#endif

#define RBLXSTB_IMAGE_IMPLEMENTATION
#include "rblxstb_image.h"

#define RBLXSTB_DEFINE
#include "deprecated/rblxstb.h"

static unsigned int fnv1a_hash32(const rblxstbi_uc *bytes, size_t len)
{
   unsigned int hash = 0x811c9dc5;
   unsigned int mul = 0x01000193;
   size_t i;

   for (i = 0; i < len; ++i)
      hash = (hash ^ bytes[i]) * mul;

   return hash;
}

// The idea for this test is to leave pngsuite/ref_results.csv checked in,
// and then you can run this test after making PNG loader changes. If the
// ref results change (as per git diff), confirm that the change was
// intentional. If so, commit them as well; if not, undo.
int main()
{
   char **files;
   FILE *csv_file;
   int i;

   files = rblxstb_readdir_recursive("pngsuite", "*.png");
   if (!files) {
      fprintf(stderr, "pngsuite files not found!\n");
      return 1;
   }

   // sort files by name
   qsort(files, rblxstb_arr_len(files), sizeof(char*), rblxstb_qsort_strcmp(0));

   csv_file = fopen("pngsuite/ref_results.csv", "w");
   if (!csv_file) {
      fprintf(stderr, "error opening ref results for writing!\n");
      rblxstb_readdir_free(files);
      return 1;
   }

   fprintf(csv_file, "filename,width,height,ncomp,error,hash\n");
   for (i = 0; i < rblxstb_arr_len(files); ++i) {
      char *filename = files[i];
      int width, height, ncomp;
      rblxstbi_uc *pixels = rblxstbi_load(filename, &width, &height, &ncomp, 0);
      const char *error = "";
      unsigned int hash = 0;

      if (!pixels)
         error = rblxstbi_failure_reason();
      else {
         hash = fnv1a_hash32(pixels, width * height * ncomp);
         rblxstbi_image_free(pixels);
      }

      fprintf(csv_file, "%s,%d,%d,%d,%s,0x%08x\n", filename, width, height, ncomp, error, hash);
   }

   fclose(csv_file);
   rblxstb_readdir_free(files);
}
