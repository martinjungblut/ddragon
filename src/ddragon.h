#ifndef __DDRAGON_H__
#define __DDRAGON_H__

#define BLOCK_SIZE (64 * 1024 * 1024)

#include <stdio.h>

int transfer(FILE *input, FILE *output, unsigned long long size, unsigned int block_size, void (*progress_handler)(float progress));
void display_usage();
void progress_handler(float progress);

#endif
