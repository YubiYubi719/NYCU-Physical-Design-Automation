#ifndef NODE_H
#define NODE_H

#include <bits/stdc++.h>
#include "Util.h"
#include "Edge.h"
using namespace std;

class Edge;

class Node{
public:
    Node();
    Node(int name, int x_in, int y_in);
    Node(int name, int x_in, int y_in, Node* node);
    ~Node();

    double dist(const Node* const node);
    Region getRegion(const Node* const node);
    Quadrant getQuadrant(const Node* const node);
    static Node* addSteinerNode(int name, int x_in, int y_in);
    static Node* addSteinerNode(int name, int x_in, int y_in, Node* node);

    list<Edge*> edgeList;
    int name;
    int x;
    int y;
    Node* parent;
    size_t rank;
    vector<Node*> nearestNodes; // R1, R2, R3, R4 4 regions
    vector<double> minDistance; // the distance between current node and nearest node in 4 regions
    Node* neighborNode; // Used in caseII, record current steiner node belongs to which edge
    bool isSteiner;
};

#endif