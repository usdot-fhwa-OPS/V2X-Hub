

#include <string>
#include <vector>
#include <memory>
#include <numeric>
#include <iostream>

using namespace std; 

string hex2bin(char c);

char bin2hex(string b);


char bin2base64(string s);


string dec2bin(int a);


string base642bin(char b64);



void  hex2base64(string hexstr, string& base64str);




void base642hex(string base64str, string& hexstr);

