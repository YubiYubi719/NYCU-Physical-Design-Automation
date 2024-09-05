#ifndef MOS_H
#define MOS_H

#include <bits/stdc++.h>
#include <unordered_set>
using namespace std;

class MOS{
public:
    MOS(): isDefined(false) { };
    MOS(string s): 
    name(s),
    gate(s),
    isDefined(true) { };
    MOS(string mosName,
        string D, 
        string G, 
        string S,
        string typ):
        name(mosName),
        drain(D),
        gate(G),
        source(S),
        isConnect_drain(false),
        isConnect_source(false),
        d_s(false),
        isDefined(true),
        insertDummy(false) {
            n_p = (typ == "nmos_rvt")? true:false;
        };
    ~MOS(){};

    string name;
    string drain, gate, source;
    bool isConnect_drain;
    bool isConnect_source;
    bool d_s; // drain_source
    bool n_p; // true: nmos, false: pmos
    bool isDefined;
    bool insertDummy;
};

struct SOL{
    SOL(){};
    SOL(vector<MOS> a,
        vector<MOS> b): 
        path_n(a),
        path_p(b) {};
    ~SOL(){};

    vector<MOS> path_n;
    vector<MOS> path_p;
};

class Schematic{
public:
    Schematic():w_n(0.0),
                w_p(0.0),
                isMultiGate(false) { };
    ~Schematic(){};

    // read file & initialize
    void build(string filename);
    void buildGateMap();

    void M1(pair<vector<MOS>,vector<MOS>> &pathOrder);
    void M2(pair<vector<MOS>,vector<MOS>> &pathOrder);
    pair<vector<MOS>,vector<MOS>> initPathOrder(vector<string> const &gateOrder); // first: nmos_path, second: pmos_path
    vector<MOS> cutPath(vector<MOS> path);
    void insertDummy(vector<MOS> &prePath_n, vector<MOS> &prePath_p);
    // list<MOS> findPinSeq(vector<MOS> path);
    double calHPWL(vector<MOS> const &path_n, vector<MOS> const &path_p);
    void printOutput(string filename);

    // SA
    void simulateAnnealing();
    double initialTemperature(pair<vector<MOS>,vector<MOS>> &pathOrder);
    void test();

    unordered_set<string> inputGates; // input gate
    vector<MOS> mosInf;
    unordered_map<string,vector<pair<MOS,MOS>>> gateMap; // key: gate
                                                         // value: vector<Nmos,Pmos>
    double w_n, w_p;
    bool isMultiGate;
    vector<MOS> path_n;
    vector<MOS> path_p;
    // list<MOS> pinSeq_n;
    // list<MOS> pinSeq_p;

    SOL results;
    double best_HPWL;
};

#endif