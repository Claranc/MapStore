//GOSSIP检测类
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
    void process_hello(string x, sockaddr_in client_socket, int received,int sock); //收到新加入节点时的处理函数
    void process_respond(string x, sockaddr_in client_socket, int received,int sock); //收到老大返回的消息时的处理函数
    void process_info(string x, sockaddr_in client_socket, int received,int sock); //收到正常心跳包时的处理函数
};

//定义节点信息结构体
struct server_info {
    string ip_addr;
    int udp_port;
    string time_version;
};

//定义消息类型
enum class data_type{hello, respond, info};