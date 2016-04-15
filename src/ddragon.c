#include "ddragon.h"
#include "file.h"
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

void display_usage(){
  printf("Usage: ddragon <input file> <output file> [size]\n");
}

int transfer(FILE *input, FILE *output, unsigned long long size, unsigned int block_size, void (*progress_handler)(float progress)){
  char *buffer;
  int i = 0;
  int passes = 0;
  int read = 0;
  long long input_size = fsize(input);
  unsigned long long remaining = 0;
  unsigned long long total_read = 0;

  if (input_size == -1)
    return -3;

  if ((input_size == 0 || input_size == -1) && size == 0)
    return -2;

  if (input_size > 0 && size == 0)
    size = input_size;

  while ( block_size > size )
    block_size /= 2;

  buffer = malloc(block_size);
  if (buffer == NULL)
    return -1;

  passes = (int)(size / block_size);

  remaining = size % block_size;
  if (remaining == size)
    remaining = 0;

  progress_handler(0);

  for (; i < passes; i++) {
    read = fread(buffer, 1, block_size, input);
    total_read += read;
    fwrite(buffer, 1, read, output);
    fsync(fileno(output));
    progress_handler((float)(total_read * 100.0) / size);
  }
  if (remaining > 0) {
    buffer = realloc(buffer, remaining);
    if (buffer == NULL)
        return -1;
    read = fread(buffer, 1, remaining, input);
    total_read += read;
    fwrite(buffer, 1, read, output);
    fsync(fileno(output));
    progress_handler((float)(total_read * 100.0) / size);
  }

  free(buffer);
  return 0;
}

void progress_handler(float progress){
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  int terminal_size = w.ws_col - 9;
  int point = (int)(terminal_size * (progress / 100));
  int i = 0;

  printf("\r[");
  for (; i < terminal_size; i++){
    if (i < point) {
      printf("=");
    } else if (i == point) {
      printf(">");
    } else {
      printf(" ");
    }
  }
  printf("] %.1f%%", progress);
  fflush(stdout);
}

int main(int argc, char **argv){
  if (argc < 3) {
    display_usage();
    exit(1);
  }

  char *input_file = argv[1];
  char *output_file = argv[2];
  int return_code = 0;
  unsigned long long size = 0;
  if (argc >= 4) {
    size = atoi(argv[3]);
    if (strcmp((*(argv + 3)) + strlen(*(argv + 3)) - 1, "K") == 0) {
      size *= 1024;
    }

    if (strcmp((*(argv + 3)) + strlen(*(argv + 3)) - 1, "M") == 0) {
      size *= 1024 * 1024;
    }

    if (strcmp((*(argv + 3)) + strlen(*(argv + 3)) - 1, "G") == 0) {
      size *= 1024 * 1024 * 1024;
    }
  }

  if (size <= 0 && argc >= 4) {
    printf("Size must be a positive integer.\n");
    display_usage();
    exit(1);
  }

  FILE *input = fopen(input_file, "r");
  FILE *output = fopen(output_file, "w");

  if (input == NULL) {
    printf("Could not open input file: %s\n", input_file);
    exit(1);
  } else if (output == NULL) {
    printf("Could not open output file: %s\n", output_file);
    fclose(input);
    exit(1);
  }

  int transfer_result = transfer(input, output, size, BLOCK_SIZE, progress_handler);
  switch (transfer_result) {
    case 0: {
        printf("\nTransfer successful.\n");
        break;
    }
    case -1: {
        printf("\nTransfer unsuccessful. Memory allocation failed.\n");
        return_code = 1;
    }
    case -2: {
        printf("\nTransfer unsuccessful. Input file has no size, and no size parameter was specified.\n");
        return_code = 1;
    }
    case -3: {
      printf("\nTransfer unsuccessful. Input file size could not be acquired.\n");
      return_code = 1;
    }
    default: {
        break;
    }
  }

  fclose(input);
  fclose(output);
  exit(return_code);
}
