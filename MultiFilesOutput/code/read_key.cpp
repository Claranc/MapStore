#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
using namespace std;

#define SIZE 50

vector<string> file_key_min;

class FindFirstKey {
public:
    void find_key() {
        int file_num = 1;
        while(1) {
            string linestr;
            string file_key = "ps_00" + to_string(file_num) + "_key.bin";
            ifstream fin(file_key, ios::binary | ios::in);
            if(!fin.is_open()) {
                break;
            }
            getline(fin,linestr); 
            int a = linestr.find_first_of(' ');
            string key = linestr.substr(0,a);
            if(key.size() != 0)file_key_min.push_back(key);
            
            file_num++;
            fin.close();
        }
    }
};


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
        int file_num;
        string key_file_name = "ps_001_key.bin";
        string value_file_name = "ps_001_value.bin";
        for(int i = 0; i < file_key_min.size(); i++) {
            if(input_key < file_key_min[0])
                return "It is not in the files";
            if(input_key < file_key_min[i]) {
                file_num = i;
                key_file_name = "ps_00" + to_string(file_num) + "_key.bin";
                value_file_name = "ps_00" + to_string(file_num) + "_value.bin";
                break;
            }
            if(i == file_key_min.size()-1 && input_key >= file_key_min[i]) {
                file_num = i + 1;
                key_file_name = "ps_00" + to_string(file_num) + "_key.bin";
                value_file_name = "ps_00" + to_string(file_num) + "_value.bin";
                break;
            }
        }
    	ifstream fin_key(key_file_name,ios::binary);
    	ifstream fin_value(value_file_name);
	    fin_key.seekg(0,ios::end);
	    n = fin_key.tellg();
	    char *key_buff = new char[n];
	    fin_key.seekg(0,ios::beg);
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
        return "It is not in the files";
	}
};
   

int main(int argc, char* argv[]) {
    FindFirstKey A;
    A.find_key();   
    string input_key;
    ReadKey R;
    cout<< R.read_key(argv[1]) << endl;
    /*while(1) {
        getline(cin,input_key);
        if(input_key.size() == 0) break;
        cout << R.read_key(input_key) << endl;
        R.value.clear();
    }*/
    return 0;
}
    
