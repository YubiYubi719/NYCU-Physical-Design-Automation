#include "FM_Partition.h"
#include <chrono>
using namespace std::chrono;

uint64_t timeSinceEpochMillisec() {
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

int main(int argc, char* argv[]){
    Graph g;
    uint64_t start = timeSinceEpochMillisec(); // timer start
    // build up
    g.build(argv[1]);

    // start FM-Partition
    int prevMinCutNum = -1;
    int consecutiveSameCutTimes = 0;
    while(true){
        bool isFinished = g.partition();
        if(isFinished){ 
            g.traceBestSol();
            g.clearMoveStack();
            g.unlock();
            g.calInitGain();
            g.renewBucketList();
            uint64_t end = timeSinceEpochMillisec(); // timer end
            cout << "Current iteration time: " << end - start << " ms\n";
            if(prevMinCutNum == g.minCutNum){
                consecutiveSameCutTimes++;
                if(consecutiveSameCutTimes == 1) break;
            }
            else consecutiveSameCutTimes = 0;
            prevMinCutNum = g.minCutNum;
        }
    }

    // print output
    g.printOutput(argv[2]);
    uint64_t end = timeSinceEpochMillisec();
    cout << "\n================End of FM_Partition================" << '\n';
    cout << "Final CutSize = " << g.outputCutNum << '\n';
    cout << "Execution time: " << end - start << " ms\n";

    return 0;
}