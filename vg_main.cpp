#include "src/voronoi_generator.h"
#include <iostream>
#include <chrono>
#include <string>

int main(int argc, char* argv[])
{
    int count = 1000000; // default number of points
    int gen = count; // default number of cells to generate
    bool writeToFile = false; // default: don't write to file
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-w") {
            writeToFile = true;
        } else if (i == 1) {
            count = atoi(arg.c_str());
            gen = count; // reset gen to match count unless overridden
        } else if (i == 2 && arg != "-w") {
            gen = atoi(arg.c_str());
        }
    }
        
    printf("Count: %d\n", count);
    if (writeToFile) {
        printf("Results will be written to file\n");
    }

    VorGen::VoronoiGenerator vg;
    glm::dvec3* points = vg.genRandomInput(count);

	auto start = std::chrono::high_resolution_clock::now();
    vg.generate(points, count, gen, writeToFile);
	double elapsed = std::chrono::duration_cast<std::chrono::microseconds>
        (std::chrono::high_resolution_clock::now() - start).count();
	std::cout << elapsed / 1000.0 << " milliseconds\n";

    delete[] points;

    return 0;
}
