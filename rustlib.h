#ifndef RUSTLIB
#define RUSTLIB

int img_vec2pdf(unsigned char* input, int input_len, unsigned char* sizes, int sizes_len, unsigned char** output);
void dealloc(unsigned char** ptr, int size);

#endif
