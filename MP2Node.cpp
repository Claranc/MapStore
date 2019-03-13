/**********************************
 * FILE NAME: MP2Node.cpp
 *
 * DESCRIPTION: MP2Node class definition
 **********************************/
#include "MP2Node.h"

/**
 * constructor
 */
MP2Node::MP2Node(Member *memberNode, Params *par, EmulNet * emulNet, Log * log, Address * address) {
	this->memberNode = memberNode;
	this->par = par;
	this->emulNet = emulNet;
	this->log = log;
	ht = new HashTable();
	this->memberNode->addr = *address;
}

/**
 * Destructor
 */
MP2Node::~MP2Node() {
	delete ht;
	delete memberNode;
}

/**
 * FUNCTION NAME: updateRing
 *
 * DESCRIPTION: This function does the following:
 * 				1) Gets the current membership list from the Membership Protocol (MP1Node)
 * 				   The membership list is returned as a vector of Nodes. See Node class in Node.h
 * 				2) Constructs the ring based on the membership list
 * 				3) Calls the Stabilization Protocol
 */
void MP2Node::updateRing() {
	/*
	 * Implement this. Parts of it are already implemented
	 */
	vector<Node> curMemList;
	bool change = false;

	/*
	 *  Step 1. Get the current membership list from Membership Protocol / MP1
	 */
	curMemList = getMembershipList();

	/*
	 * Step 2: Construct the ring
	 */
	// Sort the list based on the hashCode
	sort(curMemList.begin(), curMemList.end());


	/*
	 * Step 3: Run the stabilization protocol IF REQUIRED
	 */
	// Run stabilization protocol if the hash table size is greater than zero and if there has been a changed in the ring
	if(ring.size() != curMemList.size()) {
		//清空，更新为将最新的列表
		ring.clear();
		for(auto a : curMemList) {
			ring.emplace_back(a);
		}
		findNeighbors();
		if (!ht->isEmpty()){
			stabilizationProtocol();
		}
	}
}

//找到邻居节点
void MP2Node::findNeighbors() {
	int order; //记录自己在hash表中的位置
	for(int i = 0; i < ring.size(); i++) {
		//int id = this->memberNode->memberList.at(i).getid();
		//short port = this->memberNode->memberList.at(i).getport();
		if(this->memberNode->addr.getAddress() == ring[i].nodeAddress.getAddress()) {
			order = i;
		}
	}
	hasMyReplicas.clear();
	haveReplicasOf.clear();
	int next1 = (order+1)%ring.size();
	int next2 = (order+2)%ring.size();
	int pre1 = (order-1)%ring.size();
	int pre2 = (order-2)%ring.size();
	hasMyReplicas.emplace_back(ring[next1]);
	hasMyReplicas.emplace_back(ring[next2]);
	haveReplicasOf.emplace_back(ring[pre1]);
	haveReplicasOf.emplace_back(ring[pre2]);
}

/**
 * FUNCTION NAME: getMemberhipList
 *
 * DESCRIPTION: This function goes through the membership list from the Membership protocol/MP1 and
 * 				i) generates the hash code for each member
 * 				ii) populates the ring member in MP2Node class
 * 				It returns a vector of Nodes. Each element in the vector contain the following fields:
 * 				a) Address of the node
 * 				b) Hash code obtained by consistent hashing of the Address
 */
vector<Node> MP2Node::getMembershipList() {
	unsigned int i;
	vector<Node> curMemList;
	for ( i = 0 ; i < this->memberNode->memberList.size(); i++ ) {
		Address addressOfThisMember;
		int id = this->memberNode->memberList.at(i).getid();
		short port = this->memberNode->memberList.at(i).getport();
		memcpy(&addressOfThisMember.addr[0], &id, sizeof(int));
		memcpy(&addressOfThisMember.addr[4], &port, sizeof(short));
		curMemList.emplace_back(Node(addressOfThisMember));
	}
	return curMemList;
}

/**
 * FUNCTION NAME: hashFunction
 *
 * DESCRIPTION: This functions hashes the key and returns the position on the ring
 * 				HASH FUNCTION USED FOR CONSISTENT HASHING
 *
 * RETURNS:
 * size_t position on the ring
 */
size_t MP2Node::hashFunction(string key) {
	std::hash<string> hashFunc;
	size_t ret = hashFunc(key);
	return ret%RING_SIZE;
}

/**
 * FUNCTION NAME: clientCreate
 *
 * DESCRIPTION: client side CREATE API
 * 				The function does the following:
 * 				1) Constructs the message
 * 				2) Finds the replicas of this key
 * 				3) Sends a message to the replica
 */
void MP2Node::clientCreate(string key, string value) {
	/*
	 * Implement this
	 */	

	vector<Node> save;
	Message msgsend(g_transID++, memberNode->addr, CREATE, key, value);
	//cout<<"ddd = " <<msgsend.value << " "<< sizeof(msgsend) << " " << msgsend.key.size() 
	//    << " " << msgsend.value.size() << endl;
	Message* msgrecv = (Message*) ((char*) &msgsend);
	save = findNodes(key);
	//PRIMARY
	msgsend.replica = PRIMARY;
	//emulNet->ENsend(&memberNode->addr, save[0].getAddress(), (char*)&msgsend, sizeof(Message));
	emulNet->ENsend(&memberNode->addr, save[0].getAddress(), msgsend.toString());
	//cout << msgsend.toString() << endl;
	//SECONDARY
	msgsend.replica = SECONDARY;
	//emulNet->ENsend(&memberNode->addr, save[1].getAddress(), (char*)&msgsend, sizeof(Message));
	emulNet->ENsend(&memberNode->addr, save[1].getAddress(), msgsend.toString());
	//TERTIARY
	msgsend.replica = TERTIARY;
	//emulNet->ENsend(&memberNode->addr, save[2].getAddress(), (char*)&msgsend, sizeof(Message));
	emulNet->ENsend(&memberNode->addr, save[2].getAddress(), msgsend.toString());
}

/**
 * FUNCTION NAME: clientRead
 *
 * DESCRIPTION: client side READ API
 * 				The function does the following:
 * 				1) Constructs the message
 * 				2) Finds the replicas of this key
 * 				3) Sends a message to the replica
 */
void MP2Node::clientRead(string key){
	/*
	 * Implement this
	 */
	vector<Node> save;
	save = findNodes(key);
	Message msgsend(g_transID++, memberNode->addr, READ, key);

	CheckRead p;
	p.key = key;
	p.transId = g_transID;
	p.timestamp = par->getcurrtime();
	ReadQueue.push(p);

	//PRIMARY
	msgsend.replica = PRIMARY;
	emulNet->ENsend(&memberNode->addr, save[0].getAddress(), msgsend.toString());
	//SECONDARY
	msgsend.replica = SECONDARY;
	emulNet->ENsend(&memberNode->addr, save[1].getAddress(), msgsend.toString());
	//TERTIARY
	msgsend.replica = TERTIARY;
	emulNet->ENsend(&memberNode->addr, save[2].getAddress(), msgsend.toString());
}

/**
 * FUNCTION NAME: clientUpdate
 *
 * DESCRIPTION: client side UPDATE API
 * 				The function does the following:
 * 				1) Constructs the message
 * 				2) Finds the replicas of this key
 * 				3) Sends a message to the replica
 */
void MP2Node::clientUpdate(string key, string value){
	/*
	 * Implement this
	 */
	vector<Node> save;
	save = findNodes(key);
	Message msgsend(g_transID++, memberNode->addr, UPDATE, key, value);

	CheckUpdate p;
	p.key = key;
	p.value = value;
	p.transId = g_transID;
	p.timestamp = par->getcurrtime();
	UpdateQueue.push(p);
	//PRIMARY
	msgsend.replica = PRIMARY;
	emulNet->ENsend(&memberNode->addr, save[0].getAddress(), msgsend.toString());
	//SECONDARY
	msgsend.replica = SECONDARY;
	emulNet->ENsend(&memberNode->addr, save[1].getAddress(), msgsend.toString());
	//TERTIARY
	msgsend.replica = TERTIARY;
	emulNet->ENsend(&memberNode->addr, save[2].getAddress(), msgsend.toString());
}

/**
 * FUNCTION NAME: clientDelete
 *
 * DESCRIPTION: client side DELETE API
 * 				The function does the following:
 * 				1) Constructs the message
 * 				2) Finds the replicas of this key
 * 				3) Sends a message to the replica
 */
void MP2Node::clientDelete(string key){
	/*
	 * Implement this
	 */
	vector<Node> save;
	save = findNodes(key);
	Message msgsend(g_transID++, memberNode->addr, DELETE, key);
	//PRIMARY
	msgsend.replica = PRIMARY;
	//emulNet->ENsend(&memberNode->addr, save[0].getAddress(), (char *)&msgsend, sizeof(msgsend));
	emulNet->ENsend(&memberNode->addr, save[0].getAddress(), msgsend.toString());
	//SECONDARY
	msgsend.replica = SECONDARY;
	//emulNet->ENsend(&memberNode->addr, save[1].getAddress(), (char *)&msgsend, sizeof(msgsend));
	emulNet->ENsend(&memberNode->addr, save[1].getAddress(), msgsend.toString());
	//TERTIARY
	msgsend.replica= TERTIARY;
	//emulNet->ENsend(&memberNode->addr, save[2].getAddress(), (char *)&msgsend, sizeof(msgsend));
	emulNet->ENsend(&memberNode->addr, save[2].getAddress(), msgsend.toString());
	
}

/**
 * FUNCTION NAME: createKeyValue
 *
 * DESCRIPTION: Server side CREATE API
 * 			   	The function does the following:
 * 			   	1) Inserts key value into the local hash table
 * 			   	2) Return true or false based on success or failure
 */
bool MP2Node::createKeyValue(string key, string value, ReplicaType replica) {
	/*
	 * Implement this
	 */
	// Insert key, value, replicaType into the hash table
	if (ht->read(key) == "") {
		return ht->create(key,value);
	}
	return false;
}

/**
 * FUNCTION NAME: readKey
 *
 * DESCRIPTION: Server side READ API
 * 			    This function does the following:
 * 			    1) Read key from local hash table
 * 			    2) Return value
 */
string MP2Node::readKey(string key) {
	/*
	 * Implement this
	 */
	// Read key from local hash table and return value
	return ht->read(key);
}

/**
 * FUNCTION NAME: updateKeyValue
 *
 * DESCRIPTION: Server side UPDATE API
 * 				This function does the following:
 * 				1) Update the key to the new value in the local hash table
 * 				2) Return true or false based on success or failure
 */
bool MP2Node::updateKeyValue(string key, string value, ReplicaType replica) {
	/*
	 * Implement this
	 */
	// Update key in local hash table and return true or false
	return ht->update(key, value);
}

/**
 * FUNCTION NAME: deleteKey
 *
 * DESCRIPTION: Server side DELETE API
 * 				This function does the following:
 * 				1) Delete the key from the local hash table
 * 				2) Return true or false based on success or failure
 */
bool MP2Node::deletekey(string key) {
	/*
	 * Implement this
	 */
	// Delete the key from the local hash table
	return ht->deleteKey(key);
}

/**
 * FUNCTION NAME: checkMessages
 *
 * DESCRIPTION: This function is the message handler of this node.
 * 				This function does the following:
 * 				1) Pops messages from the queue
 * 				2) Handles the messages according to message types
 */
void MP2Node::checkMessages() {
	if(ReadQueue.size() > 0) {
		CheckRead p = ReadQueue.front();
		if(par->getcurrtime() - p.timestamp > 10) {
			int id = p.transId;
			if(recv_fail[id] + recv_success[id] <= 2) {
				log->logReadFail(&memberNode->addr, false , p.transId, p.key);
			}
			ReadQueue.pop();
		}
	}

	if(UpdateQueue.size() > 0) {
		CheckUpdate p = UpdateQueue.front();
		if(par->getcurrtime() - p.timestamp > 10) {
			int id = p.transId;
			if(recv_fail[id] + recv_success[id] <= 2) {
				log->logUpdateFail(&memberNode->addr, false , p.transId, p.key, p.value);
			}
			UpdateQueue.pop();
		}
	}
	/*
	 * Implement this. Parts of it are already implemented
	 */
	char * data;
	int size;

	/*
	 * Declare your local variables here
	 */
	Message *res;
	bool result;

	// dequeue all messages and handle them
	while ( !memberNode->mp2q.empty() ) {
		/*
		 * Pop a message from the queue
		 */
		data = (char *)memberNode->mp2q.front().elt;
		size = memberNode->mp2q.front().size;
		memberNode->mp2q.pop();

		string message(data, data + size);

		/*
		 * Handle the message types here
		 */
		//处理客户端发送过来的消息
		vector<string> par;
		SplitString(message, par, "::");
		MessageType msgType = ChooseMessageType(par[2]);
		//cout << msgType << endl;
		Address src_addr(par[1]);
		if(msgType == CREATE) {
			ReplicaType rep = ChooseReplicaType(par[5]);
			result = createKeyValue(par[3], par[4], rep);
			if(result) {
				log->logCreateSuccess(&memberNode->addr, false, stoi(par[0]), par[3], par[4]);
			} else {
				log->logCreateFail(&memberNode->addr, false, stoi(par[0]), par[3], par[4]);
			}
			Message recv_msg(stoi(par[0]), memberNode->addr, REPLY, result);
			string ms = recv_msg.toString();
			ms += "::CREATE::"+par[3]+"::"+par[4];
			emulNet->ENsend(&memberNode->addr, &src_addr, ms);
		} else if(msgType == READ) {
			string output = readKey(par[3]);
			if (output != "") {
				log->logReadSuccess(&memberNode->addr, false , stoi(par[0]), par[3], output);
			} else {
				output = " ";
				log->logReadFail(&memberNode->addr, false , stoi(par[0]), par[3]);
			}
			Message recv_msg(stoi(par[0]), memberNode->addr, output);
			string ms = recv_msg.toString();
			ms += "::" + par[3];
			emulNet->ENsend(&memberNode->addr, &src_addr, ms);
		} else if (msgType == UPDATE) {
			ReplicaType rep = ChooseReplicaType(par[5]);
			result = updateKeyValue(par[3], par[4], rep);
			if(result) {
				log->logUpdateSuccess(&memberNode->addr, false, stoi(par[0]), par[3], par[4]);
			} else {
				log->logUpdateFail(&memberNode->addr, false, stoi(par[0]), par[3], par[4]);
			}
			Message recv_msg(stoi(par[0]), memberNode->addr, REPLY, result);
			string ms = recv_msg.toString();
			ms += "::UPDATE::"+par[3]+"::"+par[4];
			emulNet->ENsend(&memberNode->addr, &src_addr, ms);
		} else if (msgType == DELETE) {
			result = deletekey(par[3]);
			if(result) {
				log->logDeleteSuccess(&memberNode->addr, false, stoi(par[0]), par[3]);
			} else {
				log->logDeleteFail(&memberNode->addr, false, stoi(par[0]), par[3]);
			}
			Message recv_msg(stoi(par[0]), memberNode->addr, REPLY, result);
			string ms = recv_msg.toString();
			ms += "::DELETE::"+par[3];
			emulNet->ENsend(&memberNode->addr, &src_addr, ms);
		} else if (par[2] == "4") {
			if (par[3] == "1") {
				recv_success[stoi(par[0])]++;
			}
			if (par[3] == "0") {
				recv_fail[stoi(par[0])]++;
			}
			if (recv_success[stoi(par[0])] == 2) {
				if(par[4] == "CREATE") {
					//recv_success[stoi(par[0])] = 0;
					log->logCreateSuccess(&memberNode->addr, false, stoi(par[0]), par[5], par[6]);
				} else if (par[4] == "UPDATE") {
					//recv_success[stoi(par[0])] = 0;
					log->logUpdateSuccess(&memberNode->addr, false, stoi(par[0]), par[5], par[6]);
				} else if (par[4] == "DELETE") {
					//recv_success[stoi(par[0])] = 0;
					log->logDeleteSuccess(&memberNode->addr, false, stoi(par[0]), par[5]);
				}
			}
			if (recv_fail[stoi(par[0])] == 2 ) {
				if(par[4] == "CREATE") {
					log->logCreateFail(&memberNode->addr, false, stoi(par[0]), par[5], par[6]);
				} else if (par[4] == "UPDATE") {
					log->logUpdateFail(&memberNode->addr, false, stoi(par[0]), par[5], par[6]);
				} else if (par[4] == "DELETE") {
					log->logDeleteFail(&memberNode->addr, false, stoi(par[0]), par[5]);
				}
			}
			/*if (recv_success[stoi(par[0])] == 1 && par[4] == "UPDATE") {
				log->logUpdateFail(&memberNode->addr, false, stoi(par[0]), par[5], par[6]);
			}*/
		} else if (par[2] == "5") {
			if (par[3] == " ") {
				recv_fail[stoi(par[0])]++;
			} else {
				recv_success[stoi(par[0])]++;
			}
			if (recv_success[stoi(par[0])] == 2) {
				//recv_success[stoi(par[0])] = 0;
				log->logReadSuccess(&memberNode->addr, false, stoi(par[0]), par[4], par[3]);
			}
			if (recv_fail[stoi(par[0])] == 2 ) {
				log->logReadFail(&memberNode->addr, false , stoi(par[0]), par[4]);
			}
			/*if (recv_success[stoi(par[0])] == 1 ) {
				log->logReadFail(&memberNode->addr, false , stoi(par[0]), par[4]);
			}*/
		}
	}

	/*
	 * This function should also ensure all READ and UPDATE operation
	 * get QUORUM replies
	 */
}

/**
 * FUNCTION NAME: findNodes
 *
 * DESCRIPTION: Find the replicas of the given keyfunction
 * 				This function is responsible for finding the replicas of a key
 */
vector<Node> MP2Node::findNodes(string key) {
	size_t pos = hashFunction(key);
	vector<Node> addr_vec;
	if (ring.size() >= 3) {
		// if pos <= min || pos > max, the leader is the min
		if (pos <= ring.at(0).getHashCode() || pos > ring.at(ring.size()-1).getHashCode()) {
			addr_vec.emplace_back(ring.at(0));
			addr_vec.emplace_back(ring.at(1));
			addr_vec.emplace_back(ring.at(2));
		}
		else {
			// go through the ring until pos <= node
			for (int i=1; i<ring.size(); i++){
				Node addr = ring.at(i);
				if (pos <= addr.getHashCode()) {
					addr_vec.emplace_back(addr);
					addr_vec.emplace_back(ring.at((i+1)%ring.size()));
					addr_vec.emplace_back(ring.at((i+2)%ring.size()));
					break;
				}
			}
		}
	}
	return addr_vec;
}

/**
 * FUNCTION NAME: recvLoop
 *
 * DESCRIPTION: Receive messages from EmulNet and push into the queue (mp2q)
 */
bool MP2Node::recvLoop() {
    if ( memberNode->bFailed ) {
    	return false;
    }
    else {
    	return emulNet->ENrecv(&(memberNode->addr), this->enqueueWrapper, NULL, 1, &(memberNode->mp2q));
    }
}

/**
 * FUNCTION NAME: enqueueWrapper
 *
 * DESCRIPTION: Enqueue the message from Emulnet into the queue of MP2Node
 */
int MP2Node::enqueueWrapper(void *env, char *buff, int size) {
	Queue q;
	return q.enqueue((queue<q_elt> *)env, (void *)buff, size);
}
/**
 * FUNCTION NAME: stabilizationProtocol
 *
 * DESCRIPTION: This runs the stabilization protocol in case of Node joins and leaves
 * 				It ensures that there always 3 copies of all keys in the DHT at all times
 * 				The function does the following:
 *				1) Ensures that there are three "CORRECT" replicas of all the keys in spite of failures and joins
 *				Note:- "CORRECT" replicas implies that every key is replicated in its two neighboring nodes in the ring
 */
void MP2Node::stabilizationProtocol() {
	/*
	 * Implement this
	 */
	for (map<string, string>::iterator it=ht->hashTable.begin(); it != ht->hashTable.end(); ++it) {
		 this->clientCreate(it->first, it->second);
	}
}

void MP2Node::SplitString(const string& s, vector<string>& v, const string& c) {
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

MessageType MP2Node::ChooseMessageType(string a) {
	MessageType result;
	if(a == "0") {
		result = CREATE;
	} else if ( a == "1") {
		result = READ;
	} else if(a == "2") {
		result = UPDATE;
	} else if(a == "3") {
		result = DELETE;
	} else if(a == "4") {
		result == REPLY;
	} else if (a == "5") {
		result == READREPLY;
	}
	return result;
}

ReplicaType MP2Node::ChooseReplicaType(string a) {
	ReplicaType result;
	if(a == "0") {
		result = PRIMARY;
	} else if (a == "1") {
		result = SECONDARY;
	} else if(a == "2") {
		result = TERTIARY;
	}
	return result;
}