#ifndef __NODE__
#define __NODE__

#include <string>
#include <vector>
#include <cstdio>
#include "token.h"
using namespace std;

class Node
{
public:
    Token token;
    Node* parent;
    vector<Node*> sons;
    Node(Token,Node*);
    Node(string,Node*);
    void add(Node*);
    void del(Node*); //delete
};

#endif