/***********************************************************************************\
* RandomGenerator:  a very lightweight pseudo random generator only with the        *
*                   necessary random double generating function needed for the      *
*                   program. The class is based on the Mother of all algorithm.     *
*                                                                                   *
* Author:    Enys Mones                                                             *
* Date:      10.08.2014                                                             *
\***********************************************************************************/


#ifndef RANDOM_GENERATOR_HPP
#define RANDOM_GENERATOR_HPP


#include "inttypes.h"
#include "stdlib.h"


class RandomGenerator
{
private:
    // Member variables
    uint32_t _x[5];

public:
    // Constructors
    RandomGenerator();
    RandomGenerator( int seed_ );

    // Initializer, generators
    void init( int seed_ );
    uint32_t random_bits();
    double random();
};

#endif // RANDOM_GENERATOR_HPP
