#ifndef PARSER_H_
#define PARSER_H_
#include <climits>
#include <LEDA/graph/graph.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <queue>

class Subgraph {
public:
    leda::GRAPH<std::string, int> Nt;
    std::unordered_map<std::string, leda::node> mapGtoNt;
    std::unordered_map<std::string, leda::node> mapNttoG;
    void nodeCollapse(std::unordered_map<leda::node, std::string> &mapZip, leda::node_array<int> &label , int &maxLabel);
    void nodeSplit();
    void dfsResidual(leda::node_array<int> &visit, std::unordered_map<leda::node, std::string> &mapGroup, leda::edge_array<int> &f, leda::node tIter);
};

class Parser {
public:
    std::string name;
    int k;
    leda::GRAPH<std::string, int> G;
    leda::list<leda::node> inputs;
    leda::list<leda::node> outputs;
    leda::list<leda::node> gate;
    std::unordered_map<leda::node, std::string> mapGroup;
    std::unordered_map<leda::node, std::string> mapZip;
    std::unordered_map<std::string, std::vector<leda::node>> tInput;
    std::unordered_map<leda::node, std::string> mapNodeName;
    std::unordered_map<std::string, std::pair<leda::node, std::string>> mapNode;

    Parser();
    void parse(char* argv[]);
    void traverseGraph(leda::GRAPH<std::string, int> &G);
    void first_stage();
    void dfsCreateNt(Subgraph &subgraph, leda::node_array<int> &visit, leda::node tIter);
    void labeling();
    void dfsMapping(leda::node_array<bool> &visit, leda::node_array<bool> &inGroup, std::queue<leda::node> &mappingQ, std::unordered_map<std::string, std::vector<leda::node>> &tInput, leda::node &cand, leda::node tIter);
    std::vector<std::vector<bool>> enumtruthTable(int bit);
    void calLUTfunc(leda::node_array<bool> &LUTnodeValue, std::vector<leda::node> &candGroup);
    void mapping(char* argv[]);
};

#endif