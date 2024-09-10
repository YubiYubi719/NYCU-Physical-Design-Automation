#include "ST.h"
#include <chrono>
using namespace std::chrono;

uint64_t timeSinceEpochMillisec() {
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

int main(int argc, char* argv[]){
    ST st;
    vector<string> s;
    srand(time(NULL));

    uint64_t start = timeSinceEpochMillisec(); // timer start
    st.build(argv[1]);
    st.initString(s);
    vector<Block*> npe;
    int iterTimes = 0;
    while(true){
        npe = st.simulateAnnealing(s);
        iterTimes++;
        cout << "Iteration: " << iterTimes;
        uint64_t end = timeSinceEpochMillisec();
        cout << "   current time: " << end - start << "ms\n";
        if(st.isValid(npe)) break;
    }
    st.retrace(npe);
    st.printResult(npe,argv[2]);
    
    for(size_t i = 0; i < s.size(); i++){
        cout << s[i];
    }
    cout << '\n';
    cout << st.bc << '\n';

    uint64_t end = timeSinceEpochMillisec();
    cout << "Total time: " << end - start << "ms\n";
    return 0;
}