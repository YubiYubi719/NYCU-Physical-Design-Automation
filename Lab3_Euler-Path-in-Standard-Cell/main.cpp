#include "mos.cpp"
#include <bits/stdc++.h>
// #include <chrono>
// using namespace std::chrono;
using namespace std;

// uint64_t timeSinceEpochMillisec() {
//   return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
// }

int main(int argc, char *argv[]){
    Schematic s;
    time_t luckyNum = time(NULL);
    srand(luckyNum);
    // cout << "Lucky number: " << luckyNum << '\n';
    
    // uint64_t start = timeSinceEpochMillisec(); // timer start
    s.build(argv[1]); 
    s.simulateAnnealing();
    s.printOutput(argv[2]);
    // cout << "Best HPWL: " << s.best_HPWL << '\n';
    // uint64_t end = timeSinceEpochMillisec();
    // cout << "Total time: " << end - start << "ms\n";

    return 0;
}