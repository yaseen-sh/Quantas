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
		//std::cout << "PERFORMCOMPUTATION()" << std::endl;

		if (crashed) {
			std::cout << id() << " is crashed" << std::endl;
			return;
		}
		else if (true){
			checkInStrm();

			checkContents();

			if(getRound() % 6 == 0) //6 is arbitrary, but we don't want to send a heartbeat every round. make it smarter later.
				sendHeartbeat();
		}

	}

	void UFDPeer::endOfRound(const vector<Peer<UFDPeerMessage>*>& _peers) {
		//std::cout << "ENDOFROUND " << getRound() << std::endl;
		const vector<UFDPeer*> peers = reinterpret_cast<vector<UFDPeer*> const&>(_peers);

		//not working for some reason
		if(crashRound != -1 && getRound() == crashRound - 1){
			std::cout << "CRASHING " << id() << std::endl;
			crash(); //crash the process this round
		}
		//LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()]["latency"].push_back(latency / length);

	}

	void UFDPeer::initParameters(const vector<Peer<UFDPeerMessage>*>& _peers, json parameters) {
		//std::cout << "INIT PARAMETERS" << std::endl;
		const vector<UFDPeer*> peers = reinterpret_cast<vector<UFDPeer*> const&>(_peers);
		
		//set up for crashing processes
		int crashCount = parameters["toCrash"];
		std::cout << crashCount << " peers to crash" <<  std::endl;

		vector<int> peersToCrash; peersToCrash.resize(crashCount);
		for(int i = 0; i < crashCount; ++i){ 						//toCrash is parameters[0]
			int index = rand() % peers.size(); 						//select a peer to crash

			while (peersToCrash.find(index) != peersToCrash.end()) {//if it's a unique index
				index = rand() % peers.size();
			}
			peersToCrash[i] = index;								//now add it to the list
 			int roundToCrash = rand() % int(parameters["totalRounds"]); 	//when to crash
			peers[index]->crashRound = roundToCrash; 				//tell process when to crash
			std::cout << "Peer " << index << " to crash in " << peers[index]->crashRound << std::endl;

		}

		for(int i = 0; i < peers.size(); ++i){
			//resize the vectors
			peers[i]->deltap.resize(peers.size());
			peers[i]->localList.resize(peers.size()); 
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
				//std::cout << "checkInStrm heartbeat" << std::endl;
				if(id() == 0)
					LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()][getRound()][id()]["Messages"].push_back(newMsg.getMessage().messageType);
				receiveHeartbeat(newMsg.getMessage());
			}
			//we use magic to tell every process to suspect a process when it crashes
			else if (newMsg.getMessage().messageType == "suspect"){
				std::cout << "checkInStrm suspect" << std::endl;
				if(id() == 0) LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()][getRound()][id()]["Messages"].push_back(newMsg.getMessage().messageType);
				PFD.suspectProcess(newMsg.getMessage().peerID);
			}
			//else if its a consensus related message
			else if (newMsg.getMessage().messageType == "consensus") {
				//if we need to push_back
				if(id() == 0)
					LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()][getRound()][id()]["Messages"].push_back(newMsg.getMessage().deltap);

				if(allMessages.size() <= iteration){
					//std::cout << "checkInStrm newRound" << std::endl;
					vector<UFDPeerMessage> stuff;
					stuff.push_back(newMsg.getMessage());
					allMessages.push_back(stuff);
				}
				else{
					//std::cout << "checkInStrm newMessage" << std::endl;
					allMessages[iteration].push_back(newMsg.getMessage());
				}
			}
			//if its phase 2
			else if (newMsg.getMessage().messageType == "consensus2"){
				if(id() == 0)
					LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()][getRound()][id()]["Messages2"].push_back(newMsg.getMessage().deltap);
				//std::cout << "checkInStrm and phase 2" << std::endl;
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
		//std::cout << "deciding..." << std::endl;
		int value = -1;
	
		for(int i = 0; i < localList.size(); ++i){
			if(localList[i] != -1){
				value = localList[i];
				break;
			}
		}
		std::cout << "decided on " << value << std::endl;
		return value;
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

			//send to self
			vector<UFDPeerMessage> stuff;
			stuff.push_back(msg);
			allMessages.push_back(stuff);
			
			++phase;
			std::cout << "checkContents phase 0 done" << std::endl;
		}
		
		//phase 1: send message with roundNum, deltaP, ID 
		if(phase == 1){
			
			//CHECK TO SEE IF WE HAVE RECIEVED MESSAGES
			if(!allMessages.size() <= iteration){

				if (PFD.checkReceived(allMessages[iteration])){
					//Initialize deltap
					for(int i = 0; i < deltap.size(); ++i){
						deltap[i] = -1;
					}
					deltap[id()] = proposal;

					//update deltap values and Vp
					for(int k = 0; k < deltap.size(); ++k){
						if(localList[k] == -1){
							for (int i = 0; i < allMessages.size(); ++i){
								for(int j = 0; j < allMessages[i].size(); ++j){
									if(allMessages[i][j].deltap[k] != -1){
										localList[k] = allMessages[i][j].deltap[k];
										deltap[k] = allMessages[i][j].deltap[k];
									}
								}
							}
						}
					}


					//starting next iteration
					if(iteration < deltap.size() -1){
						//set up the message to send deltaP
						UFDPeerMessage msg;
						msg.messageType = "consensus";
						msg.peerID = id();
						msg.roundNumber = ++iteration;
						msg.deltap = deltap;
						//send the message
						broadcast(msg);
						//send to self
						if(allMessages.size() <= iteration){
							vector<UFDPeerMessage> stuff;
							stuff.push_back(msg);
							allMessages.push_back(stuff);
						}
						else{
							allMessages[iteration].push_back(msg);
						}
					}

					else {
						++phase;
						//set up message to send Vp (phase 2)
						UFDPeerMessage msg;
						msg.messageType = "consensus2";
						msg.peerID = id();
						msg.deltap = localList;
						//send the message
						broadcast(msg);

						//send to self
						lastMessages.push_back(msg);
					}
				}
			}

			std::cout << "checkContents Phase 1 done" << std::endl;		
		}
		
		//phase 2: send Vp to all processes
		if(phase == 2){			
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
				++phase;
			}
			std::cout << "checkContents Phase 2 done" << std::endl;	
		}
		
		if (phase == 3){
			decision = decide();
			++phase;
			std::cout << "Peer " << id() << " checkContents Phase 3 done, decided on " << decision << " in round " << getRound() << std::endl;
			LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()][getRound()][id()]["Decides"].push_back(decision);
		}
		
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
