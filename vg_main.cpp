#include "src/voronoi_generator.h"
#include <iostream>
#include <chrono>

int main(int argc, char* argv[])
{
    int count = 1000000; // default number of points
    if (argc >= 2) count = atoi(argv[1]);

	int gen = count; // default number of cells to generate
	if (argc >= 3) gen = atoi(argv[2]);
        
    printf("Count: %d\n", count);

    VoronoiGenerator vg;
    glm::dvec3* points = vg.genRandomInput(count);

	auto start = std::chrono::high_resolution_clock::now();
    vg.generate(points, count, gen, false);
	double elapsed = std::chrono::duration_cast<std::chrono::microseconds>
        (std::chrono::high_resolution_clock::now() - start).count();
	std::cout << elapsed / 1000.0 << " milliseconds\n";

    delete[] points;

    return 0;
}
