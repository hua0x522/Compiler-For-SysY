#include "node.h"

void Node::add(Node* node) {
    this->sons.push_back(node);
}

Node::Node(string type, Node* parent) {
    // printf("%s\n", type.c_str());
    this->type = type;
    this->parent = parent;
    if (parent != NULL) {
       parent->add(this); 
    }
}