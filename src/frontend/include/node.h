#ifndef __NODE__
#define __NODE__

#include <string>
#include <vector>
#include <cstdio>
using namespace std;

class Node
{
public:
    string type;
    Node* parent;
    vector<Node*> sons;
    Node(string,Node*);
    void add(Node*);
};

#endif