#ifndef __TOKEN__
#define __TOKEN__

#include <string>
#include <iostream>
using namespace std;

class Token 
{
public:
    string val;
    string tp;
    int line;
    Token(string v,string t,int l): val(v), tp(t), line(l) {}
    Token() {}
};

#endif