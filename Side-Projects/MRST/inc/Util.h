#ifndef UTIL_H
#define UTIL_H

#define NODENUM 35
#define X_RANGE 100
#define Y_RANGE 100
#define SEED 1726492765
#include <bits/stdc++.h>
using namespace std;

// Path to directory that stores the output json files
constexpr const char* outputJsonFilePath = "graph_json";

enum Region{
    R1,
    R2,
    R3,
    R4,
    skip
};

enum Quadrant{
    Q1,
    Q2,
    Q3,
    Q4,
    axis
};

enum BoxCase{
    oppositeRegion,
    neighboringRegion,
    sameRegion,
    defaultBoxCase // one of the edge is vertical or horizontal
};

enum Target{
    nodeWithSameX,
    nodeWithSameY
};

struct pair_hash{
    size_t operator() (const pair<int,int> &p) const{
        size_t h1 = hash<int>{}(p.first);
        size_t h2 = hash<int>{}(p.second);
        return h1 ^ (h2 << 1);
    }
};

#endif