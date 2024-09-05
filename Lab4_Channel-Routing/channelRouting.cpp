#include "channelRouting.h"

void ChannelRouter::build(string filename){
    ifstream input(filename);
    string curLine;
    stringstream ss;
    while(getline(input,curLine)){
        ss.clear();
        ss.str(curLine);
        if(curLine[0] == 'T'){
            string trackID_s;
            ss >> trackID_s;
            int trackID = stoi(trackID_s.erase(0,1));
            if(trackID+1 > topTracks.size()) topTracks.resize(trackID+1);
            int lb, rb;
            ss >> lb >> rb;
            if(rb+1 > topTracks[trackID].trackSegs.size()) topTracks[trackID].trackSegs.resize(rb+1,false);
            for(int i = lb; i <= rb; i++) topTracks[trackID].trackSegs[i] = true;
        }
        else if(curLine[0] == 'B'){
            string trackID_s;
            ss >> trackID_s;
            int trackID = stoi(trackID_s.erase(0,1));
            if(trackID+1 > botTracks.size()) botTracks.resize(trackID+1);
            int lb, rb;
            ss >> lb >> rb;
            if(rb+1 > botTracks[trackID].trackSegs.size()) botTracks[trackID].trackSegs.resize(rb+1,false);
            for(int i = lb; i <= rb; i++) botTracks[trackID].trackSegs[i] = true;
        }
        else{
            int pinID;
            while(ss >> pinID) topPins.push_back(pinID-1);
            
            getline(input,curLine);
            ss.clear();
            ss.str(curLine);
            while(ss >> pinID) botPins.push_back(pinID-1);
        }
    }
    updateBlockage();
    updateNetRange();
}

void ChannelRouter::updateBlockage(){
    int pinNum = topPins.size();
    // ########################################
    // #          Deal with topTrack          #
    // ########################################
    int trackNum = topTracks.size();
    // resize each track
    for(int i = 0; i < trackNum; i++){
        topTracks[i].trackSegs.resize(pinNum,false);
        topTracks[i].name = "T" + to_string(i);
    }
    // update blockage
    for(int i = 0; i < trackNum-1; i++){
        for(int j = 0; j < pinNum; j++) topTracks[i+1].trackSegs[j] = topTracks[i+1].trackSegs[j] || topTracks[i].trackSegs[j];
    }

    // ########################################
    // #          Deal with botTrack          #
    // ########################################
    trackNum = botTracks.size();
    // resize each track
    for(int i = 0; i < trackNum; i++){
        botTracks[i].trackSegs.resize(pinNum,false);
        botTracks[i].name = "B" + to_string(i);
    }
    // update blockage
    for(int i = 0; i < trackNum-1; i++){
        for(int j = 0; j < pinNum; j++) botTracks[i+1].trackSegs[j] = botTracks[i+1].trackSegs[j] || botTracks[i].trackSegs[j];
    }
}

void ChannelRouter::updateNetRange(){
    int pinNum = topPins.size();
    for(int i = 0; i < pinNum; i++){
        int topPin = topPins[i];
        int botPin = botPins[i];
        int netNum = max(topPin,botPin) + 1;
        if(netNum > nets.size()) nets.resize(netNum);
        if(topPin != -1){
            if(nets[topPin].start == -1) nets[topPin].start = i;
            else nets[topPin].end = i;
        }
        if(botPin != -1){
            if(nets[botPin].start == -1) nets[botPin].start = i;
            else nets[botPin].end = i;
        }
    }
    for(int i = 0; i < nets.size(); i++) nets[i].name = i+1;
}

vector<Net*> ChannelRouter::chooseNets_TOP(){
    int netNum = nets.size();
    vector<Net*> zeroDegreeNets;
    // pick up all unplaced zero-degree nets
    for(int i = 0; i < netNum; i++)
        if(!nets[i].isPlaced && in_degree_TOP[i] == 0) zeroDegreeNets.push_back(&nets[i]);
    // sort by their start coordinate
    sort(zeroDegreeNets.begin(),zeroDegreeNets.end(),Net::myCompare);
    
    return zeroDegreeNets;
}

vector<Net*> ChannelRouter::chooseNets(){
    int netNum = nets.size();
    vector<Net*> zeroDegreeNets;
    // pick up all unplaced zero-degree nets
    for(int i = 0; i < netNum; i++)
        if(!nets[i].isPlaced && in_degree[i] == 0) zeroDegreeNets.push_back(&nets[i]);
    // sort by their start coordinate
    sort(zeroDegreeNets.begin(),zeroDegreeNets.end(),Net::myCompare);
    
    return zeroDegreeNets;
}

void ChannelRouter::routeTOP(){
    int netNum = nets.size();
    edges_TOP.resize(netNum);
    in_degree_TOP.resize(netNum, 0);
    // initialize in_degree_TOP and edges_TOP of each net
    int pinNum = topPins.size();
    for(int i = 0; i < pinNum; i++){
        if(topPins[i] == -1 || botPins[i] == -1 || topPins[i] == botPins[i]) continue;
        edges_TOP[topPins[i]].push_back(botPins[i]);
        in_degree_TOP[botPins[i]]++;
    }

    // ########################################
    // #           Topological Sort           #
    // ########################################
    // put zero-degree net into T_queue,
    // if there exist more than 1 zero-degree nets,
    // fill out the track based on their start and end coordinate,
    // and move on to the next track until all nets are placed
    queue<vector<Net*>> T_queue;
    T_queue.push(chooseNets_TOP()); // push the start nets into T_queue
    int trackID = 1; // from bot(ignore 0 since all blocked) to top --> 1 ~ n
    while(!T_queue.front().empty() && trackID < topTracks.size()){
        vector<Net*> curNets = T_queue.front();
        T_queue.pop();
        // todo: place the nets and update "in_degree"
        placeNetsFromTop(curNets,trackID);

        // todo: push new zero-degree nets into T_queue
        T_queue.push(chooseNets_TOP());
        trackID++;
    }
}

void ChannelRouter::routeBOT(){
    int netNum = nets.size();
    edges.resize(netNum);
    in_degree.resize(netNum, 0);
    // initialize in_degree and edges of each net
    int pinNum = topPins.size();
    for(int i = 0; i < pinNum; i++){
        if(topPins[i] == -1 || botPins[i] == -1 || topPins[i] == botPins[i]) continue;
        edges[botPins[i]].push_back(topPins[i]);
        in_degree[topPins[i]]++;
    }

    // ########################################
    // #           Topological Sort           #
    // ########################################
    // put zero-degree net into T_queue,
    // if there exist more than 1 zero-degree nets,
    // fill out the track based on their start and end coordinate,
    // and move on to the next track until all nets are placed
    queue<vector<Net*>> T_queue;
    T_queue.push(chooseNets()); // push the start nets into T_queue
    int trackID = 1; // from bot(ignore 0 since all blocked) to top --> 1 ~ n
    while(!T_queue.front().empty()){
        vector<Net*> curNets = T_queue.front();
        T_queue.pop();
        // todo: place the nets and update "in_degree"
        placeNetsFromBot(curNets,trackID);

        // todo: push new zero-degree nets into T_queue
        T_queue.push(chooseNets());
        trackID++;
    }
}
void ChannelRouter::placeNetsFromTop(vector<Net*> sortedNets, int trackID){
    TBtrack &curTrack = *(topTracks.rbegin()+trackID);
    for(Net* net:sortedNets){
        // check whether the track can accomodate the net
        if(curTrack.trackSegs[net->start] || curTrack.trackSegs[net->end]) continue;
        bool isConsecutive = true;
        for(int i = net->start+1; i < net->end; i++){
            if(curTrack.trackSegs[i]){
                isConsecutive = false;
                break;
            }
        }
        // place the net
        if(isConsecutive){
            net->isPlaced = true;
            net->track = curTrack.name;
            for(int i = net->start; i <= net->end; i++) curTrack.trackSegs[i] = true;
            // update vertical constraint net's in_degree
            for(int VCnet:edges_TOP[net->name-1]) in_degree_TOP[VCnet]--;
        }
    }
}

void ChannelRouter::placeNetsFromBot(vector<Net*> sortedNets, int trackID){
    // during placing the nets, remember to update their neighbor's in_degree
    // ########################################
    // #         curTrack is botTrack         #
    // ########################################
    if(trackID < botTracks.size()){
        TBtrack &curTrack = *(botTracks.rbegin()+trackID);
        for(Net* net:sortedNets){
            // check whether the track can accomodate the net
            if(curTrack.trackSegs[net->start] || curTrack.trackSegs[net->end]) continue;
            bool isConsecutive = true;
            for(int i = net->start+1; i < net->end; i++){
                if(curTrack.trackSegs[i]){
                    isConsecutive = false;
                    break;
                }
            }
            // place the net
            if(isConsecutive){
                net->isPlaced = true;
                net->track = curTrack.name;
                for(int i = net->start; i <= net->end; i++) curTrack.trackSegs[i] = true;
                // update vertical constraint net's in_degree
                for(int VCnet:edges[net->name-1]) in_degree[VCnet]--;
            }
        }
        return;
    }
    // ########################################
    // #       curTrack is not botTrack       #
    // ########################################
    // 1. build a new track
    Track curTrack("C"+to_string(trackID-botTracks.size()+1));
    // 2. place nets into it and update watermark
    for(Net* net:sortedNets){
        if(net->start > curTrack.watermark){
            net->isPlaced = true;
            net->track = curTrack.name;
            curTrack.watermark = net->end;
            curTrack.netList.push_back(net);
            for(int VCnet:edges[net->name-1]) in_degree[VCnet]--;
        }
    }
    midTracks.push_back(curTrack);
}

void ChannelRouter::writeOutput(string filename){
    ofstream output(filename);
    output << "Channel density: " << midTracks.size() << '\n';
    for(Net const &net:nets){
        output << "Net " << net.name << '\n';
        output << net.track << " " 
               << net.start << " "
               << net.end   << '\n';
    }
    output.close();
}