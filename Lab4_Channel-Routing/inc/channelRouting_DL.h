#include <bits/stdc++.h>
#include <unordered_map>
using namespace std;

class Pin_DL{
public:
    Pin_DL():originID(-1) {};
    Pin_DL(int ID):originID(ID) {}; 
    ~Pin_DL(){};

    int originID;
    string sortingName;
};

class SubNet_DL{
public:
    SubNet_DL():name("-1"),
          start(-1),
          end(-1),
          parent(-1),
          isPlaced(false) {};
    SubNet_DL(int s,
           int e,
           int p,
           string n):
           name(n),
           start(s),
           end(e),
           parent(p),
           isPlaced(false) {};
    ~SubNet_DL(){};

    bool operator< (const SubNet_DL& rhs) const{ return this->start < rhs.start; }
    static bool myCompare(const shared_ptr<SubNet_DL> a, const shared_ptr<SubNet_DL> b){ return *a < *b; }

    string track;
    string name;
    int start, end;
    int parent;
    bool isPlaced;
};

class Net_DL{
public:
    Net_DL():start(-1),
             end(-1) {};
    ~Net_DL(){};

    int name;
    int start;
    int end;
    vector<int> cutPosition;
    vector<shared_ptr<SubNet_DL>> netList;
};


struct TBtrack_DL{
    TBtrack_DL(): prevNet(-1) {};
    ~TBtrack_DL(){};

    string name;
    int prevNet;
    vector<bool> trackSegs; // false: empty
                            // true: blocked
};

struct Track_DL{
    Track_DL():watermark(-1) {};
    Track_DL(string s):name(s),watermark(-1) {};
    ~Track_DL(){};

    string name;
    int prevNet;
    int watermark;
    list<shared_ptr<SubNet_DL>> netList;
};

class ChannelRouter_DL{
public:
    ChannelRouter_DL(){};
    ~ChannelRouter_DL(){};

    void build(string filename);
    void updateBlockage();
    void updateNetInf();
    void cutNet();
    vector<shared_ptr<SubNet_DL>> chooseNets_TOP();
    vector<shared_ptr<SubNet_DL>> chooseNets_BOT();
    void routeTOP();
    void routeBOT();
    void placeNetsFromTop(vector<shared_ptr<SubNet_DL>> sortedNets, size_t trackID);
    void placeNetsFromBot(vector<shared_ptr<SubNet_DL>> sortedNets, size_t trackID);
    void writeOutput(string filename);

    // ###################################
    // #         Input Information       #
    // ###################################
    vector<TBtrack_DL> topTracks;
    vector<TBtrack_DL> botTracks;
    list<Track_DL>     midTracks;
    vector<Pin_DL> topPins;
    vector<Pin_DL> botPins;
    // ###################################
    // #    Channel Routing Variables    #
    // ###################################
    vector<Net_DL> nets;
    vector<shared_ptr<SubNet_DL>> subnets;
    // for top-routing
    unordered_map<string,int> in_degree_TOP;
    unordered_map<string,vector<string>> edges_TOP;
    // for bot-mid-routing
    unordered_map<string,int> in_degree_BOT;
    unordered_map<string,vector<string>> edges_BOT;
};