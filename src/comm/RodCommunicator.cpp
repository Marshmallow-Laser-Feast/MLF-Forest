/**
 *  RodCommunicator.cpp
 *
 *  Created by Marek Bereza on 21/02/2013.
 */

#include "RodCommunicator.h"

RodCommunicator::RodCommunicator() {
	MODE = DISCOVERING;
	totalRodCount = 0;
    paused = false;
	running = false;
    smoothedUpdateRate = 0;
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
    ForestSerialPort::font.loadFont("fonts/automat.ttf", 6);
    MODE = DISCOVERING;
    printf("======================================================================\n");
    printf("SCAN STARTED\n");
    
	// block whilst creating forest serial ports, then start thread
	vector<string> serialNos = D2xxSerial::getDeviceSerialNumbers();
	printf("Found %d serial ports\n", (int) serialNos.size());
	
    if(serialNos.size()>maxNumSerialPorts) {
        printf("Only using first %d serial ports\n", maxNumSerialPorts);
        serialNos.resize(maxNumSerialPorts);
    }
    
    ports.resize(serialNos.size());
    
    
	for(int i = 0; i < serialNos.size(); i++) {
		if(!ports[i].close()) {
			printf("Couldn't close port '%s'\n", serialNos[i].c_str());
		}
		ofSleepMillis(10);
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
	ofPushStyle();
    ofEnableAlphaBlending();
    int xOffset = 330;
	smoothedUpdateRate = updateRate * 0.05 + smoothedUpdateRate * 0.95;
    string report = "";
	report += "# rods connected: " + ofToString(totalRodCount) + "\n";
	report += "Update Rate:	     " + ofToString(smoothedUpdateRate, 0) + " Hz\n";
    report += "\n\nKey\n===\n";
    report += "A:    bad accelerometer\n";
    report += "R:    reset occurred since timeslot allocated - indicates node has power-cycled since host startup\n";
    report += "T:    tip-over detected\n";
//	ofBackground(0,0,0);
    ofSetColor(0, 100);
    ofRect(0, 0, ofGetWidth(), ofGetHeight());
    
    //ofRect(ForestSerialPort::bgRect);
	ofSetHexColor(0xFFFFFF);
    
    ForestSerialPort::drawString(report, xOffset, 30);
	
    for(int i = 0; i < ports.size(); i++) {
		ports[i].draw(xOffset, 120 + i * 85);
	}
    ofPopStyle();
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
	msPerFrame = 5;
	
	float t = 0;
	// then run
	while(running) {
		
        if(paused) {
            ofSleepMillis(100);
            for(int i = 0; i < ports.size(); i++) {
                ports[i].reset();
            }
        } else {
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
}


// this blocks until the entire network is discovered
void RodCommunicator::discover() {
    ForestSerialPort::foundDeviceIds.clear();
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
