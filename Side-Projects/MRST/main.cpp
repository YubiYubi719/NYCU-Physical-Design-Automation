#include "MST.h"

int main(int argc, char* argv[]){
    MST mst;
    mst.genNodes();
    mst.init();
    mst.Kruskal_Algorithm();
    mst.rectilinearize();
    
    mst.printGraph();
    cout << "caseI_times: " << MST::caseI_times << '\n' 
         << "caseII_times: " << MST::caseII_times << '\n' 
         << "caseIII_times: " << MST::caseIII_times << '\n'
         << "Default_times: " << MST::default_times << '\n';

    return 0;
}