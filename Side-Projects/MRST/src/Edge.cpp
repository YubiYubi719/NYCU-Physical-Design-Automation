#include "Edge.h"

Edge::Edge() = default;

Edge::Edge(Node* u_in, Node* v_in): isDone(false){
    endpoints = {u_in, v_in};
    weight = u_in->dist(v_in);
    // Add current into endpoint edge list
    u_in->edgeList.push_back(this);
    v_in->edgeList.push_back(this);
}

Edge::~Edge() = default;

bool Edge::smallerAhead(Edge* &a, Edge* &b){
    return a->weight < b->weight;
}

bool Edge::biggerAhead(Edge* &a, Edge* &b){
    return a->weight > b->weight;
}

bool Edge::isIncline(){
    return !((endpoints[0]->x == endpoints[1]->x) || (endpoints[0]->y == endpoints[1]->y));
}

Edge* Edge::getBiggestInclineNeighborEdge(){
    Edge* biggestInclineEdge = nullptr;
    double maxWeight = DBL_MIN;
    for(Node* endpoint : endpoints){
        for(Edge* edge : endpoint->edgeList){
            if(edge != this && edge->isIncline() && edge->weight > maxWeight){
                biggestInclineEdge = edge;
                maxWeight = edge->weight;
            }
        }
    }
    return biggestInclineEdge;
}