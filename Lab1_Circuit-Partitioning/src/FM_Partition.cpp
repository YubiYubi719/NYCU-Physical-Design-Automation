#include "FM_Partition.h"

void Graph::build(string fileName){
    ifstream input1(fileName);
    input1 >> balanceFactor;
    input1.ignore(2,'\n');
    netNum = 0;
    cellNum = INT_MIN;
    string curLine;
    while(getline(input1,curLine,';') && curLine != "\n"){
        set<int> tmpSet; // deal with lots of same cellIDs appear inside one net
        if(curLine[0] == '\n') curLine.erase(0,1);
        while (curLine[0] == ' ') curLine.erase(0,1);
        stringstream curLineStream(curLine);
        string tmp;
        curLineStream >> tmp >> tmp; // ignore the word "NET" and net name
        Net tmpNet;
        // start reading cells
        while(curLineStream >> tmp){
            tmp.erase(0,1);
            int curCellID = stoi(tmp)-1; // start from 0 (not 1)
            tmpSet.insert(curCellID);
            // tmpNet.cellList.push_back(curCellID);
            cellNum = (curCellID+1 > cellNum)? curCellID+1:cellNum;
        }
        for(auto iter = tmpSet.begin(); iter != tmpSet.end(); iter++){
            tmpNet.cellList.push_back(*iter);
        }
        nets.push_back(tmpNet); // push curNet into vector
        netNum++;
    }
    input1.close();

    // construct cell vector
    for(int i = 0; i < cellNum; i++){
        Cell* tmpCell = new Cell;
        cells.push_back(tmpCell);
    }

    // deal with cells
    ifstream input2(fileName);
    input2 >> balanceFactor;
    input2.ignore(2,'\n');
    int curNetID = 0;
    while(getline(input2,curLine,';') && curLine != "\n"){
        set<int> tmpSet;
        if(curLine[0] == '\n') curLine.erase(0,1);
        while(curLine[0] == ' ') curLine.erase(0,1);
        stringstream curLineStream(curLine);
        string tmp;
        curLineStream >> tmp >> tmp; // ignore the word "NET" and net name
        // start reading cells
        while(curLineStream >> tmp){
            tmp.erase(0,1);
            int curCellID = stoi(tmp)-1;
            tmpSet.insert(curCellID);
        }
        for(auto iter = tmpSet.begin(); iter != tmpSet.end(); iter++){
            cells[*iter]->netList.push_back(curNetID);// start from 0 (not 1)
            cells[*iter]->pinNum++;
            cells[*iter]->valid = true;
        }
        nets[curNetID].cell_in_A = nets[curNetID].cellList.size();
        nets[curNetID].cell_in_B = 0;
        curNetID++;
    }
    input2.close();
    
    // initialize cell gain and calculate validCellNum
    int gainUpperBound = INT_MIN;
    for(Cell* cell:cells){
        if(cell->valid){
            cell->gain = -1 * cell->pinNum;
            gainUpperBound = max(gainUpperBound,cell->pinNum);
            validCellNum++;
        }
    }
    offset = gainUpperBound;
    // initialize bucket list
    bucketList.resize(2,vector<myLinkedList>(2*gainUpperBound+1));
    for(int i = 0; i < cellNum; i++){
        Cell* curCell = cells[i];
        if(curCell->valid){
           bucketList[curCell->set][curCell->gain+offset].push_back(curCell);
        }
    }

    set_A = validCellNum;
    results.resize(cellNum);
    results_net.resize(cellNum);
    lbNum = ceil((1-balanceFactor)*(double)validCellNum / 2.0); // lowerBoundNum
    ubNum = floor((1+balanceFactor)*(double)validCellNum / 2.0); // upperBoundNum
}

void Graph::calInitGain(){
    for(Cell* cell:cells){
        if(cell->valid){
            cell->gain = 0;
            int from = cell->set;
            int to   = (from == 0)? 1:0;
            for(int netID:cell->netList){
                Net curNet = nets[netID];
                if(from == 0 && curNet.cell_in_A == 1) cell->gain++;
                else if(from == 1 && curNet.cell_in_B == 1) cell->gain++;

                if(to == 0 && curNet.cell_in_A == 0) cell->gain--;
                else if(to == 1 && curNet.cell_in_B == 0) cell->gain--;
            }
        }
    }
}

void Graph::updateBucketList(Cell* inputCell, int from, int to){
    //update input cell
    bucketList[from][inputCell->gain + offset].pop_front();
    cutNum -= inputCell->gain;
    inputCell->set = to;
    inputCell->isLocked = true;
    moveStack.push({inputCell,cutNum});

    //update cell.gain that inside inputCell's netList
    for(int netID:inputCell->netList){
        if((to == 0 && nets[netID].cell_in_A == 0) || (to == 1 && nets[netID].cell_in_B == 0)){
            for(int adjCellID:nets[netID].cellList){
                Cell* adjacentCell = cells[adjCellID];
                if(adjacentCell->isLocked == false){
                    bucketList[adjacentCell->set][adjacentCell->gain+offset].remove(adjacentCell);
                    //update adjacent cell's gain
                    adjacentCell->gain++;
                    bucketList[adjacentCell->set][adjacentCell->gain+offset].push_front(adjacentCell);
                }
            }
        }
        else if(to == 0 && nets[netID].cell_in_A == 1){
            for(int adjCellID:nets[netID].cellList){
                Cell* adjacentCell = cells[adjCellID];
                if(adjacentCell->isLocked == false){
                    if(adjacentCell->set == 0){
                        bucketList[adjacentCell->set][adjacentCell->gain+offset].remove(adjacentCell);
                        //update adjacent cell's gain
                        adjacentCell->gain--;
                        bucketList[adjacentCell->set][adjacentCell->gain+offset].push_front(adjacentCell);
                        break;
                    }
                }
            }
        }
        else if(to == 1 && nets[netID].cell_in_B == 1){
            for(int adjCellID:nets[netID].cellList){
                Cell* adjacentCell = cells[adjCellID];
                if(adjacentCell->isLocked == false){
                    if(adjacentCell->set == 1){
                        bucketList[adjacentCell->set][adjacentCell->gain+offset].remove(adjacentCell);
                        //update adjacent cell's gain
                        adjacentCell->gain--;
                        bucketList[adjacentCell->set][adjacentCell->gain+offset].push_front(adjacentCell);
                        break;
                    }
                }
            }
        }

        if(from == 0){
            nets[netID].cell_in_A--;
            nets[netID].cell_in_B++;
        }
        else{
            nets[netID].cell_in_A++;
            nets[netID].cell_in_B--;
        }

        if((from == 0 && nets[netID].cell_in_A == 0) || (from == 1 && nets[netID].cell_in_B == 0)){
            for(int adjCellID:nets[netID].cellList){
                Cell* adjacentCell = cells[adjCellID];
                if(adjacentCell->isLocked == false){
                    bucketList[adjacentCell->set][adjacentCell->gain+offset].remove(adjacentCell);
                    //update adjacent cell's gain
                    adjacentCell->gain--;
                    bucketList[adjacentCell->set][adjacentCell->gain+offset].push_front(adjacentCell);
                }
            }
        }
        else if(from == 0 && nets[netID].cell_in_A == 1){
            for(int adjCellID:nets[netID].cellList){
                Cell* adjacentCell = cells[adjCellID];
                if(adjacentCell->isLocked == false){
                    if(adjacentCell->set == 0){
                        bucketList[adjacentCell->set][adjacentCell->gain+offset].remove(adjacentCell);
                        //update adjacent cell's gain
                        adjacentCell->gain++;
                        bucketList[adjacentCell->set][adjacentCell->gain+offset].push_front(adjacentCell);
                        break;
                    }
                }
            }
        }
        else if(from == 1 && nets[netID].cell_in_B == 1){
            for(int adjCellID:nets[netID].cellList){
                Cell* adjacentCell = cells[adjCellID];
                if(adjacentCell->isLocked == false){
                    if(adjacentCell->set == 1){
                        bucketList[adjacentCell->set][adjacentCell->gain+offset].remove(adjacentCell);
                        //update adjacent cell's gain
                        adjacentCell->gain++;
                        bucketList[adjacentCell->set][adjacentCell->gain+offset].push_front(adjacentCell);
                        break;
                    }
                }
            }
        }
        
    }
}

bool Graph::partition(){
    Cell *bestA = nullptr, *bestB = nullptr;

    for(int i = bucketList[0].size()-1; i >= 0; i--){
        if(bucketList[0][i].size() != 0){
            bestA = bucketList[0][i].front();
            break;
        }
    }
    for(int i = bucketList[1].size()-1; i >= 0; i--){
        if(bucketList[1][i].size() != 0){
            bestB = bucketList[1][i].front();
            break;
        }
    }

    
    if(bestA == nullptr && bestB == nullptr) return true; // no element in bucketList
    if(set_A >= ubNum && bestA != nullptr){//setA has too many cells
        updateBucketList(bestA,0,1);
        set_A--;
    }
    else if(set_A >= ubNum && bestA == nullptr) return true;//setA has too many cells but cannot move cell to setB
    else if(set_A <= lbNum && bestB != nullptr){//setA doesn't have enough cells
        updateBucketList(bestB,1,0);
        set_A++;
    }
    else if(set_A <= lbNum && bestB == nullptr) return true;//setB has too many cells but cannot move cell to setA
    else{
        if(bestA != nullptr && bestB == nullptr){ // bucketList_B is empty
            updateBucketList(bestA,0,1);
            set_A--;
        }
        else if(bestA == nullptr && bestB != nullptr){ // bucketList_A is empty
            updateBucketList(bestB,1,0);
            set_A++;
        }
        else if(bestA->gain >= bestB->gain){
            updateBucketList(bestA,0,1);
            set_A--;
        }
        else if(bestB->gain > bestA->gain){
            updateBucketList(bestB,1,0);
            set_A++;
        }
    }

    if(cutNum < minCutNum && set_A >= lbNum && set_A <= ubNum){
        minCutNum = cutNum;
        results_net = nets;
        for(int i = 0; i < cellNum; i++){
            if(cells[i]->valid){
                results[i] = cells[i]->set;
            }
        }
    }

    return false;
}

void Graph::traceBestSol(){
    while(!moveStack.empty()){
        pair<Cell*,int> p = moveStack.top();
        moveStack.pop();

        Cell* movedCell = p.first;
        int cutNumAfterMove = p.second;
        if(cutNumAfterMove > minCutNum){
            cutNum += movedCell->gain;
            if(movedCell->set == 0){
                movedCell->set = 1;
                set_A--;
                for(int curNetID:movedCell->netList){
                    Net &curNet = nets[curNetID];
                    curNet.cell_in_A--;
                    curNet.cell_in_B++;
                }
            }
            else{// movedCell.set == 1
                movedCell->set = 0;
                set_A++;
                for(int curNetID:movedCell->netList){
                    Net &curNet = nets[curNetID];
                    curNet.cell_in_A++;
                    curNet.cell_in_B--;
                }
            }
        }
        else break;
    }
}

void Graph::renewBucketList(){
    // clean bucketList
    for(int i = 0; i < 2; i++){
        for(myLinkedList &curList:bucketList[i]){
            curList.clear();
        }
    }
    for(Cell* cell:cells){
        if(cell->valid){
            // set prev and next pointer of each cell to nullptr
            cell->prev = nullptr;
            cell->next = nullptr;
            // put cell into bucketList
            bucketList[cell->set][cell->gain+offset].push_back(cell);
        }
    }
}

void Graph::printOutput(string fileName){
    ofstream output(fileName);
    outputCutNum = 0;
    for(Net curNet:results_net){
        if(curNet.cell_in_A != 0 && curNet.cell_in_B != 0) outputCutNum++;
    }
    output << "Cutsize = " << outputCutNum << '\n';
    int cellNum_A = 0, cellNum_B = 0;
    vector<int> cell_in_A, cell_in_B;
    for(int i = 0; i < cellNum; i++){
        if(cells[i]->valid){
            if(results[i] == 0){
                cellNum_A++;
                cell_in_A.push_back(i);
            } 
            else{
                cellNum_B++;
                cell_in_B.push_back(i);
            } 
        }
    }
    output << "G1 " << cellNum_A << '\n';
    for(int cellID:cell_in_A){
        output << 'c' << cellID+1 << " ";
    }
    output << ";\n";

    output << "G2 " << cellNum_B << '\n';
    for(int cellID:cell_in_B){
        output << 'c' << cellID+1 << " ";
    }
    output << ";\n";
    output.close();
}
