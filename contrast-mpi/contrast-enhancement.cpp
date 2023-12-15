#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hist-equ.h"

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


PGM_IMG contrast_enhancement_g(unsigned char *segment, int segment_size, int width, int height) {
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    int local_hist[256] = {0};
    int global_hist[256] = {0};

    int img_size_g = width * height;
    int remainder_g = img_size_g % size;

    // Calculate local histogram for the segment
    histogram(local_hist, segment, segment_size, 256);

    // Aggregate local histograms into a global histogram
    MPI_Allreduce(local_hist, global_hist, 256, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    
    // Prepare the resulting image
    PGM_IMG result;
    if (rank == 0) {
        result.img = (unsigned char *)malloc(width * height * sizeof(unsigned char));
        result.w = width;
        result.h = height;
    } else {
        result.img = NULL;
        result.w = 0;
        result.h = 0;
    }
    
    // Apply histogram equalization to the segment
    
    unsigned char *enhanced_segment = (unsigned char *)malloc(segment_size * sizeof(unsigned char));
    histogram_equalization(enhanced_segment, segment, global_hist, segment_size, img_size_g, 256);

    // Prepare for MPI_Gatherv
    int *sendcounts = (int *)malloc(size * sizeof(int));
    int *displs = (int *)malloc(size * sizeof(int));

    // Calculate send counts and displacements
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sendcounts[i] = (i < size - 1) ? (img_size_g / size) : (img_size_g / size + remainder_g);
        displs[i] = sum;
        sum += sendcounts[i];
    }

    // Gather the processed segments into the result image using MPI_Gatherv
    MPI_Gatherv(enhanced_segment, segment_size, MPI_UNSIGNED_CHAR, result.img, sendcounts, displs, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    // Clean up
    free(enhanced_segment);
    free(sendcounts);
    free(displs);

    return result;
}


PPM_IMG contrast_enhancement_c_rgb(PPM_IMG img_in)
{
    PPM_IMG result;
    int hist[256];
    
    result.w = img_in.w;
    result.h = img_in.h;
    result.img_r = (unsigned char *)malloc(result.w * result.h * sizeof(unsigned char));
    result.img_g = (unsigned char *)malloc(result.w * result.h * sizeof(unsigned char));
    result.img_b = (unsigned char *)malloc(result.w * result.h * sizeof(unsigned char));
    
    int img_size_c = img_in.w * img_in.h;

    histogram(hist, img_in.img_r, img_in.h * img_in.w, 256);
    histogram_equalization(result.img_r,img_in.img_r,hist,result.w*result.h, img_size_c, 256);
    histogram(hist, img_in.img_g, img_in.h * img_in.w, 256);
    histogram_equalization(result.img_g,img_in.img_g,hist,result.w*result.h,img_size_c ,256);
    histogram(hist, img_in.img_b, img_in.h * img_in.w, 256);
    histogram_equalization(result.img_b,img_in.img_b,hist,result.w*result.h,img_size_c, 256);

    return result;
}


PPM_IMG contrast_enhancement_c_yuv(PPM_IMG img_in)
{
    YUV_IMG yuv_med;
    PPM_IMG result;
   
    unsigned char * y_equ;
    int hist[256];

    int img_size_c = img_in.w * img_in.h;
    
    yuv_med = rgb2yuv(img_in);
    y_equ = (unsigned char *)malloc(yuv_med.h*yuv_med.w*sizeof(unsigned char));
    
    histogram(hist, yuv_med.img_y, yuv_med.h * yuv_med.w, 256);
    histogram_equalization(y_equ,yuv_med.img_y,hist,yuv_med.h * yuv_med.w, img_size_c, 256);

    free(yuv_med.img_y);
    yuv_med.img_y = y_equ;
    
    result = yuv2rgb(yuv_med);
    free(yuv_med.img_y);
    free(yuv_med.img_u);
    free(yuv_med.img_v);
    
    return result;
}

PPM_IMG contrast_enhancement_c_hsl(unsigned char *segment_in_c_r, unsigned char *segment_in_c_g, unsigned char *segment_in_c_b, int segment_size_c, int width, int height)
{
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    PPM_IMG result,rgb_segment;
    int img_size_c = width * height; //Total Image size.
    int remainder_c = img_size_c % size;

    // Allocate result image only in the root process
    if (rank == 0) {
        result.img_r = (unsigned char *)malloc(img_size_c * sizeof(unsigned char));
        result.img_g = (unsigned char *)malloc(img_size_c * sizeof(unsigned char));
        result.img_b = (unsigned char *)malloc(img_size_c * sizeof(unsigned char));
        result.w = width;
        result.h = height;
    } else{
        result.img_r = NULL;
        result.img_g = NULL;
        result.img_b = NULL;
        result.w = 0;
        result.h = 0;
    }

    
    int local_hist[256] = {0};
    int global_hist[256] = {0};

    
    HSL_IMG hsl_segment = rgb2hsl(segment_in_c_r, segment_in_c_g, segment_in_c_b, segment_size_c,width);


    unsigned char *l_equ = (unsigned char *)malloc(segment_size_c*sizeof(unsigned char));

    histogram(local_hist, hsl_segment.l, hsl_segment.height * hsl_segment.width, 256);
    MPI_Allreduce(local_hist, global_hist, 256, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    if(rank == 0) {
        for(int i = 0; i < 256; i++) {
            printf("Histogram[%d] = %d\n", i, global_hist[i]);
        }
    }

    histogram_equalization(l_equ, hsl_segment.l,global_hist,segment_size_c,img_size_c, 256); // In case of error, try putting the hsl_segment.w*hsl_segment.l
    
    free(hsl_segment.l);
    hsl_segment.l = l_equ;
    rgb_segment = hsl2rgb(hsl_segment);

    // Prepare for MPI_Gatherv
    int *sendcounts = (int *)malloc(size * sizeof(int));
    int *displs = (int *)malloc(size * sizeof(int));
    int extra_rows = height % size;
    int sum = 0;

    for (int i = 0; i < size; i++) {
        // Calculate the number of rows for each process
        int rows_for_this_process = (i < size - 1) ? (height / size) : (height / size + extra_rows);
        // Calculate the number of elements (pixels) for each process
        sendcounts[i] = rows_for_this_process * width;
        // The displacement for each process
        displs[i] = sum;
        // Update sum for the next iteration
        sum += sendcounts[i];
    }

    
    // Gather the processed segments into the result image using MPI_Gatherv for each color channel
    MPI_Gatherv(rgb_segment.img_r, segment_size_c, MPI_UNSIGNED_CHAR, result.img_r, sendcounts, displs, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    MPI_Gatherv(rgb_segment.img_g, segment_size_c, MPI_UNSIGNED_CHAR, result.img_g, sendcounts, displs, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    MPI_Gatherv(rgb_segment.img_b, segment_size_c, MPI_UNSIGNED_CHAR, result.img_b, sendcounts, displs, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    // Clean up
    free(sendcounts);
    free(displs);

    free(rgb_segment.img_r);
    free(rgb_segment.img_g);
    free(rgb_segment.img_b);
    

    return result;
}


//Convert RGB to HSL, assume R,G,B in [0, 255]
//Output H, S in [0.0, 1.0] and L in [0, 255]
HSL_IMG rgb2hsl(unsigned char *segment_in_c_r, unsigned char *segment_in_c_g, unsigned char *segment_in_c_b, int segment_size_c, int width)
{
    int i;
    float H, S, L;
    HSL_IMG hsl_segment;

    // Allocate memory for HSL segment
    hsl_segment.h = (float *)malloc(segment_size_c * sizeof(float));
    hsl_segment.s = (float *)malloc(segment_size_c * sizeof(float));
    hsl_segment.l = (unsigned char *)malloc(segment_size_c * sizeof(unsigned char));
    hsl_segment.height = segment_size_c / width;
    hsl_segment.width = width;

    for (int i = 0; i < segment_size_c; i++) {
        float var_r = ((float)segment_in_c_r[i] / 255);
        float var_g = ((float)segment_in_c_g[i] / 255);
        float var_b = ((float)segment_in_c_b[i] / 255);

        float var_min = (var_r < var_g) ? var_r : var_g;
        var_min = (var_min < var_b) ? var_min : var_b;   //min. value of RGB
        float var_max = (var_r > var_g) ? var_r : var_g;
        var_max = (var_max > var_b) ? var_max : var_b;   //max. value of RGB
        float del_max = var_max - var_min;               //Delta RGB value

        L = (var_max + var_min) / 2;

        if ( del_max == 0 )//This is a gray, no chroma...
        {
            H = 0;         
            S = 0;    
        }
        else                                    //Chromatic data...
        {
            if ( L < 0.5 )
                S = del_max/(var_max+var_min);
            else
                S = del_max/(2-var_max-var_min );

            float del_r = (((var_max-var_r)/6)+(del_max/2))/del_max;
            float del_g = (((var_max-var_g)/6)+(del_max/2))/del_max;
            float del_b = (((var_max-var_b)/6)+(del_max/2))/del_max;

            if( var_r == var_max ){
                H = del_b - del_g;
            }
            else{       
                if( var_g == var_max ){
                    H = (1.0/3.0) + del_r - del_b;
                }
                else{
                        H = (2.0/3.0) + del_g - del_r;
                }   
            }
            
        }

        hsl_segment.h[i] = H;
        hsl_segment.s[i] = S;
        hsl_segment.l[i] = (unsigned char)(L * 255);
    }

    return hsl_segment;
}

float Hue_2_RGB( float v1, float v2, float vH )             //Function Hue_2_RGB
{
    if ( vH < 0 ) vH += 1;
    if ( vH > 1 ) vH -= 1;
    if ( ( 6 * vH ) < 1 ) return ( v1 + ( v2 - v1 ) * 6 * vH );
    if ( ( 2 * vH ) < 1 ) return ( v2 );
    if ( ( 3 * vH ) < 2 ) return ( v1 + ( v2 - v1 ) * ( ( 2.0f/3.0f ) - vH ) * 6 );
    return ( v1 );
}

//Convert HSL to RGB, assume H, S in [0.0, 1.0] and L in [0, 255]
//Output R,G,B in [0, 255]
PPM_IMG hsl2rgb(HSL_IMG img_in)
{
    int i;
    PPM_IMG result;
    
    result.w = img_in.width;
    result.h = img_in.height;
    result.img_r = (unsigned char *)malloc(result.w * result.h * sizeof(unsigned char));
    result.img_g = (unsigned char *)malloc(result.w * result.h * sizeof(unsigned char));
    result.img_b = (unsigned char *)malloc(result.w * result.h * sizeof(unsigned char));
    
    for(i = 0; i < img_in.width*img_in.height; i ++){
        float H = img_in.h[i];
        float S = img_in.s[i];
        float L = img_in.l[i]/255.0f;
        float var_1, var_2;
        
        unsigned char r,g,b;
        
        if ( S == 0 )
        {
            r = L * 255;
            g = L * 255;
            b = L * 255;
        }
        else
        {
            
            if ( L < 0.5 )
                var_2 = L * ( 1 + S );
            else
                var_2 = ( L + S ) - ( S * L );

            var_1 = 2 * L - var_2;
            r = 255 * Hue_2_RGB( var_1, var_2, H + (1.0f/3.0f) );
            g = 255 * Hue_2_RGB( var_1, var_2, H );
            b = 255 * Hue_2_RGB( var_1, var_2, H - (1.0f/3.0f) );
        }
        result.img_r[i] = r;
        result.img_g[i] = g;
        result.img_b[i] = b;
    }

    return result;
}

//Convert RGB to YUV, all components in [0, 255]
YUV_IMG rgb2yuv(PPM_IMG img_in)
{
    YUV_IMG img_out;
    int i;//, j;
    unsigned char r, g, b;
    unsigned char y, cb, cr;
    
    img_out.w = img_in.w;
    img_out.h = img_in.h;
    img_out.img_y = (unsigned char *)malloc(sizeof(unsigned char)*img_out.w*img_out.h);
    img_out.img_u = (unsigned char *)malloc(sizeof(unsigned char)*img_out.w*img_out.h);
    img_out.img_v = (unsigned char *)malloc(sizeof(unsigned char)*img_out.w*img_out.h);

    for(i = 0; i < img_out.w*img_out.h; i ++){
        r = img_in.img_r[i];
        g = img_in.img_g[i];
        b = img_in.img_b[i];
        
        y  = (unsigned char)( 0.299*r + 0.587*g +  0.114*b);
        cb = (unsigned char)(-0.169*r - 0.331*g +  0.499*b + 128);
        cr = (unsigned char)( 0.499*r - 0.418*g - 0.0813*b + 128);
        
        img_out.img_y[i] = y;
        img_out.img_u[i] = cb;
        img_out.img_v[i] = cr;
    }
    
    return img_out;
}

unsigned char clip_rgb(int x)
{
    if(x > 255)
        return 255;
    if(x < 0)
        return 0;

    return (unsigned char)x;
}

//Convert YUV to RGB, all components in [0, 255]
PPM_IMG yuv2rgb(YUV_IMG img_in)
{
    PPM_IMG img_out;
    int i;
    int  rt,gt,bt;
    int y, cb, cr;
    
    
    img_out.w = img_in.w;
    img_out.h = img_in.h;
    img_out.img_r = (unsigned char *)malloc(sizeof(unsigned char)*img_out.w*img_out.h);
    img_out.img_g = (unsigned char *)malloc(sizeof(unsigned char)*img_out.w*img_out.h);
    img_out.img_b = (unsigned char *)malloc(sizeof(unsigned char)*img_out.w*img_out.h);

    for(i = 0; i < img_out.w*img_out.h; i ++){
        y  = (int)img_in.img_y[i];
        cb = (int)img_in.img_u[i] - 128;
        cr = (int)img_in.img_v[i] - 128;
        
        rt  = (int)( y + 1.402*cr);
        gt  = (int)( y - 0.344*cb - 0.714*cr);
        bt  = (int)( y + 1.772*cb);

        img_out.img_r[i] = clip_rgb(rt);
        img_out.img_g[i] = clip_rgb(gt);
        img_out.img_b[i] = clip_rgb(bt);
    }
    
    return img_out;
}
