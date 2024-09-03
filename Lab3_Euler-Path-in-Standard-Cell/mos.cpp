#include "mos.h"

void Schematic::build(string filename){
    ifstream input(filename);
    string curLine;
    while(getline(input,curLine)){
        stringstream ss(curLine);
        string s;
        ss >> s;
        if(s == ".ENDS") break;
        if(s == ".SUBCKT") continue;
        string name, drain, gate, source, substrate, type;
        string w, l;
        name = s;
        ss >> drain >> gate >> source >> substrate >> type;
        ss >> w >> l;
        name.erase(0,1);
        w.erase(0,2);
        w.erase(w.length()-1,1);
        l.erase(0,2);
        l.erase(l.length()-1,1);
        mosInf.push_back(MOS(name,drain,gate,source,type));
        if(type == "nmos_rvt") w_n = stod(w);
        else w_p = stod(w);
        inputGates.insert(gate);
    }
    buildGateMap();
}

void Schematic::buildGateMap(){
    for(const string &gate:inputGates) gateMap.insert({gate,{}});
    for(const MOS &mos:mosInf){
        bool isFind = false;
        if(mos.n_p){        
            for(pair<MOS,MOS> &p:gateMap[mos.gate]){
                if(p.first.isDefined == false){
                    isFind = true;
                    p.first = mos;
                    break;
                }
            }
        }
        else{ // mos.type == "pmos_rvt"
            for(pair<MOS,MOS> &p:gateMap[mos.gate]){
                if(p.second.isDefined == false){
                    isFind = true;
                    p.second = mos;
                    break;
                }
            }
        }
        if(!isFind){
            if(mos.n_p) gateMap[mos.gate].push_back({mos,MOS()});
            else gateMap[mos.gate].push_back({MOS(),mos});
        }
    }
}

void Schematic::test(){
    vector<string> gateOrder;
    // for(string const &gate:inputGates) gateOrder.push_back(gate);
    gateOrder = {"A","B","C","D","E","F","G"};
    pair<vector<MOS>,vector<MOS>> pathOrder = initPathOrder(gateOrder);
    path_n = cutPath(pathOrder.first);
    path_p = cutPath(pathOrder.second);
    insertDummy(path_n,path_p);
    cout << calHPWL(path_n,path_p) << '\n';
    cout << "test" << '\n';
}

double Schematic::initialTemperature(pair<vector<MOS>,vector<MOS>> &pathOrder){
    path_n = cutPath(pathOrder.first);
    path_p = cutPath(pathOrder.second);
    insertDummy(path_n,path_p);
    double oc = 0.0;
    int uphill_move = 0;
    double uphillTemp = 0.0;
    for(int i = 0; i < 100; i++){
        if(oc == 0.0) oc = calHPWL(path_n,path_p);

        // SOL tmpResult(path_n,path_p,pinSeq_n,pinSeq_p);
        pair<vector<MOS>,vector<MOS>> tmp_pathOrder = pathOrder;

        // random swap
        int op = rand() % 2;
        switch(op){
            case 0: M1(pathOrder); break;
            case 1: M2(pathOrder); break;
        }
        path_n = cutPath(pathOrder.first);
        path_p = cutPath(pathOrder.second);
        insertDummy(path_n,path_p);

        double nc = calHPWL(path_n,path_p);
        double diff = nc - oc;
        if(diff > 0){
            // reject the move, but record uphill information
            uphill_move++;
            uphillTemp += diff;
            pathOrder = tmp_pathOrder;
        }
        else if(diff < 0) oc = nc; // accept the move
    }
    double uphillTemp_avg = uphillTemp / uphill_move;
    double T0 = 3 * uphillTemp_avg;

    return (uphill_move)? T0:100*pathOrder.first.size();
}

void Schematic::simulateAnnealing(){
    vector<string> gateOrder;
    for(string const &gate:inputGates) gateOrder.push_back(gate);
    pair<vector<MOS>,vector<MOS>> pathOrder = initPathOrder(gateOrder);
    
    const int N = 10 * pathOrder.first.size();
    double T0 = min(10000.0,initialTemperature(pathOrder));
    int total_move, uphill_move, reject_move;
    double T = T0;
    double bestCost = DBL_MAX;
    SOL best_sol;

    while(true){
        total_move = 0;
        uphill_move = 0;
        // reject_move = 0;
        double oc = 0.0;
        while(true){
            if(oc == 0.0) oc = calHPWL(path_n,path_p);
            // store current results
            // SOL tmpResult(path_n,path_p,pinSeq_n,pinSeq_p);
            pair<vector<MOS>,vector<MOS>> tmp_pathOrder = pathOrder;

            // random op
            int op = rand() % 2;
            switch(op){
                case 0: M1(pathOrder); break;
                case 1: M2(pathOrder); break;
            }
            path_n = cutPath(pathOrder.first);
            path_p = cutPath(pathOrder.second);
            insertDummy(path_n,path_p);

            total_move++;
            double nc = calHPWL(path_n,path_p);
            double diff = nc - oc;

            double R = (double)rand() / RAND_MAX;
            if(diff < 0 || R < exp(-1 * diff / T)){
                // accept current move
                oc = nc;

                if(diff > 0) uphill_move++;
                if(nc < bestCost) {
                    // store the best cost and current result
                    bestCost = nc;
                    best_HPWL = nc;
                    best_sol = SOL(path_n,path_p);
                }
            }
            else{
                // reject current move
                // thus restore old pathOrder and keep oc
                pathOrder = tmp_pathOrder;
                // reject_move++;
            }

            if(uphill_move > N || total_move > 2*N) break;
        }

        T *= 0.85;
        if(T < 0.001) break;
    }
    results = best_sol;
}

void Schematic::M1(pair<vector<MOS>,vector<MOS>> &pathOrder){
    vector<int> posVector(pathOrder.first.size());
    for(int i = 0; i < pathOrder.first.size(); i++) posVector[i] = i;

    int pos_1 = rand() % posVector.size();
    posVector.erase(posVector.begin()+pos_1);
    int pos_2 = rand() % posVector.size();

    swap(pathOrder.first[pos_1],pathOrder.first[pos_2]);
    swap(pathOrder.second[pos_1],pathOrder.second[pos_2]);
}

void Schematic::M2(pair<vector<MOS>,vector<MOS>> &pathOrder){
    // move pos_2 to pos_1's front or back
    vector<int> posVector(pathOrder.first.size());
    for(int i = 0; i < pathOrder.first.size(); i++) posVector[i] = i;

    int pos_1 = rand() % posVector.size();
    posVector.erase(posVector.begin()+pos_1);
    int pos_2 = rand() % posVector.size();

    int op = rand() % 2; // op = 0 --> move to pos_1's front
                         // op = 1 --> move to pos_1's back
    int diff = abs(pos_2 - pos_1);
    if(diff == 1){
        swap(pathOrder.first[pos_1],pathOrder.first[pos_2]);
        swap(pathOrder.second[pos_1],pathOrder.second[pos_2]);
        return;
    }
    if(pos_1 < pos_2){
        if(op) diff--;
        for(int i = 0; i < diff; i++,pos_2--){
            swap(pathOrder.first[pos_2],pathOrder.first[pos_2-1]);
            swap(pathOrder.second[pos_2],pathOrder.second[pos_2-1]);
        }
    }
    else{
        if(!op) diff--;
        for(int i = 0; i < diff; i++,pos_2++){
            swap(pathOrder.first[pos_2],pathOrder.first[pos_2+1]);
            swap(pathOrder.second[pos_2],pathOrder.second[pos_2+1]);
        }
    }
}

pair<vector<MOS>,vector<MOS>> Schematic::initPathOrder(vector<string> const &gateOrder){
    vector<MOS> nmosPath, pmosPath;
    for(const string &gate:gateOrder){
        vector<pair<MOS,MOS>> gv = gateMap[gate];
        for(pair<MOS,MOS> g:gv){
            nmosPath.push_back(g.first);
            pmosPath.push_back(g.second);
        }
    }
    return {nmosPath,pmosPath};
}

vector<MOS> Schematic::cutPath(vector<MOS> path){
    int len = path.size();
    for(int i = 0; i < len-1; i++){
        MOS &a = path[i];
        MOS &b = path[i+1];
        if(!a.isConnect_drain && (a.drain == b.drain)){
            b.isConnect_drain = true;
            a.d_s = false;
            b.d_s = true;
        }
        else if(!a.isConnect_drain && (a.drain == b.source)){
            b.isConnect_source = true;
            a.d_s = false;
            b.d_s = false;
        }
        else if(!a.isConnect_source && (a.source == b.drain)){
            b.isConnect_drain = true;
            a.d_s = true;
            b.d_s = true;
        }
        else if(!a.isConnect_source && (a.source == b.source)){
            b.isConnect_source = true;
            a.d_s = true;
            b.d_s = false;
        }
        else a.insertDummy = true;
    }
    return path;
}

void Schematic::insertDummy(vector<MOS> &prePath_n, vector<MOS> &prePath_p){
    auto iter1 = prePath_n.begin();
    auto iter2 = prePath_p.begin();
    while(iter1 != prePath_n.end() && iter2 != prePath_p.end()){
        if((*iter1).insertDummy && !(*iter2).insertDummy) (*iter2).insertDummy = true;
        else if(!(*iter1).insertDummy && (*iter2).insertDummy) (*iter1).insertDummy = true;
        iter1++;
        iter2++;
    }
}

double Schematic::calHPWL(vector<MOS> const &path_n, vector<MOS> const &path_p){
    unordered_map<string,tuple<int,int,bool,bool>> diffPos; // diffusion position
                                                            // tuple[0]: min x coordinate
                                                            // tuple[1]: max x coordinate
                                                            // tuple[2]: appear in n diffusion
                                                            // tuple[3]: appear in p diffusion
    auto iter = path_n.begin();
    // deal with first mos
    if((*iter).d_s){
        diffPos.insert({(*iter).drain,{0,0,true,false}});
        diffPos.insert({(*iter).source,{1,1,true,false}});
    }
    else{
        diffPos.insert({(*iter).source,{0,0,true,false}});
        diffPos.insert({(*iter).drain,{1,1,true,false}});
    }
    int x = ((*iter).insertDummy)?3:2;
    // deal with remain mos
    iter++;
    while(true){
        if((*iter).d_s){
            if((*prev(iter)).insertDummy){
                if(!diffPos.count((*iter).drain)){
                    diffPos.insert({(*iter).drain,{x-1,x-1,true,false}});
                }
                else{
                    if(x-1 < get<0>(diffPos[(*iter).drain])) get<0>(diffPos[(*iter).drain]) = x-1;
                    else if(x-1 > get<1>(diffPos[(*iter).drain])) get<1>(diffPos[(*iter).drain]) = x-1;
                }
            }
            if(!diffPos.count((*iter).source)){
                diffPos.insert({(*iter).source,{x,x,true,false}});
            }
            else get<1>(diffPos[(*iter).source]) = x;
            x = ((*iter).insertDummy)? x+3:x+1;
            if(next(iter) == path_n.end()) break;
            iter++;
        }
        else{
            if((*prev(iter)).insertDummy){
                if(!diffPos.count((*iter).source)){
                    diffPos.insert({(*iter).source,{x-1,x-1,true,false}});
                }
                else{
                    if(x-1 < get<0>(diffPos[(*iter).source])) get<0>(diffPos[(*iter).source]) = x-1;
                    else if(x-1 > get<1>(diffPos[(*iter).source])) get<1>(diffPos[(*iter).source]) = x-1;
                }
            }
            if(!diffPos.count((*iter).drain)){
                diffPos.insert({(*iter).drain,{x,x,true,false}});
            }
            else get<1>(diffPos[(*iter).drain]) = x;
            x = ((*iter).insertDummy)? x+3:x+1;
            if(next(iter) == path_n.end()) break;
            iter++;
        }
    }

    iter = path_p.begin();
    // deal with first mos
    if((*iter).d_s){
        // drain
        if(!diffPos.count((*iter).drain)){
            diffPos.insert({(*iter).drain,{0,0,false,true}});
        }
        else{
            get<0>(diffPos[(*iter).drain]) = 0;
            get<3>(diffPos[(*iter).drain]) = true;
        }
        // source
        if(!diffPos.count((*iter).source)){
            diffPos.insert({(*iter).source,{1,1,false,true}});
        }
        else{
            if(1 < get<0>(diffPos[(*iter).source])) get<0>(diffPos[(*iter).source]) = 1;
            else if(1 > get<1>(diffPos[(*iter).source])) get<1>(diffPos[(*iter).source]) = 1;
            get<3>(diffPos[(*iter).source]) = true;
        }
    }
    else{
        // source
        if(!diffPos.count((*iter).source)){
            diffPos.insert({(*iter).source,{0,0,false,true}});
        }
        else{
            get<0>(diffPos[(*iter).source]) = 0;
            get<3>(diffPos[(*iter).source]) = true;
        }
        // drain
        if(!diffPos.count((*iter).drain)){
            diffPos.insert({(*iter).drain,{1,1,false,true}});
        }
        else{
            if(1 < get<0>(diffPos[(*iter).drain])) get<0>(diffPos[(*iter).drain]) = 1;
            else if(1 > get<1>(diffPos[(*iter).drain])) get<1>(diffPos[(*iter).drain]) = 1;
            get<3>(diffPos[(*iter).drain]) = true;
        }
    }

    x = ((*iter).insertDummy)?3:2;
    iter++;
    while(true){
        if((*iter).d_s){
            if((*prev(iter)).insertDummy){
                if(!diffPos.count((*iter).drain)){
                    diffPos.insert({(*iter).drain,{x-1,x-1,false,true}});
                }
                else{
                    if(x-1 < get<0>(diffPos[(*iter).drain])) get<0>(diffPos[(*iter).drain]) = x-1;
                    else if(x-1 > get<1>(diffPos[(*iter).drain])) get<1>(diffPos[(*iter).drain]) = x-1;
                    get<3>(diffPos[(*iter).drain]) = true;
                }
            }
            if(!diffPos.count((*iter).source)){
                diffPos.insert({(*iter).source,{x,x,false,true}});
            }
            else{
                if(x < get<0>(diffPos[(*iter).source])) get<0>(diffPos[(*iter).source]) = x;
                else if(x > get<1>(diffPos[(*iter).source])) get<1>(diffPos[(*iter).source]) = x;
                get<3>(diffPos[(*iter).source]) = true;
            }
            x = ((*iter).insertDummy)? x+3:x+1;
            if(next(iter) == path_p.end()) break;
            iter++;
        }
        else{
            if((*prev(iter)).insertDummy){
                if(!diffPos.count((*iter).source)){
                    diffPos.insert({(*iter).source,{x-1,x-1,false,true}});
                }
                else{
                    if(x-1 < get<0>(diffPos[(*iter).source])) get<0>(diffPos[(*iter).source]) = x-1;
                    else if(x-1 > get<1>(diffPos[(*iter).source])) get<1>(diffPos[(*iter).source]) = x-1;
                    get<3>(diffPos[(*iter).source]) = true;
                }
            }
            if(!diffPos.count((*iter).drain)){
                diffPos.insert({(*iter).drain,{x,x,false,true}});
            }
            else{
                if(x < get<0>(diffPos[(*iter).drain])) get<0>(diffPos[(*iter).drain]) = x;
                else if(x > get<1>(diffPos[(*iter).drain])) get<1>(diffPos[(*iter).drain]) = x;
                get<3>(diffPos[(*iter).drain]) = true;
            }
            x = ((*iter).insertDummy)? x+3:x+1;
            if(next(iter) == path_p.end()) break;
            iter++;
        }
    }

    int totalLength = x-1;
    double HPWL = 0.0;
    for(auto const &D:diffPos){
        int x_left = get<0>(D.second);
        int x_right = get<1>(D.second);
        int x_diff = x_right - x_left;

        if(x_diff){
            if(x_left == 0 && x_right == totalLength){
                HPWL += 25 + (x_diff-1) * 34 + x_diff * 20;
            }
            else if(x_left == 0 || x_right == totalLength){
                HPWL += 12.5 + 17 + (x_diff-1) * 34 + x_diff * 20;
            }
            else {
                HPWL += x_diff * 54;
            }
        }

        if(get<2>(D.second) && get<3>(D.second)){
            HPWL += w_n/2 + w_p/2 + 27;
        }
    }

    return HPWL;
}

void Schematic::printOutput(string filename){
    ofstream output(filename);
    output << best_HPWL << '\n';
    // output gate name------------------------------------------------------------
    for(auto iter = results.path_p.begin(); iter != results.path_p.end(); iter++){
        output << (*iter).name << " ";
        if((*iter).insertDummy) output << "Dummy ";
    }
    output << '\n';
    // output diffusion name-------------------------------------------------------
    for(auto iter = results.path_p.begin(); iter != results.path_p.end(); iter++){
        if(iter == results.path_p.begin()){
            if((*iter).d_s){
                output << (*iter).drain << " " << (*iter).gate << " " << (*iter).source << " ";
            }
            else{
                output << (*iter).source << " " << (*iter).gate << " " << (*iter).drain << " ";
            }
            if((*iter).insertDummy) output << "Dummy ";
            continue;
        }

        if((*iter).d_s){
            if((*prev(iter)).insertDummy) output << (*iter).drain << " ";
            output << (*iter).gate << " " << (*iter).source << " ";
        }
        else{
            if((*prev(iter)).insertDummy) output << (*iter).source << " ";
            output << (*iter).gate << " " << (*iter).drain << " ";
        }
        if((*iter).insertDummy) output << "Dummy ";
    }
    output << '\n';

    // output gate name------------------------------------------------------------
    for(auto iter = results.path_n.begin(); iter != results.path_n.end(); iter++){
        output << (*iter).name << " ";
        if((*iter).insertDummy) output << "Dummy ";
    }
    output << '\n';
    // output diffusion name-------------------------------------------------------
    for(auto iter = results.path_n.begin(); iter != results.path_n.end(); iter++){
        if(iter == results.path_n.begin()){
            if((*iter).d_s){
                output << (*iter).drain << " " << (*iter).gate << " " << (*iter).source << " ";
            }
            else{
                output << (*iter).source << " " << (*iter).gate << " " << (*iter).drain << " ";
            }
            if((*iter).insertDummy) output << "Dummy ";
            continue;
        }

        if((*iter).d_s){
            if((*prev(iter)).insertDummy) output << (*iter).drain << " ";
            output << (*iter).gate << " " << (*iter).source << " ";
        }
        else{
            if((*prev(iter)).insertDummy) output << (*iter).source << " ";
            output << (*iter).gate << " " << (*iter).drain << " ";
        }
        if((*iter).insertDummy) output << "Dummy ";
    }
}