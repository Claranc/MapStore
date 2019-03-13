/**********************************
 * FILE NAME: MP1Node.cpp
 *
 * DESCRIPTION: Membership protocol run by this Node.
 * 				Definition of MP1Node class functions.
 **********************************/

#include "MP1Node.h"

//设置GOSSIP协议中给GOSSIP_NUM个随机节点发送消息的值
#ifdef GOSSIP
#define GPSSIP_NUM 1
#endif

//设置RING协议中给后RING_NUM个节点发送消息的值
#ifdef RING
#define RING_NUM 6
#endif

//设置SWIM协议中挑选SWIM_NUM个节点测试
#ifdef SWIM
#define SWIM_NUM 3
#endif

//添加节点信息的专用函数
void MP1Node::savetoMemberlist(int id, short port, long timestamp = 0) {
    if(!timestamp) {
        timestamp = par->getcurrtime();
    }
    MemberListEntry new_node;
    new_node.id = id;
    new_node.port = port;
    new_node.heartbeat = 0;
    new_node.timestamp = timestamp;
    memberNode->memberList.push_back(new_node); 
    if(memberNode->memberList.size() > 1) {
        Address node_addr;
        memcpy(&node_addr.addr[0], &id, sizeof(int));
        memcpy(&node_addr.addr[4], &port, sizeof(short));
        log->logNodeAdd(&memberNode->addr, &node_addr);
        #ifdef SWIM
        status.insert(make_pair(id,false));
        #endif
    } 
}

#if (!defined GOSSIP) || (!defined SWIM)
//随机函数,生成值为1～num的乱序数组，num为memberList的总长度
vector<int> MP1Node::randsend(int num) {
  std::vector<int> myvector;
  for (int i = 1; i < num; ++i) myvector.push_back(i); 
  random_shuffle ( myvector.begin(), myvector.end() );
  random_shuffle ( myvector.begin(), myvector.end(), [](int i) {return rand() % i; });
  return myvector;
}
#endif

/*
 * Note: You can change/add any functions in MP1Node.{h,cpp}
 */

/**
 * Overloaded Constructor of the MP1Node class
 * You can add new members to the class if you think it
 * is necessary for your logic to work
 */
MP1Node::MP1Node(Member *member, Params *params, EmulNet *emul, Log *log, Address *address) {
    for( int i = 0; i < 6; i++ ) {
        NULLADDR[i] = 0;
    }
    this->memberNode = member;
    this->emulNet = emul;
    this->log = log;
    this->par = params;
    this->memberNode->addr = *address;
}

/**
 * Destructor of the MP1Node class
 */
MP1Node::~MP1Node() {}

/**
 * FUNCTION NAME: recvLoop
 *
 * DESCRIPTION: This function receives message from the network and pushes into the queue
 * 				This function is called by a node to receive messages currently waiting for it
 */
int MP1Node::recvLoop() {
    if ( memberNode->bFailed ) {
        return false;
    }
    else {
        return emulNet->ENrecv(&(memberNode->addr), enqueueWrapper, NULL, 1, &(memberNode->mp1q));
    }
}

/**
 * FUNCTION NAME: enqueueWrapper
 *
 * DESCRIPTION: Enqueue the message from Emulnet into the queue
 */
int MP1Node::enqueueWrapper(void *env, char *buff, int size) {
    Queue q;
    return q.enqueue((queue<q_elt> *)env, (void *)buff, size);
}

/**
 * FUNCTION NAME: nodeStart
 *
 * DESCRIPTION: This function bootstraps the node
 * 				All initializations routines for a member.
 * 				Called by the application layer.
 */
void MP1Node::nodeStart(char *servaddrstr, short servport) {
    Address joinaddr;
    joinaddr = getJoinAddress();

    // Self booting routines
    if( initThisNode(&joinaddr) == -1 ) {
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "init_thisnode failed. Exit.");
#endif
        exit(1);
    }

    if( !introduceSelfToGroup(&joinaddr) ) {
        finishUpThisNode();
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "Unable to join self to group. Exiting.");
#endif
        exit(1);
    }

    return;
}

/**
 * FUNCTION NAME: initThisNode
 *
 * DESCRIPTION: Find out who I am and start up
 */
int MP1Node::initThisNode(Address *joinaddr) {
    /*
     * This function is partially implemented and may require changes
     */
    int id = *(int*)(&memberNode->addr.addr);
    short port = *(short*)(&memberNode->addr.addr[4]);

    memberNode->bFailed = false;
    memberNode->inited = true;
    memberNode->inGroup = false;
    // node is up!
    memberNode->nnb = 0;
    memberNode->heartbeat = 0;
    memberNode->pingCounter = TFAIL;
    memberNode->timeOutCounter = -1;
    initMemberListTable(memberNode);

    //将自己的信息存储进列表中，存在第一个
    int own_id = memberNode->addr.addr[0];
    short own_port = memberNode->addr.addr[4];
    savetoMemberlist(own_id, own_port);
    return 0;
}

/**
 * FUNCTION NAME: introduceSelfToGroup
 *
 * DESCRIPTION: Join the distributed system
 */
int MP1Node::introduceSelfToGroup(Address *joinaddr) {
    MessageHdr *msg;
#ifdef DEBUGLOG
    static char s[1024];
#endif

    if ( 0 == memcmp((char *)&(memberNode->addr.addr), (char *)&(joinaddr->addr), \
        sizeof(memberNode->addr.addr))) {
        // I am the group booter (first process to join the group). Boot up the group
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "Starting up group...");
#endif
        memberNode->inGroup = true;
    }
    else {
        size_t msgsize = sizeof(MessageHdr) + sizeof(joinaddr->addr) + sizeof(long);
        msg = (MessageHdr *) malloc(msgsize * sizeof(char));

        // create JOINREQ message: format of data is {struct Address myaddr}
        msg->msgType = JOINREQ;
        memcpy((char *)(msg+1), &memberNode->addr.addr, sizeof(memberNode->addr.addr));
        memcpy((char *)(msg+1) + sizeof(memberNode->addr.addr), &memberNode->heartbeat, sizeof(long));

#ifdef DEBUGLOG
        sprintf(s, "Trying to join...");
        log->LOG(&memberNode->addr, s);
#endif

        // send JOINREQ message to introducer member
        emulNet->ENsend(&memberNode->addr, joinaddr, (char *)msg, msgsize);
        memberNode->inGroup = true;
        free(msg);
    }

    return 1;

}

/**
 * FUNCTION NAME: finishUpThisNode
 *
 * DESCRIPTION: Wind up this node and clean up state
 */
int MP1Node::finishUpThisNode(){
   /*
    * Your code goes here
    */
    memberNode->inGroup = false;
}

/**
 * FUNCTION NAME: nodeLoop
 *
 * DESCRIPTION: Executed periodically at each member
 * 				Check your messages in queue and perform membership protocol duties
 */
void MP1Node::nodeLoop() {
    if (memberNode->bFailed) {
        return;
    }
    // Check my messages
    checkMessages();

    // Wait until you're in the group...
    if( !memberNode->inGroup ) {
        return;
    }
    // ...then jump in and share your responsibilites!
    nodeLoopOps();

    return;
}

/**
 * FUNCTION NAME: checkMessages
 *
 * DESCRIPTION: Check messages in the queue and call the respective message handler
 */
void MP1Node::checkMessages() {
    void *ptr;
    int size;

    // Pop waiting messages from memberNode's mp1q
    while ( !memberNode->mp1q.empty() ) {
        ptr = memberNode->mp1q.front().elt;
        size = memberNode->mp1q.front().size;
        memberNode->mp1q.pop();
        recvCallBack((void *)memberNode, (char *)ptr, size);
    }
    return;
}

/**
 * FUNCTION NAME: recvCallBack
 *
 * DESCRIPTION: Message handler for different message types
 */
bool MP1Node::recvCallBack(void *env, char *data, int size ) {
    /*
     * Your code goes here
     */
    //获取消息类型
    MessageHdr *rev_type = (MessageHdr*)data;
    //获取源地址
    Address src_addr;
    memcpy(&src_addr,(data+sizeof(MessageHdr)), sizeof(memberNode->addr.addr));
    int src_id = src_addr.addr[0];
    short src_port = src_addr.addr[4];
    //获取heartbeat
    long heartbeat;
    memcpy(&heartbeat, (char*)(data+sizeof(MessageHdr)+sizeof(memberNode->addr.addr)), sizeof(long));
    //处理消息，如果消息类型为JOINREQ（收到新节点加入消息）
    if(JOINREQ == rev_type->msgType) {
        //将自己列表的信息发送给新加入节点
        int memberlist_size = memberNode->memberList.size();
        size_t msg_size = sizeof(MessageHdr) + sizeof(memberNode->addr.addr) + sizeof(int) + \
            memberlist_size*sizeof(MemberListEntry);
        char *msg = new char[msg_size];
        MessageHdr send_type;
        send_type.msgType = JOINREP;
        memcpy((MessageHdr*)msg, &send_type, sizeof(MessageHdr));
        memcpy(msg + sizeof(MessageHdr), &memberNode->addr.addr, sizeof(memberNode->addr.addr));
        memcpy(msg + sizeof(MessageHdr) + sizeof(memberNode->addr.addr), &memberlist_size, sizeof(int));
        for(int i = 0; i < memberlist_size; i++) {
            memcpy((char*)msg + sizeof(MessageHdr) + sizeof(memberNode->addr.addr) + sizeof(int) + \
                i * sizeof(MemberListEntry), &memberNode->memberList[i], sizeof(MemberListEntry));
        }
        emulNet->ENsend(&memberNode->addr, &src_addr, (char*)msg, msg_size);
        //将新节点的信息存储起来
        savetoMemberlist(src_id, src_port);
        #if(!defined GOSSIP) || (!defined RING)  
        //把新节点信息发给其他人(除去自己和新加入的节点)
        size_t msg_info_size = sizeof(MessageHdr) + 2 * sizeof(memberNode->addr.addr);
        char *msg_info = new char[msg_info_size];
        MessageHdr info_type;
        info_type.msgType = UP;
        memcpy((MessageHdr*)msg_info, &info_type, sizeof(MessageHdr));
        memcpy(msg_info + sizeof(MessageHdr), &memberNode->addr.addr, sizeof(memberNode->addr.addr));
        memcpy(msg_info + sizeof(MessageHdr) + sizeof(memberNode->addr.addr), &src_addr, sizeof(memberNode->addr.addr));
        //发给除了自己和新节点之外的节点
        for(int i = 1; i < memberNode->memberList.size() - 1 ; i++) {
            Address to_addr;
            memcpy(&to_addr.addr[0],  &memberNode->memberList[i].id, sizeof(int));
            memcpy(&to_addr.addr[4],  &memberNode->memberList[i].port, sizeof(short));
            emulNet->ENsend(&memberNode->addr, &to_addr, (char*)msg_info, msg_info_size);
        }
        #endif
    }
    //JOINREP(收到老大发回来的信息)
    else if(JOINREP == rev_type->msgType) {
        //将老大发送回来的节点信息添加进列表
        int memberlist_size;
        memcpy(&memberlist_size, (char*)data + sizeof(MessageHdr) + sizeof(memberNode->addr.addr), \
            sizeof(int));
        for(int i = 0; i < memberlist_size; i++) {
            MemberListEntry temp;
            memcpy(&temp, data + sizeof(MessageHdr) + sizeof(memberNode->addr.addr) + sizeof(int) + \
                i*sizeof(MemberListEntry), sizeof(MemberListEntry));
            bool flag = false;
            for(auto it = memberNode->memberList.begin(); it != memberNode->memberList.end(); it++) {
                if(temp.id == it->id && temp.port == it->port) {
                    flag = true;
                    break;
                }
            }
            if(!flag && par->getcurrtime() - temp.timestamp < TREMOVE) {
                savetoMemberlist(temp.id, temp.port, temp.timestamp);
            }
        }  
        /*
        cout << memberNode->addr.getAddress() << ": ";
        for(auto a : memberNode->memberList) {
            cout << a.id << " ";
        }
        cout << endl;*/
    }
    //HEARTBEAT（正常心跳包）
    else if(HEARTBEAT == rev_type->msgType) {
        //判断是否是新的节点，如果是就存储，是老节点就更新
        bool flag = 0; //标记收到的心跳包节点是否为新节点
        for(size_t i = 0; i < memberNode->memberList.size(); i++) {
            if(memberNode->memberList[i].id == src_id && memberNode->memberList[i].port == src_port) {
                flag = 1;
                memberNode->memberList[i].heartbeat = heartbeat;
                memberNode->memberList[i].timestamp = par->getcurrtime();
            }
        }
        if(!flag) {
            //如果是新节点，将新节点的信息存储起来
            savetoMemberlist(src_id, src_port);
        }
        //处理数据包中的列表信息，进行更新
        int memberlist_size;
        memcpy(&memberlist_size, (char*)data + sizeof(MessageHdr) + sizeof(memberNode->addr.addr) + sizeof(long), sizeof(int));
        for(int i = 0; i < memberlist_size; i++) {
            MemberListEntry temp;
            memcpy(&temp, data + sizeof(MessageHdr) + sizeof(memberNode->addr.addr) + sizeof(int) + sizeof(long) +\
                i*sizeof(MemberListEntry), sizeof(MemberListEntry));
            bool flag_2 = 0; //标记列表信息中是否有新节点
            for(auto &a : memberNode->memberList) {
                if(temp.id == a.id && temp.port == a.port) {
                    flag_2 =1;
                    if(temp.timestamp > a.timestamp) {
                        a.timestamp = temp.timestamp;
                        a.heartbeat = temp.heartbeat;
                    }
                }
            }
            //是新节点添加进列表中
            if(!flag_2 && par->getcurrtime() - temp.timestamp < TREMOVE) {
                savetoMemberlist(temp.id, temp.port, temp.timestamp);
            }
        }
    }
    #ifdef SWIM
    //收到了别人的帮忙请求
    else if(HELP == rev_type->msgType) {
        Address objective_addr;
        memcpy(&objective_addr.addr, data + sizeof(MessageHdr) + sizeof(memberNode->addr.addr), sizeof(memberNode->addr.addr));
        size_t msg_size = sizeof(MessageHdr) + 2*sizeof(memberNode->addr.addr);
        char *msg_info = new char[msg_size];
        MessageHdr info_type;
        info_type.msgType = PLEASEGOHOME;
        memcpy(msg_info, &info_type, sizeof(MessageHdr));
        memcpy(msg_info + sizeof(MessageHdr), &memberNode->addr.addr, sizeof(memberNode->addr.addr));
        memcpy(msg_info + sizeof(MessageHdr) + sizeof(memberNode->addr.addr), &src_addr, sizeof(memberNode->addr.addr));
        emulNet->ENsend(&memberNode->addr, &objective_addr, (char*)msg_info, msg_size);
    }
    //收到了别人代发的消息，准备给中间人回复一下
    else if(PLEASEGOHOME == rev_type->msgType) {
        size_t msg_size = sizeof(MessageHdr) + 2 * sizeof(memberNode->addr.addr);
        char *msg_info = new char[msg_size];
        MessageHdr info_type;
        info_type.msgType = IWILLGOHOME;
        memcpy(msg_info, &info_type, sizeof(MessageHdr));
        memcpy(msg_info + sizeof(MessageHdr), &memberNode->addr.addr, sizeof(memberNode->addr.addr));
        memcpy(msg_info + sizeof(MessageHdr) + sizeof(memberNode->addr.addr), data + sizeof(MessageHdr) + sizeof(memberNode->addr.addr), \
            sizeof(memberNode->addr.addr));
        emulNet->ENsend(&memberNode->addr, &src_addr, (char*)msg_info, msg_size);
    }
    //代发节点收到了目标节点的回复，准备发送回去
    else if(IWILLGOHOME == rev_type->msgType) {
        size_t msg_size = sizeof(MessageHdr) + 2 * sizeof(memberNode->addr.addr);
        char *msg_info = new char[msg_size];
        MessageHdr info_type;
        info_type.msgType = HEWILLGOHOME;
        memcpy(msg_info, &info_type, sizeof(MessageHdr));
        memcpy(msg_info + sizeof(MessageHdr), &memberNode->addr.addr, sizeof(memberNode->addr.addr));
        memcpy(msg_info + sizeof(MessageHdr) + sizeof(memberNode->addr.addr), &src_addr ,sizeof(memberNode->addr.addr));
        Address to_addr;
        memcpy(&to_addr.addr, data + sizeof(MessageHdr) + sizeof(memberNode->addr.addr), sizeof(memberNode->addr.addr));
        emulNet->ENsend(&memberNode->addr, &to_addr, (char*)msg_info, msg_size);
    }
    //收到了疑似down掉的节点间接发送回来的消息，更新一波
    else if(HEWILLGOHOME == rev_type->msgType) {
        Address objective_addr;
        memcpy(&objective_addr.addr, data + sizeof(MessageHdr) + sizeof(memberNode->addr.addr), sizeof(memberNode->addr.addr));
        int down_id = objective_addr.addr[0];
        short down_port = objective_addr.addr[4];
        for(vector<MemberListEntry>::iterator it2 = memberNode->memberList.begin()+1; it2 != memberNode->memberList.end(); it2++) {
            if(down_id == it2->id && down_port == it2->port) {
                it2->timestamp = par->getcurrtime();
                for(auto it = status.begin(); it != status.end(); it++) {
                    if(it->first == down_id) {
                        it->second = true;
                    }
                }
            }
        }
    }
    //收到了SWIM协议的普通PING包
    else if(SENDPING == rev_type->msgType) {
        size_t msg_size = sizeof(MessageHdr) + sizeof(memberNode->addr.addr);
        char *msg_info = new char[msg_size];
        MessageHdr info_type;
        info_type.msgType = RESPONSE;
        memcpy(msg_info, &info_type, sizeof(MessageHdr));
        memcpy(msg_info + sizeof(MessageHdr), &memberNode->addr.addr, sizeof(memberNode->addr.addr));
        emulNet->ENsend(&memberNode->addr, &src_addr, (char*)msg_info, msg_size);
    }
    //收到了PING包节点正常回复的消息
    else if(RESPONSE == rev_type->msgType) {
        for(vector<MemberListEntry>::iterator it2 = memberNode->memberList.begin()+1; it2 != memberNode->memberList.end(); it2++) {
            if(src_id == it2->id && src_port == it2->port) {
                it2->timestamp = par->getcurrtime();
                for(auto it = status.begin(); it != status.end(); it++) {
                    if(it->first == src_id) {
                        it->second = true;
                    }
                }
            }
        }
    }
    #endif
    #if(!defined GOSSIP) || (!defined RING)
    //DOWN, 收到有节点down掉的信息
    else if(DOWN == rev_type->msgType) {
        Address down_addr;
        memcpy(&down_addr.addr, data + sizeof(MessageHdr) + sizeof(memberNode->addr.addr), sizeof(memberNode->addr.addr));
        int down_id = down_addr.addr[0];
        short down_port = down_addr.addr[4];
        //查询down掉的节点是否还存在列表中，如果存在并且确实超出时长了就删掉
        for(vector<MemberListEntry>::iterator it2 = memberNode->memberList.begin()+1; it2 != memberNode->memberList.end(); it2++) {
            if(down_id == it2->id && down_port == it2->port) {
                if(par->getcurrtime() - it2->timestamp >= TREMOVE) {
                    log->logNodeRemove(&memberNode->addr, &down_addr);
                    memberNode->memberList.erase(it2);
                }
                break;
            }
        }
    }
    //UP，收到有新节点加入的消息
    else if(UP == rev_type->msgType) {
        bool flag3 = false; //标记新节点是否已经存在于列表中
        Address up_addr;
        memcpy(&up_addr.addr, data + sizeof(MessageHdr) + sizeof(memberNode->addr.addr), sizeof(memberNode->addr.addr));
        int up_id = up_addr.addr[0];
        short up_port = up_addr.addr[4];
        for(auto it = memberNode->memberList.begin(); it != memberNode->memberList.end(); it++) {
            if(up_id == it->id && up_port == it->port) {
                flag3 = true;
                break;
            }
        }
        //如果不存在，就添加进去
        if(!flag3) {
            savetoMemberlist(up_id, up_port);
        }

    }
    #endif
}

/**
 * FUNCTION NAME: nodeLoopOps
 *
 * DESCRIPTION: Check if any node hasn't responded within a timeout period and then delete
 * 				the nodes
 * 				Propagate your membership list
 */
void MP1Node::nodeLoopOps() {

    /*
     * Your code goes here
     */
    
    #ifdef SWIM
    if(memberNode->memberList.size() == 1) {
        return;
    }
    status.clear();
    //随机选取一些节点发送ping包
    if(memberNode->memberList.size()-1 > SWIM_NUM) {
        vector<int> send_node = randsend(memberNode->memberList.size());
        for(int i = 0; i < SWIM_NUM; i++) {
            status.insert(make_pair(memberNode->memberList[send_node[i]].id, false));
        }
        for(int i = 0; i < SWIM_NUM; i++) {
            int memberlist_size = memberNode->memberList.size();
            size_t msg_size = sizeof(MessageHdr) + sizeof(memberNode->addr.addr);
            char *msg = new char[msg_size];
            MessageHdr msg_type;
            msg_type.msgType = SENDPING;
            memcpy(msg, &msg_type, sizeof(MessageHdr));
            Address to_addr;
            int t = send_node[i];
            memcpy(&to_addr.addr[0], &memberNode->memberList[t].id, sizeof(int));
            memcpy(&to_addr.addr[4],  &memberNode->memberList[t].port, sizeof(short));
            memcpy(msg + sizeof(MessageHdr), &memberNode->addr.addr, sizeof(to_addr.addr));
            emulNet->ENsend(&memberNode->addr, &to_addr, (char*)msg, msg_size);
        }
    }
    //节点总数不够SWIM_NUM,采用ALL2ALL
    else if(memberNode->memberList.size()-1 <= SWIM_NUM) {
        int memberlist_size = memberNode->memberList.size();
        for(int i = 1; i < memberlist_size; i++) {
            status.insert(make_pair(memberNode->memberList[i].id, false));
        }
        for(int i = 1; i < memberlist_size; i++) {
            size_t msg_size = sizeof(MessageHdr) + sizeof(memberNode->addr.addr);
            char *msg = new char[msg_size];
            MessageHdr msg_type;
            msg_type.msgType = SENDPING;
            memcpy(msg, &msg_type, sizeof(MessageHdr));
            Address to_addr;
            memcpy(&to_addr.addr[0], &memberNode->memberList[i].id, sizeof(int));
            memcpy(&to_addr.addr[4],  &memberNode->memberList[i].port, sizeof(short));
            memcpy(msg + sizeof(MessageHdr), &memberNode->addr.addr, sizeof(to_addr.addr));
            emulNet->ENsend(&memberNode->addr, &to_addr, (char*)msg, msg_size);
        }
    }
    //如果没收到回复，随机选取部分节点请求别人帮忙
    for(auto it = status.begin(); it != status.end(); it++) {
        if(it->second == false) {
            int memberlist_size = memberNode->memberList.size();
            vector<int> send_node = randsend(memberNode->memberList.size());
            //节点总数多余SWIM_NUM
            if(memberNode->memberList.size()-1 > SWIM_NUM) {
                for(int i = 0; i < SWIM_NUM; i++) {
                    size_t msg_size = sizeof(MessageHdr) + 2*sizeof(memberNode->addr.addr);
                    char *msg = new char[msg_size];
                    MessageHdr msg_type;
                    msg_type.msgType = HELP;
                    memcpy(msg, &msg_type, sizeof(MessageHdr));
                    Address to_addr;
                    int t = send_node[i];
                    memcpy(&to_addr.addr[0], &memberNode->memberList[t].id, sizeof(int));
                    memcpy(&to_addr.addr[4],  &memberNode->memberList[t].port, sizeof(short));
                    memcpy(msg + sizeof(MessageHdr), &memberNode->addr.addr, sizeof(to_addr.addr));
                    Address it_addr;
                    memcpy(&it_addr.addr[0], &it->first, sizeof(int));
                    short a = 0;
                    memcpy(&it_addr.addr[4],  &a, sizeof(short));
                    memcpy(msg + sizeof(MessageHdr) + sizeof(memberNode->addr.addr), &it_addr.addr, sizeof(memberNode->addr.addr));
                    emulNet->ENsend(&memberNode->addr, &to_addr, (char*)msg, msg_size);
                }
            } 
            //节点总数不多于SWIM_NUM      
            else {
                int memberlist_size = memberNode->memberList.size();
                for(int i = 1; i < memberlist_size; i++) {
                    size_t msg_size = sizeof(MessageHdr) + 2 * sizeof(memberNode->addr.addr);
                    char *msg = new char[msg_size];
                    MessageHdr msg_type;
                    msg_type.msgType = HELP;
                    memcpy(msg, &msg_type, sizeof(MessageHdr));
                    Address to_addr;
                    memcpy(&to_addr.addr[0], &memberNode->memberList[i].id, sizeof(int));
                    memcpy(&to_addr.addr[4],  &memberNode->memberList[i].port, sizeof(short));
                    memcpy(msg + sizeof(MessageHdr), &memberNode->addr.addr, sizeof(to_addr.addr));
                    Address it_addr;
                    memcpy(&it_addr.addr[0], &it->first, sizeof(int));
                    short a = 0;
                    memcpy(&it_addr.addr[4],  &a, sizeof(short));
                    memcpy(msg + sizeof(MessageHdr) + sizeof(memberNode->addr.addr), &it_addr.addr, sizeof(memberNode->addr.addr));
                    emulNet->ENsend(&memberNode->addr, &to_addr, (char*)msg, msg_size);
                }
            }
        }
    }
    //最后再核查,如果还是为false就判定掉线
    for(auto it = status.begin();it != status.end(); it++) {
        if(it->second == false ) {
            Address node_addr;
            memcpy(&node_addr.addr[0], &it->first, sizeof(int));
            short a = 0;
            memcpy(&node_addr.addr[4], &a, sizeof(short));
            for(vector<MemberListEntry>::iterator it2 = memberNode->memberList.end() -1; it2 != memberNode->memberList.begin(); it2--) {
                //加上par->getcurrtime()-it2->timestamp > TREMOVE是担心判断频率过快带来不稳定
                if(it->first == it2->id && par->getcurrtime() - it2->timestamp > TREMOVE) {
                //if(it->first == it2->id) {
                    memberNode->memberList.erase(it2);
                    log->logNodeRemove(&memberNode->addr, &node_addr);
                    continue;
                }
            }
        }
    }
    return;
    #endif

    //检查其余节点有没有谁挂掉了
    for(vector<MemberListEntry>::iterator it = memberNode->memberList.end() -1; \
        it != memberNode->memberList.begin(); it--) {
        //如果时RING协议，并且K值小于总长度，就break，用下面的函数进行检测
        #ifdef RING
        if(memberNode->memberList.size()-1 > RING_NUM) {
            break;
        }
        #endif
        //如果某个节点信息长时间没更新
        if(par->getcurrtime() - it->timestamp >= TREMOVE) {
            Address node_addr;
            memcpy(&node_addr.addr[0], &it->id, sizeof(int));
            memcpy(&node_addr.addr[4], &it->port, sizeof(short));
            //GOSSIP和RING需要把down信息发给其他节点
            #if(!defined GOSSIP) || (!defined RING)
            //把挂掉节点信息发给其他人
            size_t msg_info_size = sizeof(MessageHdr) + 2 * sizeof(memberNode->addr.addr);
            char *msg_info = new char[msg_info_size];
            MessageHdr info_type;
            info_type.msgType = DOWN;
            memcpy(msg_info, &info_type, sizeof(MessageHdr));
            memcpy(msg_info + sizeof(MessageHdr), &memberNode->addr.addr, sizeof(memberNode->addr.addr));
            memcpy(msg_info + sizeof(MessageHdr) + sizeof(memberNode->addr.addr), &node_addr.addr, sizeof(memberNode->addr.addr));
            //发给除了自己之外的节点
            for(int i = 1; i < memberNode->memberList.size(); i++) {
                Address to_addr;
                memcpy(&to_addr.addr[0], &memberNode->memberList[i].id, sizeof(int));
                memcpy(&to_addr.addr[4],  &memberNode->memberList[i].port, sizeof(short));
                emulNet->ENsend(&memberNode->addr, &to_addr, (char*)msg_info, msg_info_size);
            }
            #endif
            log->logNodeRemove(&memberNode->addr, &node_addr);
            memberNode->memberList.erase(it);
        }
    }
    //RING协议专用的检测节点是否down掉的函数
    #ifdef RING
    if(memberNode->memberList.size() > 1 && memberNode->memberList.size()-1 > RING_NUM) {
        vector<MemberListEntry> node_num = memberNode->memberList;
        int cur = memberNode->memberList[0].id; //cur为当前节点的ip
        vector<int> send_num; //用来存储给谁发
        //按照ip地址从大到小排序
        sort(node_num.begin(), node_num.end(),[](MemberListEntry a, MemberListEntry b) {return a.id > b.id;});
        int l; //记录当前节点在按照IP地址排好序的数组中的位置
        for(int i = 0; i < node_num.size(); i++) {
            if(node_num[i].id == cur) {
                l = i;
            }
        }
        //记录当前位置后面的RING_NUM个节点的坐标
        for(int i = 0; i < RING_NUM; i++) {
            int t = (++l)%node_num.size();
            send_num.push_back(t);
        }
        //检测RING_NUM个节点的状态
        for(int i= 0; i < RING_NUM; i++) {
            if(par->getcurrtime() - node_num[send_num[i]].timestamp >= TREMOVE) {
                int down_idd = node_num[send_num[i]].id; //记录第i个节点的ip，并在memberList中找到它的位置
                for(auto it = memberNode->memberList.begin(); it != memberNode->memberList.end(); it++) {
                    if(down_idd == it->id && par->getcurrtime() - it->timestamp >= TREMOVE) {
                        Address node_addr;
                        memcpy(&node_addr.addr[0], &it->id, sizeof(int));
                        memcpy(&node_addr.addr[4], &it->port, sizeof(short));
                        //把挂掉节点信息发给其他人
                        size_t msg_info_size = sizeof(MessageHdr) + 2 * sizeof(memberNode->addr.addr);
                        char *msg_info = new char[msg_info_size];
                        MessageHdr info_type;
                        info_type.msgType = DOWN;
                        memcpy(msg_info, &info_type, sizeof(MessageHdr));
                        memcpy(msg_info + sizeof(MessageHdr), &memberNode->addr.addr, sizeof(memberNode->addr.addr));
                        memcpy(msg_info + sizeof(MessageHdr) + sizeof(memberNode->addr.addr), &node_addr.addr, sizeof(memberNode->addr.addr));
                        //发给除了自己之外的节点
                        for(int i = 1; i < memberNode->memberList.size(); i++) {
                            Address to_addr;
                            memcpy(&to_addr.addr[0], &memberNode->memberList[i].id, sizeof(int));
                            memcpy(&to_addr.addr[4],  &memberNode->memberList[i].port, sizeof(short));
                            emulNet->ENsend(&memberNode->addr, &to_addr, (char*)msg_info, msg_info_size);
                        }
                        log->logNodeRemove(&memberNode->addr, &node_addr);
                        memberNode->memberList.erase(it);
                        break;
                    }
                }
            }
        }
    }
    #endif

    //给活着的节点发送心跳包
    //GOSSIP协议，如果列表长度小于GOSSIP_NUM,则替换为ALL2ALL发送
    #ifdef GOSSIP
    if(memberNode->memberList.size() > 1 && memberNode->memberList.size()-1 > GPSSIP_NUM) {
        vector<int> send_node = randsend(memberNode->memberList.size());
        for(int i = 0; i < GPSSIP_NUM; i++) {
            int memberlist_size = memberNode->memberList.size();
            size_t msg_size = sizeof(MessageHdr) + sizeof(memberNode->addr.addr) + sizeof(long) + \
                sizeof(int) + memberlist_size * sizeof(MemberListEntry);
            char *msg = new char[msg_size];
            MessageHdr msg_type;
            msg_type.msgType = HEARTBEAT;
            memcpy((MessageHdr*)msg, &msg_type, sizeof(MessageHdr));
            Address to_addr;
            int t = send_node[i];
            memcpy(&to_addr.addr[0], &memberNode->memberList[t].id, sizeof(int));
            memcpy(&to_addr.addr[4],  &memberNode->memberList[t].port, sizeof(short));
            memcpy(msg + sizeof(MessageHdr), &memberNode->addr.addr, sizeof(to_addr.addr));
            memcpy(msg + sizeof(MessageHdr) + sizeof(to_addr.addr), &memberNode->heartbeat, sizeof(long));
            memcpy(msg + sizeof(MessageHdr) + sizeof(memberNode->addr.addr) + sizeof(long), \
                &memberlist_size, sizeof(int));
            for(int j = 0; j < memberlist_size; j++) {
                memcpy(msg + sizeof(MessageHdr) + sizeof(memberNode->addr.addr) + sizeof(long) + \
                    sizeof(int) + j * sizeof(MemberListEntry), &memberNode->memberList[j], sizeof(MemberListEntry));
            }
            emulNet->ENsend(&memberNode->addr, &to_addr, (char*)msg, msg_size);
            memberNode->heartbeat++;
        }
        return; //如果满足GOSSIP发送心跳包，就结束了，不会用到后面的发送函数
    }
    #endif
    //RING协议， 如果列表长度小于RING_NUM，就替换为ALL2ALL发送
    #ifdef RING
    if(memberNode->memberList.size() > 1 && memberNode->memberList.size()-1 > RING_NUM) {
        vector<MemberListEntry> node_num = memberNode->memberList;
        int cur = memberNode->memberList[0].id;
        vector<int> send_num;
        sort(node_num.begin(), node_num.end(),[](MemberListEntry a, MemberListEntry b) {return a.id < b.id;});
        int l;
        for(int i = 0; i < node_num.size(); i++) {
            if(node_num[i].id == cur) {
                l = i;
            }
        }
        for(int i = 0; i < RING_NUM; i++) {
            int t = (++l)%node_num.size();
            send_num.push_back(t);
        }
        for(int i = 0; i < RING_NUM; i++) {
            int memberlist_size = memberNode->memberList.size();
            size_t msg_size = sizeof(MessageHdr) + sizeof(memberNode->addr.addr) + sizeof(long) + \
                sizeof(int) + memberlist_size * sizeof(MemberListEntry);
            char *msg = new char[msg_size];
            MessageHdr msg_type;
            msg_type.msgType = HEARTBEAT;
            memcpy((MessageHdr*)msg, &msg_type, sizeof(MessageHdr));
            Address to_addr;
            int t = send_num[i];
            memcpy(&to_addr.addr[0], &node_num[send_num[i]].id, sizeof(int));
            memcpy(&to_addr.addr[4], &node_num[send_num[i]].port, sizeof(short));
            cout << "send to " << node_num[send_num[i]].id << " ";
            memcpy(msg + sizeof(MessageHdr), &memberNode->addr.addr, sizeof(to_addr.addr));
            memcpy(msg + sizeof(MessageHdr) + sizeof(to_addr.addr), &memberNode->heartbeat, sizeof(long));
            memcpy(msg + sizeof(MessageHdr) + sizeof(memberNode->addr.addr) + sizeof(long), \
                &memberlist_size, sizeof(int));
            for(int j = 0; j < memberlist_size; j++) {
                memcpy(msg + sizeof(MessageHdr) + sizeof(memberNode->addr.addr) + sizeof(long) + \
                    sizeof(int) + j * sizeof(MemberListEntry), &memberNode->memberList[j], sizeof(MemberListEntry));
            }
            emulNet->ENsend(&memberNode->addr, &to_addr, (char*)msg, msg_size);
            memberNode->heartbeat++;
        }
        return; //使用了RING发送心跳把，结束
    }
    #endif
    //以ALL2ALL的形式发送心跳包
    if(memberNode->memberList.size() > 1) {
        for(auto it2 = memberNode->memberList.begin()+1; it2 != memberNode->memberList.end(); it2++) {
            int memberlist_size = memberNode->memberList.size();
            size_t msg_size = sizeof(MessageHdr) + sizeof(memberNode->addr.addr) + sizeof(long) + \
                sizeof(int) + memberlist_size * sizeof(MemberListEntry);
            char *msg = new char[msg_size];
            MessageHdr msg_type;
            msg_type.msgType = HEARTBEAT;
            memcpy((MessageHdr*)msg, &msg_type, sizeof(MessageHdr));
            Address to_addr;
            memcpy(&to_addr.addr[0], &it2->id, sizeof(int));
            memcpy(&to_addr.addr[4], &it2->port, sizeof(short));
            memcpy(msg + sizeof(MessageHdr), &memberNode->addr.addr, sizeof(to_addr.addr));
            memcpy(msg + sizeof(MessageHdr) + sizeof(to_addr.addr), &memberNode->heartbeat, sizeof(long));
            memcpy(msg + sizeof(MessageHdr) + sizeof(memberNode->addr.addr) + sizeof(long), \
                &memberlist_size, sizeof(int));
            for(int i = 0; i < memberlist_size; i++) {
                memcpy(msg + sizeof(MessageHdr) + sizeof(memberNode->addr.addr) + sizeof(long) + \
                    sizeof(int) + i * sizeof(MemberListEntry), &memberNode->memberList[i], sizeof(MemberListEntry));
            }
            emulNet->ENsend(&memberNode->addr, &to_addr, (char*)msg, msg_size);
            memberNode->heartbeat++;
        }
    }
    return;
}

/**
 * FUNCTION NAME: isNullAddress
 *
 * DESCRIPTION: Function checks if the address is NULL
 */
int MP1Node::isNullAddress(Address *addr) {
    return (memcmp(addr->addr, NULLADDR, 6) == 0 ? 1 : 0);
}

/**
 * FUNCTION NAME: getJoinAddress
 *
 * DESCRIPTION: Returns the Address of the coordinator
 */
Address MP1Node::getJoinAddress() {
    Address joinaddr;

    memset(&joinaddr, 0, sizeof(Address));
    *(int *)(&joinaddr.addr) = 1;
    *(short *)(&joinaddr.addr[4]) = 0;

    return joinaddr;
}

/**
 * FUNCTION NAME: initMemberListTable
 *
 * DESCRIPTION: Initialize the membership list
 */
void MP1Node::initMemberListTable(Member *memberNode) {
    memberNode->memberList.clear();
}

/**
 * FUNCTION NAME: printAddress
 *
 * DESCRIPTION: Print the Address
 */
void MP1Node::printAddress(Address *addr)
{
    printf("%d.%d.%d.%d:%d \n",  addr->addr[0],addr->addr[1],addr->addr[2],
                                                       addr->addr[3], *(short*)&addr->addr[4]) ;    
}




