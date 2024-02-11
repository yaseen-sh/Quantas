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
        int                     peerID = -1;
        vector<int>             deltap; // new values recieved in last round
        string messageType = ""; //heartbeat or consensus

    };


    class UFDPeer : public Peer<UFDPeerMessage>{
    public:
        // methods that must be defined when deriving from Peer
        UFDPeer                             (long);
        UFDPeer                             (const UFDPeer& &rhs);
        ~UFDPeer                            ();

        //= if we crash dont do anything
        bool                                crashed;

        //= vector of vectors of messages that have been received
        vector<vector<UFDPeerMessage>>      allMessages;

        //= vector of messages from phase 2
        vector<UFDPeerMessage>              lastMessages;

        //= vector of process's local list of values (Vp)
        vector<int>                         localList;

        //= deltap - what I heard LAST round, not the whole record which is Vp
        int                                 deltap;

        //= local perfect failure detector
        PerfectFailureDetector*              PFD; 

        //= status to indicate phase 1-3
        int                                 phase = 1;

        //= store the decision value
        int                                 decision = NULL;

        //= different from regular messages, we need to send a heartbeat
        void                  sendHeartbeat()

        //= make our local failure detector receive a heartbeat
        void                  receiveHeartbeat(UFDPeerMessage){
            PFD.receiveHeartbeat(UFDPeerMessage)
        }

        //function to make the process crash, gets called however setup
        void                    crash(){
            PFD.suspectProcess(id()); //use magic to make the FD perfect!

        }
        void                    decide();

        #pragma region carried over from PBFT
        // perform one step of the Algorithm with the messages in inStream
        //void                 performComputation();
        // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
        //void                 endOfRound(const vector<Peer<UFDPeerMessage>*>& _peers);

        // addintal method that have defulte implementation from Peer but can be overwritten
        void                 log()const { printTo(*_log); };
        ostream&             printTo(ostream&)const;
        friend ostream& operator<<         (ostream&, const UFDPeer&);

        // string indicating the current status of a node
        //string                          status = "pre-prepare";

        // latency of confirmed transactions
        //int                             latency = 0;
        // rate at which to submit transactions ie 1 in x chance for all n nodes
        //int                             submitRate = 20;
        
        // the id of the next transaction to submit
        //static int                      currentTransaction;

        // checkInStrm loops through the in stream adding messsages to receivedMessages or transactions
        void                  checkInStrm();
        // checkContents loops through the receivedMessages attempting to advance the status of consensus
        void                  checkContents();
        // submitTrans creates a transaction and broadcasts it to everyone
        void                  submitTrans(int tranID);

        #pragma endregion

    };

    class PerfectFailureDetector{
    public:
        PerfectFailureDetector(int a) : timeTolerance(a) {}

        // when we want to suspect a process
        void                    suspectProcess(int peerID){
           auto found = processList.find(peerID);
           //if it's in the list, of course
           if (found != processList.end()) {
                found->second->second = true; //set flag to true
           }
        }
        // if we get a message after suspected a process, we update our timeTolerance
        void                    updateTolerance(int peerID, int roundNum){
            int oldRound = processList.find(peerID)->second->first;
            if(timeTolerance < newNum) //only change the tolerance if it's an increase
                timeTolerance = roundNum - oldRound;

            //also need to change the flag to false because no longer suspected
            processList.find(peerID)->second->second = false;
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
                if((msg.roundNumber - found->second->first) > timeTolerance
                    && found->second->second == true)
                    updateTolerance(msg.peerID, msg.roundNumber);

                // then (regardless) we update the map value for most recent heartbeat
                found->second = msg.roundNumber;
            }


        }
    
    private:
        // maintain a list of processes, last round we receive heartbeat, suspected (T) or not (F)
        std::map<int, pair<int, bool>>   processList;
        // how many rounds before we suspect a process?
        int                      timeTolerance;
    }
    
    Simulation<quantas::UFDPeerMessage, quantas::UFDPeer>* generateSim();
}
#endif /* UFDPeer_hpp */
