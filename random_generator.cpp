#include "random_generator.h"


/**
 * Empty constructor.
 * Initializes random generator with a random seed.
 */
RandomGenerator::RandomGenerator()
{
    init( rand() % 10000 );
}


/**
 * Constructor with seed given.
 *
 * @param seed_ seed of the random generator
 */
RandomGenerator::RandomGenerator( int seed_ )
{
    init( seed_ );
}


/**
 * Initializes random generator.
 *
 * @param seed_  seed of the random generator
 */
void RandomGenerator::init( int seed_ )
{
    uint32_t s = (uint32_t)seed_;
    // make random numbers and put them into the buffer
    for( int i = 0; i < 5; i++ ) {
        s = 29943829 * s - 1;
        _x[i] = s;
    }
    // randomize some more
    for( int i = 0; i < 19; i++ )
        random_bits();
}


/**
 * Generates random bits.
 *
 * @return  a 32-long random bit
 */
uint32_t RandomGenerator::random_bits()
{
    uint64_t sum;
    sum = (uint64_t)2111111111UL * (uint64_t)_x[3] +
          (uint64_t)1492 * (uint64_t)(_x[2]) +
          (uint64_t)1776 * (uint64_t)(_x[1]) +
          (uint64_t)5115 * (uint64_t)(_x[0]) +
          (uint64_t)(_x[4]);
    _x[3] = _x[2];
    _x[2] = _x[1];
    _x[1] = _x[0];
    _x[4] = (uint32_t)(sum >> 32);
    _x[0] = (uint32_t)sum;
    return _x[0];
}


/**
 * Generates a random double between 0 and 1.
 *
 * @return  random double between 0 and 1
 */
double RandomGenerator::random()
{
    return random_bits() / (65536.*65536.);
}
