#include "Header.h"


int choose(string a){
	string b;
	for (string::iterator it = a.begin(); it != a.end(); it++){
		if (*it >= 'a' && *it <= 'z') b.push_back(*it);
		if (*it == 9 || *it == ' ' || it == a.end()-1){
			if (b.size() == 0);
			else if (b == "create") return 1;
			else if (b == "update") return 2;
			else if (b == "delete") return 3;
			else if (b == "get") return 4;
			else if (b == "help") return 5;
			else if (b == "show") return 6;
			else return 0;
		}
	}
	return 0;
}

int get_values(string a, vector<string>& b,int num){
	bool flag = 0;
	string c;
	int count = 0;

	for (string::iterator it4 = a.begin(); it4 != a.end(); it4++){
		if (*it4 == '"')count++;
	}

	if (count % 2 != 0) return 1;

	for (string::iterator it = a.begin(); it != a.end(); it++){
		if ((*it >= 'a' && *it <= 'z') || (*it >= '0' && *it <= '9') ||
			(*it >= 'A' && *it <= 'Z') || *it == '.' || *it == '-' || *it == '/'){
			if (*it == '.'){
				if (b.size() == num){
					bool flag2 = 0;
					for (int j = c[0]; j < c.size(); j++){
						if (c[j] >= '0' && c[j] <= '9');
						else return 1;
					}
					if (*(it + 1) >= '0' && *(it + 1) <= '9')c.push_back(*it);
				}
				else return 1;
			}
			else c.push_back(*it);
		}
		else if (*it == ' ' || *it == 9){
			if (c.size() == 0);
			else b.push_back(c);
			c.clear();
		}
		else if (b.size() == num && *it == '"' &&
			c.size() == 0 && a[a.find_last_not_of(' ')] == '"'){
			if (count == 2 && *(it + 1) == '"') return 1;       
			for (string::iterator it2 = it + 1; it2 != a.end(); it2++){
				if (*it2 != '"'){
					if ((*it2 == ' ' || *it2 == 9) && (*(it2 - 1) == ' ' || *(it2-1) == 9));
					else c.push_back(*it2);
				}
				else{
					int flag = 0;  
					for (string::iterator it3 = it2 + 1; it3 != a.end(); it3++){
						if (*it3 == '"') flag = 1;
					}
					if (flag == 0){
						string::iterator it6 = c.begin();
						if (*it6 == ' ' || *it6 == 9) c.erase(it6);
						b.push_back(c);
						return 0;
					}
					else c.push_back(*it2);
				}

			}
		}
		else return 1;
	}
	if (c.size() != 0)b.push_back(c);
	return 0;
}
