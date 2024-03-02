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
		//std::cout << "PERFORMCOMPUTATION()" << std::endl
		std::cout << "Peer " << id() << " " << "perform computation" << std::endl;
		//PFD.printProcessList(); 
				
		if(getRound() == crashRound){
			std::cout << "CRASHING " << id() << std::endl;
			crash(); //crash the process this round
			//LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()][getRound()]["Process " + std::to_string(id()) + " Crashes in round " + std::to_string(getRound())].push_back("suspected by all");

			return;
		}

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
		std::cout << "END OF ROUND " << getRound() << std::endl;
		const vector<UFDPeer*> peers = reinterpret_cast<vector<UFDPeer*> const&>(_peers);

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

			//if it's a unique index
			while (find(peersToCrash.begin(), peersToCrash.begin() + i, index) != peersToCrash.begin() + i) {
				index = rand() %  peers.size();
			}
			peersToCrash[i] = index;									//now add it to the list
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

			for(int j = 0; j < peers.size(); ++j){
				//update its tolerance
				peers[i]->PFD.updateTolerance(j, parameters["tolerance"]);
				//update processList for all PFDs
				peers[i]->PFD.processList.insert(std::make_pair(j, std::make_pair(0, false)));
			}
		}

	}


	void UFDPeer::checkInStrm() {
		while (!inStreamEmpty()) {
			Packet<UFDPeerMessage> newMsg = popInStream();

			//handle receiving a heartbeat
			if(newMsg.getMessage().messageType == "heartbeat"){
				//std::cout << "checkInStrm heartbeat" << std::endl;
				//if(id() == 0)
					//LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()][getRound()][id()]["Message from " +std::to_string(newMsg.getMessage().peerID) + ", iteration " + std::to_string(iteration)  + ", round " + std::to_string(getRound())].push_back(newMsg.getMessage().messageType);
				receiveHeartbeat(newMsg.getMessage());
				std::cout << "heartbeat from " + std::to_string(newMsg.getMessage().peerID) << std::endl;
			}
			//we use magic to tell every process to suspect a process when it crashes
			else if (newMsg.getMessage().messageType == "suspect"){
				std::cout << "checkInStrm suspect" << std::endl;
				//if(id() == 0) LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()][getRound()][id()]["Message from " +std::to_string(newMsg.getMessage().peerID) + ", iteration " + std::to_string(iteration),  + ", round " + std::to_string(getRound())].push_back(newMsg.getMessage().messageType);
				PFD.suspectProcess(newMsg.getMessage().peerID);
			}
			//else if its a consensus related message in phase 1
			else if (newMsg.getMessage().messageType == "consensus") {
				//if we need to push_back
				//if(id() == 0)
					//LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()][getRound()][id()]["Message from " + std::to_string(newMsg.getMessage().peerID) + ", iteration " + std::to_string(newMsg.getMessage().iteration)  + ", round " + std::to_string(getRound())].push_back(newMsg.getMessage().deltap);
				std::cout << "consensus message from " + std::to_string(newMsg.getMessage().peerID) << std::endl;
			
				while(allMessages.size() <= newMsg.getMessage().iteration){
					//std::cout << "checkInStrm newRound" << std::endl;
					vector<UFDPeerMessage> stuff;
					allMessages.push_back(stuff);
				}
				//std::cout << "checkInStrm newMessage" << std::endl;
				allMessages[newMsg.getMessage().iteration].push_back(newMsg.getMessage());
				receiveHeartbeat(newMsg.getMessage()); //regular messages should still serve the effect of a heartbeat
			}
			//if its a phase 2 message
			else if (newMsg.getMessage().messageType == "consensus2"){
				//if(id() == 0)
					//LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()][getRound()][id()]["Messages2 from " + std::to_string(newMsg.getMessage().peerID) + ", iteration " + std::to_string(iteration) + ", round " + std::to_string(getRound())].push_back(newMsg.getMessage().deltap);
				//std::cout << id () << " checkInStrm and phase 2" << std::endl;
				lastMessages.push_back(newMsg.getMessage());
				receiveHeartbeat(newMsg.getMessage()); //regular messages should still serve the effect of a heartbeat
			}
				
		}
	}

	void UFDPeer::sendHeartbeat(){
		UFDPeerMessage msg;
		msg.messageType = "heartbeat";
		msg.roundNumber = getRound(); //iteration;
		msg.peerID = id();
		broadcast(msg);
	}

	int UFDPeer::decide(){
		//given the values that we have in Vp now, we select the first non default value
		//as per Chandra's algorithm (page 16 of UFD Paper)
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
			msg.iteration = iteration;
			msg.roundNumber = getRound(); //iteration;
			msg.deltap = deltap;
			//send the message
			broadcast(msg);

			//send to self
			vector<UFDPeerMessage> stuff;
			stuff.push_back(msg);
			allMessages.push_back(stuff);
			
			++phase;
			//std::cout << "checkContents phase 0 done" << std::endl;
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
					if(iteration < deltap.size() - 1){
						//set up the message to send deltaP
						UFDPeerMessage msg;
						msg.messageType = "consensus";
						msg.peerID = id();
						msg.roundNumber = getRound();
						msg.iteration = ++iteration;
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
					}
					std::cout << "checkContents Phase 1 done" << std::endl;	
				}
			}
			
		}
		
		//phase 2 (second half)
		else if(phase == 2){			

			//set up message to send Vp (phase 2)
			UFDPeerMessage msg;
			msg.messageType = "consensus2";
			msg.roundNumber = getRound();
			++iteration;
			msg.peerID = id();
			msg.deltap = localList;
			//send the message
			broadcast(msg);

			//send to self
			lastMessages.push_back(msg);
			
			//query the failure detector
			if(PFD.checkReceived(lastMessages)){
				std::cout << "checkReceived lastMessages" << std::endl;
				//look for default values and set localList to match them
				for(int i = 0; i < lastMessages.size(); ++i){
					for(int j = 0; j < deltap.size(); ++j){
						if(lastMessages[i].deltap[j] == -1){
							localList[j] = -1;
						}
					}
				}
				++phase;
				std::cout << "checkContents Phase 2 done" << std::endl;	
			}
			else std::cout << "Phase 2 checkReceived failed" << std::endl;
		}
		
		else if (phase == 3){
			decision = decide();
			++phase;
			std::cout << "Peer " << id() << " Phase 3 done, decided on " << decision << " in round " << getRound() << std::endl;

			//LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()][getRound()]["Process " + std::to_string(id()) + " Decides in round " + std::to_string(getRound())].push_back(decision);
			LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()].push_back(1);

		}

		else {
			LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()].push_back(0);

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
