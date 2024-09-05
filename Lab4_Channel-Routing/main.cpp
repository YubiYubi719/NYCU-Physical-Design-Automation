#include "channelRouting_DL.cpp"
#include "channelRouting.cpp"
#include <ctime>
// #define TEST

int main(int argc, char* argv[]){
#ifdef TEST
    const clock_t start = clock();
#endif
    // #######################################
    // #    Left Edge Dogleg Channel Router  #
    // #######################################
    ChannelRouter_DL cr_dl;
    cr_dl.build(argv[1]);
    cr_dl.routeTOP();
    cr_dl.routeBOT();
    // #######################################
    // #       Left Edge Channel Router      #
    // #######################################
    ChannelRouter cr;
    cr.build(argv[1]);
    cr.routeTOP();
    cr.routeBOT();
    // #######################################
    // #             Write Output            #
    // #######################################
    if(cr_dl.midTracks.size() < cr.midTracks.size()) cr_dl.writeOutput(argv[2]);
    else cr.writeOutput(argv[2]);

#ifdef TEST
    cout << "CPU runtime: " << (double)(clock() - start) / CLOCKS_PER_SEC << " seconds\n";
#endif
    return 0;
}