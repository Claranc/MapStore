#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
using namespace std;

class ReadKey {
public:
    string key;
    vector<int> value_offset;
    vector<int> length;
    string value;
    int offset = 0;
    string line;
    int value_length = 0;
    char c;
    int m,n;
public:
    string read_key(string input_key) {
        int file_num = 1;
        string key_file_name = "ps_001_key.bin";
        string value_file_name = "ps_001_value.bin";
        while(1) {
        	ifstream fin_key(key_file_name,ios::binary);
        	ifstream fin_value(value_file_name);
        	if(fin_key.is_open()) {
        		fin_key.seekg(0,ios::end);
        		n = fin_key.tellg();
                fin_key.seekg(0,ios::beg);
	    		char *key_buff = new char[n];
	    		fin_key.read((char*)key_buff,n);
	    		int flag_exist = 0;
			    for(int k = 0; k < n; k++) {
			        if(key_buff[k] != ' ') {
			            key.push_back(key_buff[k]);
			        }
			        else {
			            if(key == input_key) {
			                offset = *((int*)&key_buff[++k]);
			                k += 4;
			                value_length = *((int*)&key_buff[k]);
			                k += 3;
			                key.clear();
		                    flag_exist = 1;
			                continue;
			            }
			            else {
			                k += 8;
			                key.clear();
			                continue;
			            }
			        }
			    }
		        if(flag_exist == 1) {               
                    fin_value.seekg(offset,ios::beg);
			        char *kbuff = new char[value_length];
			        fin_value.read(kbuff,value_length);
			        for(int i = 0; i < value_length; i++) value.push_back(kbuff[i]);
			        return value;
	        	}
	        	key.clear();
	        	value.clear();
	        	file_num++;
	        	delete[] key_buff;
	        	fin_key.close();
	        	fin_value.close();
	        	key_file_name = "ps_00" + to_string(file_num) + "_key.bin";
	        	value_file_name = "ps_00" + to_string(file_num) + "_value.bin";
        	}
        	else {
        		break;
        	}
        	
        }
        return "It is not in the files";
	}
};
   

int main(int argc, char* argv[]) {
    ReadKey R;
    cout<< R.read_key(argv[1]) << endl;
    return 0;
}
    
