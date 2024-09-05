#include "channelRouting_DL.h"
#include "channelRouting.h"
#include <ctime>

int main(int argc, char* argv[]){
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


    return 0;
}