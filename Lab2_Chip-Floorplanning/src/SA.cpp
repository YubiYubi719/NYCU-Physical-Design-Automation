#include "SA.h"
#define ALPHA 0.85

void SA::build(string filename){
    ifstream input(filename);
    input >> R_lowerBound >> R_upperBound;
    input.ignore(2,'\n');
    string curLine;
    while(getline(input,curLine)){
        stringstream ss(curLine);
        InputInf inf;
        ss >> inf.name >> inf.width >> inf.height;
        inputInf.push_back(inf);
        blockNum++;
    }
    calTotalArea(inputInf); // update totalArea
}

void SA::calTotalArea(vector<InputInf> const &inputInf){
    int len = inputInf.size();
    for(int i = 0; i < len; i++){
        totalArea += inputInf[i].width * inputInf[i].height;
    }
}

vector<int> SA::initRemainBlocks(){
    vector<int> remainBlockID(blockNum);
    for(int i = 0; i < blockNum; i++){
        remainBlockID[i] = i;
    }

    return remainBlockID;
}

void SA::initBlocks(){
    // if there exist block inside blocks, delete all of it
    if(!blocks.empty()) blocks.clear();
    for(InputInf inf:inputInf){
        blocks.push_back(Block(inf.name,inf.width,inf.height));
    }
}

void SA::buildBST(){
    // initialize a B* tree
    vector<int> remainBlockID(initRemainBlocks());
    initBlocks();
    BST.resize(blockNum);
    queue<int> q;
    // randomly pickup a block as a root
    int randNum = rand() % remainBlockID.size();
    int rootID = remainBlockID[randNum];
    root = &BST[rootID];
    remainBlockID.erase(remainBlockID.begin() + randNum);
    q.push(rootID);

    while(!q.empty()){
        int curBlockID = q.front();
        BST[curBlockID].block = &blocks[curBlockID];
        q.pop();

        // first find a left child
        int left = -1, right = -1;
        if(!remainBlockID.empty()){
            randNum = rand() % remainBlockID.size();
            left = remainBlockID[randNum];
            remainBlockID.erase(remainBlockID.begin() + randNum);
            
            q.push(left);

            // then find a right child
            if(!remainBlockID.empty()){
                randNum = rand() % remainBlockID.size();
                right = remainBlockID[randNum];
                remainBlockID.erase(remainBlockID.begin() + randNum);
                
                q.push(right);
            }
        }
        
        
        if(left != -1){
            BST[curBlockID].leftChild = &BST[left];
            BST[left].parent = &BST[curBlockID];
        }
        if(right != -1){
            BST[curBlockID].rightChild = &BST[right];
            BST[right].parent = &BST[curBlockID];
        }
    }
}

void SA::updateContour(Block* block){
    if(contour.empty()){
        int start = block->x;
        int end = block->x + block->w;
        int new_y = block->h;
        ContourElement ce;
        ce.x_start = start;
        ce.x_end = end-1;
        ce.y = new_y;
        contour.push_back(ce);

        ContourElement ce2;
        ce2.x_start = end;
        ce2.x_end = INT_MAX;
        ce2.y = 0;
        contour.push_back(ce2);

        return;
    }

    int start = block->x;
    int end = block->x + block->w;
    int max_y = 0;

    auto iter1 = contour.begin();
    while(next(iter1) != contour.end()){
        if(start >= (*iter1).x_start && start <= (*iter1).x_end) break;
        iter1++;
    }
    auto iter2 = iter1;
    while(next(iter2) != contour.end()){
        max_y = max(max_y,(*iter2).y);
        if(end-1 >= (*iter2).x_start && end-1 <= (*iter2).x_end) break;
        iter2++;
    }
    // update block's y
    block->y = max_y;
    int new_y = max_y + block->h;

    if(iter1 != iter2){
        list<ContourElement>::iterator iter3;
        list<ContourElement>::iterator iter4;
        if((*iter1).x_start == start) iter3 = iter1;
        else iter3 = next(iter1);
        
        if((*iter2).x_end == end-1) iter4 = next(iter2);
        else iter4 = iter2;

        auto iter = contour.erase(iter3,iter4);
        ContourElement ce(start,end-1,new_y);
        iter = contour.insert(iter,ce);
        if((*prev(iter)).x_end >= start) (*prev(iter)).x_end = start - 1;
        if((*next(iter)).x_start <= end-1) (*next(iter)).x_start = end;
    }
    else{
        if(start == (*iter1).x_start && end-1 == (*iter1).x_end){
            (*iter1).y = new_y;
        }
        else if(start == (*iter1).x_start){
            ContourElement ce(start,end-1,new_y);
            contour.insert(iter1,ce);
            (*iter1).x_start = end;
        }
        else if(end-1 == (*iter1).x_end){
            ContourElement ce((*iter1).x_start,start-1,(*iter1).y);
            contour.insert(iter1,ce);
            (*iter1).x_start = start;
        }
        else{
            ContourElement ce1((*iter1).x_start,start-1,(*iter1).y);
            contour.insert(iter1,ce1);
            ContourElement ce2(start,end-1,new_y);
            contour.insert(iter1,ce2);
            (*iter1).x_start = end;
        }
    }
}

void SA::preOrderTraversal(Node* curNode, bool isLeft){
    Node* parent = curNode->parent;
    if(isLeft){
        curNode->block->x = parent->block->x + parent->block->w;
    }
    else{
        curNode->block->x = parent->block->x;
    }
    // update contour and block's y
    updateContour(curNode->block);

    if(curNode->leftChild != nullptr) preOrderTraversal(curNode->leftChild, true);
    if(curNode->rightChild != nullptr) preOrderTraversal(curNode->rightChild, false);

}

void SA::BST_2_FP(){ // transform B* tree into floorplan
    contour.clear();
    Block* rootBlock = root->block;
    rootBlock->x = 0;
    rootBlock->y = 0;

    updateContour(root->block);

    if(root->leftChild != nullptr) preOrderTraversal(root->leftChild, true);
    if(root->rightChild != nullptr) preOrderTraversal(root->rightChild, false);
}

double SA::calCost(){
    BST_2_FP();
    int width = 0, height = 0;
    for(Block const &block:blocks){
        width = max(width, block.x + block.w);
        height = max(height, block.y + block.h);
    }

    return width * height;
}

void SA::M1(int id){
    // swap width and height
    Block &b = blocks[id];
    swap(b.w, b.h);
    b.isRotate = (b.isRotate)? false:true;
}

void SA::M2(Node* n1, Node* n2){ // swap 2 block
    // swap parent
    Node* p1 = n1->parent;
    if(p1 != nullptr){
        if(p1->leftChild == n1) p1->leftChild = n2;
        else p1->rightChild = n2;
    }
    Node* p2 = n2->parent;
    if(p2 != nullptr){
        if(p2->leftChild == n2) p2->leftChild = n1;
        else p2->rightChild = n1;
    }
    n1->parent = p2;
    n2->parent = p1;

    // swap children
    swap(n1->leftChild, n2->leftChild);
    swap(n1->rightChild, n2->rightChild);
    if(n1->leftChild != nullptr) n1->leftChild->parent = n1;
    if(n1->rightChild != nullptr) n1->rightChild->parent = n1;
    if(n2->leftChild != nullptr) n2->leftChild->parent = n2;
    if(n2->rightChild != nullptr) n2->rightChild->parent = n2;
    
    // relationship of b1 and b2 are parent and child
    if (n1->parent == n1) n1->parent = n2;
    else if (n1->leftChild == n1) n1->leftChild = n2;
    else if (n1->rightChild == n1) n1->rightChild = n2;

    if (n2->parent == n2) n2->parent = n1;
    else if (n2->leftChild == n2) n2->leftChild = n1;
    else if (n2->rightChild == n2) n2->rightChild = n1;

    // root may change
    if(root == n1) root = n2;
    else if(root == n2) root = n1;
}

void SA::M3(Node* from, Node* to){ // remove and insert block
    // delete
    if(from->leftChild == nullptr && from->rightChild == nullptr){
        // from has no child
        // thus we remove it directly
        Node* parent = from->parent;
        if(parent != nullptr){
            if(parent->leftChild == from) parent->leftChild = nullptr;
            else parent->rightChild = nullptr;
        }
    }
    else if(from->leftChild != nullptr && from->rightChild != nullptr){
        // from has 2 children
        // swap it til the bottom of BST
        while(true){
            bool swapLeft;
            if(from->leftChild != nullptr && from->rightChild != nullptr) swapLeft = rand() % 2;
            else if(from->leftChild != nullptr) swapLeft = true;
            else swapLeft = false;

            if(swapLeft) M2(from, from->leftChild);
            else M2(from, from->rightChild);

            if(from->leftChild == nullptr && from->rightChild == nullptr) break;
        }
        // now that "from" is at the bottom of the tree, remove it from its parent
        Node* parent = from->parent;
        if(parent->leftChild == from) parent->leftChild = nullptr;
        else parent->rightChild = nullptr;
    }
    else{
        // from has only 1 child
        Node* child = nullptr;
        Node* parent = from->parent;
        if(from->leftChild != nullptr) child = from->leftChild;
        else child = from->rightChild;
        // update child
        child->parent = parent;
        // update parent
        if(parent != nullptr){
            if(parent->leftChild == from) parent->leftChild = child;
            else parent->rightChild = child;
        }

        // root may change
        if(root == from) root = child;
    }

    // insert
    Node* child = nullptr;
    int op = rand() % 4;
    switch(op){
        case 0:{
            // put "from" into "to"'s leftChild
            child = to->leftChild;
            from->leftChild = child;
            from->rightChild = nullptr;
            to->leftChild = from;
            break;
        }
        case 1:{
            // put "from" into "to"'s leftChild
            // but put [to's leftChild] into [from's rightChild]
            child = to->leftChild;
            from->leftChild = nullptr;
            from->rightChild = child;
            to->leftChild = from;
            break;
        }
        case 2:{
            // put "from" into "to"'s rightChild
            child = to->rightChild;
            from->leftChild = nullptr;
            from->rightChild = child;
            to->rightChild = from;
            break;
        }
        case 3:{
            // put "from" into "to"'s rightChild
            // but put [to's rightChild] into [from's leftChild]
            child = to->rightChild;
            from->leftChild = child;
            from->rightChild = nullptr;
            to->rightChild = from;
            break;
        }
    }
    from->parent = to;
    if(child != nullptr) child->parent = from;
}

double SA::calRatio(){
    int width = 0, height = 0;
    for(Block const &block:blocks){
        width = max(width, block.x + block.w);
        height = max(height, block.y + block.h);
    }

    return (double) width / (double)height;
}

double SA::initialTemperature(){
    BST_2_FP();
    double prevR = calRatio();
    double minDistance = min(abs(prevR-R_upperBound),abs(prevR-R_lowerBound));

    while(true){
        vector<Block> tmpBlocks = blocks;
        vector<Node> tmpBST = BST;
        Node* tmpRoot = root;
        int op = rand() % 3;
        switch (op){
            case 0:{
                int randID = rand() % blockNum;
                M1(randID);
                break;
            }
            case 1:{
                vector<int> remainBlockID(initRemainBlocks());
                int randNum = rand() % remainBlockID.size();
                int randID_1 = remainBlockID[randNum];
                remainBlockID.erase(remainBlockID.begin()+randNum);

                randNum = rand() % remainBlockID.size();
                int randID_2 = remainBlockID[randNum];

                Node* n1 = &BST[randID_1];
                Node* n2 = &BST[randID_2];
                M2(n1,n2);
                break;
            }
            case 2:{
                vector<int> remainBlockID(initRemainBlocks());
                int randNum = rand() % remainBlockID.size();
                int randID_1 = remainBlockID[randNum];
                remainBlockID.erase(remainBlockID.begin()+randNum);

                randNum = rand() % remainBlockID.size();
                int randID_2 = remainBlockID[randNum];
                while(&BST[randID_2] == BST[randID_1].parent){
                    randNum = rand() % remainBlockID.size();
                    randID_2 = remainBlockID[randNum];
                }

                Node* from = &BST[randID_1];
                Node* to = &BST[randID_2];
                M3(from,to);
                break;
            }
        }
        BST_2_FP();
        double newR = calRatio();
        double curDistance = min(abs(newR-R_upperBound),abs(newR-R_lowerBound));
        if(curDistance < minDistance){
            minDistance = curDistance;
        }
        else{
            BST = tmpBST;
            root = tmpRoot;
            blocks = tmpBlocks;
        }

        if(newR >= R_lowerBound && newR <= R_upperBound) break;
    }

    int width = 0;
    int height = 0;
    for(int i = 0; i < blockNum; i++){
        width = max(width, blocks[i].x + blocks[i].w);
        height = max(height, blocks[i].y + blocks[i].h);
    }

    double area = width * height;
    double T0 = (-(area/totalArea) * blockNum) / log(0.95);
    // std::cout << "Initial temperature: " << T0 << '\n';
    return T0;
}

void SA::simulateAnnealing(){
    const int N = 10 * blockNum;
    double T0 = initialTemperature();
    int total_move, uphill_move, reject_move;
    double T = T0;
    double bestCost = DBL_MAX;
    vector<Block> bestBlocks;
    
    while(true){
        total_move = 0;
        uphill_move = 0;
        reject_move = 0;
        double oc = 0.0;
        while(true){
            if(oc == 0.0) oc = calCost();
            // store current results
            vector<Block> tmpBlocks = blocks;
            vector<Node> tmpBST = BST;
            Node* tmpRoot = root;

            // do operations
            int op = rand() % 3;
            switch (op){
                case 0:{
                    int randID = rand() % blockNum;
                    M1(randID);
                    break;
                }
                case 1:{
                    vector<int> remainBlockID(initRemainBlocks());
                    int randNum = rand() % remainBlockID.size();
                    int randID_1 = remainBlockID[randNum];
                    remainBlockID.erase(remainBlockID.begin()+randNum);

                    randNum = rand() % remainBlockID.size();
                    int randID_2 = remainBlockID[randNum];

                    Node* n1 = &BST[randID_1];
                    Node* n2 = &BST[randID_2];
                    M2(n1,n2);
                    break;
                }
                case 2:{
                    vector<int> remainBlockID(initRemainBlocks());
                    int randNum = rand() % remainBlockID.size();
                    int randID_1 = remainBlockID[randNum];
                    remainBlockID.erase(remainBlockID.begin()+randNum);

                    randNum = rand() % remainBlockID.size();
                    int randID_2 = remainBlockID[randNum];
                    while(&BST[randID_2] == BST[randID_1].parent){
                        randNum = rand() % remainBlockID.size();
                        randID_2 = remainBlockID[randNum];
                    }

                    Node* from = &BST[randID_1];
                    Node* to = &BST[randID_2];
                    M3(from,to);
                    break;
                }
            }

            total_move++;
            double nc = calCost();
            double diff = nc - oc;

            double R = (double)rand() / RAND_MAX;
            if(diff < 0 || R < exp(-1 * diff / T)){
                // accept current move
                oc = nc;

                if(diff > 0) uphill_move++;
                if(nc < bestCost) {
                    // store the best cost and current result
                    bestCost = nc;
                    bestBlocks = blocks;
                }
            }
            else{
                // reject current move
                // thus restore old blocks result and keep oc
                root = tmpRoot;
                BST = tmpBST;
                blocks = tmpBlocks; // restore last result
                reject_move++;
            }
            
            if(uphill_move > N || total_move > 2*N) break;
        }
        
        T *= 0.9;
        // std::cout << T << '\n';
        if((double)reject_move / (double)total_move > 0.95 || T < 0.001) break;
    }

    blocks = bestBlocks;
}

bool SA::isValid(){
    // Check whether current floorplan is legal
    int width = 0, height = 0;
    for(Block const &block:blocks){
        width = max(width, block.x + block.w);
        height = max(height, block.y + block.h);
    }
    double R = (double)width / height;

    return (R >= R_lowerBound) && (R <= R_upperBound);
}

void SA::printOutput(string filename){
    ofstream output(filename);
    int width = 0, height = 0;
    for(Block const &block:blocks){
        width = max(width, block.x + block.w);
        height = max(height, block.y + block.h);
    }

    int area = width * height;
    output << "A = " << area << '\n';
    output << "R = " << (double)width / height << '\n';

    for(Block const &block:blocks){
        output << block.name << " " << block.x << " " << block.y;
        if(block.isRotate) output << " R";
        output << '\n';
    }

    output.close();
}