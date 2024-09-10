#ifndef ST_H
#define ST_H

#include <bits/stdc++.h>
using namespace std;

class Block{
public:
    Block(){};
    Block(string a):type(a) {
        x = 0;
        y = 0;
        w = 0;
        h = 0;
        leftChild = nullptr;
        rightChild = nullptr;
    };
    Block(string a, list<pair<int,int>> b):type(a),whs(b) {
        x = 0;
        y = 0;
        w = 0;
        h = 0;
        leftChild = nullptr;
        rightChild = nullptr;
    };

    string type;
    int x, y;
    int w, h;
    list<pair<int,int>> whs;// all possible width and height
    Block* parent;
    // vector<Block*> children;
    Block *leftChild, *rightChild;
};

class ST{
public:
    ST(): blockNum(0),
          totalArea(0) {};
    ~ST(){};

    size_t blockNum;
    int totalArea;
    double bc;
    double R_lowerBound, R_upperBound; // aspect ratio constraint
    vector<pair<int,int>> blocks;

    // bool myCmp(pair<int,int> &a, pair<int,int> &b);
    // bool myCmp1(Block* a, Block* b);
    void build(string filename);
    void calTotalArea(vector<pair<int,int>> const &blocks);
    void initString(vector<string> &s);
    vector<Block*> createNPE(vector<string>const &s);
    void deleteNPE(vector<Block*> &npe);
    list<pair<int,int>> findWH(pair<int,int> wh); // find possible width height pair
    void stackBlock(Block* a, Block* b, Block* op);
    pair<int,int> verticalMerge(pair<int,int> const &a, pair<int,int> const &b);
    pair<int,int> horizontalMerge(pair<int,int> const &a, pair<int,int> const &b);
    double calCost(vector<Block*> &npe);
    void M1(vector<string> &s);
    void M2(vector<string> &s);
    void M3(vector<string> &s);
    bool isBallot(vector<string>const &s);
    bool isSkewed(vector<string>const &s);
    double initialTemperature(vector<string> &s);
    vector<Block*> simulateAnnealing(vector<string> &s);
    void retrace(vector<Block*> &npe);
    void printResult(vector<Block*> npe, string filename);
    bool isValid(vector<Block*> const &npe);
};

#endif