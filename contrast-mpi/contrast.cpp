#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hist-equ.h"
#include <mpi.h>

void run_cpu_color_test(PPM_IMG img_in);
void run_cpu_gray_test(unsigned char *segment, int segment_size, int width, int height);

int main(int argc, char** argv){
    
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double tstart = MPI_Wtime();

    PGM_IMG img_ibuf_g;
    //PPM_IMG img_ibuf_c;


    int img_size_g,  segment_size_g , remainder_g, width, height;
    //int img_size_c, segment_size_c,remainder_c;

    // Process 0 reads the image and broadcasts image dimensions
    if (rank == 0) {
        img_ibuf_g = read_pgm("in.pgm");
        //img_ibuf_c = read_ppm("in.ppm");
        width      = img_ibuf_g.w;
        height     = img_ibuf_g.h;
        img_size_g = img_ibuf_g.w * img_ibuf_g.h;
        //img_size_c = img_ibuf_c.w * img_ibuf_c.h;
    }

    MPI_Bcast(&img_size_g, 1, MPI_INT, 0, MPI_COMM_WORLD);
    //MPI_Bcast(&img_size_c, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Calculate the size of each segment for grayscale and color images
    segment_size_g = img_size_g / size;
    //segment_size_c = img_size_c / size;
    remainder_g = img_size_g % size;
    //remainder_c = img_size_c % size;
    
    if (rank == size - 1) {
        segment_size_g += remainder_g; // Last process takes the remainder
        //segment_size_c += remainder_c; // Last process takes the remainder
    }

    // Allocate memory for each segment
    unsigned char *segment_in_g = (unsigned char *)malloc(segment_size_g * sizeof(unsigned char));
    //unsigned char *segment_in_c_r = (unsigned char *)malloc(segment_size_c * sizeof(unsigned char));
    //unsigned char *segment_in_c_g = (unsigned char *)malloc(segment_size_c * sizeof(unsigned char));
    //unsigned char *segment_in_c_b = (unsigned char *)malloc(segment_size_c * sizeof(unsigned char));

    // Scatter the gray image data
    MPI_Scatter(img_ibuf_g.img, segment_size_g, MPI_UNSIGNED_CHAR, segment_in_g, segment_size_g, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    // Scatter the color image data
    // MPI_Scatter(img_ibuf_c.img_r, segment_size_c, MPI_UNSIGNED_CHAR, segment_in_c_r, segment_size_c, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    // MPI_Scatter(img_ibuf_c.img_g, segment_size_c, MPI_UNSIGNED_CHAR, segment_in_c_g, segment_size_c, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    // MPI_Scatter(img_ibuf_c.img_b, segment_size_c, MPI_UNSIGNED_CHAR, segment_in_c_b, segment_size_c, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    // Process each segment for grayscale and color images

    // GRAY

    run_cpu_gray_test(segment_in_g, segment_size_g, width, height);  
    free(segment_in_g);

    // COLOR
    // run_cpu_color_test(segment_in_c_r, segment_in_c_g, segment_in_c_b, segment_size_c, img_ibuf_c.w, img_ibuf_c.h);
    // free(segment_in_c_r);
    // free(segment_in_c_g);
    // free(segment_in_c_b);

    double tfinish = MPI_Wtime();
    double localTime = tfinish - tstart;
    double maxTime;
    MPI_Reduce(&localTime, &maxTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("TOTAL TIME: %f (ms)\n", maxTime * 1000.0);
    }


    MPI_Finalize();
    return 0;
}


// void run_cpu_color_test(PPM_IMG img_in)
// {
//     int rank, size;
//     MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//     MPI_Comm_size(MPI_COMM_WORLD, &size);
    
//     PPM_IMG img_obuf_hsl, img_obuf_yuv;

//     //printf("Starting CPU processing...\n");

//     double tstart = MPI_Wtime();
//     img_obuf_hsl = contrast_enhancement_c_hsl(img_in);
//     double tfinish = MPI_Wtime();
//     double TotalTime = tfinish - tstart;
//     //printf("HSL processing time: %f (ms)\n", TotalTime * 1000.0);
//     if (rank==0){
//         write_ppm(img_obuf_hsl, "out_hsl.ppm");
//     }
//     tstart = MPI_Wtime();
//     img_obuf_yuv = contrast_enhancement_c_yuv(img_in);
//     tfinish = MPI_Wtime();
//     TotalTime = tfinish - tstart;
//     //printf("YUV processing time: %f (ms)\n", TotalTime * 1000.0);
//     if (rank==0){
//         write_ppm(img_obuf_yuv, "out_yuv.ppm");
//     }
//     free_ppm(img_obuf_hsl);
//     free_ppm(img_obuf_yuv);
// }



void run_cpu_gray_test(unsigned char *segment, int segment_size, int width, int height) {
    
    int rank, size;
    PGM_IMG img_obuf;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Each process performs contrast enhancement on its segment
    img_obuf = contrast_enhancement_g(segment, segment_size, width, height); 

    // Only the root process writes the output image
    if (rank == 0) {
        // Assuming the full image has been gathered in the root process
        write_pgm(img_obuf, "out.pgm");
    }

}


PPM_IMG read_ppm(const char * path){
    FILE * in_file;
    char sbuf[256];

    char *ibuf;
    PPM_IMG result;
    int v_max, i;
    in_file = fopen(path, "r");
    if (in_file == NULL){
        printf("Input file not found!\n");
        exit(1);
    }
    /*Skip the magic number*/
    fscanf(in_file, "%s", sbuf);


    //result = malloc(sizeof(PPM_IMG));
    fscanf(in_file, "%d",&result.w);
    fscanf(in_file, "%d",&result.h);
    fscanf(in_file, "%d\n",&v_max);
    //printf("Image size: %d x %d\n", result.w, result.h);


    result.img_r = (unsigned char *)malloc(result.w * result.h * sizeof(unsigned char));
    result.img_g = (unsigned char *)malloc(result.w * result.h * sizeof(unsigned char));
    result.img_b = (unsigned char *)malloc(result.w * result.h * sizeof(unsigned char));
    ibuf         = (char *)malloc(3 * result.w * result.h * sizeof(char));


    fread(ibuf,sizeof(unsigned char), 3 * result.w*result.h, in_file);

    for(i = 0; i < result.w*result.h; i ++){
        result.img_r[i] = ibuf[3*i + 0];
        result.img_g[i] = ibuf[3*i + 1];
        result.img_b[i] = ibuf[3*i + 2];
    }

    fclose(in_file);
    free(ibuf);

    return result;
}

void write_ppm(PPM_IMG img, const char * path){
    FILE * out_file;
    int i;

    char * obuf = (char *)malloc(3 * img.w * img.h * sizeof(char));

    for(i = 0; i < img.w*img.h; i ++){
        obuf[3*i + 0] = img.img_r[i];
        obuf[3*i + 1] = img.img_g[i];
        obuf[3*i + 2] = img.img_b[i];
    }
    out_file = fopen(path, "wb");
    fprintf(out_file, "P6\n");
    fprintf(out_file, "%d %d\n255\n",img.w, img.h);
    fwrite(obuf,sizeof(unsigned char), 3*img.w*img.h, out_file);
    fclose(out_file);
    free(obuf);
}

void free_ppm(PPM_IMG img)
{
    free(img.img_r);
    free(img.img_g);
    free(img.img_b);
}

PGM_IMG read_pgm(const char * path){
    FILE * in_file;
    char sbuf[256];


    PGM_IMG result;
    int v_max;//, i;
    in_file = fopen(path, "r");
    if (in_file == NULL){
        printf("Input file not found!\n");
        exit(1);
    }

    fscanf(in_file, "%s", sbuf); /*Skip the magic number*/
    fscanf(in_file, "%d",&result.w);
    fscanf(in_file, "%d",&result.h);
    fscanf(in_file, "%d\n",&v_max);
    //printf("Image size: %d x %d\n", result.w, result.h);


    result.img = (unsigned char *)malloc(result.w * result.h * sizeof(unsigned char));


    fread(result.img,sizeof(unsigned char), result.w*result.h, in_file);
    fclose(in_file);

    return result;
}

void write_pgm(PGM_IMG img, const char * path){
    FILE * out_file;
    out_file = fopen(path, "wb");
    fprintf(out_file, "P5\n");
    fprintf(out_file, "%d %d\n255\n",img.w, img.h);
    fwrite(img.img,sizeof(unsigned char), img.w*img.h, out_file);
    fclose(out_file);
}

void free_pgm(PGM_IMG img)
{
    free(img.img);
}

