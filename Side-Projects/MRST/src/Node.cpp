#include "Node.h"

Node::Node() = default;

Node::Node(
    int idx, 
    int x_in, 
    int y_in): 
    name(idx), 
    x(x_in), 
    y(y_in), 
    parent(this),
    rank(0),
    neighborNode(nullptr),
    isSteiner(false) {
    nearestNodes.resize(4,nullptr);
    minDistance.resize(4,DBL_MAX);
}

Node::Node(
    int idx, 
    int x_in, 
    int y_in,
    Node* node): 
    name(idx), 
    x(x_in), 
    y(y_in), 
    parent(this),
    rank(0),
    neighborNode(node),
    isSteiner(false) {
    nearestNodes.resize(4,nullptr);
    minDistance.resize(4,DBL_MAX);
}

Node::~Node() = default;

double Node::dist(const Node* const node){
    int dx = x - node->x;
    int dy = y - node->y;
    return sqrt(dx*dx + dy*dy);
}

Region Node::getRegion(const Node* const node){
    int dx = node->x - x;
    int dy = node->y - y;
    if(dx == 0) return (dy < 0)? R4 : skip;

    double slope = (double)dy / dx;
    if(slope >= 1) return R1;
    if(slope < 1 && slope >= 0)  return R2;
    if(slope < 0 && slope >= -1) return R3;
    if(slope < -1) return R4;
    return skip;
}

Quadrant Node::getQuadrant(const Node* const node){
    int dx = node->x - x;
    int dy = node->y - y;

    if(dx > 0 && dy > 0) return Q1;
    if(dx < 0 && dy > 0) return Q2;
    if(dx < 0 && dy < 0) return Q3;
    if(dx > 0 && dy < 0) return Q4;
    return axis; // node is on x-axis or y-axis
}

Node* Node::addSteinerNode(int name, int x_in, int y_in){
    Node* node = new Node(name, x_in, y_in);
    node->isSteiner = true;
    return node;
}

Node* Node::addSteinerNode(int name, int x_in, int y_in, Node* node){
    Node* newNode = new Node(name, x_in, y_in, node);
    newNode->isSteiner = true;
    return newNode;
}