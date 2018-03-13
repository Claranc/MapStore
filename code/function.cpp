#include "Header.h"

int create_key(map<string,string>& map1,string a, string b)
{
	map<string, string>::iterator it = map1.find(a);
	if (it == map1.end())
	{
		map1.insert(pair<string, string>(a, b));
		cout << "Success" << endl;
		return 0;
	}
	else 
		{cout << "It does exist already" << endl;
		return 1;}
}

int update_key(map<string, string>& map1, string a, string b)
{
	map<string, string>::iterator it = map1.find(a);
	if (it != map1.end())
	{
		map1.at(a) = b;
		cout << "Success" << endl;
	}
	else cout << "It does not exist" << endl;
	return 0;
}

int delete_key(map<string, string>& map1, string a)
{
	map<string, string>::iterator it = map1.find(a);
	if (it != map1.end())
	{
		map1.erase(a);
		cout << "Success" << endl;
	}
	else cout << "It does not exist" << endl;
	return 0;
}

string get_key(map<string, string>& map1, string a)
{
	map<string, string>::iterator it = map1.find(a);
	if (it == map1.end())
	{
		return "It does not exist";
	}
	else
	{
		return map1.find(a)->second;
	}
}

int show_keys(map<string, string>& map1)
{
	for(map<string,string>::iterator it = map1.begin();it != map1.end(); it++)
		cout<<it->first<<endl;
}
