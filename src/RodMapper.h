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
	RodMapper();
	void update(RodCommunicator *comms, vector<Rod> &rods);
	
	void reset();
	
	// this maps device id's to rod object pointers
	// if there is no mapping for a rod, it's not
	// in the list
	map<int,Rod*> rodCommunicationMapping;
	
	map<int,int> blobToDevice;
	
	
	
	int findDeviceIdForRod(Rod &rod);
	
    void updateRodCommunicationMapping(vector<Rod> &rods);
	void updateRodsFromSerial(RodCommunicator *comms);
	void loadRodMapFile();
	void saveRodMapFile();
	
    void setMapping(int blobId, int deviceId);
	
	bool hasLoaded;
};
