#include <bits/stdc++.h>
using namespace std;

class InputInf{
public:
    InputInf(){};
    ~InputInf(){};

    string name;
    int width;
    int height;
};

class ContourElement{
public:
    ContourElement(): x_start(0),
                      x_end(0),
                      y(0) {};
    ContourElement(int start, int end, int new_y): x_start(start),
                                                   x_end(end),
                                                   y(new_y) {};
    ~ContourElement(){};
    int x_start;
    int x_end;
    int y;
};

class Block{
public:
    Block():x(0),
            y(0),
            w(0),
            h(0),
            isRotate(false) {};
    Block(string n, int w_in, int h_in):name(n),
                                        x(0),
                                        y(0),
                                        w(w_in),
                                        h(h_in),
                                        isRotate(false) {};
    ~Block(){};

    string name;
    int x, y;
    int w, h;
    bool isRotate;
};

class Node{ // for B* tree representation
public:
    Node(): parent(nullptr), 
            leftChild(nullptr),
            rightChild(nullptr),
            block(nullptr) { };
    ~Node(){};

    Node* parent;
    Node* leftChild;
    Node* rightChild;
    
    Block* block;
};


class SA{
public:
    SA(): root(nullptr),
          R_lowerBound(0.0),
          R_upperBound(0.0),
          blockNum(0) {};
    ~SA(){};

    // file I/O
    void build(string fildname);
    void printOutput(string filename);

    // BST
    void buildBST();
    vector<int> initRemainBlocks();
    void initBlocks();
    void calTotalArea(vector<InputInf> const &inputInf);
    void BST_2_FP();
    void preOrderTraversal(Node* curNode, bool isLeft);
    void updateContour(Block* block);
    double calCost();
    double calRatio();
    bool isValid();

    // annealing schedule
    void M1(int id);               // rotate block
    void M2(Node* n1, Node* n2);   // swap 2 nodes
    void M3(Node* from, Node* to); // remove and insert block

    double initialTemperature();
    void simulateAnnealing();

    // member variables 
    Node* root;
    double R_lowerBound, R_upperBound;
    int blockNum;
    int totalArea;
    vector<InputInf> inputInf;
    vector<Node> BST;
    vector<Block> blocks; // floorplan inf
    list<ContourElement> contour;
};