//  _  _   __   ____  ____  _  _  _  _   __   __    __     __   _  _    __     __   ____  ____  ____    ____  ____   __   ____  ____
// ( \/ ) / _\ (  _ \/ ___)/ )( \( \/ ) / _\ (  )  (  )   /  \ / )( \  (  )   / _\ / ___)(  __)(  _ \  (  __)(  __) / _\ / ___)(_  _)
// / \/ \/    \ )   /\___ \) __ (/ \/ \/    \/ (_/\/ (_/\(  O )\ /\ /  / (_/\/    \\___ \ ) _)  )   /   ) _)  ) _) /    \\___ \  )(
// \_)(_/\_/\_/(__\_)(____/\_)(_/\_)(_/\_/\_/\____/\____/ \__/ (_/\_)  \____/\_/\_/(____/(____)(__\_)  (__)  (____)\_/\_/(____/ (__)
//
//
//  Created by Memo Akten, www.memo.tv
//  Marshmallow Laser Feast
//  Forest
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
bool Rod::bLaserAlwaysOn = true;
bool Rod::bLaserRandom = false;
int Rod::laserRandomSkipFrame = 2;
float Rod::laserRandomProbability = 0.3;
float Rod::laserCutoffThreshold = 0.3;
float Rod::laserTriggerThreshold = 0.6;

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
    rodMapFile << "# deviceId:index\n";
    for(int i =0; i< rods.size(); i++) {
        rodMapFile << ofToString(rods[i].getDeviceId()) + ":" + ofToString(rods[i].getIndex()) + "\n";
    }
	
	rodMapFile.close();
}
