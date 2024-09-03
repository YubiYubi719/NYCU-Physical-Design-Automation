#include "SA.h"
#include <ctime>

int main(int argc, char* argv[]){
    SA sa;
    srand(time(NULL));

    // Timer start
    clock_t start = clock();

    // Read file
    sa.build(argv[1]);
    // Construct B*-Tree
    sa.buildBST();
    // SA
    while(true){
        sa.simulateAnnealing();
        if(sa.isValid()) break; // Break if current floorplan is legal
    }
    // Write output
    sa.printOutput(argv[2]);

    // Timer End
    clock_t end = clock();
    cout << "Execution time: " << (double)(end - start) / CLOCKS_PER_SEC * 1000 << "ms" << '\n';

    return 0;
}
