/**
 *  RodMapper.cpp
 *
 *  Created by Marek Bereza on 22/02/2013.
 */

#include "RodMapper.h"



void RodMapper::update(RodCommunicator *comms, vector<Rod> &rods) {
	// this reorganizes the mapping from the
	// physical mapping of the rods to the
	// Rod objects if anything has changed
	updateRodCommunicationMapping(rods);
	
	// this sends and receives the physical rod data
	updateRodsFromSerial(comms);
}

int RodMapper::findIdForRod(Rod &rod) {
	return 0;
}

void RodMapper::loadRodMapFile() {
	string path = "rodMap.txt";
	string line;
	
	if(!ofFile(path).exists()) {
		// make a dummy
	}
	
	ifstream rodMapFile(ofToDataPath(path).c_str());
	
	if (rodMapFile.is_open()) {
		
		while(rodMapFile.good()) {
			getline (rodMapFile,line);
			// parse line here
			vector<string> parts = ofSplitString(line, ":");
			if(parts.size()==2) {
				int blobId = ofToInt(parts[0]);
				int deviceId = ofToInt(parts[1]);
			}
		
		}
		rodMapFile.close();
	}
}


void RodMapper::updateRodCommunicationMapping(vector<Rod> &rods) {
	
	// check nothing's changed.
	
	// if it has, set the rod class id's
	// and rebuild the id table
	if(rodCommunicationMapping.size()!=rods.size()) {
		loadRodMapFile();
		rodCommunicationMapping.clear();
		
		for(int i = 0; i < rods.size(); i++) {
			// try to find a mapping for each rod
			int id = findIdForRod(rods[i]);
			if(id>0) {
				rods[i].deviceId = id;
				rodCommunicationMapping[id] = &rods[i];
			}
		}
	}
	
}
void RodMapper::updateRodsFromSerial(RodCommunicator *comms) {
	
	map<int,Rod*>::iterator it;
	for(it = rodCommunicationMapping.begin(); it != rodCommunicationMapping.end(); it++) {
		// set whether the laser is on
		comms->setLaser((*it).first, (*it).second->laserAlpha?true:false);
		
		// read amplitude
		(*it).second->ampFromSerial = comms->getAmplitude((*it).first);
	}
	
}




