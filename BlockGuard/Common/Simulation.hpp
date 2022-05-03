<<<<<<< HEAD
//
//  Simulation.hpp
//  BlockGuard
//
//  Created by Joseph Oglio on 4/14/22.
//  Copyright � 2022 Kent State University. All rights reserved.
//
=======
// This class handles reading in a configuration file, setting up log files for the simulation, 
// initializing the network class, and repeating a simulation according to the configuration file 
// (i.e., running multiple experiments with the same configuration).  It is templated with a user 
// defined message and peer class, used for the underlaying network instance. 

>>>>>>> 2f4e9ab31a45a81bb5c0deba8c0e533eda7388d0

#ifndef Simulation_hpp
#define Simulation_hpp

#include "Network.hpp"

using std::ofstream;

namespace blockguard {
<<<<<<< HEAD
	template<class type_msg, class peer_type>
	class Simulation {
	private:
		Network<type_msg, peer_type> 		system;

	public:
		// Config file
		void 				run(json config);

		// logging functions
		ostream& printTo(ostream& out)const;
		friend ostream& operator<<  (ostream& out, const Simulation& sim) { return sim.printTo(out); };
=======
    template<class type_msg, class peer_type>
    class Simulation {
    private:
        Network<type_msg, peer_type> 		system;
        ostream                             *_log;

    public:
        // Name of log file, will have Test number appended
        void 				run			(json);

        // logging functions
        ostream& 			printTo		(ostream &out)const;
        friend ostream&     operator<<  (ostream &out, const Simulation &sim)      {return sim.printTo(out);};
>>>>>>> 2f4e9ab31a45a81bb5c0deba8c0e533eda7388d0

    };

<<<<<<< HEAD
	template<class type_msg, class peer_type>
	ostream& Simulation<type_msg, peer_type>::printTo(ostream& out)const {
		return out;
	}

	template<class type_msg, class peer_type>
	void Simulation<type_msg, peer_type>::run(json config) {
		ofstream out;
		if (config["logFile"] == "cout") {
			LogWritter::instance()->setLog(cout); // Set the log file to the console
		}
		else {
			string file = config["logFile"];
			out.open(file);
			if (out.fail()) {
				cout << "Error: could not open file " << file << ". Writing to console" << endl;
				LogWritter::instance()->setLog(cout); // If the file doesn't open set the log file to the console
			}
			else {
				LogWritter::instance()->setLog(out); // Otherwise set the log file to the user given file
			}
		}
		
		for (int i = 0; i < config["tests"]; i++) {
			// Configure the delay properties and initial topology of the network
			system.setDistribution(config["distribution"]);
			system.initNetwork(config["topology"]);
			LogWritter::instance()->setTest(i);

			for (int j = 0; j < config["rounds"]; j++) {
				LogWritter::instance()->setRound(0); // Set the round number for logging
				system.receive(); // do the receive phase of the round
				system.performComputation();  // do the perform computation phase of the round
				system.endOfRound(); // do any end of round computations
				system.transmit(); // do the transmit phase of the round
			}
		}
		LogWritter::instance()->print();
		out.close();
	}
=======
    template<class type_msg, class peer_type>
    ostream& Simulation<type_msg,peer_type>::printTo(ostream &out)const{
        return out;
    }

    template<class type_msg, class peer_type>
    void Simulation<type_msg, peer_type>::run(json config) {
        for (int i = 1; i <= config["tests"]; i++) {
            ofstream out;
            
            if (config["logFile"] == "cout") {
                system.setLog(cout); // Set the log file to the console
            }
            else {
                string file = config["logFile"];
                file += "Test" + std::to_string(i) + ".log";
                out.open(file);
                if (out.fail()) {
                    cout << "Error: could not open file " << file << std::endl;
                    system.setLog(cout); // If the file doesn't open set the log file to the console
                }
                else {
                    system.setLog(out); // Otherwise set the log file to the user given file
                }
                
            }
            // Configure the delay properties and initial topology of the network
            system.setDistribution(config["distribution"]);
            system.initNetwork(config["topology"]);

            for (int j = 1; j <= config["rounds"]; j++) {
                system.receive(); // do the receive phase of the round
                system.log(); // log the system state
                system.performComputation();  // do the perform computation phase of the round
                system.log();
                system.transmit(); // do the transmit phase of the round
                system.log();
            }
        }
    }
>>>>>>> 2f4e9ab31a45a81bb5c0deba8c0e533eda7388d0

}

#endif /* Simulation_hpp */