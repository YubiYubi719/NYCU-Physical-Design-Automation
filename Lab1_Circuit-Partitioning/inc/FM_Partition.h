#ifndef FM_PARTITION_H
#define FM_PARTITION_H
#include <bits/stdc++.h>
using namespace std;

struct Cell{
    Cell():pinNum(0),
           set(0),
           gain(0),
           valid(false),
           isLocked(false),
           prev(nullptr),
           next(nullptr) {};
    ~Cell(){};

    int  pinNum;
    int  set;
    int  gain;
    bool valid;
    bool isLocked;
    Cell *prev, *next;
    vector<int> netList;
};

class myLinkedList{
public:
    myLinkedList():head(nullptr),
                   tail(nullptr), 
                   length(0) {};
    ~myLinkedList(){};

    void  push_back (Cell* inputCell);
    void  push_front(Cell* inputCell);
    void  pop_front();
    void  remove(Cell* inputCell);
    void  clear();
    int   size()  { return length; }
    Cell* front() { return head; }

private:
    Cell* head;
    Cell* tail;
    int length;
};

struct Net{
    Net():cell_in_A(0),
          cell_in_B(0) { };
    ~Net(){};

    vector<int> cellList;
    int cell_in_A, cell_in_B;
};

class Graph{
public:
    Graph():validCellNum(0),
            netNum(0),
            cellNum(0),
            cutNum(0), 
            minCutNum(INT_MAX),
            lbNum(0),
            ubNum(0),
            offset(0),
            balanceFactor(0.0),
            set_A(0.0) { };
    ~Graph(){ };

    void build(string fileName);
    void calInitGain();
    void updateBucketList(Cell* inputCell, int from, int to);
    bool partition();
    void traceBestSol();
    void renewBucketList();
    void printOutput(string fileName);

    void clearMoveStack() { while(!moveStack.empty()) moveStack.pop(); }
    void unlock() { for(Cell* cell:cells) cell->isLocked = false; }

    int validCellNum;
    int netNum, cellNum; // cellNum equals to cells.size()
    int cutNum, minCutNum;
    int lbNum , ubNum;
    int offset;   //bucketList offset
    int outputCutNum;
    double balanceFactor;
    double set_A; //node num in setA
    vector<Net>   nets;
    vector<Cell*> cells;
    vector<int>   results;
    vector<Net>   results_net;
    vector<vector<myLinkedList>> bucketList;
    stack<pair<Cell*,int>> moveStack; // pair.first = the cell we move
                                      // pair.second = cutNum after we move the cell
};

#endif