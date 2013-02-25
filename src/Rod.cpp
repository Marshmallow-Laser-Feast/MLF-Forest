//
//  Rod.cpp
//  Forest Performance Simulator
//
//  Created by Memo Akten on 30/01/2013.
//
//

#include "Rod.h"

#define ROD_MAP_FILE "layout/rodMap.txt"


int Rod::angleAmp;
float Rod::dampSpeed;
//bool Rod::showPitchIndex;
int Rod::heightMin;
int Rod::heightMax;
int Rod::diameterMin;
int Rod::diameterMax;
//int Rod::brightness;

bool Rod::displayLaser;
int Rod::laserHeight;
int Rod::laserDiameter;
bool Rod::bLaserAlwaysOn;
float Rod::laserAlphaThreshold;

//bool Rod::showDeviceIds = false;

Rod::IDDisplayType Rod::idDisplayType = DISPLAY_PITCH_INDEX;
//bool Rod::bDisplaySelectedId = false;

map<int,Rod*> Rod::deviceIdToRod;


void Rod::loadDeviceIdToRodMap(vector<Rod> &rods) {
    string path = ROD_MAP_FILE;
	string line;
	
	if(!ofFile(path).exists()) {
		// make a dummy?
		return;
	}
	
	deviceIdToRod.clear();
	
	ifstream rodMapFile(ofToDataPath(path).c_str());
	
	if (rodMapFile.is_open()) {
		
		while(rodMapFile.good()) {
			getline (rodMapFile,line);
			if(line.size()>0 && line[0]=='#') continue;
			// parse line here
			vector<string> parts = ofSplitString(line, ":");
			if(parts.size()==2) {
				int index = ofToInt(parts[1]);
				int deviceId = ofToInt(parts[0]);
                rods[index].setDeviceId(deviceId);
			}
            
		}
		rodMapFile.close();
	}

}


void Rod::saveDeviceIdToRodMap(vector<Rod> &rods) {

    string path = ROD_MAP_FILE;
	ofstream rodMapFile;
	
	rodMapFile.open (ofToDataPath(path).c_str());

    for(int i =0; i< rods.size(); i++) {
        rodMapFile << ofToString(rods[i].getDeviceId()) + ":" + ofToString(rods[i].getIndex()) + "\n";
    }
	
	rodMapFile.close();
}
