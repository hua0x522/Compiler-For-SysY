//
// Created by wangxuezhu on 2022/11/27.
//
#include "graph.h"
#include <cstdio>

void Graph::getDom(int x) {
    if (x == 0) {
        for (int i = 0; i < n; i++) {
            doms[x].push_back(i);
        }
        return;
    }
    doms[x].push_back(x);
    vector<int> vis(n,0);
    vis[x] = 1;
    dfs(0, vis);
    for (int i = 0; i < n; i++) {
        if (vis[i] == 0 && reach[i]) {
            doms[x].push_back(i);
        }
    }
}

void Graph::dfs(int u, vector<int>& vis) {
    vis[u] = 1;
    for (int i = 0; i < edge[u].size(); i++) {
        int v = edge[u][i];
        if (vis[v] == 0) {
            dfs(v, vis);
        }
    }
}

void Graph::getDF() {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < doms[i].size(); j++) {
            int u = doms[i][j];
            for (int k = 0; k < edge[u].size(); k++) {
                int v = edge[u][k];
                if (i == v || !dom(i, v)) {
                    // i dominate u(precessor of v), and i don't strictly dominate v
                    DF[i].push_back(v);
                }
            }
        }
    }
}

void Graph::iter(int u, set<int>& iDF, vector<int>& vis) {
    vis[u] = 1;
    for (int i = 0; i < DF[u].size(); i++) {
        iDF.insert(DF[u][i]);
    }
    for (int i = 0; i < DF[u].size(); i++) {
        if (!vis[DF[u][i]]) iter(DF[u][i], iDF, vis);
    }
}

void Graph::getIterDF() {
    for (int i = 0; i < n; i++) {
        if (reach[i]) {
            vector<int> vis(n, 0);
            iter(i, iterDF[i], vis);
        }
    }
}

void Graph::getIdom() {
    //idom has the minimum size during doms, size is the num of BB it dominates
    vector<int> dsize(n, 1e9);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < doms[i].size(); j++) {
            int u = doms[i][j];
            if (i != u && doms[i].size() < dsize[u]) {
                dsize[u] = doms[i].size();
                iDom[u] = i;
            }
        }
    }
}

void Graph::checkUnreach() {
    vector<int> vis(n, 0);
    dfs(0, vis);
    for (int i = 0; i < n; i++) {
        reach[i] = vis[i];
    }
}

void Graph::buildDomTree() {
    tree = new DomTree(n);
    for (int i = 1; i < n; i++) {
        if (reach[i]) {
            tree->addSon(iDom[i], i);
        }
    }
}

void Graph::init() {
    checkUnreach();
    for (int i = 0; i < n; i++) {
        if (reach[i]) {
            getDom(i);
        }
    }
    getDF();
    getIterDF();
    getIdom();
    buildDomTree();
}