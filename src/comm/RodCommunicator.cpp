/**
 *  RodCommunicator.cpp
 *
 *  Created by Marek Bereza on 21/02/2013.
 */

#include "RodCommunicator.h"

RodCommunicator::RodCommunicator() {
	MODE = DISCOVERING;
	totalRodCount = 0;
	running = false;
}
RodCommunicator::~RodCommunicator() {
	running = false;
	
	waitForThread();
	for(int i = 0; i < ports.size(); i++) {
		ports[i].close();
	}
	ForestSerialPort::allRodInfos.clear();
	ports.clear();
}

void RodCommunicator::reset() {
	running = false;

	waitForThread();
	ForestSerialPort::allRodInfos.clear();
	start();
}


string RodCommunicator::getReport() {
	string report = "";
	for(int i = 0; i < ports.size(); i++) {
		report += ports[i].report;
		report += "\n\n";
	}
	return report;
}

string RodCommunicator::getValues() {
	return "";
}
void RodCommunicator::start() {

	// block whilst creating forest serial ports, then start thread
	vector<string> serialNos = D2xxSerial::getDeviceSerialNumbers();
	printf("Found %d serial ports\n", (int) serialNos.size());
	ports.resize(serialNos.size());
	for(int i = 0; i < serialNos.size(); i++) {
		if(!ports[i].close()) {
			printf("Couldn't close port '%s'\n", serialNos[i].c_str());
		}
		ofSleepMillis(100);
		ports[i].open(serialNos[i]);
	}
	startThread();
}


//
bool RodCommunicator::doneDiscovering() {
	return MODE==RUNNING;
}



// make an average of all the ports' progress
float RodCommunicator::getProgress() {
	if(ports.size()==0) return 1;
	
	float out = 0;
	
	for(int i = 0; i < ports.size(); i++) {
		out += ports[i].progress;
	}
	out /= ports.size();
	
	return out;
}




void RodCommunicator::draw() {
	int xOffset = 330;
	string report = "";
	report += "# rods connected: " + ofToString(totalRodCount) + "\n";
	report += "Update Rate:	     " + ofToString(updateRate,1) + " Hz\n";
//	ofBackground(0,0,0);
    ofSetColor(0, 100);
    ofRect(0, 0, ofGetWidth(), ofGetHeight());
	ofSetHexColor(0xFFFFFF);
	ofDrawBitmapString(report, xOffset, 30);
	for(int i = 0; i < ports.size(); i++) {
		ports[i].draw(xOffset, 100 + i * 85);
	}
}




void RodCommunicator::setLaser(int deviceId, bool on) {
	ForestSerialPort::setLaser(deviceId, on);
}


float RodCommunicator::getAmplitude(int deviceId) {
	// TODO: this - actually I think this is done
//	return ofGetMousePressed()?1:0;
	if(ForestSerialPort::allRodInfos.find(deviceId)!=ForestSerialPort::allRodInfos.end()) {
        
		return ForestSerialPort::allRodInfos[deviceId]->getAmplitude();
	}
	return 0;
}

int RodCommunicator::findRodWithBiggestAmplitude(float &outAmplitude) {
    
	map<int,RodInfo*>::iterator it = ForestSerialPort::allRodInfos.begin();
    float maxAmp = 0;
    int maxDeviceId = -1;
    for(; it != ForestSerialPort::allRodInfos.end(); it++){
        if((*it).second->getAmplitude()>maxAmp) {
            maxAmp = (*it).second->getAmplitude();
            maxDeviceId = (*it).first;
        }
        
    }
    outAmplitude = maxAmp;
    return maxDeviceId;
}

// pair device/rod id

////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
////////////////////////////////////////////////////////////////////////////////////////////////////


void RodCommunicator::threadedFunction() {
	running = true;
	discover();
	
	
	
	int maxRodCount = 0;
	totalRodCount = 0;
	for(int i = 0; i < ports.size(); i++) {
		int rodCount = ports[i].getRodCount();
		maxRodCount = MAX(maxRodCount, rodCount);
		totalRodCount += rodCount;
	}
	float msPerFrame = maxRodCount*40; // this is wrong
	msPerFrame = 10;
	
	float t = 0;
	// then run
	while(running) {
		
		float tm = ofGetElapsedTimef();
		// timer for reporting
		updateRate = 1.f/(tm - t);
		t = tm;
		
		// this is also wrong - should be a proper timer
		ofSleepMillis(msPerFrame);
		
		// ask the rods for data
		for(int i = 0; i < ports.size(); i++) {
			ports[i].request();
		}
		
		//usleep(10000);
		
		// read the data back
		for(int i = 0; i < ports.size(); i++) {
			ports[i].retrieve();
		}
		
	}
}


// this blocks until the entire network is discovered
void RodCommunicator::discover() {
	for(int i = 0; i < ports.size(); i++) {
		ports[i].discover();
	}
	MODE = RUNNING;
}


void RodCommunicator::inspect() {
	// block whilst creating forest serial ports, then start thread
	vector<string> serialNos = D2xxSerial::getDeviceSerialNumbers();
	printf("Found %d serial ports\n", (int) serialNos.size());
	ports.resize(serialNos.size());
	for(int i = 0; i < serialNos.size(); i++) {
		if(!ports[i].close()) {
			printf("Couldn't close port '%s'\n", serialNos[i].c_str());
		}
		ofSleepMillis(100);
		ports[i].open(serialNos[i]);
	}
	for(int i = 0; i < ports.size(); i++) {
		ports[i].inspect();
	}
}