#include <iostream>
#include "UFDPeer.hpp"

namespace quantas {

	//int UFDPeer::currentTransaction = 1;

	UFDPeer::~UFDPeer() {
	}

	UFDPeer::UFDPeer() : Peer<UFDPeerMessage>() {
		
	}

	UFDPeer::UFDPeer(const UFDPeer& rhs) : Peer<UFDPeerMessage>(rhs) {
	}

	UFDPeer::UFDPeer(long id) : Peer(id) {
	}

	void UFDPeer::performComputation() {
		std::cout << "PERFORMCOMPUTATION()" << std::endl;

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
		std::cout << "ENDOFROUND()" << std::endl;
		const vector<UFDPeer*> peers = reinterpret_cast<vector<UFDPeer*> const&>(_peers);
		
		//LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()]["latency"].push_back(latency / length);

	}

	void UFDPeer::initParameters(const vector<Peer<UFDPeerMessage>*>& _peers, json parameters) {
		std::cout << "INIT PARAMETERS" << std::endl;
		const vector<UFDPeer*> peers = reinterpret_cast<vector<UFDPeer*> const&>(_peers);
		for(int i = 0; i < peers.size(); ++i){
			//resize the vectors
			peers[i]->deltap.resize(peers.size());
			peers[i]->localList.resize(peers.size()); 
			std::cout << "resized deltap and localList" << std::endl;
			//initialize vectors to bottom (-1)
			for(int j = 0; j < deltap.size(); ++j){
				peers[i]->deltap[j] = -1;
				peers[i]->localList[j] = -1;
			}
			//set proposal values and update deltap's
			peers[i]->proposal = rand() % 2; // 0 or 1
			peers[i]->deltap[peers[i]->id()] = peers[i]->proposal;
		}

	}

	//NEED TO EDIT
	void UFDPeer::checkInStrm() {
		while (!inStreamEmpty()) {
			Packet<UFDPeerMessage> newMsg = popInStream();

			//handle receiving a heartbeat
			if(newMsg.getMessage().messageType == "heartbeat"){
				std::cout << "checkInStrm heartbeat" << std::endl;
				LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()][getRound()][id()]["Messages"].push_back(newMsg.getMessage().messageType);
				receiveHeartbeat(newMsg.getMessage());
			}
			//we use magic to tell every process to suspect a process when it crashes
			else if (newMsg.getMessage().messageType == "suspect"){
				std::cout << "checkInStrm suspect" << std::endl;
				PFD.suspectProcess(newMsg.getMessage().peerID);
			}
			//else if its a consensus related message
			else if (newMsg.getMessage().messageType == "consensus" && phase == 1) {
				//if we need to push_back
				LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()][getRound()][id()]["Messages"].push_back(newMsg.getMessage().deltap);

				if(allMessages.size() <= iteration){
					std::cout << "checkInStrm newRound" << std::endl;
					vector<UFDPeerMessage> stuff;
					stuff.push_back(newMsg.getMessage());
					allMessages.push_back(stuff);
				}
				else{
					std::cout << "checkInStrm newMessage" << std::endl;
					allMessages[iteration].push_back(newMsg.getMessage());
				}
			}
			//if its phase 2
			else if (newMsg.getMessage().messageType == "consensus" && phase == 2){
				LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()][getRound()][id()]["Messages"].push_back(newMsg.getMessage().deltap);
				std::cout << "checkInStrm and phase 2" << std::endl;
				lastMessages.push_back(newMsg.getMessage());
			}
				
		}
	}

	void UFDPeer::sendHeartbeat(){
		UFDPeerMessage msg;
		msg.messageType = "heartbeat";
		msg.peerID = id();
		broadcast(msg);
	}

	int UFDPeer::decide(){
		//given the values that we have in Vp now, we select the first non default value
		//as per Chandra's algorithm (page 16 of UFD Paper)
		std::cout << "deciding..." << std::endl;
		int i = 0;
		if(!localList.empty()){
			while(i < localList.size() && localList[i] == -1){
				++i;
			}
			std::cout << "decided on " << localList[i] << std::endl;
			return localList[i];
		}
		else return -1;

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
		std::cout << "checkContents phase 0 done" << std::endl;
		//phase 1: send message with roundNum, deltaP, ID 
		if(phase == 1){
			
			//CHECK TO SEE IF WE HAVE RECIEVED MESSAGES
			if(!allMessages.size() <= iteration){

				if (PFD.checkReceived(allMessages[iteration])){
					std::cout << "if checkReceived(allMessages[iteration])" << std::endl;
					//Initialize deltap
					for(int i = 0; i < deltap.size(); ++i){
						deltap[i] = -1;
					}
					deltap[id()] = proposal;

					//update deltap values and Vp
					for(int k = 0; k < deltap.size(); ++k){
						// if(localList[k] == -1){
						// 	for (int i = 0; i < allMessages.size(); ++i){
						// 		if(allMessages[i][k].peerID == k && allMessages[i][k].deltap[k] != -1){
						// 			localList[k] = allMessages[i][k].deltap[k];
						// 			deltap[k] = allMessages[i][k].deltap[k];
						// 		}
						// 	}
						// }
						std::cout << "k: " << k << std::endl;
						if(localList[k] == -1){
							std::cout << "if localList[k] == -1" << std::endl;
							for (int i = 0; i < allMessages.size(); ++i){
								std::cout << "i: " << i << std::endl;
								for(int j = 0; j < allMessages[i].size(); ++j){
									std::cout << "j: " << j << std::endl;
									if(/*allMessages[i][j].peerID == k && */ allMessages[i][j].deltap[k] != -1){
										std::cout << "if allMessages[i][j].deltap[k] != -1" << std::endl;
										localList[k] = allMessages[i][j].deltap[k];
										deltap[k] = allMessages[i][j].deltap[k];
									}
								}
							}
						}
					}
			}
			else std::cout << "allMessages is empty" << std::endl;
			
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
			if(iteration == deltap.size())
				++phase;
		}
		std::cout << "checkContents Phase 1 done" << std::endl;
		//phase 2: send Vp to all processes
		if(phase == 2){
			//set up message to send Vp
			UFDPeerMessage msg;
			msg.messageType = "consensus";
			msg.peerID = id();
			msg.roundNumber = ++iteration;
			msg.deltap = localList;

			//send the message
			broadcast(msg);

			//query the failure detector
			if(PFD.checkReceived(lastMessages)){
				//look for default values and set localList to match them
				for(int i = 0; i < lastMessages.size(); ++i){
					for(int j = 0; j < deltap.size(); ++j){
						if(lastMessages[i].deltap[j] == -1){
							localList[j] = -1;
						}
					}
				}
			}
			
			++phase;
		}
		std::cout << "checkContents Phase 2 done" << std::endl;
		if (phase == 3){
			decision = decide();
		}
		std::cout << "checkContents Phase 3 done" << std::endl;
	}


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
