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
			//insert heartbeat every so often

	}

	void UFDPeer::endOfRound(const vector<Peer<UFDPeerMessage>*>& _peers) {
		const vector<UFDPeer*> peers = reinterpret_cast<vector<UFDPeer*> const&>(_peers);
		double length = peers[0]->confirmedTrans.size();
		LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()]["latency"].push_back(latency / length);
	}




	//NEED TO EDIT
	void UFDPeer::checkInStrm() {
		while (!inStreamEmpty()) {
			Packet<UFDPeerMessage> newMsg = popInStream();
			
			if (newMsg.getMessage().messageType == "trans") {
				transactions.push_back(newMsg.getMessage());
			}
			else {
				while (receivedMessages.size() < newMsg.getMessage().sequenceNum + 1) {
					receivedMessages.push_back(vector<UFDPeerMessage>());
				}
				receivedMessages[newMsg.getMessage().sequenceNum].push_back(newMsg.getMessage());
			}
		}
	}

	void UFDPeer::sendHeartbeat(){
		UFDPeerMessage msg;
		msg.messageType = "heartbeat";
		msg.peerID = id();
		broadcast(msg);
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
