#ifndef __LEXER__
#define __LEXER__

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include "token.h"
using namespace std; 
vector<Token> lexer();
bool isDigit(char);
#endif