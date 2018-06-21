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
#include <algorithm>
#include <ctime>
using namespace std;

//定义节点信息结构体
struct server_info {
    string ip_addr;
    int udp_port;
    string time_version;
};