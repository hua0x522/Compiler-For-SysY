#include "node.h"

void Node::add(Node* node) {
    this->sons.push_back(node);
    node->parent = this;
}

void Node::del(Node* node) {
    for (auto iter = this->sons.begin(); iter != this->sons.end(); iter++) {
        if (*iter == node) {
            this->sons.erase(iter);
            break;
        }
    }
    node->parent = NULL;
}

Node::Node(string type, Node* parent) {
    Token t(type, type, 0);
    this->token = t;
    if (parent != NULL) {
       parent->add(this); 
    }
}

Node::Node(Token token, Node* parent) {
    this->token = token;
    if (parent != NULL) {
        parent->add(this);
    }
}
