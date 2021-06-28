#include "parser.hpp"
#include <iostream>
#include <sstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <utility>
#include <LEDA/graph/graph.h>
#include <LEDA/graph/templates/shortest_path.h>
#include <queue>
using namespace std;

class Compare {
public:
    leda::node_array<int> ord;
    bool operator()(leda::node &a, leda::node &b) {
        if (ord[a] < ord[b])
            return 1;
        else
            return 0;
    }
    Compare(leda::node_array<int> &ord) {
        this->ord = ord;
    }
};
Parser::Parser()
{
    this->k = 0;
}
void Parser::parse(char* argv[])
{
    cout << "read_blif " << argv[3] << "\n";
    ifstream input_file(argv[3]);
    stringstream ss;
    string line, tmp;
    k = atoi(argv[2]);

    getline(input_file, line);
    ss << line;
    ss >> tmp >> this->name;
    ss.str("");
    ss.clear();

    leda::node n = G.new_node("source");
    this->mapNodeName[n] = "source";
    this->mapNode["source"] = make_pair(n, "source");

    getline(input_file, line);
    ss << line;
    ss >> tmp;
    while (ss >> tmp) {
        if (tmp == "\\") {
            ss.str("");
            ss.clear();
            getline(input_file, line);
            ss << line;
        }
        else {
            leda::node n = G.new_node(tmp);
            this->mapNodeName[n] = tmp;
            this->mapNode[tmp] = make_pair(n, "pi");
            this->inputs.push(n);
            leda::edge e = G.new_edge(mapNode["source"].first, n, 0);
        }
    }
    ss.str("");
    ss.clear();
    
    getline(input_file, line);
    ss << line;
    ss >> tmp;
    while (ss >> tmp) {
        if (tmp == "\\") {
            ss.str("");
            ss.clear();
            getline(input_file, line);
            ss << line;
        }
        else {
            leda::node n = G.new_node(tmp);
            this->mapNodeName[n] = tmp;
            this->mapNode[tmp] = make_pair(n, "po");
            this->outputs.push(n);
        }
        
    }
    ss.str("");
    ss.clear();

    bool zeroAppear = 0;
    while(1) {
        if (!zeroAppear) {
            if (!getline(input_file, line))
                break;
        }
        else
            zeroAppear = 0;
        ss << line;
        ss >> tmp;
        if (tmp == ".names") {
            vector<string> fanin;
            while (ss >> tmp) {
                fanin.push_back(tmp);
                bool exist = 0;
                for (auto kv : mapNodeName) {
                    if (kv.second == tmp) {
                        exist = 1;
                        break;
                    }
                }
                if (!exist) {
                    leda::node n = G.new_node(tmp);
                    this->mapNodeName[n] = tmp;
                    this->mapNode[tmp] = make_pair(n, "pin");
                }
            }
            int faninSize = fanin.size();
            if (faninSize > 1) {
                for (int i = 0; i < faninSize - 1; ++i) {
                    leda::edge e = G.new_edge(mapNode[fanin[i]].first, mapNode[fanin[faninSize-1]].first, -1);
                }
            }
            ss.str("");
            ss.clear();

            getline(input_file, line);
            if (line.substr(0, 2) == "1-") {
                mapNode[fanin[faninSize-1]].second = "or";
            }
            else if (line.substr(0, 2) == "11") {
                mapNode[fanin[faninSize-1]].second = "and";
            }
            else if (line.substr(0, 2) == "0 ") {
                mapNode[fanin[faninSize-1]].second = "not";
            }
            else if (line.substr(0, 2) == "1 ") {
                mapNode[fanin[faninSize-1]].second = "buf";
            }
            else if (line.size() == 1) {
                mapNode[fanin[faninSize-1]].second = "one";
            }
            else if (line.substr(0, 6) == ".names") {
                mapNode[fanin[faninSize-1]].second = "zero";
                zeroAppear = 1;
            }
        }
        ss.str("");
        ss.clear();
    }
}
void Parser::traverseGraph(leda::GRAPH<std::string, int> &G) {
    leda::node nIter;
    forall_nodes(nIter, G) {
        G.print_node(nIter);
        cout << endl;
    }
    leda::edge eIter;
    forall_edges(eIter, G) {
        G.print_edge(eIter);
        cout << endl;
    }
}
void Parser::first_stage()
{
    leda::node vIter;
    leda::node_array<int> level(G);
    leda::node_array<leda::edge> prev(G);
    
    leda::SHORTEST_PATH_T(G, G.first_node(), G.edge_data(), level, prev);
    leda::node nIter;
    std::unordered_map<leda::node, int> mapNodeLevel;
    forall_nodes (nIter, G) {
        mapNodeLevel[nIter] = -level[nIter];
    }
    forall_nodes(vIter, G) {
        int dcpCnt = 0;
        while (G.indeg(vIter) > 2) {
            leda::list<leda::edge> inEdges = G.in_edges(vIter);
            leda::edge eIter, e1, e2;
            leda::node n1, n2;
            int min1 = INT_MAX, min2 = INT_MAX;
            
            forall(eIter, inEdges) {
                leda::node n = G.source(eIter);
                int nLevel = mapNodeLevel[n];
                if (nLevel < min1) {
                    e2 = e1;
                    n2 = n1;
                    min2 = min1;
                    e1 = eIter;
                    n1 = n;
                    min1 = nLevel;
                }
                else if (nLevel < min2) {
                    e2 = eIter;
                    n2 = n;
                    min2 = nLevel;
                }
            }
            string dcpNode = mapNodeName[vIter] + to_string(dcpCnt);
            leda::node n = G.new_node(dcpNode);
            this->mapNodeName[n] = dcpNode;
            this->mapNode[dcpNode] = make_pair(n, mapNode[mapNodeName[vIter]].second);
            mapNodeLevel[n] = max(mapNodeLevel[n1], mapNodeLevel[n2]) + 1;
            G.move_edge(e1 , n1, n);
            G.move_edge(e2 , n2, n);
            leda::edge e = G.new_edge(n, vIter, -1);
            ++dcpCnt;
        }
    }
    leda::node n = G.new_node("target");
    this->mapNodeName[n] = "target";
    this->mapNode["target"] = make_pair(n, "target");
    forall(nIter, outputs) {
        leda::edge e = G.new_edge(nIter, mapNode["target"].first, 0);
    }
}
void Parser::dfsCreateNt(Subgraph &subgraph, leda::node_array<int> &visit, leda::node tIter)
{
    leda::list<leda::edge> inEdges = G.in_edges(tIter);
    leda::edge eIter;
    visit[tIter] = 1;


    forall(eIter, inEdges) {
        leda::node tFather = G.source(eIter);
        if (!visit[tFather]) {
            leda::node n = subgraph.Nt.new_node(mapNodeName[tFather]);
            subgraph.mapGtoNt[mapNodeName[tFather]] = n;
            subgraph.mapNttoG[subgraph.Nt.inf(n)] = tFather;
            dfsCreateNt(subgraph, visit, tFather);
        }
        leda::edge e = subgraph.Nt.new_edge(subgraph.mapGtoNt[mapNodeName[tFather]], subgraph.mapGtoNt[mapNodeName[tIter]]);
    }
}
void Subgraph::nodeCollapse(unordered_map<leda::node, string> &mapZip, leda::node_array<int> &label, int &maxLabel)
{
    leda::node tIter;
    leda::edge eIter;
    
    forall_nodes(tIter, Nt) {
        if (label[mapNttoG[Nt.inf(tIter)]] == maxLabel) {
            mapZip[mapNttoG[Nt.inf(Nt.first_node())]] = mapZip[mapNttoG[Nt.inf(Nt.first_node())]] + " " + Nt.inf(tIter);
            leda::list<leda::edge> inEdges = Nt.in_edges(tIter);
            forall(eIter, inEdges) {
                leda::node tFather = Nt.source(eIter);
                Nt.move_edge(eIter, tFather, Nt.first_node());
            }
            Nt.del_node(tIter);
        }
    }
}
void Subgraph::nodeSplit()
{
    leda::node tIter;
    leda::edge eIter;
    forall_edges(eIter, Nt) {
        Nt.assign(eIter, 999);
    }
    forall_nodes(tIter, Nt) {
        if (!(Nt.inf(tIter).size() > 4 && Nt.inf(tIter).substr(Nt.inf(tIter).size()-3, 3) == "dup")) {
            leda::list<leda::edge> outEdges = Nt.out_edges(tIter);
            if (tIter != Nt.first_node() && Nt.inf(tIter) != "source") {
                leda::node t_dup = Nt.new_node(Nt.inf(tIter) + "_dup");
                forall(eIter, outEdges) {
                    leda::node tSon = Nt.target(eIter);
                    Nt.move_edge(eIter, t_dup, tSon);
                }
                leda::edge e_dup = Nt.new_edge(tIter, t_dup, 1);
            }
        }
    }
}
void Subgraph::dfsResidual(leda::node_array<int> &visit, std::unordered_map<leda::node, std::string> &mapGroup, leda::edge_array<int> &f, leda::node tIter)
{
    leda::list<leda::edge> inEdges = Nt.in_edges(tIter);
    leda::edge eIter;
    visit[tIter] = 1;

    forall(eIter, inEdges) {
        leda::node tFather = Nt.source(eIter);
        if (!visit[tFather] && f[eIter] > 0) {
            dfsResidual(visit, mapGroup, f, tFather);
        }
    }
    if (!(Nt.inf(tIter).size() > 4 && Nt.inf(tIter).substr(Nt.inf(tIter).size()-3, 3) == "dup"))
        mapGroup[mapNttoG[Nt.inf(Nt.first_node())]] += " " + Nt.inf(tIter);
}
void Parser::labeling()
{
    leda::list<leda::node> ord;
    leda::TOPSORT(G, ord);
    leda::node_array<int> label(G);
    int LUTlevel = 0;

    while (ord.size() > 0 ) {
        leda::node t = ord.front();
        ord.pop_front();
        leda::list<leda::edge> inEdges = G.in_edges(t);
        leda::edge eIter;
        int maxLabel = 0;
        
        if (mapNode[mapNodeName[t]].second != "source" && mapNode[mapNodeName[t]].second != "pi" && mapNode[mapNodeName[t]].second != "one" && mapNode[mapNodeName[t]].second != "zero") {
            forall(eIter, inEdges) {
                leda::node n = G.source(eIter);
                if (maxLabel < label[n])
                    maxLabel = label[n];
            }
            if (!maxLabel) {
                label[t] = 1;
                mapGroup[t] = mapNodeName[t];
            }
            else {
                Subgraph subgraph;
                leda::node t_Nt = subgraph.Nt.new_node(mapNodeName[t]);
                subgraph.mapGtoNt[mapNodeName[t]] = t_Nt;
                subgraph.mapNttoG[subgraph.Nt.inf(t_Nt)] = t;
                leda::node_array<int> visit1(G);
                dfsCreateNt(subgraph, visit1, t);
                //cout << "-----Nt1------" << endl;
                //traverseGraph(subgraph.Nt);

                subgraph.nodeCollapse(mapZip, label, maxLabel);
                //cout << "-----Nt2------" << endl;
                //traverseGraph(subgraph.Nt);
                
                subgraph.nodeSplit();
                //cout << "-----Nt3------" << endl;
                //traverseGraph(subgraph.Nt);

                leda::edge_array<int> f(subgraph.Nt);
                int maxFlow = 0;
                maxFlow = MAX_FLOW_T(subgraph.Nt, subgraph.mapGtoNt["source"], subgraph.Nt.first_node(), subgraph.Nt.edge_data(), f);
                leda::edge eIt;
                forall_edges(eIt, subgraph.Nt) {
                    f[eIt] = subgraph.Nt.inf(eIt) - f[eIt];
                }              
                if (maxFlow <= k) {
                    label[t] = maxLabel;
                    leda::node_array<int> visit4(subgraph.Nt);
                    subgraph.dfsResidual(visit4, mapGroup, f, subgraph.Nt.first_node());
                    //cout << "-----Nt4------" << endl;
                    //traverseGraph(subgraph.Nt);
                }       
                else {
                    label[t] = maxLabel + 1;
                    mapGroup[t] = mapNodeName[t];
                    mapZip[t] = "";
                }
            }
           
        }
    }
    leda::node o;
    forall(o, outputs) {
        if (LUTlevel < label[o])
            LUTlevel = label[o];
    }
    cout << "The circuit level is " << LUTlevel << "\n";
}
void Parser::dfsMapping(leda::node_array<bool> &visit, leda::node_array<bool> &inGroup, queue<leda::node> &mappingQ, unordered_map<std::string, std::vector<leda::node>> &tInput, leda::node &cand, leda::node tIter)
{
    leda::list<leda::edge> inEdges = G.in_edges(tIter);
    leda::edge eIter;
    visit[tIter] = 1;

    forall(eIter, inEdges) {
        leda::node tFather = G.source(eIter);
        if (!visit[tFather] && mapNodeName[tFather] != "source") {
            if (inGroup[tFather]) {
                dfsMapping(visit, inGroup, mappingQ, tInput, cand, tFather);
            }
            else {
                mappingQ.push(tFather);
                tInput[mapNodeName[cand]].push_back(tFather);
            }
        }
    }
}
vector<vector<bool>> Parser::enumtruthTable(int bit)
{
    if (bit == 1)
    {
      return {{0}, {1}};
    }
    vector<vector<bool>> recAns = enumtruthTable(bit - 1);
    vector<vector<bool>> mainAns;
    for (size_t i = 0; i < recAns.size(); ++i) {
        vector<bool> tmp = recAns[i];
        tmp.push_back(0);
        mainAns.push_back(tmp);
    }
    for (int i=recAns.size()-1;i>=0;i--) {
        vector<bool> tmp = recAns[i];
        tmp.push_back(1);
        mainAns.push_back(tmp);
    }
    return mainAns;
}
void Parser::calLUTfunc(leda::node_array<bool> &LUTnodeValue, vector<leda::node> &candGroup)
{
    leda::node tFather;
    leda::edge eIter;

    for (size_t i = 0; i < candGroup.size(); ++i) {
        leda::list<leda::edge> inEdges = G.in_edges(candGroup[i]);
        if (mapNode[mapNodeName[candGroup[i]]].second == "and") {
            bool value = 1;
            forall(eIter, inEdges) {
                tFather = G.source(eIter);
                value = value & LUTnodeValue[tFather];
            }
            LUTnodeValue[candGroup[i]] = value;
        }
        else if (mapNode[mapNodeName[candGroup[i]]].second == "or") {
            bool value = 0;
            forall(eIter, inEdges) {
                tFather = G.source(eIter);
                value = value | LUTnodeValue[tFather];
            }
            LUTnodeValue[candGroup[i]] = value;
        }
        else if (mapNode[mapNodeName[candGroup[i]]].second == "not") {
            bool value = 0;
            forall(eIter, inEdges) {
                tFather = G.source(eIter);
                value = !LUTnodeValue[tFather];
            }
            LUTnodeValue[candGroup[i]] = value;
        }
        else if (mapNode[mapNodeName[candGroup[i]]].second == "buf" || mapNode[mapNodeName[candGroup[i]]].second == "one") {
            bool value = 0;
            forall(eIter, inEdges) {
                tFather = G.source(eIter);
                value = value | LUTnodeValue[tFather];
            }
            LUTnodeValue[candGroup[i]] = value;
        }
        else if (mapNode[mapNodeName[candGroup[i]]].second == "buf") {
            LUTnodeValue[candGroup[i]] = 1;
        }
        else if (mapNode[mapNodeName[candGroup[i]]].second == "zero") {
            LUTnodeValue[candGroup[i]] = 0;
        }
    }   
}
void Parser::mapping(char* argv[]) {
    int LUTCnt = 0;
    ofstream output_file(argv[4]);
    leda::node nIter;

    output_file << ".model " << name << "\n";
    output_file << ".inputs ";
    forall(nIter, inputs) {
        output_file << mapNodeName[nIter] << " ";
    }
    output_file << "\n";
    output_file << ".outputs ";
    forall(nIter, outputs) {
        output_file << mapNodeName[nIter] << " ";
    }
    output_file << "\n";
    
    queue<leda::node> mappingQ;
    leda::node_array<bool> assign(G, 0);
    forall(nIter, outputs) {
        mappingQ.push(nIter);
    }
    
    leda::node_array<int> ord(G);
    leda::TOPSORT(G, ord);
    stringstream ss;
    string line, tmp;

    while (!mappingQ.empty()) {
        leda::node cand = mappingQ.front();
        mappingQ.pop();
        if (mapNode[mapNodeName[cand]].second != "pi" && mapNode[mapNodeName[cand]].second != "one" && mapNode[mapNodeName[cand]].second != "zero" && !assign[cand]) {
            ++LUTCnt;
            assign[cand] = 1;
            mapGroup[cand] += mapZip[cand];
            line = mapGroup[cand];
            ss << line;
            leda::node_array<bool> visit(G);
            leda::node_array<bool> inGroup(G);
            vector<leda::node> candGroup;
            while (ss >> tmp) {
                inGroup[mapNode[tmp].first] = 1;
                candGroup.push_back(mapNode[tmp].first);
            }
            ss.str("");
            ss.clear();

            dfsMapping(visit, inGroup, mappingQ, tInput, cand, cand);
            
            output_file << ".names ";
            for (auto i : tInput[mapNodeName[cand]]) {
                output_file << mapNodeName[i] << " ";
            }
            output_file << mapNodeName[cand] << "\n";

            int kLUT = tInput[mapNodeName[cand]].size();
            vector<vector<bool>> truthTable = enumtruthTable(kLUT);
            leda::node_array<bool> LUTnodeValue(G);
            sort(candGroup.begin(), candGroup.end(), Compare(ord));
            
            for (size_t row = 0; row < truthTable.size(); ++row) {
                for (int column = 0; column < kLUT; ++column) {
                    LUTnodeValue[tInput[mapNodeName[cand]][column]] = truthTable[row][column];
                }
                calLUTfunc(LUTnodeValue, candGroup);
                if (LUTnodeValue[cand]) {
                    for (int column = 0; column < kLUT; ++column) {
                        output_file << truthTable[row][column];
                    }
                    output_file << " 1\n";
                }
            }       
        }
    }
    output_file << "\n";
    output_file.close();
    cout << "The number of LUTs is " << LUTCnt << "\n\n";
}
