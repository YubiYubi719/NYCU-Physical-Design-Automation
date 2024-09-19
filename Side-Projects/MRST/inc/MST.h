#ifndef MST_H
#define MST_H

#include "Util.h"
#include "Node.h"
#include "Edge.h"
#include <bits/stdc++.h>
#include <json/json.h>
#include <filesystem>
using namespace std;

class MST{
public:
    MST();
    ~MST();

    void genNodes();
    void init();
    void Kruskal_Algorithm();
    Node* findParent(Node* node);
    void unionSet(Node* n1, Node* n2);
    vector<Node*> getNodeRelation(Edge* e1, Edge* e2);
    BoxCase getBoxCase(Node* centerNode, Node* n1, Node* n2);
    void rectilinearize();
    void caseI(Edge* e1, Edge* e2, Node* centerNode, Node* n1, Node* n2);
    void caseII(Edge* e1, Edge* e2, Node* centerNode, Node* n1, Node* n2);
    void caseIII(Edge* e1, Edge* e2, Node* centerNode, Node* n1, Node* n2);
    void defaultCase(Edge* e1);
    vector<Node*> findCandidates(vector<Node*> &candidates, Target target);
    void printGraph();
    Json::Value nodeToJson(Node* node);
    Json::Value edgeToJson(Edge* edge);
    Json::Value graphToJson(list<Edge*> edgesToPrint);
    void writeJsonFile(const string &prefix, const string &filename, const Json::Value& jsonData);
    void saveGraph(const string &prefix, size_t step, list<Edge*> edgesToPrint);

    static size_t glbNodeID;
    static size_t caseI_times, caseII_times, caseIII_times, default_times;
    vector<Node*> nodes;
    list<Edge*> edges;
    list<Edge*> mst;
    unordered_set<Edge*> edgeInMST;
};

#endif