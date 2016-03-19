# PFlockC

Parallel flocking application written in C using MPI. I haven't written a makefile yet, so just compile with `mpicc *.c -O3 -lm -o pflockc`

No custom MPI datatypes were created here, since they typically incur a performance overhead, and the Vec and Boid structs are contiguously allocated

### TODO
1. Parallel IO
2. Random seed values
3. Clean up init.*
4. Write makefile
5. More complete simulation options
