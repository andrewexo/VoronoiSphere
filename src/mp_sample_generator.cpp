#include "mp_sample_generator.h"
#include "../glm/glm.hpp"

#include <iostream>
#include <cstdlib>
#include <time.h>

#define _USE_MATH_DEFINES
#include <math.h>

namespace VorGen {

SampleGenerator::SampleGenerator()
{
    seed = (unsigned int) time(NULL);
    //::std::cout << "seed = " << seed << "\n";

    unif = ::std::uniform_real_distribution<double>(0.0,1.0);
}

SampleGenerator::SampleGenerator(unsigned int seed) : seed(seed)
{
    unif = ::std::uniform_real_distribution<double>(0.0,1.0);
}

// Generates n * n jittered samples in the range [0,1) x [0,1)
glm::dvec3* SampleGenerator::getJitteredSamples(int n)
{
    glm::dvec3* samples = new glm::dvec3[n*n];

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            double x = unif(re);
            double y = unif(re);

            samples[i*n + j] = glm::dvec3((x+i)/n, (y+j)/n, 0.0);
        }
    }

    return samples;
}

// Generates n random samples in the range [0,1) x [0,1)
glm::dvec3* SampleGenerator::getRandomSamples(int n)
{
    glm::dvec3* samples = new glm::dvec3[n];

    for (int i = 0; i < n; i++)
    {
        double x = unif(re);
        double y = unif(re);

        samples[i] = glm::dvec3(x, y, 0.0);
    }

    return samples;
}

glm::dvec3* SampleGenerator::getJitteredPointsSphere(int n)
{
    const double PI = M_PI;

    glm::dvec3* samples = getJitteredSamples(n);

    for (int i = 0; i < n; i++)
    {
        glm::dvec3 sample = samples[i];

        // scale sample into range [0,2pi) x [-1,1]
        sample = glm::dvec3(2.0*PI*sample.x, 2.0*sample.y - 1.0, 0.0);

        double T = sqrt(1.0 - sample.y * sample.y);
        double x = T * cos(sample.x);
        double y = T * sin(sample.x);
        double z = sample.y;

        samples[i] = glm::normalize(glm::dvec3(x, y, z));
    }

    return samples;
}

glm::dvec3* SampleGenerator::getRandomPointsSphere(int n)
{
    const double PI = M_PI;
    glm::dvec3* samples = getRandomSamples(n);

    for (int i = 0; i < n; i++)
    {
        glm::dvec3 sample = samples[i];

        // scale sample into range [0,2pi) x [-1,1]
        sample = glm::dvec3(2.0*PI*sample.x, 2.0*sample.y - 1.0, 0.0);

        double T = sqrt(1.0 - sample.y * sample.y);
        double x = T * cos(sample.x);
        double y = T * sin(sample.x);
        double z = sample.y;

        samples[i] = glm::normalize(glm::dvec3(x, y, z));
    }

    return samples;
}

}
