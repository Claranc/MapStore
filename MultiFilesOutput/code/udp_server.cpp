#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <unistd.h>
#include <cstdlib>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <signal.h>   
#include <string.h>  
#include <fcntl.h>  
#include <sys/stat.h>  
#include <time.h>  
using namespace std;

#define SIZE 10
string file_prefix = "/home/xinjie/MapStore/MultiFilesOutput/data/ps_00";

//daemon
static bool flag = true;  
void create_daemon();  
void handler(int);  

static vector<string> file_key_min;
static int num_count = 0;
map<string,string> lastfile;

//文件内容排序
class OrderlyFiles {
    public:
        int num;
        string key;
        string value;
        map<string,string> mymap;
        int offset,value_length;
        static string file_key_name;
        static string file_value_name;
    public:
        OrderlyFiles(int l) {
            num = l;
        }
        
        //将第i个文件的map传入内存中
        void ExtractDataToMap(int i) {
            file_key_name = file_prefix + to_string(i) + "_key.bin";
            file_value_name = file_prefix + to_string(i) + "_value.bin";
            ifstream fin_key(file_key_name, ios::in);
            ifstream fin_value(file_value_name, ios::in);
            fin_key.seekg(0, ios::end);
            int n = fin_key.tellg();
            char *kbuff = new char[n];
            fin_key.seekg(0, ios::beg);
            fin_key.read((char*)kbuff,n);
            int j = 0;
            while(j < n) {
                if(kbuff[j] != ' ') key.push_back(kbuff[j++]);
                else {
                    offset = *((int*)&kbuff[++j]);
                    j += 4;
                    value_length = *((int*)&kbuff[j]);
                    fin_value.seekg(offset, ios::beg);
                    char *vbuff = new char[value_length];
                    fin_value.read(vbuff,value_length);
                    for(int k = 0; k < value_length; k++) value.push_back(vbuff[k]);
                    if(key.size() != 0 && value.size() != 0) { 
                        mymap.insert(pair<string,string>(key,value));
                    }
                    key.clear();
                    value.clear();
                    delete[] vbuff;
                    j+=4;
                }
            }
            fin_key.close();
            fin_value.close();
            delete[] kbuff;
        }
        
        //将内存中的一部分map输出到文件x中
        int OutputToFiles(int x) {
        	cout << "size" << lastfile.size() << endl;
        	if(mymap.size() == 0) return 0;
            int sum = 0;
            int offset = 0;
            int value_length;
            file_key_name = file_prefix + to_string(x) + "_key.bin";
            file_value_name = file_prefix + to_string(x) + "_value.bin";
            ofstream fout_key_resort(file_key_name, ios::out);
            ofstream fout_value_resort(file_value_name, ios::out);
            map<string,string>::iterator it = mymap.begin();
            if(file_key_min.size() < x ) {
                file_key_min.push_back(it->first);
            }
            else file_key_min[x-1] = it->first;
            for(; it != mymap.end(); it++) {
                fout_key_resort << it->first << " ";
                value_length = it->second.size();
                fout_key_resort.write((char*)&offset,sizeof(int));
                fout_key_resort.write((char*)&value_length,sizeof(int));
                fout_value_resort << it->second << endl;
                offset+=(value_length+sizeof(char));
                sum++;
                if(sum == SIZE) {
                    it++;
                    break;
                }
            }
            fout_key_resort.close();
            fout_value_resort.close();
            mymap.erase(mymap.begin(), it);
            return sum;
        }
        
        void SortFiles() {
            ExtractDataToMap(num);
            for(int p = 1; p <= num - 1; p++) {
                ExtractDataToMap(p);
            	OutputToFiles(p);
            }
            OutputToFiles(num);
        }
};      
string OrderlyFiles::file_key_name = file_prefix + "1_key.bin";
string OrderlyFiles::file_value_name = file_prefix + "1_value.bin";



//输出到多个文件的类
class MultifulFilesWriting {
    public:
        string linestr;
        string key;
        string value;
        int value_length;
        int file_num = 1;
        static string key_file_name;
        static string value_file_name;
        int offset = 0;
        int file_count;

    public:
    	int file_exist_num_write() {
		    int file_num_count = 1;
		    while(1) {
		        string file_key = file_prefix + to_string(file_num_count) + "_key.bin";
		        ifstream fin(file_key, ios::binary | ios::in);
		        if(!fin.is_open()) {
		            break;
		        } 
		        file_num_count++;
		        fin.close();
		    }
		    file_num_count--;
		    return file_num_count;
		}


    	void check_file() {
    		file_count = file_exist_num_write();
    		cout << "file_count " << file_count << endl;
            if(file_count) {
            	ifstream fin_value(file_prefix + to_string(file_count)+"_value.bin", ios::in);
            	ifstream fin_key(file_prefix + to_string(file_count)+"_key.bin", ios::in);
			    string key;
			    vector<int> value_offset;
			    vector<int> length;
			    string value;
			    int offset_in = 0;
			    string line;
			    int value_length = 0;
			    char c;
			    int n;
			    fin_key.seekg(0,ios::end);
			    n = fin_key.tellg();
			    char *key_buff = new char[n];
			    fin_key.seekg(0,ios::beg);
			    fin_key.read((char*)key_buff,n);
			    for(int k = 0; k < n; k++) {
			        if(key_buff[k] != ' ') {
			            key.push_back(key_buff[k]);
			        }
			        else {
			        	offset_in = *((int*)&key_buff[++k]);
		                k += 4;
		                value_length = *((int*)&key_buff[k]);
		                k += 3;
			        	fin_value.seekg(offset_in,ios::beg);
					    char *kbuff = new char[value_length];
					    fin_value.read(kbuff,value_length);
					    for(int i = 0; i < value_length; i++) value.push_back(kbuff[i]);
					    delete[] kbuff;
						cout << "key = " << key << endl;
					    cout << "value = " << value << endl;
						if(key.size() > 0)lastfile.insert(pair<string,string>(key,value));
						key.clear();
						value.clear();
			        }
			    }
			    delete[] key_buff;
            	fin_value.seekg(0,ios::end);
            	offset = fin_value.tellg();
            	if(file_count == 0) {
	            	string key_file_name = file_prefix + "1_key.bin";
	        		string value_file_name = file_prefix + "1_value.bin";
	            }
	            else {
	            	if(lastfile.size() == SIZE) {
	            		offset = 0;
	            		key_file_name = file_prefix + to_string(file_count+1) + "_key.bin";
	            		value_file_name = file_prefix + to_string(file_count+1) + "_value.bin";
	            	}
	            	else {
		            	key_file_name = file_prefix + to_string(file_count) + "_key.bin";
		            	value_file_name = file_prefix + to_string(file_count) + "_value.bin";
		            }
	        	}
            }           
    	}


        void WriteToFiles(string key, string value) {
            if(lastfile.size() == SIZE) lastfile.clear();          
            ofstream fout_key(key_file_name, ios::out | ios::app);
            ofstream fout_value(value_file_name, ios::out | ios::app);
            value_length = value.size();
            //输出到key文件中
            fout_key << key << " ";
            fout_key.write((char*)&offset, sizeof(int));
            fout_key.write((char*)&value_length, sizeof(int));
            //输出到value文件中
            fout_value << value << "\n";
            offset += (value_length + sizeof(char));
            map<string,string>::iterator it = lastfile.find(key);
            if(it == lastfile.end())lastfile.insert(pair<string,string>(key,value));
            else lastfile.at(key) = value;
            key.clear();
            value.clear();
            if(lastfile.size() == SIZE ) {
            	int n = file_exist_num_write();
                fout_key.close();
                fout_value.close();
                cout << "success" << endl;
                SortFiles(n);
                key_file_name = file_prefix + to_string(n+1) + "_key.bin";
                value_file_name = file_prefix + to_string(n+1) + "_value.bin";
                offset = 0;
                lastfile.clear();
            }
            fout_key.close();
            fout_value.close();
        }

        void SortFiles(int x) {
            OrderlyFiles* rec = new OrderlyFiles(x);
            rec->SortFiles();
            delete rec;
        }

        int choose(string a){
			string b;
			for (string::iterator it = a.begin(); it != a.end(); it++){
				if (*it >= 'a' && *it <= 'z') b.push_back(*it);
				if (*it == 9 || *it == ' ' || it == a.end()-1){
					if (b.size() == 0);
					else if ("create" == b) return 1;
                    else if ("get" == b) return 2;
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
    };

string MultifulFilesWriting::key_file_name = file_prefix + "1_key.bin";
string MultifulFilesWriting::value_file_name = file_prefix + "1_value.bin";


class ReadKey {
	public:
	    
	    int read_count;
	public: 
		int file_exist_num_read() {
	    	int read_count = 1;
		    while(1) {
		        string linestr;
		        string file_key = file_prefix + to_string(read_count) + "_key.bin";
		        ifstream fin(file_key, ios::binary | ios::in);
		        if(!fin.is_open()) {
		            break;
		        } 
		        getline(fin,linestr); 
		        int a = linestr.find_first_of(' ');
		        string key = linestr.substr(0,a);
		        file_key_min.push_back(key);
		        read_count++;
		        fin.close();
		    }
		    if(file_key_min.size() > 0)file_key_min.pop_back();
		    --read_count;
		    return read_count;
		}

		int file_exist_num_sum() {
			    int file_num_count = 1;
			    while(1) {
			        string file_key = file_prefix + to_string(file_num_count) + "_key.bin";
			        ifstream fin(file_key, ios::binary | ios::in);
			        if(!fin.is_open()) {
			            break;
			        } 
			        file_num_count++;
			        fin.close();
			    }
			    file_num_count--;
			    return file_num_count;
			}

	    string read_key(string input_key) {
	    	string key;
		    vector<int> value_offset;
		    vector<int> length;
		    string value;
		    int offset = 0;
		    string line;
		    int value_length = 0;
		    char c;
		    int m,n;
	    	cout << lastfile.size() << "size" << endl;
	    	int file_num_count = file_exist_num_sum();
	    	map<string, string>::iterator it = lastfile.find(input_key);
	    	if(it != lastfile.end()) return it->second;
	    	else {
		    	cout << "file_num_count = " << file_num_count << endl;
		        int file_num_key;
		        string key_file_name;
		        string value_file_name;
		        cout << "f_size" << file_key_min.size() << endl;
		        for(int i = 0; i < file_key_min.size(); i++) {
		            if(input_key < file_key_min[0])
		                return "It is not in the files";
		            if(input_key < file_key_min[i]) {
		                file_num_key = i;
		                key_file_name = file_prefix + to_string(file_num_key) + "_key.bin";
		                value_file_name = file_prefix + to_string(file_num_key) + "_value.bin";
		                break;
		            }
		            if(i == file_key_min.size()-1 && input_key >= file_key_min[i]) {
		                file_num_key = i + 1;
		                key_file_name = file_prefix + to_string(file_num_key) + "_key.bin";
		                value_file_name = file_prefix + to_string(file_num_key) + "_value.bin";
		                break;
		            }
		        }
		        cout << "file_num = " << file_num_key << endl;	        
	        	key_file_name = file_prefix + to_string(file_num_key) + "_key.bin";
	            value_file_name = file_prefix + to_string(file_num_key) + "_value.bin";
		    	ifstream fin_key(key_file_name,ios::binary);
		    	ifstream fin_value(value_file_name);
		    	fin_key.seekg(0,ios::end);
		    	n = fin_key.tellg();
			    char *key_buff = new char[n];
			    memset(key_buff,0,sizeof(key_buff));
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
			        memset(kbuff,0,sizeof(kbuff));
			        fin_value.read(kbuff,value_length);
			        for(int i = 0; i < value_length; i++) value.push_back(kbuff[i]);
			        delete[] kbuff;
			    	string x = value;
			    	value.clear();
			        return x;
		        }
		        delete[] key_buff;
		        fin_key.close();
		        fin_value.close();
	    	
		    }
	        return "It is not in the files";
		}
	};
   
void server_main() {
    cout << "This is UDP server\n";
    int sock;
    struct sockaddr_in server_socket;
    struct sockaddr_in client_socket;
    MultifulFilesWriting A;
    ReadKey B;
    B.file_exist_num_read();
    A.check_file();
	vector<string> vals;
	string x;
	int flag1;
	int flag2;
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        exit(0);

    int one=1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one));
    memset(&server_socket, 0, sizeof(server_socket));
    server_socket.sin_family = AF_INET;
    server_socket.sin_addr.s_addr = htonl(INADDR_ANY);
    server_socket.sin_port = htons(8643);

    if (bind(sock, (struct sockaddr *) &server_socket, sizeof(server_socket)) < 0)
        exit(0);
    int received;
    while (1) {
    	int flag_error = 0;
    	char *buffer = new char[255];
        socklen_t client_len = sizeof(client_socket);
        if ((received = recvfrom(sock, buffer, 255, 0, (struct sockaddr *) &client_socket, &client_len)) < 0) 
            exit(0);
        buffer[received] = '\0';
        cout << "Client connected: "<< inet_ntoa(client_socket.sin_addr)<<"\t"<<ntohs(client_socket.sin_port)<<endl;
        cout << buffer<<endl<<endl;

	    if(buffer == "quit") {
	    	exit(0);
	    }
	    flag1 = A.choose(buffer);
	    if(flag1 == 1) {
	    	flag2 = A.get_values(buffer,vals,2);
	    	if(flag2 == 0 && vals.size() == 3) {
	    		A.WriteToFiles(vals[1],vals[2]);
	    		string feedback;
	    		feedback = "success";
	    		char *rev = new char[feedback.size()+1];
	    		rev[feedback.size()] = '\0';
	    		for(int i = 0; i < feedback.size(); i++) rev[i] = feedback[i];
	    		feedback.clear();
	    		if (sendto(sock, rev,strlen(rev), 0, (struct sockaddr *) &client_socket, sizeof(client_socket)) < 0) {
			        cout<<"here";
			        	exit(0);
			    }
			    delete[] rev;
	    	}
	    	else flag_error = 1;
	    	vals.clear();
	    }
	    else if(flag1 == 2) {
	    	flag2 = A.get_values(buffer,vals,1);
	    	if(flag2 == 0 && vals.size() == 2) {
	    		string value;
	    		value = B.read_key(vals[1]);
	    		vals.clear();
	    		string feedback;
	    		feedback = value;
	    		char *rev = new char[feedback.size()+1];
	    		for(int i = 0; i < feedback.size(); i++) rev[i] = feedback[i];
	    		rev[feedback.size()] = '\0';
	    		feedback.clear();
	    		cout << rev << endl;
	    		if (sendto(sock, rev,strlen(rev), 0, (struct sockaddr *) &client_socket, sizeof(client_socket)) < 0) {
			        cout<<"here";
			        	exit(0);
			    }
	    		delete[] rev;
	    	}
	    	else flag_error = 1;
	    	vals.clear();
	    }
    	else flag_error = 1;
	    if(flag_error == 1) {
	    	string feedback;
    		feedback = "your input is not correct";
    		char *rev = new char[feedback.size()+1];
    		for(int i = 0; i < feedback.size(); i++) rev[i] = feedback[i];
    		rev[feedback.size()] = '\0';
    		feedback.clear();
    		cout << rev << endl;
    		if (sendto(sock, rev,strlen(rev), 0, (struct sockaddr *) &client_socket, sizeof(client_socket)) < 0) {
		        cout<<"here";
		        	exit(0);
		    }
    		delete[] rev;
	    }
	    delete[] buffer;
    }
}


void handler(int sig)  {  
    printf("I got a signal %d\nI'm quitting.\n", sig);  
    flag = false;  
}

//create daemon  
void create_daemon() {  
    pid_t pid;  
    pid = fork();  
      
    if(pid == -1) {  
        printf("fork error\n");  
        exit(1);  
    }  
    else if(pid) {  
        exit(0);  
    }  
  
    if(-1 == setsid()) {  
        printf("setsid error\n");  
        exit(1);  
    }  
  
    pid = fork();  
    if(pid == -1) {  
        printf("fork error\n");  
        exit(1);  
    }  
    else if(pid) {  
        exit(0);  
    }  
  
    chdir("/home/xinjie/MapStore/MultiFilesOutput/code");  
    int i;  
    for(i = 0; i < 3; ++i) {  
        close(i);  
    }  
    umask(0);  
    return;  
}  



int main(int argc, char* argv[]) {
    time_t t;  
    int fd;  
    create_daemon();  
    struct sigaction act;  
    act.sa_handler = handler;  
    sigemptyset(&act.sa_mask);  
    act.sa_flags = 0;  
    if(sigaction(SIGQUIT, &act, NULL)) {  
        printf("sigaction error.\n");  
        exit(0);  
    }  
    server_main(); 
    return 0;
}

    
