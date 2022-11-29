//
// Created by wangxuezhu on 2022/11/27.
//

#ifndef COMPILER_GRAPH_H
#define COMPILER_GRAPH_H
#include <vector>
#include <set>
using namespace std;

class DomTree
{
public:
    int n;
    int root = 0;
    vector<vector<int>> sons;
    vector<int> father;

    DomTree(int n) {
        for (int i = 0; i < n; i++) {
            sons.push_back(vector<int>());
            father.push_back(-1);
        }
    }
    void addSon(int u, int v) {
        sons[u].push_back(v);
        father[v] = u;
    }
};

class Graph
{
public:
    int n;
    vector<vector<int>> edge;
    vector<vector<int>> prec;
    vector<vector<int>> doms;
    vector<vector<int>> DF;
    vector<set<int>> iterDF;
    vector<bool> reach; //if dfs can reach the block
    vector<int> iDom;   //immediate dominator
    DomTree* tree;

    Graph(int size) {
        n = size;
        for(int i = 0; i < n; i++) {
            edge.push_back(vector<int>());
            prec.push_back(vector<int>());
            doms.push_back(vector<int>());
            DF.push_back(vector<int>());
            iterDF.push_back(set<int>());
            reach.push_back(true);
            iDom.push_back(-1);
        }
    }
    void add(int u,int v) {
        edge[u].push_back(v);
        prec[v].push_back(u);
    }
    bool dom(int u, int v) {
        for (int i = 0; i < doms[u].size(); i++) {
            if (doms[u][i] == v) {
                return true;
            }
        }
        return false;
    }
    void getDom(int);
    void dfs(int, vector<int>&);
    void getDF();
    void getIterDF();
    void iter(int, set<int>&, vector<int>&);
    void getIdom();
    void buildDomTree();
    void checkUnreach();
    void init();
};

#endif //COMPILER_GRAPH_H
