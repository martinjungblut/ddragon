#include "file.h"

long long fsize(FILE *fh){
  struct stat s;
  if (fstat(fileno(fh), &s) == -1 ) {
    return -1;
  }
  return s.st_size;
}
