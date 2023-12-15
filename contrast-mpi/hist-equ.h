#ifndef HIST_EQU_COLOR_H
#define HIST_EQU_COLOR_H

typedef struct{
    int w;
    int h;
    unsigned char * img;
} PGM_IMG;    

typedef struct{
    int w;
    int h;
    unsigned char * img_r;
    unsigned char * img_g;
    unsigned char * img_b;
} PPM_IMG;

typedef struct{
    int w;
    int h;
    unsigned char * img_y;
    unsigned char * img_u;
    unsigned char * img_v;
} YUV_IMG;


typedef struct
{
    int width;
    int height;
    float * h;
    float * s;
    unsigned char * l;
} HSL_IMG;

    

PPM_IMG read_ppm(const char * path);
void write_ppm(PPM_IMG img, const char * path);
void free_ppm(PPM_IMG img);

PGM_IMG read_pgm(const char * path);
void write_pgm(PGM_IMG img, const char * path);
void free_pgm(PGM_IMG img);

HSL_IMG rgb2hsl(unsigned char *segment_in_c_r, unsigned char *segment_in_c_g, unsigned char *segment_in_c_b, int segment_size_c, int width_color);
PPM_IMG hsl2rgb(HSL_IMG img_in);

YUV_IMG rgb2yuv(PPM_IMG img_in);
PPM_IMG yuv2rgb(YUV_IMG img_in);    

void histogram(int * local_hist, unsigned char * segment_in, int segment_size, int nbr_bin);
void histogram_equalization(unsigned char *segment_out, unsigned char *segment_in, int *global_hist, int segment_size, int img_size, int nbr_bin);



//Contrast enhancement for gray-scale images
PGM_IMG contrast_enhancement_g(unsigned char *segment, int segment_size, int width, int height);

//Contrast enhancement for color images
PPM_IMG contrast_enhancement_c_rgb(PPM_IMG img_in);
PPM_IMG contrast_enhancement_c_yuv(PPM_IMG img_in);
PPM_IMG contrast_enhancement_c_hsl(unsigned char *segment_in_c_r, unsigned char *segment_in_c_g, unsigned char *segment_in_c_b, int segment_size_c, int width, int length);



#endif
