# PFlockC

Parallel flocking application written in C using MPI. I haven't written a makefile yet, so just compile with `mpicc *.c -lm -o pflockc`

No custom MPI datatypes were created here, since they typically incure a performance overhead, and the Vec and Boid structs are contiguously allocated
