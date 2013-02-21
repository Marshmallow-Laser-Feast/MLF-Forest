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
 *  Description: 
 *				 
 *  rodStructs.h, created by Marek Bereza on 21/02/2013.
 */

#pragma once

#include "ofMain.h"
#define ROD_STATUS_OK				0
#define ROD_STATUS_TIPPED 			1
#define ROD_STATUS_RESET 			2
#define ROD_STATUS_TIPPED_AND_RESET 3







////////////////////////////////////////////////////////////////////////////////////////////////////
// Below are structs that represent packets that come back to the computer from the rods.
// You read serial directly into these structs and then you can access their attributes.
////////////////////////////////////////////////////////////////////////////////////////////////////
struct RodIdentity {
	
	unsigned char deviceId;
	unsigned char firmwareVersion;
	unsigned char hardwareVersion;
	
	unsigned char timeslot;
	
	
	unsigned char r0, r1; // reserved
	
	
	void printDebug() {
		printf("-----------------------------\n");
		printf("BOARD\n");
		
		printf("Device ID: 0x%x\n", deviceId);
		printf("Firmware : 0x%x\n", firmwareVersion);
		printf("Timeslot : %d\n", timeslot);
		
		printf("-----------------------------\n");
	}
};





struct RawAccelerometerData {
	unsigned char id;
	unsigned char x;
	unsigned char y;
	unsigned char z;
	unsigned char status;
	
	void printDebug() {
		printf("-----------------------------\n");
		printf("RAW ACCELEROMETER READING\n");
		printf("Device ID: 0x%x\n", id);
		printf("x: %d\n", x);
		printf("y: %d\n", y);
		printf("z: %d\n", z);
		printf("Status: %d\n", status);
		printf("-----------------------------\n");
	}
	
};



struct ProcessedAccelerometerData {
	unsigned char id;
	unsigned char motion;
	unsigned char motionSpare;
	unsigned char tip;
	unsigned char status;
	
	void printDebug() {
		printf("-----------------------------\n");
		printf("PROCESSED ACCELEROMETER READING\n");
		printf("Device ID: 0x%x\n", id);
		printf("motion: %d\n", motion);
		printf("motion spare: %d\n", motionSpare);
		printf("tip: %d\n", tip);
		printf("Status: %d\n", status);
		printf("-----------------------------\n");
	}
	
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// END SERIAL READING STRUCTS
////////////////////////////////////////////////////////////////////////////////////////////////////

float lookupXY(int i);
float lookupZ(int i);

// this is what a ForestSerialPort knows about each laser
class RodInfo {
public:
	unsigned char id;
	unsigned char timeslot;
	ofVec3f rawData;
	
	float motion;
	float motionSpare;
	
	RodInfo(unsigned char id = 0, unsigned char timeslot = 0) {
		this->timeslot = timeslot;
		this->id = id;
		motion = 0;
		motionSpare = 0;
	}
	
	void setProcessedData(const ProcessedAccelerometerData &data) {
		motion = data.motion;
		motionSpare = data.motionSpare;
		//printf("Processed: %f %f\n", motion, motionSpare);
	}
	
	void setRawData(const RawAccelerometerData &data) {

		rawData.set(lookupXY(data.x), lookupXY(data.y), lookupZ(data.z));
		//printf("Raw\n");
	}
};