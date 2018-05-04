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

#define SIZE 2
string file_prefix = "/home/xinjie/MapStore/MultiFilesOutput/data/ps_00";

//daemon
static bool flag = true;  
void create_daemon();  
void handler(int);  

map<string,string> lastfile;


//输出到多个文件的类
class MultifulFilesWriting {
    private:
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
                key_file_name = file_prefix + to_string(n+1) + "_key.bin";
                value_file_name = file_prefix + to_string(n+1) + "_value.bin";
                offset = 0;
                lastfile.clear();
            }
            fout_key.close();
            fout_value.close();
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
	private:
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
	        int file_num = 1;
	        map<string, string>::iterator it = lastfile.find(input_key);
	    	if(it != lastfile.end()) return it->second;
	    	else {
		        string key_file_name = file_prefix + "1_key.bin";
		        string value_file_name = file_prefix + "1_value.bin";
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
					        string x = value;
					    	value.clear();
					        return x;
			        	}
			        	key.clear();
			        	value.clear();
			        	file_num++;
			        	delete[] key_buff;
			        	fin_key.close();
			        	fin_value.close();
			        	key_file_name = file_prefix + to_string(file_num) + "_key.bin";
			        	value_file_name = file_prefix + to_string(file_num) + "_value.bin";
		        	}
		        	else {
		        		break;
		        	}
		        	
		        }
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
    //B.file_exist_num_read();
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
  
    chdir("/home/xinjie/MapStore/MultiFilesOutput/withoutsort");  
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

    
