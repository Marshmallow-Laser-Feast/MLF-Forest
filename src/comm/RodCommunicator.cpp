/**
 *  RodCommunicator.cpp
 *
 *  Created by Marek Bereza on 21/02/2013.
 */

#include "RodCommunicator.h"

RodCommunicator::RodCommunicator() {
	MODE = DISCOVERING;
	totalRodCount = 0;
}



void RodCommunicator::start() {

	// block whilst creating forest serial ports, then start thread
	vector<string> serialNos = D2xxSerial::getDeviceSerialNumbers();
	printf("Found %d serial ports\n", (int) serialNos.size());
	ports.resize(serialNos.size());
	for(int i = 0; i < serialNos.size(); i++) {
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
	string report = "";
	report += "# rods connected: " + ofToString(totalRodCount) + "\n";
	report += "Update Rate:	     " + ofToString(updateRate,1) + " Hz\n";
	ofBackground(0,0,0);
	ofSetHexColor(0xFFFFFF);
	ofDrawBitmapString(report, 400, 30);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
////////////////////////////////////////////////////////////////////////////////////////////////////


void RodCommunicator::threadedFunction() {
	discover();
	
	
	
	int maxRodCount = 0;
	totalRodCount = 0;
	for(int i = 0; i < ports.size(); i++) {
		int rodCount = ports[i].getRodCount();
		maxRodCount = MAX(maxRodCount, rodCount);
		totalRodCount += rodCount;
	}
	float msPerFrame = maxRodCount*10; // this is wrong
	msPerFrame = 0;
	
	float t = 0;
	// then run
	while(1) {
		
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
		
		usleep(10000);
		
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
