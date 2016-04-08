# PFlockC

clcg4 package is from http://web.stanford.edu/class/msande223/clcg4/readme.txt
init package is from https://github.com/benhoyt/inih

Parallel flocking application written in C using MPI. I haven't written a real makefile yet, so just compile with `mpicc *.c -O3 -lm -o pflockc`

No custom MPI datatypes were created here, since they typically incur a performance overhead, and the Vec and Boid structs are contiguously allocated

Sanity check is currently still enabled for testing purposes. Disabling it will lead to greater performance.

Number of MPI ranks uses must be a power of 4, since the global simulation box is square, and the section each rank takes care of is required to be a symmetric. This is both a performance boost, and is also just easier to program.
