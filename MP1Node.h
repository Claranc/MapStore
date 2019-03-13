/**********************************
 * FILE NAME: MP1Node.cpp
 *
 * DESCRIPTION: Membership protocol run by this Node.
 * 				Header file of MP1Node class.
 **********************************/

#ifndef _MP1NODE_H_
#define _MP1NODE_H_

/*
**保留GOSSIP这一行代码则使用GOSSIP协议，保留RING这一行则为RING协议，保留SWIM这一行代表SWIM协议
**三行都注释掉则使用ALL2ALL（不要把两行都保留，谢谢，防御式编程，，，不存在的）
**/
//#define GOSSIP
//#define RING
//#define SWIM

#include "stdincludes.h"
#include "Log.h"
#include "Params.h"
#include "Member.h"
#include "EmulNet.h"
#include "Queue.h"

/**
 * Macros
 */
#define TREMOVE 20
#define TFAIL 5

/*
 * Note: You can change/add any functions in MP1Node.{h,cpp}
 */

/**
 * Message Types
 */
enum MsgTypes{
	JOINREQ, //新节点向老大发送消息
	JOINREP, //老大向新节点回复消息
	HEARTBEAT, //普通心跳
	DOWN,
	UP,
	#ifdef SWIM
	HELP, //找人帮忙发送
	PLEASEGOHOME, //帮别人发送
	IWILLGOHOME, //间接收到别人的包，准备回复
	HEWILLGOHOME, //帮别人发送的包收到类回复
	RESPONSE, //收到SWIM普通的PING包
	SENDPING //发送PING 包
	#endif
};

/**
 * STRUCT NAME: MessageHdr
 *
 * DESCRIPTION: Header and content of a message
 */
typedef struct MessageHdr {
	enum MsgTypes msgType;
}MessageHdr;

/**
 * CLASS NAME: MP1Node
 *
 * DESCRIPTION: Class implementing Membership protocol functionalities for failure detection
 */
class MP1Node {
private:
	EmulNet *emulNet;
	Log *log;
	Params *par;
	Member *memberNode;
	char NULLADDR[6];
	map<int,bool> status;

public:
	MP1Node(Member *, Params *, EmulNet *, Log *, Address *);
	Member * getMemberNode() {
		return memberNode;
	}
	int recvLoop();
	static int enqueueWrapper(void *env, char *buff, int size);
	void nodeStart(char *servaddrstr, short serverport);
	int initThisNode(Address *joinaddr);
	int introduceSelfToGroup(Address *joinAddress);
	int finishUpThisNode();
	void nodeLoop();
	void checkMessages();
	bool recvCallBack(void *env, char *data, int size);
	void nodeLoopOps();
	int isNullAddress(Address *addr);
	Address getJoinAddress();
	void initMemberListTable(Member *memberNode);
	void printAddress(Address *addr);
	virtual ~MP1Node();
	void savetoMemberlist(int id, short port, long timestamp);
	vector<int> randsend(int num); //此函数GOSSIP专用
};

#endif /* _MP1NODE_H_ */
