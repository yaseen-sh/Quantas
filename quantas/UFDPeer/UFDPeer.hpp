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
        vector<int>             content;

    };


    class UFDPeer : public Peer<UFDPeerMessage>{
    public:
        // methods that must be defined when deriving from Peer
        UFDPeer                             (long);
        UFDPeer                             (const UFDPeer& &rhs);
        ~UFDPeer                            ();

        //= process includes its ID in every message it sends
        int                                 peer_ID;

        //= vector of vectors of messages that have been received
        vector<vector<UFDPeerMessage>>      allMessages;

        //= vector of messages from this round
        vector<UFDPeerMessage>              roundMessages;

        //= vector of process's local list of values
        vector<int>                         localList;

        //= local perfect failure detector
        PerfectFailureDetector*              PFD; 

        //= status to indicate phase 1-3
        int                                 status = 1;

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
        // current squence number
        //int                             sequenceNum = 0;

        // vector of recieved transactions
        //vector<UFDPeerMessage>		    transactions;
        // vector of confirmed transactions
        //vector<UFDPeerMessage>		    confirmedTrans;

        // latency of confirmed transactions
        int                             latency = 0;
        // rate at which to submit transactions ie 1 in x chance for all n nodes
        int                             submitRate = 20;
        
        // the id of the next transaction to submit
        //static int                      currentTransaction;

        // checkInStrm loops through the in stream adding messsages to receivedMessages or transactions
        void                  checkInStrm();
        // checkContents loops through the receivedMessages attempting to advance the status of consensus
        void                  checkContents();
        // submitTrans creates a transaction and broadcasts it to everyone
        void                  submitTrans(int tranID);

        //= different from regular messages, we need to send a heartbeat
        //void                  sendHeartbeat()
    };

    class PerfectFailureDetector{
    public:
        PerfectFailureDetector(int a) : timeTolerance(a) {}

        // when we want to suspect a process
        void                    suspectProcess(int ID)

        // if we get a message after suspected a process, we update our timeTolerance
        void                    updateTolerance(UFDPeer peer, int roundNum){
            int newNum;

            //find the currently suspected process
            auto found = suspectList.find(peer);
            if(found != suspectList.end()) newNum = timeTolerance;

            
            else{
                //find the round number we suspected it at
                int oldRound = found->second;

                //update the tolerance
                newNum = roundNum - oldRound;
            }

            timeTolerance = newNum;

        }

        // if we receive a heartbeat message from a process
        void                    receiveHeartbeat(UFDPeer peer, int roundNum){
            //look for it in the suspectList
            auto found = suspectList.find(peer);
            //if found, then need to update round number we received a heartbeat most recently.
            if(found != suspectList.end()){
                found.second = roundNum;
            }
        }
    private:
        // maintain a list of processes, last round we receive heartbeat
        std::map<UFDPeer, int>   processList;

        // how many rounds before we suspect a process
        int                      timeTolerance;
    }
    
    Simulation<quantas::UFDPeerMessage, quantas::UFDPeer>* generateSim();
}
#endif /* UFDPeer_hpp */
