#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
using namespace std;


int choose(string);
int run(map<string, string>&, string);
int get_values(string, vector<string>&, int);
int create_key(map<string, string>&, string, string);
int update_key(map<string, string>&, string, string);
int delete_key(map<string, string>&, string);
string get_key(map<string, string>&, string);
int show_keys(map<string, string>&);
