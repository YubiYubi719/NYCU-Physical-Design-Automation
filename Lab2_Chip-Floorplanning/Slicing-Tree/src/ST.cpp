#include "ST.h"
#define ALPHA 0.7

void ST::build(string filename){
    ifstream input(filename);
    input >> R_lowerBound >> R_upperBound;
    input.ignore(2,'\n');
    string curLine;
    while(getline(input,curLine)){
        stringstream ss(curLine);

        string blockName;
        ss >> blockName;
        int blockID = stoi(blockName.erase(0,1));
        if(blockID > blocks.size()) blocks.resize(blockID);

        int width, height;
        ss >> width >> height;
        blocks[blockID-1] = {width,height};
        blockNum++;
    }
    calTotalArea(blocks); // update totalArea
}

void ST::calTotalArea(vector<pair<int,int>> const &blocks){
    size_t len = blocks.size();
    for(size_t i = 0; i < len; i++){
        totalArea += blocks[i].first * blocks[i].second;
    }
}

void ST::initString(vector<string> &s){
    s.push_back("1");
    s.push_back("2");
    s.push_back("V");
    for(size_t i = 3; i <= blockNum; i++){
        s.push_back(to_string(i));
        s.push_back("V");
    }
}

list<pair<int,int>> ST::findWH(pair<int,int> wh){
    list<pair<int,int>> width_height;
    // rotate the hard block and store its possible width-height pairs
    // store width_height pair in "increasing width, decreasing height"
    if(wh.first == wh.second) width_height.push_back(wh);
    else {
        if(wh.first > wh.second) swap(wh.first,wh.second);
        width_height.push_back(wh);
        width_height.push_back({wh.second,wh.first});
    }

    return width_height;
}

vector<Block*> ST::createNPE(vector<string>const &s){
    vector<Block*> npe;
    for(int i = 0; i < s.size(); i++){
        if(s[i] != "H" && s[i] != "V"){ // s[i] is an operand
            // store all possible width-height pair into block.whs
            Block* block = new Block(s[i],findWH(blocks[stoi(s[i])-1]));
            npe.push_back(block);
        }
        else{ // s[i] is an operator
            Block* block = new Block(s[i]);
            npe.push_back(block);
        }
    }

    return npe;
}

void ST::stackBlock(Block* a, Block* b, Block* op){
    // a and b is operand, op is operator

    // Stockmeyer Algorithm
    // at the beginning we stored the whs as "increasing width, decreasing height"
    // thus we don't need to do extra sorting for operation
    if(op->type == "V"){
        auto it1 = a->whs.begin();
        auto it2 = b->whs.begin();
        while(it1 != a->whs.end() && it2 != b->whs.end()){
            op->whs.push_back(verticalMerge((*it1),(*it2)));
            int h_1 = (*it1).second;
            int h_2 = (*it2).second;
            if(h_1 > h_2) it1++;
            else if(h_2 > h_1) it2++;
            else { // h_1 == h_2
                it1++; 
                it2++; 
            }
        }
    }
    else{ // op->type == "H"
        auto it1 = a->whs.rbegin();
        auto it2 = b->whs.rbegin();
        while(it1 != a->whs.rend() && it2 != b->whs.rend()){
            op->whs.push_front(horizontalMerge((*it1),(*it2)));
            int w_1 = (*it1).first;
            int w_2 = (*it2).first;
            if(w_1 > w_2) it1++;
            else if(w_2 > w_1) it2++;
            else { // w_1 == w_2
                it1++; 
                it2++; 
            }
        }
    }

    // store operand into operator
    op->leftChild = a;
    op->rightChild = b;
    // update operand's parent
    a->parent = op;
    b->parent = op;
}
pair<int,int> ST::verticalMerge(pair<int,int> const &a, pair<int,int> const &b){
    int width = a.first + b.first;
    int height = max(a.second,b.second);
    return {width,height};
}
pair<int,int> ST::horizontalMerge(pair<int,int> const &a, pair<int,int> const &b){
    int width = max(a.first,b.first);
    int height = a.second + b.second;
    return {width,height};
}

double ST::calCost(vector<Block*> &npe){
    // vector<Block*> npe = createNPE(s);
    stack<Block*> bs;
    size_t len = npe.size();
    for(size_t i = 0; i < len; i++){
        if(npe[i]->type != "V" && npe[i]->type != "H"){
            bs.push(npe[i]);
        }
        else{
            Block* operand2 = bs.top();
            bs.pop();
            Block* operand1 = bs.top();
            bs.pop();
            stackBlock(operand1,operand2,npe[i]);
            bs.push(npe[i]);
        }
    }
    Block* a = bs.top();
    double minCost = DBL_MAX;
    int minArea = INT_MAX;
    for(auto it = a->whs.begin(); it != a->whs.end(); it++){
        int area = (*it).first * (*it).second;
        int longer = max((*it).first, (*it).second);
        int shorter = min((*it).first, (*it).second);
        double R = (double)longer / (double)shorter;
        double penalty = (R >= R_lowerBound && R <= R_upperBound)? 0.0:1.0;
        // todo: cost function
        double curCost = ALPHA * (double)area / (double)totalArea  + (1-ALPHA) * penalty;
        if(curCost < minCost){
            minCost = curCost;
            a->w = (*it).first;
            a->h = (*it).second;
        }
    }

    return minCost;
}

void ST::M1(vector<string> &s){
    // swap two adjacent operand(ignoring operator)
    vector<int> operand_pos;
    size_t len_s = s.size();
    for(size_t i = 0; i < len_s; i++){
        if(s[i] != "H" && s[i] != "V"){
            operand_pos.push_back(i);
        }
    }
    int len_pos = operand_pos.size();
    int id = rand() % (len_pos-1);
    swap(s[operand_pos[id]],s[operand_pos[id+1]]);
}

void ST::M2(vector<string> &s){
    // complement operator
    vector<int> operator_pos;
    size_t len_s = s.size();
    for(size_t i = 1; i < len_s; i++){
        if((s[i] == "H" || s[i] == "V") && (s[i-1] != "H" && s[i-1] != "V")){
            operator_pos.push_back(i);
        }
    }
    // invert operator chain from id = len_pos[i] to chain end
    int operator_pos_id = rand() % operator_pos.size();
    int curID = operator_pos[operator_pos_id];
    while(s[curID] == "H" || s[curID] == "V"){
        if(s[curID] == "H") s[curID] = "V";
        else s[curID] = "H";
        curID++;
        if(curID == len_s) break;
    }
    
}

void ST::M3(vector<string> &s){
    // swap two adjacent operand and operator
    vector<int> pos;
    int len_s = s.size();
    for(int i = 0; i < len_s-1; i++){
        if((s[i] != "H" && s[i] != "V") && (s[i+1] == "H" || s[i+1] == "V")
        || (s[i] == "H" || s[i] == "V") && (s[i+1] != "H" && s[i+1] != "V")){
            pos.push_back(i);
        }
    }
    while(!pos.empty()){
        int id = rand() % pos.size();
        id = pos[id];
        vector<string> tmp = s;
        swap(tmp[id],tmp[id+1]);
        if(isBallot(tmp) && isSkewed(tmp)){
            s = tmp;
            break;
        }
        else{
            pos.erase(find(pos.begin(),pos.end(),id));
        }
    }
}

bool ST::isBallot(vector<string>const &s){
    int operandNum = 0, operatorNum = 0;
    size_t len = s.size();
    for(size_t i = 0; i < len; i++){
        if(s[i] != "H" && s[i] != "V") operandNum++;
        else operatorNum++;

        if(operandNum <= operatorNum) return false;
    }

    return true;
}

bool ST::isSkewed(vector<string>const &s){
    size_t len_s = s.size() - 1;
    for(size_t i = 0; i < len_s; i++){
        if((s[i] == "H" && s[i+1] == "H")
        || (s[i] == "V" && s[i+1] == "V")) return false;
    }

    return true;
}

double ST::initialTemperature(vector<string> &s){
    double total_diff = 0.0;
    int n = 0;
    vector<Block*> old_npe = createNPE(s); // original NPE
    double oc = calCost(old_npe);// original cost
    while(total_diff == 0){
        for(int i = 0; i < 40; i++){
            int op = rand() % 3;
            switch (op){
                case 0: M1(s); break;
                case 1: M2(s); break;
                case 2: M3(s); break;
                default: break;
            }

            vector<Block*> new_npe = createNPE(s); // new NPE
            double nc = calCost(new_npe); // new cost
            double diff = nc - oc;
            oc = nc;
            if(diff > 0){
                total_diff += diff;
                n++;
            }
            deleteNPE(new_npe);
        }
    }
    double diff_avg = total_diff / n;
    const double P = 0.99;
    double T0 = -1 * diff_avg / log(P);
    deleteNPE(old_npe);

    return T0;
}

vector<Block*> ST::simulateAnnealing(vector<string> &s){
    const int N = 10 * (int)s.size();
    vector<string> best = s; // stores best optimized topology
    vector<string> tmp = s;  // stores temporary string
    double T0 = initialTemperature(tmp);
    int total_move, uphill_move, reject_move;
    double T = T0;
    vector<Block*> best_npe = createNPE(best);
    double bestCost = calCost(best_npe);

    
    while(true){
        total_move = 0;
        uphill_move = 0;
        reject_move = 0;
        vector<Block*> old_npe;
        double oc;
        while(true){
            if(old_npe.empty()){
                old_npe = createNPE(tmp);
                oc = calCost(old_npe);
            }
            int op = rand() % 3;
            // cout << op << '\n';
            switch (op){
                case 0: M1(tmp); break;
                case 1: M2(tmp); break;
                case 2: M3(tmp); break;
                default: break;
            }


            total_move++;
            vector<Block*> new_npe = createNPE(tmp);
            double nc = calCost(new_npe);
            double diff = nc - oc;

            double R = (double)rand() / RAND_MAX;
            if(diff < 0 || R < exp(-1 * diff / T)){
                // we accept current move
                // thus we don't need to store old_npe
                // we set current new_npe as next iteration's old_npe
                // also set nc as next iteration's oc
                if(old_npe != best_npe) deleteNPE(old_npe);
                old_npe = new_npe;
                oc = nc;
                s = tmp;

                if(diff > 0) uphill_move++;
                if(nc < bestCost) {
                    // store the best cost and its npe
                    best = s;
                    bestCost = nc;
                    if(!best_npe.empty()) deleteNPE(best_npe);
                    best_npe = new_npe;
                    bc = nc; // just want to see current best cost
                }
            }
            else{
                // reject current move
                // thus keep old_npe and oc, and delete new_npe
                tmp = s; // restore last result
                deleteNPE(new_npe);
                reject_move++;
            }
            
            if(uphill_move > N || total_move > 2*N) break;
        }
        
        T = (T > 0.05*T0)? 0.1*T : 0.85*T;
        // std::cout << T << '\n';
        if((double)reject_move / (double)total_move > 0.95 || T < 0.001) break;
    }
    
    s = best; // update s into current optimal sol
    return best_npe;
}

void ST::deleteNPE(vector<Block*> &npe){
    int len = npe.size();
    for(int i = 0; i < len; i++){
        delete npe[i];
    }
}
void ST::retrace(vector<Block*> &npe){
    // Placement Rule:
    // 1) op == "V" --> put first block on the left side, second on right
    // 2) op == "H" --> put first block at the bottom, second on top
    queue<Block*> q;
    q.push(npe[npe.size()-1]);
    while(!q.empty()){
        Block* curBlock = q.front();
        int curWidth = curBlock->w;
        int curHeight = curBlock->h;
        q.pop();
        
        Block* left = curBlock->leftChild;
        Block* right = curBlock->rightChild;
        for(auto it1 = left->whs.begin(); it1 != left->whs.end(); it1++){
            int w_1 = (*it1).first;
            int h_1 = (*it1).second;
            for(auto it2 = right->whs.begin(); it2 != right->whs.end(); it2++){
                int w_2 = (*it2).first;
                int h_2 = (*it2).second;
                if(curBlock->type == "V"){
                    if(w_1 + w_2 == curWidth && max(h_1,h_2) == curHeight){
                        left->w = w_1;
                        left->h = h_1;
                        right->w = w_2;
                        right->h = h_2;
                    }
                }
                else if(curBlock->type == "H"){
                    if(max(w_1,w_2) == curWidth && h_1 + h_2 == curHeight){
                        left->w = w_1;
                        left->h = h_1;
                        right->w = w_2;
                        right->h = h_2;
                    }
                }
            }
        }
        // update coordinates of curBlock's children
        left->x = curBlock->x;
        left->y = curBlock->y;
        
        if(curBlock->type == "H"){
            right->x = left->x;
            right->y = left->y + left->h;
        }
        else{
            right->x = left->x + left->w;
            right->y = left->y;
        }

        if(left->type == "V" || left->type == "H") q.push(left);
        if(right->type == "V" || right->type == "H") q.push(right);
    }
}

bool ST::isValid(vector<Block*> const &npe){
    Block* topBlock = npe[npe.size()-1];
    int curWidth = topBlock->w;
    int curHeight = topBlock->h;
    double curAspectRatio = (double)curWidth/(double)curHeight;
    return (curAspectRatio <= R_upperBound) && (curAspectRatio >= R_lowerBound);
}

bool myCmp(Block* a, Block* b){
    return stoi(a->type) < stoi(b->type);
}

void ST::printResult(vector<Block*> npe, string filename){
    ofstream output(filename);
    Block* finalFPInf = npe[npe.size()-1];
    output << "A = " << finalFPInf->w * finalFPInf->h << '\n';
    output << "R = " << (double)finalFPInf->w / (double)finalFPInf->h << '\n';
    vector<Block*> tmp;
    for(int i = 0; i < npe.size(); i++){
        if(npe[i]->type != "H" && npe[i]->type != "V") tmp.push_back(npe[i]);
    }
    sort(tmp.begin(), tmp.end(), myCmp);
    for(int i = 0; i < tmp.size(); i++){
        output << "b" << i+1 << " " << tmp[i]->x << " " << tmp[i]->y;
        if(tmp[i]->w != blocks[i].first) output << " R";
        output << '\n';
    }
    
    output.close();
}