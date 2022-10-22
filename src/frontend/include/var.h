#ifndef __VAR__
#define __VAR__

#include <string>
using namespace std;

class Var
{
public:
    string id;
    string tp;
    int dim;
    Var(string i, string t, int d): id(i), tp(t), dim(d) {}
};

#endif