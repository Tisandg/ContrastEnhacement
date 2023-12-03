#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hist-equ.h"
#include <omp.h>

void histogram(int * hist_out, unsigned char * img_in, int img_size, int nbr_bin){
    int i;

    //The loop iterations will be divided into the threads defined
    #pragma omp parallel for
    for ( i = 0; i < nbr_bin; i ++){
        hist_out[i] = 0;
    }

    // To ensure initialization is completed before to pass to the next loop
    #pragma omp barrier

    #pragma omp parallel for shared(hist_out)
    for ( i = 0; i < img_size; i ++){
        //Adding this line will produce the same binaries as the sequential ones which is good
        // #pragma omp critical
        #pragma omp atomic
        hist_out[img_in[i]] ++;
    }

    /*To see the values of the histogram*/
    // for ( i = 0; i < nbr_bin; i ++){
    //     printf("%d ",hist_out[i]);
    // }
}

void histogram_equalization(unsigned char * img_out, unsigned char * img_in, 
                            int * hist_in, int img_size, int nbr_bin){
    int *lut = (int *)malloc(sizeof(int)*nbr_bin);
    int i, cdf, min, d;
    /* Construct the LUT by calculating the CDF */
    cdf = 0;
    min = 0;
    i = 0;
    while(min == 0){
        min = hist_in[i++];
    }
    d = img_size - min;
    for(i = 0; i < nbr_bin; i ++){
        cdf += hist_in[i];
        //lut[i] = (cdf - min)*(nbr_bin - 1)/d;
        lut[i] = (int)(((float)cdf - min)*255/d + 0.5);
        if(lut[i] < 0){
            lut[i] = 0;
        }
        
        
    }
    
    /* Get the result image */
    #pragma omp parallel for
    for(i = 0; i < img_size; i ++){
        if(lut[img_in[i]] > 255){
            img_out[i] = 255;
        }
        else{
            img_out[i] = (unsigned char)lut[img_in[i]];
        }
        
    }
}



