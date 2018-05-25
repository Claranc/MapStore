#include "ring.h"

#ifdef RING
//每个节点每次给它后面NUM个节点发送心跳包
#define NUM 1
#endif

//环形检测类
class Ring {
private:
    vector<struct server_info> server_list;  //存储节点信息
    vector<int> server_state; //存储节点状态
public:
    void CheckAlive(int sock);  //发送心跳及检测心跳
    void SplitString(const string& s, vector<string>& v, const string& c);  //分割字符串
    void ProcessData(string x, sockaddr_in client_socket, int received,int sock);  //处理接收数据
    void server_main(int udp_port, int argc, const char *address, const char *port);  //UDP运行函数
    void SocketSend(int sock, int m, string x);
};

void Ring::SocketSend(int sock, int m, string x) {
    struct sockaddr_in *q = new sockaddr_in;
    memset(q,0,sizeof(sockaddr_in));
    const char *addr = server_list[m].ip_addr.c_str();
    q->sin_addr.s_addr = inet_addr(addr);
    q->sin_port = htons(server_list[m].udp_port);
    const char *messages = x.c_str();
    //cout << "I send " << x << " to " << server_list[m].udp_port << endl;
    if (sendto(sock, messages,strlen(messages), 0, (struct sockaddr *) q, sizeof(sockaddr_in)) < 0) {
        cerr << "Your UDP server cannot send messages\n";
        exit(0);
    }
    delete q;
    q = NULL;
}

//sock为UDP状态
void Ring::CheckAlive(int sock) {
    while(1) {
        int i = 0; 
        int len =  server_list.size(); //记录开始发送时的节点总数
        while(i < 4) {
            //节点数大于1则发送心跳包
            if(server_list.size() > 1 ) {
                for(int j = 1; j < server_list.size(); j++) {
                    string x_send = "I am still alive";
                    SocketSend(sock, j, x_send);
                    #ifdef RING
                    //已经发送了NUM个心跳包，break;
                    if(NUM == j) {
                        break;
                    }
                    #endif
                }
            }
            i++;
            sleep(1);
        }
        //节点总数大于1时相互检测运行状态
        if(server_state.size() > 1) {
            vector<int> server_down; //保存down掉的节点编号
            int check_start = 1;
            #ifdef RING
            //如果检测时相比于发送心跳包时有新节点加入，则重新发送数据包检测
            if(len != server_state.size()) continue; 
            //设置开始检测的起始点
            check_start = len - NUM;
            if (check_start < 1) {
                check_start = 1;
            }
            #endif
            for(int i = check_start; i < len; i++) {
                //检测到节点down掉就将信息发送给后面n个节点
                if(server_state[i] == 0) {
                    cout << server_list[0].udp_port << " " << server_list[i].ip_addr << " " << server_list[i].udp_port << " is dead" << endl;
                    server_down.push_back(i);
                    #ifdef RING
                    if(server_list.size() > 1) {
                        for(int j = 1; j < len; j++) {
                            string x_send = "dead" +  server_list[i].ip_addr + "_" + to_string(server_list[i].udp_port);
                            SocketSend(sock, j, x_send);
                            //已经发送了NUM个心跳包，break;
                            if(NUM == j) {
                                break;
                            }
                        }
                    }
                    #endif
                }
            }
            //将down掉的节点信息删去（从后往前删，不然指针信息会过期）
            for (int i = server_down.size()-1; i >= 0; --i) {
                server_list.erase(server_list.begin()+server_down[i]);
                server_state.erase(server_state.begin()+server_down[i]);
            }
            //置0,为新的一轮检测做准备
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
    //有新节点加入
    if(x == "hello") {
        //检测信息表中是否存在新节点数据
        int flag = 0;
        for(int i = 1; i < server_list.size(); i++) {
            if(ntohs(client_socket.sin_port) == server_list[i].udp_port) {
                if(inet_ntoa(client_socket.sin_addr) == server_list[i].ip_addr) {
                    flag = 1;
                }
            }
        }
        //向其他已存在的节点发送新节点数据
        if(server_list.size() > 1) {
            string send_data = "info";
            int k = server_list.size();
            if(flag == 1) {
                k--;
            }
            for(int i = 1; i < k; i++) {
                send_data = send_data + server_list[i].ip_addr + " " + to_string(server_list[i].udp_port) + "~";
            }
            const char *send_mess = send_data.c_str();
            if (sendto(sock, send_mess,strlen(send_mess), 0, (struct sockaddr *) &client_socket, sizeof(sockaddr_in)) < 0) {
                cerr << "Your UDP server cannot send messages\n";
                exit(0);
            }
            #ifdef RING
            string ip_address = inet_ntoa(client_socket.sin_addr);
            string add_message = "modify" +  ip_address + "_" + to_string(ntohs(client_socket.sin_port));
            SocketSend(sock, 1, add_message);
            #endif
        } 
        //将新节点数据添加进自己的信息表中
        if(flag == 0) {
            struct server_info *r = new server_info;
            memset(r,0,sizeof(server_info));
            r->ip_addr = inet_ntoa(client_socket.sin_addr);
            r->udp_port = ntohs(client_socket.sin_port);
            cout << server_list[0].udp_port << " " << r->ip_addr << " " << r->udp_port << " is online" << endl;
            server_list.push_back(*r);
            delete r;
            r = NULL;
            server_state.push_back(0);  
        }   
    }
    //接收到心跳包
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
        //如果是新节点，则将信息添加进自己的信息表中
        if(flag == 0) {
            struct server_info *t = new server_info;
            memset(t, 0, sizeof(server_info));
            t->ip_addr = inet_ntoa(client_socket.sin_addr);
            t->udp_port = ntohs(client_socket.sin_port);
            cout << server_list[0].udp_port << " "<< t->ip_addr << " " << t->udp_port << " is online" << endl;
            server_list.push_back(*t);
            delete t;
            t = NULL;
            server_state.push_back(0);
        }
        //cout << "Connected: "<< inet_ntoa(client_socket.sin_addr)<<"\t"<<ntohs(client_socket.sin_port)<<endl;
    }
    //处理向老大发送数据后收到的数据
    else if(x.substr(0,strlen("info")) == "info") {
        struct server_info *s = new server_info;
        memset(s, 0, sizeof(server_info));
        s->ip_addr = inet_ntoa(client_socket.sin_addr);
        s->udp_port = ntohs(client_socket.sin_port);
        cout << server_list[0].udp_port << " " << s->ip_addr << " " << s->udp_port << " is online" << endl;
        server_list.push_back(*s);
        delete s;
        s = NULL;
        server_state.push_back(0);
        string x_data = x.substr(4);
        int count = 0;
        for(int i = 0; i < x_data.size(); i++ ) {
            if(x_data[i] == '~') count++;
        }
        //将老大发回的信息添加进信息表中
        for(int j = 0; j < count; j++) {
            int a = x_data.find_first_of('~');
            string process_data = x_data.substr(0,a);
            int b = process_data.find_first_of(' ');
            struct server_info *s = new server_info;
            memset(s, 0, sizeof(server_info));
            s->ip_addr = process_data.substr(0,b);
            const char* tmp = process_data.substr(b+1).c_str();
            s->udp_port = atoi(tmp);
            cout << server_list[0].udp_port << " " << s->ip_addr << " " << s->udp_port << " is online" << endl;
            server_list.push_back(*s);
            server_state.push_back(0);
            delete s;
            s = NULL;
            if(j < count-1) x_data = x_data.substr(a+1);
        }
    }
    #ifdef RING
    //有新节点加入更改信息并传给下一个（顺便将老大的信息添加进去）
    else if(x.substr(0,strlen("modify")) == "modify") {
        if(server_list.size() > 2) {
            string cli_addr = inet_ntoa(client_socket.sin_addr);
            string send_mess = "transfer" + cli_addr + "_" + to_string(ntohs(client_socket.sin_port)) + "_" + x.substr(6);
            SocketSend(sock, 1, send_mess);
        }
        string x_pro = x.substr(strlen("modify"));
        vector<string> a;
        SplitString(x_pro,a,"_");
        struct server_info *s = new server_info;
        s->ip_addr = a[0];
        const char *uport = a[1].c_str();
        s->udp_port = atoi(uport);
        server_list.insert(server_list.end()-1, *s);
        server_state.push_back(0);
        cout << server_list[0].udp_port << " " << s->ip_addr << " " << s->udp_port << " is online" << endl;
        delete s;
        s = NULL;
    }
    //有新节点加入信息更改（含老大的信息）
    else if(x.substr(0,strlen("transfer")) == "transfer") {
        string x_pro = x.substr(strlen("transfer"));
        vector<string> a;
        SplitString(x_pro,a,"_");
        struct server_info *s = new server_info;
        //a[2]和a[3]分别为新加入节点的ip和port
        s->ip_addr = a[2];
        const char *uport = a[3].c_str();
        s->udp_port = atoi(uport);
        int trans_flag = 0;
        int k = 0;
        //a[0]和a[1]为新加入节点选择的老大，需要在每个节点的信息表中老大的前面加入新节点的信息
        for(int j = 1; j < server_list.size(); j++) {
            if(server_list[j].ip_addr == a[0]) {
                const char* u_port = a[1].c_str();
                if(server_list[j].udp_port == atoi(u_port)) {
                    k = j;
                    vector<struct server_info>::iterator it = server_list.begin()+j;
                    server_list.insert(it, *s);
                    server_state.push_back(0);
                    cout << server_list[0].udp_port << " " << s->ip_addr << " " << s->udp_port << " is online" << endl;
                    trans_flag = 1;
                    break;
                }
            }
        }
        //如果当前节点的下一个节点不是老大，则把信息传给下一个节点
        if(k != 1 && trans_flag == 1) {
            SocketSend(sock, 1, x);
        }
        delete s;
        s = NULL;
    }
    //有节点down掉，信息更改
    else if(x.substr(0,strlen("dead")) == "dead") {
        string x_process = x.substr(strlen("dead"));
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
                    cout << server_list[0].udp_port << " " << server_list[i].ip_addr << " " << server_list[i].udp_port << " is dead" << endl;
                    vector<struct server_info>::iterator it = server_list.begin()+i;
                    server_list.erase(it);
                    vector<int>::iterator it2 = server_state.begin() + i;
                    server_state.erase(it2);
                    break;
                }
            }
        }
        if(dead_flag == 1 && k !=1) {
            SocketSend(sock, 1, x);
        }
    }
    #endif
}


void Ring::server_main(int udp_port, int argc, const char *address, const char *port) {
    //将自己的信息放在信息表中第一个
    server_state.push_back(0);
    struct server_info *p = new server_info;
    memset(p,0,sizeof(server_info));
    p->ip_addr = "localhost";
    p->udp_port = udp_port;
    server_list.push_back(*p);
    cout << " This is UDP server,"<< " port: " << p->udp_port << endl;
    delete p;
    p = NULL;
    int sock;
    struct sockaddr_in server_socket;
    struct sockaddr_in client_socket;
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        cerr << port <<" The value of sock is not correct\n";
        exit(0);
    }
    int one=1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one));
    memset(&server_socket, 0, sizeof(server_socket));
    server_socket.sin_family = AF_INET;
    server_socket.sin_addr.s_addr = htonl(INADDR_ANY);
    server_socket.sin_port = htons(udp_port); 
    if (bind(sock, (struct sockaddr *) &server_socket, sizeof(server_socket)) < 0) {
        cerr << "There is something wrong with bind\n";
        cerr << "The port is " << port;
        exit(0);
    }
    //新加入向老大发送新加入的信息
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
            cerr << port << " Your UDP server cannot send messages\n";
            exit(0);
        }
    }  
    char buffer[255];
    int received;
    //发送并检测心跳包的线程
    thread check(&Ring::CheckAlive, this, sock);
    check.detach();
    //无限循环监听
    while (1) {
        socklen_t client_len = sizeof(client_socket);
        if ((received = recvfrom(sock, buffer, 255, 0, (struct sockaddr *) &client_socket, &client_len)) < 0) {
            cerr << port << " Your UDP Server cannot receive messages\n";
        }
        //处理收到的数据线程
        buffer[received] = '\0';
        string x = buffer;
        thread processdata(&Ring::ProcessData, this, x,client_socket, received, sock);
        processdata.detach();
        
    }
}


int main(int argc, char * argv[]) {
    Ring A;
    if(argc == 2 || argc == 4) {
        A.server_main(atoi(argv[1]), argc, argv[2], argv[3]);
    }
    else {
        cout << "your parameter is not correct.\n";
        cout << "The first parameter is your own port (the second parameter is the server's ip, ";
        cout << "the third parameter is server's port, if need)" << endl;
    }
    return 0;
}
