#include "MST.h"

MST::MST(){
    filesystem::create_directory(outputJsonFilePath);
    filesystem::remove_all(string(outputJsonFilePath) + "/ST");
    filesystem::remove_all(string(outputJsonFilePath) + "/MST");
    filesystem::remove_all(string(outputJsonFilePath) + "/steiner");
}
MST::~MST() {
    for(Node* &node : nodes) delete node;

    unordered_set<Edge*> deletedEdges;
    edges.insert(edges.end(), mst.begin(), mst.end());
    for(Edge* &edge : edges) {
        if(deletedEdges.count(edge)) continue;
        delete edge;
        deletedEdges.insert(edge);
    }
}

size_t MST::glbNodeID = NODENUM;
size_t MST::caseI_times = 0;
size_t MST::caseII_times = 0;
size_t MST::caseIII_times = 0;
size_t MST::default_times = 0;

void MST::genNodes(){
    srand(SEED);
    unordered_set<pair<int,int>, pair_hash> existCoor;
    nodes.reserve(NODENUM);
    for(int i = 0; i < NODENUM; i++){
        int x, y;
        while(true){
            x = rand() % X_RANGE;
            y = rand() % Y_RANGE;
            if(!existCoor.count({x,y})) break;
        }
        nodes.emplace_back(new Node(i, x, y));
        existCoor.insert({x,y});
    }

    // sort the node in ascending order of x
    sort(nodes.begin(), nodes.end(), [](const Node* const n1, const Node* const n2){
        if(n1->x == n2->x) return n1->y > n2->y;
        return n1->x < n2->x;
    });
}

void MST::init(){
    // Construct a spanning graph that contains O(n) edges
    for(int i = 0; i < NODENUM; i++){
        Node* centerNode = nodes[i];
        for(int j = i+1; j < NODENUM; j++){
            Node* checkNode = nodes[j];
            Region region = centerNode->getRegion(checkNode);
            if(region == skip) continue;
            double dist = centerNode->dist(checkNode);
            if(dist < centerNode->minDistance[region]){
                centerNode->nearestNodes[region] = checkNode;
                centerNode->minDistance[region] = dist;
            }
        }
        // Construct edge
        for(Node* node : centerNode->nearestNodes){
            if(node == nullptr) continue;
            edges.emplace_back(new Edge(centerNode,node));
        }
        saveGraph("ST", i, edges);
    }
}

void MST::Kruskal_Algorithm(){
    edges.sort(Edge::smallerAhead);
    size_t MST_step = 0;
    for(Edge* edge : edges){
        if(findParent(edge->endpoints[0]) != findParent(edge->endpoints[1])){
            mst.push_back(edge);
            edgeInMST.insert(edge);
            unionSet(edge->endpoints[0],edge->endpoints[1]);
            saveGraph("MST",MST_step++, mst);
        }
    }
}

Node* MST::findParent(Node* node){
    if(node != node->parent) node->parent = findParent(node->parent);
    return node->parent;
}

void MST::unionSet(Node* n1, Node* n2){
    Node* root1 = findParent(n1);
    Node* root2 = findParent(n2);

    if(root1 != root2){
        if(root1->rank > root2->rank) root2->parent = root1;
        else if(root1->rank < root2->rank) root1->parent = root2;
        else{
            root2->parent = root1;
            root1->rank++;
        }
    }
}

/**
 * @brief The function returns {centerNode, n1, n2}, 
 * where {centerNode, n1} belongs to e1, 
 * and {centerNode, n2} belongs to e2
 */
vector<Node*> MST::getNodeRelation(Edge* e1, Edge* e2){
    if(e2 == nullptr) return {nullptr,nullptr,nullptr};

    Node* centerNode = nullptr;
    bool isFind = false;
    for(Node* ep1 : e1->endpoints){
        for(Node* ep2 : e2->endpoints){
            if(ep1 == ep2){
                centerNode = ep1;
                isFind = true;
                break;
            }
        }
        if(isFind) break;
    }
    Node *n1 = nullptr, *n2 = nullptr;
    for(Node* ep1 : e1->endpoints){
        if(ep1 == centerNode) continue;
        n1 = ep1;
        break;
    }
    for(Node* ep2 : e2->endpoints){
        if(ep2 == centerNode) continue;
        n2 = ep2;
        break;
    }
    return {centerNode,n1,n2};
}

BoxCase MST::getBoxCase(Node* centerNode, Node* n1, Node* n2){
    Quadrant r1 = centerNode->getQuadrant(n1);
    Quadrant r2 = centerNode->getQuadrant(n2);
    if(r1 == axis || r2 == axis) return defaultBoxCase;
    if(r1 == r2) return sameRegion;
    if((r1 == Q1 && r2 == Q3)
    || (r1 == Q2 && r2 == Q4)
    || (r1 == Q3 && r2 == Q1)
    || (r1 == Q4 && r2 == Q2)) return oppositeRegion;
    return neighboringRegion;
}

void MST::rectilinearize(){
    // First remove the edge not inside mst
    for(Node* node : nodes){
        list<Edge*> &edgeList = node->edgeList;
        for(auto iter = edgeList.begin(); iter != edgeList.end();){
            if(!edgeInMST.count(*iter)) iter = edgeList.erase(iter);
            else iter++;
        }
    }
    mst.sort(Edge::biggerAhead); // Sort the edges in descending weight
    queue<Edge*> q;
    for(Edge* edge : mst) q.push(edge);
    size_t steiner_step = 0;
    saveGraph("steiner", steiner_step++, mst);
    while(!q.empty()){
        Edge* e1 = q.front();
        q.pop();
        if(e1->isDone || !e1->isIncline()) continue;
        Edge* e2 = e1->getBiggestInclineNeighborEdge();
        vector<Node*> relation = getNodeRelation(e1,e2);
        Node *centerNode = relation[0], *n1 = relation[1], *n2 = relation[2];
        BoxCase op = (e2 == nullptr)? defaultBoxCase : getBoxCase(centerNode,n1,n2);
        switch(op){
            case oppositeRegion:
                caseI(e1,e2,centerNode,n1,n2);
                break;
            case neighboringRegion:
                caseII(e1,e2,centerNode,n1,n2);
                break;
            case sameRegion:
                caseIII(e1,e2,centerNode,n1,n2);
                break;
            default:
                defaultCase(e1);
                break;
        }
        saveGraph("steiner", steiner_step++, mst);
    }
}

/**
 * @brief Two edges in opposite region, generate point at the corner of rectangle of edge
 * @param e1 The longest edge
 * @param e2 The longest neighboring edge of e1
 * @param centerNode Common endpoint of e1 and e2
 * @param n1 The other endpoint of e1
 * @param n2 The other endpoint of e2
 */
void MST::caseI(Edge* e1, Edge* e2, Node* centerNode, Node* n1, Node* n2){
    caseI_times++;
// 1. Generate one of possible corner nodes of e1 and e2
    Node* s1 = Node::addSteinerNode(glbNodeID++, n1->x, centerNode->y); // Corner node of e1
    Node* s2 = Node::addSteinerNode(glbNodeID++, n2->x, centerNode->y); // Corner node of e2

// 2. Connect s1 and s2 to corresponding nodes
    // Connect s1 to centerNode and n1
    Edge* _e1 = new Edge(s1, centerNode);
    Edge* _e2 = new Edge(s1, n1);
    // Connect s2 to centerNode and n2
    Edge* _e3 = new Edge(s2, centerNode);
    Edge* _e4 = new Edge(s2, n2);

// 3. Store _e1 ~ _e4 to mst
    mst.push_back(_e1);
    mst.push_back(_e2);
    mst.push_back(_e3);
    mst.push_back(_e4);
// 4. Store steiners to nodes
    nodes.push_back(s1);
    nodes.push_back(s2);

// 5. Delete e1 and e2 in following lists:
// mst, centerNode->edgeList, n1->edgeList, n2->edgeList
    centerNode->edgeList.remove(e1);
    n1->edgeList.remove(e1);
    mst.remove(e1);
    e1->isDone = true;

    centerNode->edgeList.remove(e2);
    n2->edgeList.remove(e2);
    mst.remove(e2);
    e2->isDone = true;
}

/**
 * @brief Two edges in neighboring region, generate point at the corner of rectangle of edge
 * @param e1 The longest edge
 * @param e2 The longest neighboring edge of e1
 * @param centerNode Common endpoint of e1 and e2
 * @param n1 The other endpoint of e1
 * @param n2 The other endpoint of e2
 */
void MST::caseII(Edge* e1, Edge* e2, Node* centerNode, Node* n1, Node* n2){
    caseII_times++;
// 1. Check the relative position of n1, n3 and centerNode
    vector<Node*> steiners;
    Quadrant r1 = centerNode->getQuadrant(n1);
    Quadrant r2 = centerNode->getQuadrant(n2);
    if(((r1 == Q1 || r1 == Q2) && (r2 == Q1 || r2 == Q2))
    || ((r1 == Q3 || r1 == Q4) && (r2 == Q3 || r2 == Q4))){
        // Both n1 and n3 are below/higher than centerNode
        Node* s1 = Node::addSteinerNode(glbNodeID++, centerNode->x, n1->y, n1);
        Node* s2 = Node::addSteinerNode(glbNodeID++, centerNode->x, n2->y, n2);
        steiners = {s1,s2};
    }
    else{
        // Both n1 and n3 are in the LHS or RHS of centerNode
        Node* s1 = Node::addSteinerNode(glbNodeID++, n1->x, centerNode->y, n1);
        Node* s2 = Node::addSteinerNode(glbNodeID++, n2->x, centerNode->y, n2);
        steiners = {s1,s2};
    }

// 2. Construct new edge to steiner points
    list<Edge*> newEdges;
    double dist1 = steiners[0]->dist(centerNode);
    double dist2 = steiners[1]->dist(centerNode);
    if(dist1 < dist2){
        Edge* _e1 = new Edge(centerNode,steiners[0]);
        Edge* _e2 = new Edge(steiners[0],steiners[0]->neighborNode);
        Edge* _e3 = new Edge(steiners[0],steiners[1]);
        Edge* _e4 = new Edge(steiners[1],steiners[1]->neighborNode);
        newEdges = {_e1,_e2,_e3,_e4};
    }
    else if(dist1 > dist2){
        Edge* _e1 = new Edge(centerNode,steiners[1]);
        Edge* _e2 = new Edge(steiners[1],steiners[1]->neighborNode);
        Edge* _e3 = new Edge(steiners[1],steiners[0]);
        Edge* _e4 = new Edge(steiners[0],steiners[0]->neighborNode);
        newEdges = {_e1,_e2,_e3,_e4};
    }
    else{
        // Two steiners are actually in the same position
        Edge* _e1 = new Edge(steiners[0],centerNode);
        Edge* _e2 = new Edge(steiners[0],n1);
        Edge* _e3 = new Edge(steiners[0],n2);
        newEdges = {_e1,_e2,_e3};
        // Delete unused steiner node
        delete steiners[1];
        steiners[1] = nullptr;
    }
    // Store _e1 ~ _e4 to edges
    mst.insert(mst.end(),newEdges.begin(),newEdges.end());
    // Store steiners to nodes
    for(Node* node : steiners) if(node != nullptr) nodes.push_back(node);

// 3. Delete e1 and e2 in following lists:
// mst, centerNode->edgeList, n1->edgeList, n2->edgeList
    centerNode->edgeList.remove(e1);
    n1->edgeList.remove(e1);
    mst.remove(e1);
    e1->isDone = true;

    centerNode->edgeList.remove(e2);
    n2->edgeList.remove(e2);
    mst.remove(e2);
    e2->isDone = true;
}

void MST::caseIII(Edge* e1, Edge* e2, Node* centerNode, Node* n1, Node* n2){
    caseIII_times++;
// 1. First find the left-most node
    Node* s3 = nullptr;
    Node *left = nullptr, *right = nullptr;
    if(n1->x < n2->x){
        left = n1; right = n2;
    }
    else{
        left = n2; right = n1;
    }
// 2. Generate s3 and relative edges
    list<Edge*> newEdges;
    vector<Node*> newNodes;
    if(n1->x != n2->x){
        // s3 has different position to n1 and n2, then generate it
        s3 = Node::addSteinerNode(glbNodeID++, left->x, right->y);
        // Generate another steiner node
        Node* s1 = Node::addSteinerNode(glbNodeID++, s3->x, centerNode->y);
        // Connect steiner nodes
        Edge* _e1 = new Edge(s3,n1);
        Edge* _e2 = new Edge(s3,n2);
        Edge* _e3 = new Edge(s3,s1);
        Edge* _e4 = new Edge(s1,centerNode);
        newNodes = {s1,s3};
        newEdges = {_e1,_e2,_e3,_e4};
    }
    else{
        // s3 has same position of the nearest node of centerNode in n1 and n2
        s3 = (centerNode->dist(n1) < centerNode->dist(n2))? n1 : n2;
        // Generate a steiner node between centerNode and s3
        Node* s1 = Node::addSteinerNode(glbNodeID++, s3->x, centerNode->y);
        // Connect steiner nodes
        Edge* _e1 = new Edge(s1, centerNode);
        Edge* _e2 = new Edge(s1, s3);
        Edge* _e3 = new Edge(n1, n2);
        newNodes = {s1};
        newEdges = {_e1,_e2,_e3};
    }
// 3. Store newEdges and newNodes
    mst.insert(mst.end(), newEdges.begin(), newEdges.end());
    nodes.insert(nodes.end(), newNodes.begin(), newNodes.end());

// 4. Delete e1 and e2 in following lists:
// mst, centerNode->edgeList, n1->edgeList, n2->edgeList
    centerNode->edgeList.remove(e1);
    n1->edgeList.remove(e1);
    mst.remove(e1);
    e1->isDone = true;

    centerNode->edgeList.remove(e2);
    n2->edgeList.remove(e2);
    mst.remove(e2);
    e2->isDone = true;
}

/**
 * @brief Change e1 into 2 perpendicular edges, preferring the steiner point lies in other edges
 */
void MST::defaultCase(Edge* e1){
    default_times++;
    Node* n1 = e1->endpoints[0];
    Node* n2 = e1->endpoints[1];
    Node* candidate = nullptr;
    // 1. Generate candidate steiner node
    Node* s1 = Node::addSteinerNode(glbNodeID++, n1->x, n2->y);
    Node* s2 = Node::addSteinerNode(glbNodeID++, n2->x, n1->y);
    
    // 2. Choose the steiner that lies in existing neighbor edge, if no, choose s1
    list<Edge*> neighborEdges(n1->edgeList);
    neighborEdges.insert(neighborEdges.end(), n2->edgeList.begin(), n2->edgeList.end());
    bool same_x = false;
    Edge* candidate_edge = nullptr;
    for(Edge* edge : neighborEdges){
        Node* u = edge->endpoints[0];
        Node* v = edge->endpoints[1];
        if(s1->x == u->x && s1->x == v->x){
            same_x = true;
            candidate_edge = edge;
            candidate = s1;
        }
        else if(s1->y == u->y && s1->y == v->y){
            candidate_edge = edge;
            candidate = s1;
        }
        else if(s2->x == u->x && s2->x == v->x){
            same_x = true;
            candidate_edge = edge;
            candidate = s2;
        }
        else if(s2->y == u->y && s2->y == v->y){
            candidate_edge = edge;
            candidate = s2;
        }
    }
    
    if(candidate == s1) delete s2;
    else if(candidate == s2) delete s1;
    else delete s2;

    // 3. Add edges
    if(candidate != nullptr){
        Node* u = candidate_edge->endpoints[0];
        Node* v = candidate_edge->endpoints[1];
        vector<Node*> tmpNodes = {candidate, u, v};
        unordered_set tmpSet = {u, v};
        if(same_x) sort(tmpNodes.begin(), tmpNodes.end(), [](Node* &a, Node* &b){
            return a->x < b->x;
        });
        else sort(tmpNodes.begin(), tmpNodes.end(), [](Node* &a, Node* &b){
            return a->y < b->y;
        });

        // Delete candidate_edge
        u->edgeList.remove(candidate_edge);
        v->edgeList.remove(candidate_edge);
        mst.remove(candidate_edge);
        // connect with new edges
        Edge* _e1 = new Edge(tmpNodes[0], tmpNodes[1]);
        Edge* _e2 = new Edge(tmpNodes[1], tmpNodes[2]);
        Edge* _e3 = nullptr;
        if(!tmpSet.count(n1)) _e3 = new Edge(candidate,n1);
        else _e3 = new Edge(candidate, n2);
        // Store _e1, _e2 and candidate
        mst.push_back(_e1);
        mst.push_back(_e2);
        mst.push_back(_e3);
        nodes.push_back(candidate);
    }
    else{
        Edge* _e1 = new Edge(s1,n1);
        Edge* _e2 = new Edge(s1,n2);
        // Store _e1, _e2 and s1
        mst.push_back(_e1);
        mst.push_back(_e2);
        nodes.push_back(s1);
    }

    // 4. Delete e1 in following lists:
    // mst, n1->edgeList, n2->edgeList
    n1->edgeList.remove(e1);
    n2->edgeList.remove(e1);
    mst.remove(e1);
    e1->isDone = true;
    return;
}

/**
 * @param candidates A set of nodes to traverse
 * @param target The target the returns node needs to met
 * @return Returns a node-pair fulfill the target; otherwise, return an empty set
 */
vector<Node*> MST::findCandidates(vector<Node*> &candidates, Target target){
    // Find nodes with same x
    if(target == nodeWithSameX){
        for(size_t i = 0; i < 3; i++){
            for(size_t j = i+1; j < 4; j++){
                Node* n1 = candidates[i];
                Node* n2 = candidates[j];
                if(n1->x == n2->x) return {n1,n2};
            }
        }
    }

    // Find nodes with same y
    for(size_t i = 0; i < 3; i++){
        for(size_t j = i+1; j < 4; j++){
            Node* n1 = candidates[i];
            Node* n2 = candidates[j];
            if(n1->y == n2->y) return {n1,n2};
        }
    }

    // Should not get into this part
    return {};
}

void MST::printGraph(){
    ofstream fout(string(outputJsonFilePath) + "/graph.inf");
    // nodeNum
    fout << nodes.size() << '\n';

    // Coordinates of each node
    for(Node* node : nodes){
        fout << node->name << " " 
             << node->x << " " 
             << node->y << " " 
             << ((node->isSteiner)? "s\n" : "\n");
    }
    fout << '\n';

    // MST edgeNum
    fout << mst.size() << '\n';
    for(Edge* edge : mst){
        fout << edge->endpoints[0]->name << " " << edge->endpoints[1]->name << '\n';
    }
    fout << '\n';
    // MST edge weight
    for(Edge* edge : mst){
        fout << edge->weight << ' ';
    }
}

Json::Value MST::nodeToJson(Node* node) {
    Json::Value nodeJson;
    nodeJson["id"] = node->name;
    nodeJson["x"] = node->x;
    nodeJson["y"] = node->y;
    nodeJson["isSteiner"] = node->isSteiner;
    return nodeJson;
}

Json::Value MST::edgeToJson(Edge* edge) {
    Json::Value edgeJson;
    edgeJson["start"] = edge->endpoints[0]->name;
    edgeJson["end"] = edge->endpoints[1]->name;
    return edgeJson;
}

Json::Value MST::graphToJson(list<Edge*> edgesToPrint){
    Json::Value jsonGraph;

    Json::Value jsonNodes;
    for (const auto& node : nodes) {
        jsonNodes.append(nodeToJson(node));
    }
    jsonGraph["nodes"] = jsonNodes;

    Json::Value jsonEdges;
    for (const auto& edge : edgesToPrint) {
        jsonEdges.append(edgeToJson(edge));
    }
    jsonGraph["edges"] = jsonEdges;

    return jsonGraph;
}

void MST::writeJsonFile(const string &prefix, const string &filename, const Json::Value& jsonData){
    filesystem::create_directory(string(outputJsonFilePath) + "/" + prefix);
    ofstream fout(string(outputJsonFilePath) + "/" + prefix + "/" + filename);
    fout << jsonData.toStyledString();;
    fout.close();
}

void MST::saveGraph(const string &prefix, size_t step, list<Edge*> edgesToPrint) {
    string filename = to_string(step) + ".json";
    Json::Value jsonGraph = graphToJson(edgesToPrint);
    writeJsonFile(prefix, filename, jsonGraph);
}