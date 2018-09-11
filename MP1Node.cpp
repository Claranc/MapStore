/**********************************
 * FILE NAME: MP1Node.cpp
 *
 * DESCRIPTION: Membership protocol run by this Node.
 * 				Definition of MP1Node class functions.
 **********************************/

#include "MP1Node.h"

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

    //将自己的信息存储进列表中
    MemberListEntry own_info;
    int own_id = memberNode->addr.addr[0];
    short own_port = memberNode->addr.addr[4];
    own_info.id = own_id;
    own_info.port = own_port;
    own_info.heartbeat = 0;
    own_info.timestamp = par->getcurrtime();
    memberNode->memberList.push_back(own_info);
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
    memcpy(&src_addr,(char*)(data+sizeof(MessageHdr)), sizeof(memberNode->addr.addr));
    //获取heartbeat
    long heartbeat;
    memcpy(&heartbeat, (char*)(data+sizeof(MessageHdr)+sizeof(memberNode->addr.addr)), sizeof(long));
    //处理消息，如果消息类型为JOINREQ（收到新节点加入消息）
    if(0 == rev_type->msgType) {
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
        int src_id = src_addr.addr[0];
        short src_port = src_addr.addr[4];
        MemberListEntry new_node;
        new_node.id = src_id;
        new_node.port = src_port;
        new_node.heartbeat = heartbeat;
        new_node.timestamp = par->getcurrtime();
        memberNode->memberList.push_back(new_node);
        log->logNodeAdd(&memberNode->addr, &src_addr);
    }
    //JOINREP(收到老大发回来的信息)
    else if(1 == rev_type->msgType) {
        //将老大发送回来的节点信息添加进列表
        int memberlist_size;
        memcpy(&memberlist_size, (char*)data + sizeof(MessageHdr) + sizeof(memberNode->addr.addr), \
            sizeof(int));
        for(int i = 0; i < memberlist_size; i++) {
            MemberListEntry temp;
            memcpy(&temp, data + sizeof(MessageHdr) + sizeof(memberNode->addr.addr) + sizeof(int) + \
                i*sizeof(MemberListEntry), sizeof(MemberListEntry));
            memberNode->memberList.push_back(temp);
            Address node_addr;
            memcpy(&node_addr.addr[0], &temp.id, sizeof(int));
            memcpy(&node_addr.addr[4], &temp.port, sizeof(short));
            log->logNodeAdd(&memberNode->addr, &node_addr);
        }  
    }
    //HEARTBEAT（正常心跳包）
    else if(2 == rev_type->msgType) {
        int src_id = src_addr.addr[0];
        short src_port = src_addr.addr[4];
        //判断是否是新的节点，如果是就存储，是老节点就更新
        bool flag = 0;
        for(size_t i = 0; i < memberNode->memberList.size(); i++) {
            if(memberNode->memberList[i].id == src_id && memberNode->memberList[i].port == src_port) {
                flag = 1;
                memberNode->memberList[i].heartbeat = heartbeat;
                memberNode->memberList[i].timestamp = par->getcurrtime();
            }
        }
        if(!flag) {
            //将新节点的信息存储起来
            MemberListEntry new_node;
            new_node.id = src_id;
            new_node.port = src_port;
            new_node.heartbeat = heartbeat;
            new_node.timestamp = par->getcurrtime();
            memberNode->memberList.push_back(new_node);
            log->logNodeAdd(&memberNode->addr, &src_addr);
        }
        //处理数据包中的列表信息，进行更新
        int memberlist_size;
        memcpy(&memberlist_size, (char*)data + sizeof(MessageHdr) + sizeof(memberNode->addr.addr) + sizeof(long), sizeof(int));
        for(int i = 0; i < memberlist_size; i++) {
            MemberListEntry temp;
            memcpy(&temp, data + sizeof(MessageHdr) + sizeof(memberNode->addr.addr) + sizeof(int) + sizeof(long) +\
                i*sizeof(MemberListEntry), sizeof(MemberListEntry));
            bool flag_2 = 0;
            for(auto &a : memberNode->memberList) {
                if(temp.id == a.id && temp.port == a.port) {
                    flag_2 =1;
                    if(temp.timestamp > a.timestamp) {
                        a.timestamp = temp.timestamp;
                        a.heartbeat = temp.heartbeat;
                    }
                }
            }
            if(!flag_2) {
                memberNode->memberList.push_back(temp);
                Address node_addr;
                memcpy(&node_addr.addr[0], &temp.id, sizeof(int));
                memcpy(&node_addr.addr[4], &temp.port, sizeof(short));
                log->logNodeAdd(&memberNode->addr, &node_addr);
            }
        }
    }
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
    //检查其余节点有没有谁挂掉了
    for(vector<MemberListEntry>::iterator it = memberNode->memberList.end() -1; \
        it != memberNode->memberList.begin(); it--) {
        if(par->getcurrtime() - it->timestamp >= TREMOVE) {
            Address node_addr;
            memcpy(&node_addr.addr[0], &it->id, sizeof(int));
            memcpy(&node_addr.addr[4], &it->port, sizeof(short));
            log->logNodeRemove(&memberNode->addr, &node_addr);
            memberNode->memberList.erase(it);
        }
    }
    //给活着的节点发送信息
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
        memcpy(msg + sizeof(MessageHdr), &to_addr.addr, sizeof(to_addr.addr));
        memcpy(msg + sizeof(MessageHdr) + sizeof(to_addr.addr), &it2->heartbeat, sizeof(long));
        memcpy(msg + sizeof(MessageHdr) + sizeof(memberNode->addr.addr) + sizeof(long), \
            &memberlist_size, sizeof(int));
        for(int i = 0; i < memberlist_size; i++) {
            memcpy(msg + sizeof(MessageHdr) + sizeof(memberNode->addr.addr) + sizeof(long) + \
                sizeof(int) + i * sizeof(MemberListEntry), &memberNode->memberList[i], sizeof(MemberListEntry));
        }
        emulNet->ENsend(&memberNode->addr, &to_addr, (char*)msg, msg_size);
        memberNode->heartbeat++;
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
