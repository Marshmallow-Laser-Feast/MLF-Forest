/**     ___           ___           ___                         ___           ___     
 *     /__/\         /  /\         /  /\         _____         /  /\         /__/|    
 *    |  |::\       /  /::\       /  /::|       /  /::\       /  /::\       |  |:|    
 *    |  |:|:\     /  /:/\:\     /  /:/:|      /  /:/\:\     /  /:/\:\      |  |:|    
 *  __|__|:|\:\   /  /:/~/::\   /  /:/|:|__   /  /:/~/::\   /  /:/  \:\   __|__|:|    
 * /__/::::| \:\ /__/:/ /:/\:\ /__/:/ |:| /\ /__/:/ /:/\:| /__/:/ \__\:\ /__/::::\____
 * \  \:\~~\__\/ \  \:\/:/__\/ \__\/  |:|/:/ \  \:\/:/~/:/ \  \:\ /  /:/    ~\~~\::::/
 *  \  \:\        \  \::/          |  |:/:/   \  \::/ /:/   \  \:\  /:/      |~~|:|~~ 
 *   \  \:\        \  \:\          |  |::/     \  \:\/:/     \  \:\/:/       |  |:|   
 *    \  \:\        \  \:\         |  |:/       \  \::/       \  \::/        |  |:|   
 *     \__\/         \__\/         |__|/         \__\/         \__\/         |__|/   
 *
 *  Description: This facilitates looking up device id's for rods in software
 *				 
 *  RodMapper.h, created by Marek Bereza on 22/02/2013.
 */

#pragma once

#include "ofMain.h"
#include "Rod.h"
#include "RodCommunicator.h"
class RodMapper {
public:
	
	void update(RodCommunicator *comms, vector<Rod> &rods);
	
	
	// this maps device id's to rod object pointers
	// if there is no mapping for a rod, it's not
	// in the list
	map<int,Rod*> rodCommunicationMapping;
	
	int findIdForRod(Rod &rod);
	void updateRodCommunicationMapping(vector<Rod> &rods);
	void updateRodsFromSerial(RodCommunicator *comms);
	void loadRodMapFile();
};
