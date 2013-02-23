/**
 *  RodMapper.cpp
 *
 *  Created by Marek Bereza on 22/02/2013.
 */

#include "RodMapper.h"

#define ROD_MAP_FILE "layout/rodMap.txt"

RodMapper::RodMapper() {
	hasLoaded = false;
}


void RodMapper::update(RodCommunicator *comms, vector<Rod> &rods) {
	// this reorganizes the mapping from the
	// physical mapping of the rods to the
	// Rod objects if anything has changed
	updateRodCommunicationMapping(rods);
	
	// this sends and receives the physical rod data
	updateRodsFromSerial(comms);
}

int RodMapper::findDeviceIdForRod(Rod &rod) {
	if(blobToDevice.find(rod.index)!=blobToDevice.end()) {

		return blobToDevice[rod.index];
	} else {
		//printf("Couldn't find device for rod with blob ID %d\n", rod.blobId);
	}
	return 0;
}

void RodMapper::saveRodMapFile() {
	string path = ROD_MAP_FILE;
	ofstream rodMapFile;
	
	rodMapFile.open (ofToDataPath(path).c_str());
	
	rodMapFile << "";
	map<int,int>::iterator it;
	for(it = blobToDevice.begin(); it != blobToDevice.end(); it++) {
		rodMapFile << ofToString((*it).first) + ":" + ofToString((*it).second) + "\n";
	}
	
	rodMapFile.close();
}


void RodMapper::loadRodMapFile() {
	string path = ROD_MAP_FILE;
	string line;
	
	if(!ofFile(path).exists()) {
		// make a dummy?
		return;
	}
	
	blobToDevice.clear();
	
	ifstream rodMapFile(ofToDataPath(path).c_str());
	
	if (rodMapFile.is_open()) {
		
		while(rodMapFile.good()) {
			getline (rodMapFile,line);
			if(line.size()>0 && line[0]=='#') continue;
			// parse line here
			vector<string> parts = ofSplitString(line, ":");
			if(parts.size()==2) {
				int blobId = ofToInt(parts[0]);
				int deviceId = ofToInt(parts[1]);
				blobToDevice[blobId] = deviceId;
				printf("Blob ID: %d  =>  Device ID: %d\n", blobId, deviceId);
			}
		
		}
		rodMapFile.close();
	}
}



void RodMapper::updateRodCommunicationMapping(vector<Rod> &rods) {
	
	// check nothing's changed.
	
	// if it has, set the rod class id's
	// and rebuild the id table
	if(!hasLoaded) {
		hasLoaded = true;

		loadRodMapFile();
		
		rodCommunicationMapping.clear();
		
		for(int i = 0; i < rods.size(); i++) {

			// try to find a mapping for each rod
			int deviceId = findDeviceIdForRod(rods[i]);
			
			rods[i].deviceId = deviceId;
			
			blobToDevice[rods[i].index] = deviceId;

			if(deviceId>0) {
				rodCommunicationMapping[deviceId] = &rods[i];
			} else {
				rodCommunicationMapping[deviceId] = NULL;
			}
		}
	}
}




void RodMapper::updateRodsFromSerial(RodCommunicator *comms) {
	
	map<int,Rod*>::iterator it;

	for(it = rodCommunicationMapping.begin(); it != rodCommunicationMapping.end(); it++) {

		if((*it).second!=NULL) {
			// set whether the laser is on
			comms->setLaser((*it).first, (*it).second->laserAlpha?true:false);

			// read amplitude
			(*it).second->ampFromSerial = comms->getAmplitude((*it).first);
		}
	}
	
}

void RodMapper::reset() {
	hasLoaded = false;
	rodCommunicationMapping.clear();
	blobToDevice.clear();
}















