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

vector<struct server_info> server_list;
vector<int> server_state;

struct server_info {
	string ip_addr;
	int udp_port;
};

void CheckAlive(int sock) {
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
            for(int i = 1; i < len; i++) {
                if(server_state[i] == 0) {
                    cout << server_list[i].ip_addr << " " << server_list[i].udp_port << " is dead" << endl;
                    server_down.push_back(i);
                }
            }
            for (int i = 0; i < server_down.size(); ++i)
            {
                server_list.erase(server_list.begin()+server_down[i]);
                server_state.erase(server_state.begin()+server_down[i]);
            }
            for(int i = 0; i < server_state.size(); i++) server_state[i] = 0;
        }
        
    }
}

void ProcessData(string x, sockaddr_in client_socket, int received,int sock) {
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
    else { 
        if(x.substr(0,4) == "info") {
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
    }
}


void server_main(int udp_port, int argc, const char *address, const char *port) {
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

    thread third(CheckAlive,sock);
    third.detach();

    while (1) 
    {
        socklen_t client_len = sizeof(client_socket);
        if ((received = recvfrom(sock, buffer, 255, 0, (struct sockaddr *) &client_socket, &client_len)) < 0) 
            exit(0);
        buffer[received] = '\0';
        string x = buffer;
        thread second(ProcessData,x,client_socket, received, sock);
        second.detach();
        
    }
}

int main(int argc, char * argv[])
{
    server_main(atoi(argv[1]), argc, argv[2], argv[3]);
    return 0;
}