#ifndef UFDPeer_hpp
#define UFDPeer_hpp

#include <deque>
#include "../Common/Peer.hpp"
#include "../Common/Simulation.hpp"

namespace quantas{

    struct UFDPeerMessage {

        // int 				Id = -1; // node who sent the message
        // int					trans = -1; // the transaction id
        // int                 sequenceNum = -1;
        // string              messageType = ""; // phase
        // int                 roundSubmitted;

        int                     roundNumber = -1;
        int                     iteration = -1;
        int                     peerID = -1;
        vector<int>             deltap; // new values recieved in last round
        string messageType = ""; //heartbeat or consensus

    };

    class PerfectFailureDetector{
    public:
        PerfectFailureDetector() : timeTolerance(10) {}
        PerfectFailureDetector(int a) : timeTolerance(a) {}
        //copy constructor with delegation
        PerfectFailureDetector(PerfectFailureDetector& rhs) : PerfectFailureDetector(rhs.timeTolerance){
            for(const auto& e : rhs.processList){
                processList.insert(e);
            }
        }
        // when we want to suspect a process
        void                    suspectProcess(int peerID){
           auto found = processList.find(peerID);
           //if it's in the list, of course
           if (found != processList.end()) {
                found->second.second = true; //set flag to true
           }
        }
        // if we get a message after suspected a process, we update our timeTolerance
        void                    updateTolerance(int peerID, int roundNum){
            int oldRound = processList[peerID].first;
            int newNum = roundNum - oldRound; //find the difference between last detection and now
            if(timeTolerance < newNum) //only change the tolerance if it's an increase
                timeTolerance = roundNum - oldRound;

            //also need to change the flag to false because no longer suspected
            processList[peerID].second = false;
        }
        // if we receive a heartbeat message from a process
        void                    receiveHeartbeat(UFDPeerMessage msg){
            
            //look for the process in our suspectList
            auto found = processList.find(msg.peerID);

            //if somehow we don't have record of this process, insert it
            if(found == processList.end()) 
                processList.insert(std::make_pair(msg.peerID, std::make_pair(msg.roundNumber, false)));

            //if the process is in the map
            else{
                // and if the process is mistakenly suspected
                if((msg.roundNumber - found->second.first) > timeTolerance
                    && found->second.second == true)
                    updateTolerance(msg.peerID, msg.roundNumber);

                // then (regardless) we update the map value for most recent heartbeat
                found->second.first = msg.roundNumber;
                found->second.second = false; //don't suspect it anymore
            }


        }
        //checks if all non suspected processes have a msg in vec
        bool                    checkReceived(vector<UFDPeerMessage> vec){
            for(const auto& a : processList){
                //if we DONT suspect it
                if(!a.second.second){
                    bool found = false;
                    //check if we received a message from it
                    for(int i = 0; i < vec.size(); ++i){
                        if(vec[i].peerID == a.first){
                            found = true;
                            break;
                        }                        
                    }
                    if(!found) {
                        return false;
                    }
                }
            }
            return true;

        }

        void                    printProcessList(){
            std::cout << "================================================================" << std::endl;
            std::cout << "ProcessList contents: " <<  std::endl;
            for (const auto & p : processList){
                std::cout << "ID: " << p.first << " " << " Last heard in round " << p.second.first;
                if(p.second.second) std::cout << " SUSPECT" << std::endl;
                else std::cout << " NONSUSPECT" << std::endl;
            }
        }
    //private:
        // maintain a list of processes, last round we receive heartbeat, suspected (T) or not (F)
        std::map<int, std::pair<int, bool>>   processList;
        // how many rounds before we suspect a process?
        int                      timeTolerance;
    };


    class UFDPeer : public Peer<UFDPeerMessage>{
    public:
        // methods that must be defined when deriving from Peer
        UFDPeer                             ();
        UFDPeer                             (long);
        UFDPeer                             (const UFDPeer&);
        ~UFDPeer                            ();

        //= crashing related variables
        int                                 crashRound = -1; //if -1 dont crash, else crash at given round

        //= if we crash dont do anything
        bool                                crashed = false;

        //= vector of vectors of messages that have been received
        vector<vector<UFDPeerMessage>>      allMessages;

        //= vector of messages from phase 2
        vector<UFDPeerMessage>              lastMessages;

        //= vector of process's local list of values (Vp)
        vector<int>                         localList;

        //= 
        vector<int>                         deltap;

        //= local perfect failure detector
        PerfectFailureDetector              PFD; 

        //= status to indicate phase 1-3
        int                                 phase = 0;

        //= value to propose
        int                                 proposal = -1;
        
        //= store the decision value
        int                                 decision = -1;

        //= phase 1 keep track var
        int                                 iteration = 0;



        //= different from regular messages, we need to send a heartbeat
        void                  sendHeartbeat();

        //= make our local failure detector receive a heartbeat
        void                  receiveHeartbeat(UFDPeerMessage msg){
            PFD.receiveHeartbeat(msg);
        }

        //= function to make the process crash (not working)
        void                    crash(){
            //create a special suspect message to others and broadcast
            UFDPeerMessage msg;
            msg.peerID = id();
            msg.messageType = "suspect";
            broadcast(msg);
            std::cout << "Crashed " << id() << std::endl;
            crashed = true;
            PFD.suspectProcess(id()); //suspect itself
        }

        //= select the first non default value as our decision
        int                    decide();

        // perform one step of the Algorithm with the messages in inStream
        void                 performComputation();
        // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
        void                 endOfRound(const vector<Peer<UFDPeerMessage>*>& _peers);

        // additional method that have default implementation from Peer but can be overwritten
        void                 log()const { printTo(*_log); };
        ostream&             printTo(ostream&)const;
        friend ostream& operator<<         (ostream&, const UFDPeer&);

        // checkInStrm loops through the in stream adding messsages to receivedMessages or transactions
        void                  checkInStrm();
        // checkContents loops through the receivedMessages attempting to advance the status of consensus
        void                  checkContents();

        //setup function
        void                  initParameters(const vector<Peer<UFDPeerMessage>*>&, json) override;

    };

   
    
    Simulation<quantas::UFDPeerMessage, quantas::UFDPeer>* generateSim();
}
#endif /* UFDPeer_hpp */
