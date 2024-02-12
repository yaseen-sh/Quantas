#include <iostream>
#include "UFDPeer.hpp"

namespace quantas {

	int UFDPeer::currentTransaction = 1;

	UFDPeer::~UFDPeer() {

	}

	UFDPeer::UFDPeer(const UFDPeer& rhs) : Peer<UFDPeerMessage>(rhs) {
		
	}

	UFDPeer::UFDPeer(long id) : Peer(id) {

	}

	void UFDPeer::performComputation() {
		if (crashed) {
			return;
		}
		if (true)
			checkInStrm();

		if (true)
			checkContents();
		
		if(true){
			if(getRound() % 6 == 0) //6 is arbitrary, but we don't want to send a heartbeat every round. make it smarter later.
				sendHeartbeat();
		}

	}

	void UFDPeer::endOfRound(const vector<Peer<UFDPeerMessage>*>& _peers) {
		const vector<UFDPeer*> peers = reinterpret_cast<vector<UFDPeer*> const&>(_peers);
		double length = peers[0]->confirmedTrans.size();
		LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()]["latency"].push_back(latency / length);
	}

	void UFDPeer::initParameters(const vector<Peer<UFDPeerMessage>*>& _peers, json parameters) {
		const vector<UFDPeer*> peers = reinterpret_cast<vector<UFDPeer*> const&>(_peers);
		for(int i = 0; i < peers.size(); ++i){
			peers[i]->deltap.resize(peers.size());
			for(int j = 0; j < deltap.size(); ++j){
				peers[i]->deltap[j] = -1;
			}
			peers[i]->proposal = rand() % 2; // 0 or 1
			peers[i]->deltap[peers[i]->id()] = peers[i]->proposal;
		}

	}

	//NEED TO EDIT
	void UFDPeer::checkInStrm() {
		while (!inStreamEmpty()) {
			Packet<UFDPeerMessage> newMsg = popInStream();
			
/* 			if (newMsg.getMessage().messageType == "trans") {
				transactions.push_back(newMsg.getMessage());
			} */

			//handle receiving a heartbeat
			if(newMsg.getMessage().messageType == "heartbeat")
				receiveHeartbeat(newMsg);

			//else if its a consensus related message
			else {
				//if we need to push_back
				if(allMessages.size() < getRound()){
					vector<UFDPeerMessage> stuff;
					stuff.push_back(newMsg.getMessage());
					allMessages.push_back(stuff);
				}
				else{
					allMessages[getRound()].push_back(newMsg.getMessage());
				}
			}
				
/* 			else {
				while (receivedMessages.size() < newMsg.getMessage().sequenceNum + 1) {
					receivedMessages.push_back(vector<UFDPeerMessage>());
				}
				receivedMessages[newMsg.getMessage().sequenceNum].push_back(newMsg.getMessage());
			} */
		}
	}

	void UFDPeer::sendHeartbeat(){
		UFDPeerMessage msg;
		msg.messageType = "heartbeat";
		msg.peerID = id();
		broadcast(msg);
	}

	void UFDPeer::checkContents() {

		if(phase == 0){
			
			//set up the message to send deltaP
			UFDPeerMessage msg;
			msg.messageType = "consensus";
			msg.peerID = id();
			msg.roundNumber = iteration;
			msg.deltap = deltap;
			//send the message
			broadcast(msg);
			++phase;
		}
		//phase 1: send message with roundNum, deltaP, ID 
		if(phase == 1){
			
			//CHECK TO SEE IF WE HAVE RECIEVED MESSAGES
			if (PFD.checkReceived(allMessages[iteration])){
				//Initialize deltap
				for(int i = 0; i < deltap.size(); ++i){
					deltap[i] = -1;
				}
				deltap[id()] = proposal;

				//update deltap values and Vp
				for(int k = 0; k = deltap.size(); ++k){
					if(localList[k] == -1)
					for (all messages){
						if(allMessages[i].peerID == k && allMessages[i].deltap[k] != -1){
							localList[k] = allMessages[i].deltap[k];
							deltap[k] = allMessages[i].deltap[k];
						}
					}
				}
				//set up the message to send deltaP
				UFDPeerMessage msg;
				msg.messageType = "consensus";
				msg.peerID = id();
				msg.roundNumber = ++iteration;
				msg.deltap = deltap;
				//send the message
				broadcast(msg);
				//TODO: also message our own vector
			}
			if(iteration == deltap().size())
				++phase;
		}

		//phase 2: send Vp to all processes
		else if(phase == 2){
			//set up message to send Vp
			UFDPeerMessage msg;
			msg.messageType = "consensus";
			msg.peerID = id();
			msg.roundNumber = getRound();
			msg.deltap = localList;

			//send the message
			broadcast(msg);

			//add processing stuff & query FD
			++phase;
		}

		else if (phase == 3){
			decision = decide();
		}
	}

	void decide(){
		//given the values that we have in Vp now, we select the first non default value
		//as per Chandra's algorithm (page 16 of UFD Paper)

		int i = 0;
		while(localList[i] != -1 && i < localList.size()){
			++i;
		}
		return localList[i];

	}

	void UFDPeer::checkContents() {
		if (id() == 0 && status == "pre-prepare") {
			for (int i = 0; i < transactions.size(); i++) {
				bool skip = false;
				for (int j = 0; j < confirmedTrans.size(); j++) {
					if (transactions[i].trans == confirmedTrans[j].trans) {
						skip = true;
						break;
					}
				}
				if (!skip) {
					status = "prepare";
					UFDPeerMessage message = transactions[i];
					message.messageType = "pre-prepare";
					message.Id = id();
					message.sequenceNum = sequenceNum;
					broadcast(message);
					if (receivedMessages.size() < sequenceNum + 1) {
						receivedMessages.push_back(vector<UFDPeerMessage>());
					}
					receivedMessages[sequenceNum].push_back(message);
					break;
				}
			}
		} else if (status == "pre-prepare" && receivedMessages.size() >= sequenceNum + 1) {
			for (int i = 0; i < receivedMessages[sequenceNum].size(); i++) {
				UFDPeerMessage message = receivedMessages[sequenceNum][i];
				if (message.messageType == "pre-prepare") {
					status = "prepare";
					UFDPeerMessage newMsg = message;
					newMsg.messageType = "prepare";
					newMsg.Id = id();
					broadcast(newMsg);
					receivedMessages[sequenceNum].push_back(newMsg);
				}
			}
		}

		if (status == "prepare") {
			int count = 0;
			for (int i = 0; i < receivedMessages[sequenceNum].size(); i++) {
				UFDPeerMessage message = receivedMessages[sequenceNum][i];
				if (message.messageType == "prepare") {
					count++;
				}
			}
			if (count > (neighbors().size() * 2 / 3)) {
				status = "commit";
				UFDPeerMessage newMsg = receivedMessages[sequenceNum][0];
				newMsg.messageType = "commit";
				newMsg.Id = id();
				broadcast(newMsg);
				receivedMessages[sequenceNum].push_back(newMsg);
			}
		}

		if (status == "commit") {
			int count = 0;
			for (int i = 0; i < receivedMessages[sequenceNum].size(); i++) {
				UFDPeerMessage message = receivedMessages[sequenceNum][i];
				if (message.messageType == "commit") {
					count++;
				}
			}
			if (count > (neighbors().size() * 2 / 3)) {
				status = "pre-prepare";
				confirmedTrans.push_back(receivedMessages[sequenceNum][0]);
				latency += getRound() - receivedMessages[sequenceNum][0].roundSubmitted;
				sequenceNum++;
				if (id() == 0) {
					submitTrans(currentTransaction);
				}
				checkContents();
			}
		}

	}

	// void UFDPeer::submitTrans(int tranID) {
	// 	UFDPeerMessage message;
	// 	message.messageType = "trans";
	// 	message.trans = tranID;
	// 	message.Id = id();
	// 	message.roundSubmitted = getRound();
	// 	broadcast(message);
	// 	transactions.push_back(message);
	// 	currentTransaction++;
	// }

	ostream& UFDPeer::printTo(ostream& out)const {
		Peer<UFDPeerMessage>::printTo(out);

		out << id() << endl;
		out << "counter:" << getRound() << endl;

		return out;
	}

	ostream& operator<< (ostream& out, const UFDPeer& peer) {
		peer.printTo(out);
		return out;
	}

	Simulation<quantas::UFDPeerMessage, quantas::UFDPeer>* generateSim() {
        
        Simulation<quantas::UFDPeerMessage, quantas::UFDPeer>* sim = new Simulation<quantas::UFDPeerMessage, quantas::UFDPeer>;
        return sim;
    }
}
