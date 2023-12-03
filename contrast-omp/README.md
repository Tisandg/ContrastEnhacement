# Observationsn for the implementaion with OpenMP


# Execution with OpenMP
- The execution with OpenMP can be also being executed in Windows

To compile the files:
```
g++ -fopenmp contrast.cpp contrast-enhancement.cpp histogram-equalization.cpp -o test1
```

To run in windows:
```
$env:OMP_NUM_THREADS=4
./test1
```

To run in linux
```
export OMP_NUM_THREADS=4
srun -p gpus -N 1 -n 1 ./test1
```

# About this project
This project is part is activities carried out in the Subject "High computing performance" thaught in the Computer Science Master Program in the University Carlos III de Madrid.

# Authors
- David Santiago Garcia Chicangana
- Frank
- Fredy
- Sebastian cruz
