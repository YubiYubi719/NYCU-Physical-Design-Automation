#include "channelRouting_DL.h"
#define DEBUG

void ChannelRouter_DL::build(string filename){
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
            while(ss >> pinID) topPins.push_back(Pin_DL(pinID-1));
            
            getline(input,curLine);
            ss.clear();
            ss.str(curLine);
            while(ss >> pinID) botPins.push_back(Pin_DL(pinID-1));
        }
    }
    updateBlockage();
    updateNetInf();
    cutNet();
}

void ChannelRouter_DL::updateBlockage(){
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

void ChannelRouter_DL::updateNetInf(){
    int pinNum = topPins.size();
    for(int i = 0; i < pinNum; i++){
        int topPin = topPins[i].originID;
        int botPin = botPins[i].originID;
        int netNum = max(topPin,botPin) + 1;
        if(netNum > nets.size()) nets.resize(netNum);
        if(topPin != -1){
            if(nets[topPin].start == -1) nets[topPin].start = i;
            else{
                nets[topPin].end = i;
                if(botPin == -1) nets[topPin].cutPosition.push_back(i);
            }
        }
        if(botPin != -1){
            if(nets[botPin].start == -1) nets[botPin].start = i;
            else{
                nets[botPin].end = i;
                if(topPin == -1) nets[botPin].cutPosition.push_back(i);
            }
        }

    }
    // update name of each net and insert "end" into their cutPosition
    int len = nets.size();
    for(int i = 0; i < len; i++){
        Net_DL &curNet = nets[i];
        curNet.name = i+1;
        if(curNet.cutPosition.empty()){
            curNet.cutPosition.push_back(curNet.end);
            continue;
        }
        int curEnd = curNet.cutPosition.back();
        if(curEnd != curNet.end) curNet.cutPosition.push_back(curNet.end);
    }
}

void ChannelRouter_DL::cutNet(){
    for(Net_DL &net:nets){
        // ########################################
        // #           Don't need to cut          #
        // ########################################
        if(net.cutPosition.size() == 1){
            shared_ptr<SubNet_DL> subnet = make_shared<SubNet_DL>(net.start,net.end,net.name,to_string(net.name));
            subnets.push_back(subnet);
            net.netList.push_back(subnet);
            // Update pin's name
            // Start of subnet
            if(topPins[net.start].originID == net.name-1) topPins[net.start].sortingName = (*subnet).name;
            else botPins[net.start].sortingName = (*subnet).name;
            // End of subnet
            if(topPins[net.end].originID == net.name-1) topPins[net.end].sortingName = (*subnet).name;
            else botPins[net.end].sortingName = (*subnet).name;
            // Middle of subnet
            for(int i = net.start; i <= net.end; i++){
                if(topPins[i].originID == net.name-1) topPins[i].sortingName = (*subnet).name;
                if(botPins[i].originID == net.name-1) botPins[i].sortingName = (*subnet).name;
            }
            continue;
        }
        // ########################################
        // #             Need to cut              #
        // ########################################
        // Deal with first subnet------------------
        shared_ptr<SubNet_DL> subnet0 = make_shared<SubNet_DL>(net.start,net.cutPosition[0],net.name,to_string(net.name)+"_0");
        subnets.push_back(subnet0);
        net.netList.push_back(subnet0);
        // Update pin's name
        // Start of subnet
        if(topPins[net.start].originID == net.name-1) topPins[net.start].sortingName = (*subnet0).name;
        else botPins[net.start].sortingName = (*subnet0).name;
        // End of subnet
        if(topPins[net.cutPosition[0]].originID == net.name-1) topPins[net.cutPosition[0]].sortingName = (*subnet0).name;
        else botPins[net.cutPosition[0]].sortingName = (*subnet0).name;
        // Middle of subnet
        for(int i = net.start; i <= net.cutPosition[0]; i++){
            if(topPins[i].originID == net.name-1) topPins[i].sortingName = (*subnet0).name;
            if(botPins[i].originID == net.name-1) botPins[i].sortingName = (*subnet0).name;
        }
        // Deal with others------------------------
        int len = net.cutPosition.size();
        for(int i = 0; i < len-1; i++){
            shared_ptr<SubNet_DL> subnet = make_shared<SubNet_DL>(net.cutPosition[i],net.cutPosition[i+1],net.name,to_string(net.name)+"_"+to_string(i+1));
            subnets.push_back(subnet);
            net.netList.push_back(subnet);
            // Update pin's name
            // Start of subnet
            if(topPins[net.cutPosition[i]].originID == net.name-1) topPins[net.cutPosition[i]].sortingName = (*subnet).name;
            else botPins[net.cutPosition[i]].sortingName = (*subnet).name;
            // End of subnet
            if(topPins[net.cutPosition[i+1]].originID == net.name-1) topPins[net.cutPosition[i+1]].sortingName = (*subnet).name;
            else botPins[net.cutPosition[i+1]].sortingName = (*subnet).name;
            // Middle of subnet
            for(int j = net.cutPosition[i]; j <= net.cutPosition[i+1]; j++){
                if(topPins[j].originID == net.name-1) topPins[j].sortingName = (*subnet).name;
                if(botPins[j].originID == net.name-1) botPins[j].sortingName = (*subnet).name;
            }
        }
    }
}

vector<shared_ptr<SubNet_DL>> ChannelRouter_DL::chooseNets_TOP(){
    // Choose net from "top-->bot VCG" to avoid vertical constraint
    int subnetNum = subnets.size();
    vector<shared_ptr<SubNet_DL>> zeroDegreeNets;
    // Pick up all unplaced zero-degree nets
    for(shared_ptr<SubNet_DL> subnet:subnets)
        if(!subnet->isPlaced && (!in_degree_TOP.count(subnet->name) || in_degree_TOP[subnet->name] == 0)) zeroDegreeNets.push_back(subnet);
    // Sort by their start coordinate
    sort(zeroDegreeNets.begin(),zeroDegreeNets.end(),SubNet_DL::myCompare);
    
    return zeroDegreeNets;
}


vector<shared_ptr<SubNet_DL>> ChannelRouter_DL::chooseNets_BOT(){
    // Choose net from "bot-->top VCG" to avoid vertical constraint
    int subnetNum = subnets.size();
    vector<shared_ptr<SubNet_DL>> zeroDegreeNets;
    // Pick up all unplaced zero-degree nets
    for(shared_ptr<SubNet_DL> subnet:subnets)
        if(!subnet->isPlaced && (!in_degree_BOT.count(subnet->name) || in_degree_BOT[subnet->name] == 0)) zeroDegreeNets.push_back(subnet);
    // Sort by their start coordinate
    sort(zeroDegreeNets.begin(),zeroDegreeNets.end(),SubNet_DL::myCompare);
    
    return zeroDegreeNets;
}

void ChannelRouter_DL::routeTOP(){
    // Initialize in_degree_TOP and edges_TOP of each net
    int pinNum = topPins.size();
    for(int i = 0; i < pinNum; i++){
        if(topPins[i].originID == -1 || botPins[i].originID == -1 || (topPins[i].originID == botPins[i].originID)) continue;
        // Push "topPin --> botPin" edge into unordered_map
        if(!edges_TOP.count(topPins[i].sortingName)) edges_TOP.insert({topPins[i].sortingName,{botPins[i].sortingName}});
        else edges_TOP[topPins[i].sortingName].push_back(botPins[i].sortingName);
        // in_degree[botPins[i]]++;
        if(!in_degree_TOP.count(botPins[i].sortingName)) in_degree_TOP.insert({botPins[i].sortingName,1});
        else in_degree_TOP[botPins[i].sortingName]++;
    }

    // ########################################
    // #           Topological Sort           #
    // ########################################
    // Put unplaced and zero-degree net into T_queue,
    // if there exist more than 1 zero-degree nets,
    // fill out the track based on their start and end coordinate,
    // and move on to the next track until all nets are placed or topTrack are filled
    queue<vector<shared_ptr<SubNet_DL>>> T_queue;
    T_queue.push(chooseNets_TOP()); // push the start nets into T_queue
    int trackID = 1; // from bot(ignore 0 since all blocked) to top --> 1 ~ n
    while(!T_queue.front().empty() && trackID < topTracks.size()){
        vector<shared_ptr<SubNet_DL>> curNets = T_queue.front();
        T_queue.pop();
        // todo: place the nets and update "in_degree_TOP"
        placeNetsFromTop(curNets,trackID);

        // todo: push new zero-degree nets into T_queue
        T_queue.push(chooseNets_TOP());
        trackID++;
    }
}

void ChannelRouter_DL::routeBOT(){
    // Initialize in_degree and edges of each net
    int pinNum = topPins.size();
    for(int i = 0; i < pinNum; i++){
        if(topPins[i].originID == -1 || botPins[i].originID == -1 || (topPins[i].originID == botPins[i].originID)) continue;
        // Push "botPin --> topPin" edge into unordered_map
        if(!edges_BOT.count(botPins[i].sortingName)) edges_BOT.insert({botPins[i].sortingName,{topPins[i].sortingName}});
        else edges_BOT[botPins[i].sortingName].push_back(topPins[i].sortingName);
        // in_degree[topPins[i]]++;
        if(!in_degree_BOT.count(topPins[i].sortingName)) in_degree_BOT.insert({topPins[i].sortingName,1});
        else in_degree_BOT[topPins[i].sortingName]++;
    }

    // ########################################
    // #           Topological Sort           #
    // ########################################
    // Put unplaced and zero-degree net into T_queue,
    // if there exist more than 1 zero-degree nets,
    // fill out the track based on their start and end coordinate,
    // and move on to the next track until all nets are placed
    queue<vector<shared_ptr<SubNet_DL>>> T_queue;
    T_queue.push(chooseNets_BOT()); // push the start nets into T_queue
    int trackID = 1; // from bot(ignore 0 since all blocked) to top --> 1 ~ n
    while(!T_queue.front().empty()){
        vector<shared_ptr<SubNet_DL>> curNets = T_queue.front();
        T_queue.pop();
        // todo: place the nets and update "in_degree"
        placeNetsFromBot(curNets,trackID);

        // todo: push new zero-degree nets into T_queue
        T_queue.push(chooseNets_BOT());
        trackID++;
    }
}

void ChannelRouter_DL::placeNetsFromTop(vector<shared_ptr<SubNet_DL>> sortedNets, int trackID){
    // todo: During placing the nets, remember to update their neighbor's in_degree
    // ########################################
    // #         curTrack is topTrack         #
    // ########################################
    TBtrack_DL &curTrack = *(topTracks.rbegin()+trackID);
    for(shared_ptr<SubNet_DL> subnet:sortedNets){
        // Check whether the track can accomodate the subnet
        if((curTrack.trackSegs[subnet->start] && curTrack.prevNet != subnet->parent) || curTrack.trackSegs[subnet->end]) continue;
        bool isConsecutive = true;
        for(int i = subnet->start+1; i < subnet->end; i++){
            if(curTrack.trackSegs[i]){
                isConsecutive = false;
                break;
            }
        }
        // Place the subnet
        if(isConsecutive){
            subnet->isPlaced = true;
            subnet->track = curTrack.name;
            for(int i = subnet->start; i <= subnet->end; i++) curTrack.trackSegs[i] = true;
            curTrack.prevNet = subnet->parent;
            // Update vertical constraint subnet's in_degree
            for(string VCnet:edges_TOP[subnet->name]) in_degree_TOP[VCnet]--;
        }
    }
}

void ChannelRouter_DL::placeNetsFromBot(vector<shared_ptr<SubNet_DL>> sortedNets, int trackID){
    // todo: During placing the nets, remember to update their neighbor's in_degree
    // ########################################
    // #         curTrack is botTrack         #
    // ########################################
    if(trackID < botTracks.size()){
        TBtrack_DL &curTrack = *(botTracks.rbegin()+trackID);
        for(shared_ptr<SubNet_DL> subnet:sortedNets){
            // Check whether the track can accomodate the subnet
            if((curTrack.trackSegs[subnet->start] && curTrack.prevNet != subnet->parent) || curTrack.trackSegs[subnet->end]) continue;
            bool isConsecutive = true;
            for(int i = subnet->start+1; i < subnet->end; i++){
                if(curTrack.trackSegs[i]){
                    isConsecutive = false;
                    break;
                }
            }
            // Place the subnet
            if(isConsecutive){
                subnet->isPlaced = true;
                subnet->track = curTrack.name;
                for(int i = subnet->start; i <= subnet->end; i++) curTrack.trackSegs[i] = true;
                curTrack.prevNet = subnet->parent;
                // Update vertical constraint subnet's in_degree
                for(string VCnet:edges_BOT[subnet->name]) in_degree_BOT[VCnet]--;
            }
        }
        return;
    }
    // ########################################
    // #         curTrack is midTrack         #
    // ########################################
    // 1. Build a new track
    Track_DL curTrack("C"+to_string(trackID-botTracks.size()+1));
    // 2. Place nets into it and update watermark
    for(shared_ptr<SubNet_DL> subnet:sortedNets){
        if(subnet->start < curTrack.watermark
        || (subnet->start == curTrack.watermark && curTrack.prevNet != subnet->parent)) continue;
        subnet->isPlaced = true;
        subnet->track = curTrack.name;
        curTrack.watermark = subnet->end;
        curTrack.netList.push_back(subnet);
        curTrack.prevNet = subnet->parent;
        for(string VCnet:edges_BOT[subnet->name]) in_degree_BOT[VCnet]--;
    }
    midTracks.push_back(curTrack);
}

void ChannelRouter_DL::writeOutput(string filename){
    ofstream output(filename);
    output << "Channel density: " << midTracks.size() << '\n';
    for(Net_DL const &net:nets){
        output << "Net " << net.name << '\n';
        int len = net.netList.size();
        if(len > 1){
            // First output 2 subnets
            for(int i = 0; i < 2; i++){
                shared_ptr<SubNet_DL> const &subnet = net.netList[i];
                output << subnet->track << " " 
                       << subnet->start << " "
                       << subnet->end   << '\n';
            }
            if(net.netList[0]->track != net.netList[1]->track) output << "Dogleg " << net.netList[1]->start << '\n';
            // Output remaining subnets
            for(int i = 2; i < len; i++){
                shared_ptr<SubNet_DL> const &subnet = net.netList[i];
                shared_ptr<SubNet_DL> const &subnet_prev = net.netList[i-1];
                output << subnet->track << " " 
                       << subnet->start << " "
                       << subnet->end   << '\n';
                if(subnet->track != subnet_prev->track) output << "Dogleg " << subnet->start << '\n';
            }
        }
        else{
            // output the only subnet
            shared_ptr<SubNet_DL> const &subnet_last = net.netList.back();
            output << subnet_last->track << " "
                   << subnet_last->start << " "
                   << subnet_last->end   << '\n';
        }
    }
    output.close();
}