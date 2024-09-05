#include <bits/stdc++.h>
using namespace std;

class Net{
public:
    Net():name(-1),
          start(-1),
          end(-1),
          isPlaced(false) {};
    ~Net(){};

    bool operator < (const Net& rhs) const{ return this->start < rhs.start; }
    static bool myCompare(const Net* a, const Net* b){ return *a < *b; }

    string track;
    int name;
    int start, end;
    bool isPlaced;
};

struct TBtrack{
    TBtrack(){};
    ~TBtrack(){};

    string name;
    vector<bool> trackSegs; // false: empty
                            // true: blocked
};

struct Track{
    Track():watermark(-1) {};
    Track(string s):name(s),watermark(-1) {};
    ~Track(){};

    string name;
    int watermark;
    list<Net*> netList;
};

class ChannelRouter{
public:
    ChannelRouter(){};
    ~ChannelRouter(){};

    void build(string filename);
    void updateBlockage();
    void updateNetRange();
    vector<Net*> chooseNets_TOP();
    vector<Net*> chooseNets();
    void routeTOP();
    void routeBOT();
    void placeNetsFromTop(vector<Net*> sortedNets, size_t trackID);
    void placeNetsFromBot(vector<Net*> sortedNets, size_t trackID);
    void writeOutput(string filename);

    // input information
    vector<TBtrack> topTracks;
    vector<TBtrack> botTracks;
    list<Track>     midTracks;
    vector<int> topPins;
    vector<int> botPins;
    // channel routing variables
    vector<Net> nets;
    vector<int> in_degree_TOP;
    vector<vector<int>> edges_TOP;
    vector<int> in_degree;
    vector<vector<int>> edges;
};