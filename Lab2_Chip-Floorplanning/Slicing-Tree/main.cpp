#include "ST.h"
#include <chrono>
using namespace std::chrono;

int main(int argc, char* argv[]){
    ST st;
    vector<string> s;
    srand(time(NULL));

    st.build(argv[1]);
    st.initString(s);
    vector<Block*> npe;
    while(true){
        npe = st.simulateAnnealing(s);
        if(st.isValid(npe)) break;
    }
    st.retrace(npe);
    st.printResult(npe,argv[2]);
    
    // Show NPE
    for(size_t i = 0; i < s.size(); i++) cout << s[i];
    cout << '\n';
    cout << st.bc << '\n';

    return 0;
}