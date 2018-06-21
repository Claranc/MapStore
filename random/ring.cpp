#include "ring.h"

//每个节点每次给它后面NUM个节点发送心跳包
#define NUM 3

//获取系统当前时间
string getTime() {
    time_t timep;
    time (&timep);
    char tmp[64];
    strftime(tmp, sizeof(tmp), "%Y/%m/%d %H:%M:%S",localtime(&timep) );
    return tmp;
}
 
//环形检测类
class Ring {
private:
    vector<struct server_info> server_list;  //存储节点信息
public:
    void CheckAlive(int sock);  //发送心跳及检测心跳
    void SplitString(const string& s, vector<string>& v, const string& c);  //分割字符串
    void ProcessData(string x, sockaddr_in client_socket, int received,int sock);  //处理接收数据
    void server_main(int udp_port, int argc, const char *address, const char *port);  //UDP运行函数
    void SocketSend(int sock, int m, string x); //给其他节点发送数据
    vector<int> randsend(int num); //产生随机数
};

//随机函数
int myrandom (int i) { return std::rand()%i;}
vector<int> Ring::randsend(int num) {
  srand ( unsigned ( std::time(0) ) );
  std::vector<int> myvector;
  for (int i = 1; i < num; ++i) myvector.push_back(i); 
  random_shuffle ( myvector.begin(), myvector.end() );
  random_shuffle ( myvector.begin(), myvector.end(), myrandom);
  return myvector;
}

//给其他节点发送数据
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

void Ring::CheckAlive(int sock) {
    while(1) {
        int count = 0;
        //存储开始发送时的节点总数
        int size = server_list.size();
        if(count < 10) {
            //总数多于GOSSIP参数值，则随机选取NUM个节点发送
            if(size > NUM+1) {
                vector<int> a = randsend(size);
                for(int i = 0; i < NUM; i++) {
                    string send_data = "info";
                    if(server_list.size() > 1) {
                        for(int i = 1; i < size; i++) {
                            send_data = send_data + server_list[i].ip_addr + "_" + to_string(server_list[i].udp_port) + "_"
                            + server_list[i].time_version + "~";
                        }
                    }
                    SocketSend(sock, a[i], send_data);
                }
            }
            //否则，all2all
            else {
                for(int i = 1; i < size; i++) {
                    string send_data = "info";
                    if(server_list.size() > 1) {
                        for(int i = 1; i < size; i++) {
                            send_data = send_data + server_list[i].ip_addr + "_" + to_string(server_list[i].udp_port) + "_"
                            + server_list[i].time_version + "~";
                        }
                    }
                    SocketSend(sock, i, send_data);
                }
            }
            sleep(2);
            count++;
        }
        //没发送10次，检测是否有节点down掉
        if(size > 1) {
            for(int m = size-1; m > 0; m--) {
                //获取节点更新时间
                tm tm_2;
                time_t t_2; 
                const char *time2_1 = server_list[m].time_version.c_str();
                strptime(time2_1, "%Y/%m/%d %H:%M:%S", &tm_2);
                tm_2.tm_isdst = -1; 
                t_2  = mktime(&tm_2);
                //获取系统当前时间
                string nowtime = getTime();
                tm tm_1;
                time_t t_1; 
                const char *time1_1 = nowtime.c_str();
                strptime(time1_1, "%Y/%m/%d %H:%M:%S", &tm_1);
                tm_1.tm_isdst = -1;  
                t_1  = mktime(&tm_1);
                //节点更新时间与系统当前时间相差过大，则删掉
                if(t_1 - t_2 > 10) {
                    cout << server_list[0].udp_port << " " << server_list[m].ip_addr << " " 
                        << server_list[m].udp_port << " is dead" << endl;
                    server_list.erase(server_list.begin()+m);
                }
            }
        }
    }
}

//字符串分割
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
        //给新加入节点返回节点存储的数据
        string send_data = "respond";
        if(server_list.size() > 1) {
            for(int i = 1; i < server_list.size(); i++) {
                send_data = send_data + server_list[i].ip_addr + "_" + to_string(server_list[i].udp_port) + "_"
                + server_list[i].time_version + "~";
            }
        }
        const char *send_mess = send_data.c_str();
        if (sendto(sock, send_mess,strlen(send_mess), 0, (struct sockaddr *) &client_socket, sizeof(sockaddr_in)) < 0) {
            cerr << "Your UDP server cannot send messages\n";
            exit(0);
        }
        //本机没有存储发送hello的节点信息，将它添加进来（安全）
        if(flag == 0) {
            struct server_info *r = new server_info;
            memset(r,0,sizeof(server_info));
            r->ip_addr = inet_ntoa(client_socket.sin_addr);
            r->udp_port = ntohs(client_socket.sin_port);
            r->time_version = getTime();
            cout << server_list[0].udp_port << " " << r->ip_addr << " " << r->udp_port << " is online" << endl;
            server_list.push_back(*r);
            delete r;
            r = NULL;
        }   
    }
    //接收到心跳包
    else if(x.substr(0,strlen("info")) == "info") {
        //如果为新节点则加入其信息
        int flag = 0;
        for(int i = 1; i < server_list.size(); i++) {
            if(ntohs(client_socket.sin_port) == server_list[i].udp_port) {
                if(inet_ntoa(client_socket.sin_addr) == server_list[i].ip_addr) {
                    flag = 1;
                    server_list[i].time_version = getTime();
                    //cout << server_list[i].udp_port << "update version: " << server_list[i].time_version << endl;
                }
            }
        }
        if(flag == 0) {
            struct server_info *r = new server_info;
            memset(r,0,sizeof(server_info));
            r->ip_addr = inet_ntoa(client_socket.sin_addr);
            r->udp_port = ntohs(client_socket.sin_port);
            r->time_version = getTime();
            cout << server_list[0].udp_port << " " << r->ip_addr << " " << r->udp_port << " is online" << endl;
            server_list.push_back(*r);
            delete r;
            r = NULL;
        }
        //对数据进行处理
        string x_data = x.substr(strlen("info"));
        vector<string> info_split;
        SplitString(x_data, info_split, "~");
        //依次处理分割后的每个节点的信息
        for(int i = 0; i < info_split.size(); i++) {
            vector<string> info_each;
            SplitString(info_split[i], info_each, "_");
            //找到相应的节点进行更新
            int flag1 = 0;
            for(int j = 0; j < server_list.size(); j++) {
                const char *tmp = info_each[1].c_str();
                if(atoi(tmp) == server_list[j].udp_port) {
                    flag1 = 1;
                    if(server_list[j].time_version < info_each[2]) {
                        server_list[j].time_version = info_each[2];
                        //cout << server_list[j].udp_port << "update version: " << server_list[j].time_version << endl;
                    }
                }
            }
            //如果为新节点，则添加其信息
            if(0 == flag1) {
                struct server_info *r = new server_info;
                memset(r,0,sizeof(server_info));
                r->ip_addr = info_each[0];
                const char* tmp = info_each[1].c_str();
                r->udp_port = atoi(tmp);
                r->time_version = info_each[2];
                cout << server_list[0].udp_port << " " << r->ip_addr << " " << r->udp_port << " is online" << endl;
                server_list.push_back(*r);
                delete r;
                r = NULL;
            }
        }
    }
    else if(x.substr(0,strlen("respond")) == "respond") {
        //添加老大节点的信息
        struct server_info *s = new server_info;
        memset(s, 0, sizeof(server_info));
        s->ip_addr = inet_ntoa(client_socket.sin_addr);
        s->udp_port = ntohs(client_socket.sin_port);
        s->time_version = getTime();
        cout << server_list[0].udp_port << " " << s->ip_addr << " " << s->udp_port << " is online" << endl;
        server_list.push_back(*s);
        delete s;
        s = NULL;
        //将老大返回的信息添加进来
        string x_data = x.substr(strlen("respond"));
        vector<string> insert_info;
        SplitString(x_data, insert_info, "~");
        for(int i = 0; i < insert_info.size(); i++) {
            vector<string> info_each;
            SplitString(insert_info[i], info_each, "_");
            int flag_exist = 0;
            for(int j = 0; j < server_list.size(); j++) {
                const char* tmp1 = info_each[1].c_str();
                if(atoi(tmp1) == server_list[j].udp_port) {
                    flag_exist = 1;
                }
            }
            if(flag_exist == 0) {
                struct server_info *r = new server_info;
                memset(r,0,sizeof(server_info));
                r->ip_addr = info_each[0];
                const char* tmp = info_each[1].c_str();
                r->udp_port = atoi(tmp);
                r->time_version = info_each[2];
                cout << server_list[0].udp_port << " " << r->ip_addr << " " << r->udp_port << " is online" << endl;
                server_list.push_back(*r);
                delete r;
                r = NULL;
            }
        }
    }
}


void Ring::server_main(int udp_port, int argc, const char *address, const char *port) {
    //将自己的信息放在信息表中第一个
    struct server_info *p = new server_info;
    memset(p,0,sizeof(server_info));
    p->ip_addr = "localhost";
    p->udp_port = udp_port;
    p->time_version = getTime();
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
    int one = 1;
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
    char buffer[255550];
    int received;
    //发送并检测心跳包的线程
    thread check(&Ring::CheckAlive, this, sock);
    check.detach();
    //无限循环监听
    while (1) {
        socklen_t client_len = sizeof(client_socket);
        if ((received = recvfrom(sock, buffer, 255550, 0, (struct sockaddr *) &client_socket, &client_len)) < 0) {
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
        cout << "the third parameter is server's port, if necessary)" << endl;
    }
    return 0;
}
