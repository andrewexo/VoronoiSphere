#ifndef MP_SAMPLE_GENERATOR_H
#define MP_SAMPLE_GENERATOR_H

#include "../glm/glm.hpp"
#include "globals.h"
#include <random>

namespace VorGen {

class SampleGenerator
{
    public:

        SampleGenerator();
        SampleGenerator(unsigned int seed);

        glm::dvec3* getJitteredSamples(int n);
        glm::dvec3* getRandomSamples(int n);

        glm::dvec3* getJitteredPointsSphere(int n);
        glm::dvec3* getRandomPointsSphere(int n);

    private:

        unsigned int seed;
        ::std::uniform_real_distribution<double> unif;
        ::std::default_random_engine re;

};

}

#endif