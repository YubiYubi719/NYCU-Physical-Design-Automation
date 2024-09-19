#ifndef EDGE_H
#define EDGE_H

#include "Node.h"
#include <bits/stdc++.h>
using namespace std;

class Node;

class Edge{
public:
    Edge();
    Edge(Node* u_in, Node* v_in);
    ~Edge();

    static bool smallerAhead(Edge* &a, Edge* &b);
    static bool biggerAhead(Edge* &a, Edge* &b);
    bool isIncline();
    Edge* getBiggestInclineNeighborEdge();

    array<Node*,2> endpoints; // endpoints
    double weight;
    bool isDone; // Whether current edge is rectilinearized or not
};

#endif
