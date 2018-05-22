#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <vector>
#include <thread>
using namespace std;


struct server_info {
    string ip_addr;
    int udp_port;
};

class Ring {
private:
    vector<struct server_info> server_list;
    vector<int> server_state;
public:
    void CheckAlive(int sock, int n);
    void SplitString(const string& s, vector<string>& v, const string& c);
    void ProcessData(string x, sockaddr_in client_socket, int received,int sock);
    void server_main(int udp_port, int argc, const char *address, const char *port, int num);
};




<<<<<<< HEAD
void Ring::CheckAlive(int sock, int n) {
    while(1) {
        int i = 0;
        int len =  server_list.size();
        while(i < 4) {
            if(server_list.size() > 1 ) {
                for(int j = 1; j < server_list.size(); j++) {
                    struct sockaddr_in *q = new sockaddr_in;
                    memset(q,0,sizeof(sockaddr_in));
                    const char *addr = server_list[j].ip_addr.c_str();
                    q->sin_addr.s_addr = inet_addr(addr);
                    q->sin_port = htons(server_list[j].udp_port);
                    cout << "I send to " << server_list[j].udp_port << endl;
                    char messages[] = "I am still alive\0";
                    if (sendto(sock, messages,strlen(messages), 0, (struct sockaddr *) q, sizeof(sockaddr_in)) < 0) {
                        cout<<"I am dead";
                        exit(0);
                    }
                    delete q;
                    q = NULL;
                    if(j == n)
                        break;
                    }
            }
            i++;
            sleep(1);
        }
        if(server_state.size() > 1) {
            vector<int> server_down;
            if(len != server_state.size()) continue;
            for(int i = len-1; i < len; i++) {
                if(server_state[i] == 0) {
                    cout << server_list[i].ip_addr << " " << server_list[i].udp_port << " is dead" << endl;
                    server_down.push_back(i);
                    struct sockaddr_in *q = new sockaddr_in;
                    memset(q,0,sizeof(sockaddr_in));
                    const char *addr = server_list[1].ip_addr.c_str();
                    q->sin_addr.s_addr = inet_addr(addr);
                    q->sin_port = htons(server_list[1].udp_port);
                    string x_send = "dead" +  server_list[i].ip_addr + "_" + to_string(server_list[i].udp_port);
                    const char *messages = x_send.c_str();
                    if (sendto(sock, messages,strlen(messages), 0, (struct sockaddr *) q, sizeof(sockaddr_in)) < 0) {
                        cout<<"I am dead";
                        exit(0);
                    }
                    delete q;
                    q = NULL;
                }
=======
void Ring::CheckAlive(int sock) {
	while(1) {
		int i = 0;
		int len =  server_list.size();
		while(i < 4) {
    		if(server_list.size() > 1 ) {
				for(int j = 1; j < 2; j++) {
					struct sockaddr_in *q = new sockaddr_in;
					memset(q,0,sizeof(sockaddr_in));
					const char *addr = server_list[j].ip_addr.c_str();
					q->sin_addr.s_addr = inet_addr(addr);
					q->sin_port = htons(server_list[j].udp_port);
					//cout << "I send to " << server_list[j].udp_port << endl;
					char messages[] = "I am still alive\0";
					if (sendto(sock, messages,strlen(messages), 0, (struct sockaddr *) q, sizeof(sockaddr_in)) < 0) {
					    cout<<"I am dead";
					    exit(0);
					}
					delete q;
					q = NULL;
					}
        	}
			i++;
			sleep(1);
		}
		if(server_state.size() > 1) {
			vector<int> server_down;
			if(len != server_state.size()) continue;
			for(int i = len-1; i < len; i++) {
				if(server_state[i] == 0) {
					cout << server_list[i].ip_addr << " " << server_list[i].udp_port << " is dead" << endl;
					server_down.push_back(i);
					struct sockaddr_in *q = new sockaddr_in;
					memset(q,0,sizeof(sockaddr_in));
					const char *addr = server_list[1].ip_addr.c_str();
					q->sin_addr.s_addr = inet_addr(addr);
					q->sin_port = htons(server_list[1].udp_port);
					string x_send = "dead" +  server_list[i].ip_addr + "_" + to_string(server_list[i].udp_port);
					const char *messages = x_send.c_str();
					if (sendto(sock, messages,strlen(messages), 0, (struct sockaddr *) q, sizeof(sockaddr_in)) < 0) {
						cout<<"I am dead";
						exit(0);
					}
					delete q;
				    q = NULL;
				}
>>>>>>> bbee44464fe3fa411f3d609d6e7dab2bcfabb2c6
            }
            for (int i = 0; i < server_down.size(); ++i) {
                server_list.erase(server_list.begin()+server_down[i]);
                server_state.erase(server_state.begin()+server_down[i]);
            }
            server_down.clear();
            for(int i = 0; i < server_state.size(); i++) server_state[i] = 0;
        }	       
    }
}

void Ring::SplitString(const string& s, vector<string>& v, const string& c) {
    string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(string::npos != pos2) {
        v.push_back(s.substr(pos1, pos2-pos1));        
        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if(pos1 != s.length())
        v.push_back(s.substr(pos1));
}
 

void Ring::ProcessData(string x, sockaddr_in client_socket, int received,int sock) {
    if(x == "hello") {
        if(server_list.size() > 1) {
            string send_data = "info";
            int k = server_list.size();
            for(int i = 1; i < k; i++) {
                send_data = send_data + server_list[i].ip_addr + " " + to_string(server_list[i].udp_port) + "~";
            }
            const char *send_mess = send_data.c_str();
            if (sendto(sock, send_mess,strlen(send_mess), 0, (struct sockaddr *) &client_socket, sizeof(sockaddr_in)) < 0) {
                cout<<"I am dead";
                exit(0);
            }
            struct sockaddr_in *q = new sockaddr_in;
            memset(q,0,sizeof(sockaddr_in));
            const char *addr = server_list[1].ip_addr.c_str();
            q->sin_addr.s_addr = inet_addr(addr);
            q->sin_port = htons(server_list[1].udp_port);
            string ip_address = inet_ntoa(client_socket.sin_addr);
            string add_message = "modify" +  ip_address + "_" + to_string(ntohs(client_socket.sin_port));
            //cout << add_message << endl;
            const char *messages = add_message.c_str();
            //char messages[] = "I am still alive\0";
            if (sendto(sock, messages,strlen(messages), 0, (struct sockaddr *) q, sizeof(sockaddr_in)) < 0) {
                cout<<"I am dead";
                exit(0);
            }
            delete q;
            q = NULL;
        } 
        struct server_info *r = new server_info;
        memset(r,0,sizeof(server_info));
        r->ip_addr = inet_ntoa(client_socket.sin_addr);
        r->udp_port = ntohs(client_socket.sin_port);
        cout << r->ip_addr << " " << r->udp_port << " is online" << endl;
        server_list.push_back(*r);
        delete r;
        r = NULL;
        server_state.push_back(0);      
    }
    else if(x == "I am still alive") {
        int flag = 0;
        for(int i = 1; i < server_list.size(); i++) {
            if(ntohs(client_socket.sin_port) == server_list[i].udp_port) {
                if(inet_ntoa(client_socket.sin_addr) == server_list[i].ip_addr) {
                    flag = 1;
                    server_state[i]++;
                }
            }
        }
        if(flag == 0) {
            struct server_info *t = new server_info;
            memset(t, 0, sizeof(server_info));
            t->ip_addr = inet_ntoa(client_socket.sin_addr);
            t->udp_port = ntohs(client_socket.sin_port);
            cout << t->ip_addr << " " << t->udp_port << " is online" << endl;
            server_list.push_back(*t);
            delete t;
            t = NULL;
            server_state.push_back(0);
        }
        //cout << "Connected: "<< inet_ntoa(client_socket.sin_addr)<<"\t"<<ntohs(client_socket.sin_port)<<endl;
    }
    else if(x.substr(0,4) == "info") {
        struct server_info *s = new server_info;
        memset(s, 0, sizeof(server_info));
        s->ip_addr = inet_ntoa(client_socket.sin_addr);
        s->udp_port = ntohs(client_socket.sin_port);
        cout << s->ip_addr << " " << s->udp_port << " is online" << endl;
        server_list.push_back(*s);
        delete s;
        s = NULL;
        server_state.push_back(0);
        string x_data = x.substr(4);
        int count = 0;
        for(int i = 0; i < x_data.size(); i++ ) {
            if(x_data[i] == '~') count++;
        }
        for(int j = 0; j < count; j++) {
            int a = x_data.find_first_of('~');
            string process_data = x_data.substr(0,a);
            int b = process_data.find_first_of(' ');
            struct server_info *s = new server_info;
            memset(s,0,sizeof(server_info));
            s->ip_addr = process_data.substr(0,b);
            const char* tmp = process_data.substr(b+1).c_str();
            s->udp_port = atoi(tmp);
            cout << s->ip_addr << " " << s->udp_port << " is online" << endl;
            server_list.push_back(*s);
            server_state.push_back(0);
            delete s;
            s = NULL;
            if(j < count-1) x_data = x_data.substr(a+1);
        }
    }
    else if(x.substr(0,6) == "modify") {
        if(server_list.size() > 2) {
            string cli_addr = inet_ntoa(client_socket.sin_addr);
            string send_mess = "transfer" + cli_addr + "_" + to_string(ntohs(client_socket.sin_port)) + "_" + x.substr(6);
            struct sockaddr_in *q = new sockaddr_in;
            memset(q,0,sizeof(sockaddr_in));
            const char *addr = server_list[1].ip_addr.c_str();
            q->sin_addr.s_addr = inet_addr(addr);
            q->sin_port = htons(server_list[1].udp_port);
            string ip_address = inet_ntoa(client_socket.sin_addr);
            //string add_message = "modify_" +  ip_address + "_" + to_string(ntohs(client_socket.sin_port));
            //cout << add_message << endl;
            const char *messages = send_mess.c_str();
            //char messages[] = "I am still alive\0";
            if (sendto(sock, messages,strlen(messages), 0, (struct sockaddr *) q, sizeof(sockaddr_in)) < 0) {
                cout<<"I am dead";
                exit(0);
            }
            delete q;
            q = NULL;
        }
        string x_pro = x.substr(6);
        vector<string> a;
        SplitString(x_pro,a,"_");
        struct server_info *s = new server_info;
        s->ip_addr = a[0];
        const char *uport = a[1].c_str();
        s->udp_port = atoi(uport);
        server_list.insert(server_list.end()-1, *s);
        server_state.push_back(0);
        cout << s->ip_addr << " " << s->udp_port << " is online" << endl;
        delete s;
        s = NULL;
    }
    else if(x.substr(0,8) == "transfer") {
        string x_pro = x.substr(8);
        vector<string> a;
        SplitString(x_pro,a,"_");
        struct server_info *s = new server_info;
        s->ip_addr = a[2];
        const char *uport = a[3].c_str();
        s->udp_port = atoi(uport);
        int trans_flag = 0;
        int k = 0;
        for(int j = 1; j < server_list.size(); j++) {
            if(server_list[j].ip_addr == a[0]) {
                const char* u_port = a[1].c_str();
                if(server_list[j].udp_port == atoi(u_port)) {
                    k = j;
                    vector<struct server_info>::iterator it = server_list.begin()+j;
                    server_list.insert(it, *s);
                     server_state.push_back(0);
                     cout << s->ip_addr << " " << s->udp_port << " is online" << endl;
                     trans_flag = 1;
                     break;
                }
            }
        }
        if(k != 1 && trans_flag == 1) {
            struct sockaddr_in *q = new sockaddr_in;
            memset(q,0,sizeof(sockaddr_in));
            const char *addr = server_list[1].ip_addr.c_str();
            q->sin_addr.s_addr = inet_addr(addr);
            q->sin_port = htons(server_list[1].udp_port);
            string ip_address = inet_ntoa(client_socket.sin_addr);
            //string add_message = "modify_" +  ip_address + "_" + to_string(ntohs(client_socket.sin_port));
            //cout << add_message << endl;
            const char *messages = x.c_str();
            //char messages[] = "I am still alive\0";
            if (sendto(sock, messages,strlen(messages), 0, (struct sockaddr *) q, sizeof(sockaddr_in)) < 0) {
                cout<<"I am dead";
                exit(0);
            }
            delete q;
            q = NULL;
        }
        delete s;
        s = NULL;
    }
    else if(x.substr(0,4) == "dead") {
        string x_process = x.substr(4);
        vector<string> a;
        SplitString(x_process,a,"_");
        int k = 0;
        int dead_flag = 0;
        for(int i = 1; i < server_list.size(); i++) {
            if(server_list[i].ip_addr == a[0]) {
                const char* u_port = a[1].c_str();
                if(server_list[i].udp_port == atoi(u_port)) {
                    dead_flag = 1;
                    k = i;
                    cout << server_list[i].ip_addr << " " << server_list[i].udp_port << " is dead" << endl;
                    vector<struct server_info>::iterator it = server_list.begin()+i;
                    server_list.erase(it);
                    vector<int>::iterator it2 = server_state.begin() + i;
                    server_state.erase(it2);
                    break;
                }
            }
        }

        if(dead_flag == 1 && k !=1) {
            struct sockaddr_in *q = new sockaddr_in;
            memset(q,0,sizeof(sockaddr_in));
            const char *addr = server_list[1].ip_addr.c_str();
            q->sin_addr.s_addr = inet_addr(addr);
            q->sin_port = htons(server_list[1].udp_port);
            //string ip_address = inet_ntoa(client_socket.sin_addr);
            //string add_message = "modify_" +  ip_address + "_" + to_string(ntohs(client_socket.sin_port));
            //cout << add_message << endl;
            //string x_send = "dead" +  server_list[i].ip_addr + "_" + to_string(server_list[i].udp_port);
            const char *messages = x.c_str();
            //char messages[] = "I am still alive\0";
            if (sendto(sock, messages,strlen(messages), 0, (struct sockaddr *) q, sizeof(sockaddr_in)) < 0) {
                cout<<"I am dead";
                exit(0);
            }
            delete q;
            q = NULL;
        }
    }
}


void Ring::server_main(int udp_port, int argc, const char *address, const char *port, int num) {
    server_state.push_back(0);
    struct server_info *p = new server_info;
    memset(p,0,sizeof(server_info));
    p->ip_addr = "localhost";
    p->udp_port = udp_port;
    server_list.push_back(*p);
    cout << "This is UDP server,"<< " port: " << p->udp_port << endl;
    delete p;
    p = NULL;
    int sock;
    struct sockaddr_in server_socket;
    struct sockaddr_in client_socket;
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        exit(0);
    int one=1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one));
    memset(&server_socket, 0, sizeof(server_socket));
    server_socket.sin_family = AF_INET;
    server_socket.sin_addr.s_addr = htonl(INADDR_ANY);
    server_socket.sin_port = htons(udp_port); 
    if (bind(sock, (struct sockaddr *) &server_socket, sizeof(server_socket)) < 0)
        exit(0);
    if(argc == 4) {
        struct sockaddr_in gateserver;
        gateserver.sin_family = AF_INET;
        memset(&gateserver,0,sizeof(gateserver));
        gateserver.sin_addr.s_addr = inet_addr(address);
        gateserver.sin_port = htons(atoi(port));
        string x_input = "hello";
        const char *buffer1 = x_input.c_str();
        if (sendto(sock, buffer1,x_input.size(), 0, (struct sockaddr *) &gateserver, sizeof(gateserver)) < 0)
        {
            cout<<"I am dead1" << endl;
            exit(0);
        }
    }  
    char buffer[255];
    int received;
    thread third(&Ring::CheckAlive, this, sock, num);
    third.detach();
    while (1) {
        socklen_t client_len = sizeof(client_socket);
        if ((received = recvfrom(sock, buffer, 255, 0, (struct sockaddr *) &client_socket, &client_len)) < 0) 
            exit(0);
        buffer[received] = '\0';
        string x = buffer;
        thread second(&Ring::ProcessData, this, x,client_socket, received, sock);
        second.detach();
        
    }
}

int main(int argc, char * argv[]) {
    int m = 3;
    Ring A;
    A.server_main(atoi(argv[1]), argc, argv[2], argv[3], m);
    return 0;
}
