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



#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))


enum RodStatus {
	ROD_STATUS_OK = 0,
	ROD_STATUS_TIPPED = 1,
	ROD_STATUS_RESET = 2,
	ROD_STATUS_BAD_ACCELEROMETER = 4
};











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
	signed char x;
	signed char y;
	signed char z;
	unsigned char status;
	
	void printDebug() const {
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
	
	void printDebug() const {
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
	float tip;
	unsigned char status;
	RodInfo(unsigned char id = 0, unsigned char timeslot = 0) {
		this->timeslot = timeslot;
		this->id = id;
		resetMotion();

	}
	bool getStatus(RodStatus statusType) {
		return status & statusType;
	}
	
	
	void setProcessedData(const ProcessedAccelerometerData &data) {
		motion = data.motion;
		motionSpare = data.motionSpare;
		tip = data.tip;
		status = data.status;
	}
	
	void resetMotion() {
		motion = 0;
		motionSpare = 0;
		lastX = 0;
		lastZ = 0;
		avg = 0;
		peak = 0;
		curr = 0;
		cnt = 0;
		printf("Reseting motion\n");
	}
	
	
	void setRawData(const RawAccelerometerData &data) {

//		data.printDebug();
		if(data.x==0x40) return;
		// this is the raw data, normalized and wrapped.
		rawData.set(lookupXY(data.x), lookupZ(data.y), lookupXY(data.z));//lookupXY(data.x), lookupXY(data.y), lookupZ(data.z));
		
		if(rawData.y<0) {
			rawData.y += 180;
		}
		rawData.y -= 90;
		
		
		
		motion = processRawAccelerometerData(data.x, data.y);
		if(motion!=motion || motion>100) resetMotion();
		rawData.z = motion;
		status = data.status;
		// do yer filterin' ere innit.
	}
	int processRawAccelerometerData(int x, int z);


	int lastX;
	int lastZ;

	int avg;
	int peak;
	int curr;
	int cnt;


	// goes from 1-100 - 1 is most powerful
	static int dcFilterStrength ;

	// how long to hold amplitude once we've seen a peak
	static int holdAmt ;

	// threshold for the noise gate ~ [0-8000]
	static int threshold ;

};