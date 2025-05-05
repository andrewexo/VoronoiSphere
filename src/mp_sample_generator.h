#pragma once

#include "../glm/glm.hpp"
#include "globals.h"
#include <random>

namespace VorGen {

class SampleGenerator
{
    public:

        SampleGenerator();
        SampleGenerator(size_t seed);

        glm::dvec3* getJitteredSamples(int n);
        glm::dvec3* getRandomSamples(int n);

        glm::dvec3* getJitteredPointsSphere(int n);
        glm::dvec3* getRandomPointsSphere(int n);

    private:

        size_t seed;
        ::std::uniform_real_distribution<double> unif;
        ::std::default_random_engine re;

};

}