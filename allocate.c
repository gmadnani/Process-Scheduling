#include "cpu.h"

// start with the parallel
int main(int argc, char** argv) {
    void * cpu = cpuInit(argc, argv); // create the cpu

    if (!cpu) { // invalid argument input.
        return EXIT_FAILURE;
    }

    // run the simulation
    unsigned int time = 0;

    while (cpuRun(cpu, time)) {
        
        time++; // increment time to execute the next frame
    }

    // show stats
    cpuStats(cpu, time);

    // release memory
    cpuDelete(cpu);

    return EXIT_SUCCESS;
}
