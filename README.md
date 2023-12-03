# Contrast Enhacement
Contrast enhancement is a common operation in image processing. It is a useful method for the processing of scientific images, such as X-ray images or satellite images. It is also a useful technique for enhancing detail in photographs that are over- or underexposed. Based on that, the objective of this work is to develop a contrast enhancement application using acceleration over MPI and OpenMP.

It's important to mention that the operation will be tested with a gray-scale and color-scale image.

# File structure
To achieve it, we start from a based code written in c++ that is modified based in the parallel model used. The based code is located at the root of this repository. For a better organization of the code, this repository contains 3 folders:
- contrast-mpi: Contains the files used to execute the project with mpi
- contrast-omp: Contains the files used to execute the project with OpenMP
- contrast-mpi-omp: Contains the files used to execute the project with mpi and openMP

In the root of the project, there are the following files
- contrast.cpp: Main file of the project
- contrast-enhancement.cpp: Contains the functions to execute the contrast enhancement. It also contains the functions to convert an image from one color model to another e.g. rgb to hsl
- histogram-equalization.cpp: Functions for the histogram computations
- hist-equ.h: contains the structures used by the contrast enhancement operation
- highres.jpg: The image used for testing
- CMakeLists.txt: file for the compilation with make command

# Environment
The test are executed in a linux environment. It's important to have installed the libraries for c++ compilation.

# About this project
This project is part is activities carried out in the Subject "High computing performance" thaught in the Computer Science Master Program in the University Carlos III de Madrid.

# Authors
David Santiago Garcia Chicangana
Frank
Fredy
Sebastian cruz
