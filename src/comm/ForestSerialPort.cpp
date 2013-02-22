/**
 *  ForestSerialPort.cpp
 *
 *  Created by Marek Bereza on 21/02/2013.
 */

#include "ForestSerialPort.h"


// begin: laser parameters

int ForestSerialPort::param1 = 0;
int ForestSerialPort::param2 = 0;
int ForestSerialPort::param3 = 0;

int ForestSerialPort::tipOverTimeConstant = 10;
int ForestSerialPort::tipThreshold = 45;
int ForestSerialPort::laserTimeoutValue = 10; // ??
// end: laser parameters




bool hasInitedLaserBitmap = false;

unsigned char ForestSerialPort::laserBitmap[LASER_BITMAP_SIZE];


ForestSerialPort::ForestSerialPort() {
	
//	currentCommandType = RETURN_RAW_ACCELEROMETER;
	currentCommandType = RETURN_PROCESSED_MOTION_DATA;
	
	progress = 0;
	if(!hasInitedLaserBitmap) {
		memset(laserBitmap, 0, LASER_BITMAP_SIZE);
		hasInitedLaserBitmap = true;
	}
}

void ForestSerialPort::setLaser(int laserId, bool on) {
	
	// we're 1-indexed
	laserId--;
	int whichChar = laserId / 8;
	int whichBit = laserId % 8;
	if(on) bitSet(laserBitmap[whichChar], whichBit);
	else bitClear(laserBitmap[whichChar], whichBit);
}



ForestSerialPort::~ForestSerialPort() {
	close();
}

// opens the serial communications port
void ForestSerialPort::open(string portSerialNumber) {
	this->serialNo = portSerialNumber;
	if(serial.open(portSerialNumber, SERIAL_PORT_SPEED)) {
		printf("Connected to '%s' successfully\n", portSerialNumber.c_str());
		
		// this is supposed to be good
		serial.setLatencyTimer(2);
	}
}

// discover lasers on the network and give them timeslots
void ForestSerialPort::discover() {
	rodInfos.clear();
	int slotId = 1;
	progress = 0;
	float progressIncrement = 1.f/MAX_BOARDS_PER_NETWORK;
	for(int id = 1; id <= MAX_BOARDS_PER_NETWORK; id++) {
		bool success = setTimeslot(id, slotId);
		if(success) {
			slotId++;
		}
		progress += progressIncrement;
	}
	printf("=================================\n");
	printf("Forest Serial Port '%s'\n", serialNo.c_str());
	printf("Scanned for %d boards, found: %d\n", MAX_BOARDS_PER_NETWORK, slotId-1);
	printf("=================================\n");
}




bool ForestSerialPort::tryToRead(unsigned char *buff, int length, int timeout) {
	// wait for buffer to fill up, or timeout
	while(serial.available()<length && --timeout>0) usleep(100);
	if(serial.available()>=length) {
		if(serial.read(buff, length)==length) {
			return true;
		} else {
			printf("Error, read returned number not equal to available\n");
			
		}
	} else {
		//printf("Error, read timed out\n");
	}
	return false;
}




bool ForestSerialPort::setTimeslot(int boardId, int timeslot) {
	unsigned char buff[] = {0xFF, 0x06, 0x01, 0x01, 0x00, 0x00, 0x00};
	buff[3] = boardId;
	serial.write(buff, 7);
	
	RodIdentity ident;
	
	
	// read the current identity packet of the rod
	if(tryToRead((unsigned char *)&ident, sizeof(ident))) {

		
		// now set its timeslot
		buff[4] = timeslot;
		serial.write(buff, 7);
		
		// read the acknowledgement
		if(tryToRead((unsigned char *)&ident, sizeof(ident))) {
			rodInfos[ident.deviceId] = RodInfo(ident.deviceId, ident.timeslot);
			return true;
		} else {
			printf("Didn't get a response from rod with id %d\n", boardId);
		}
	} else {
		printf("Couldn't find rod with id %d\n", boardId);
	}
	
	return false;
}



// sends out a packet requesting data from lasers
// and also asks each laser to be on or off according
// to a bitmap
void ForestSerialPort::retrieve() {
	


	if(currentCommandType==RETURN_RAW_ACCELEROMETER) {
		for(int i = 0; i < rodInfos.size(); i++) {
			RawAccelerometerData accel;
			if(tryToRead((unsigned char*)&accel, sizeof(accel))) {
				if(rodInfos.find(accel.id)!=rodInfos.end()) {
					if(accel.x!=0x40) {
						rodInfos[accel.id].setRawData(accel);
					} else {
						printf("Rod id %d accelerometer is faulty\n", accel.id);
					}
				}
			}
		}
	} else if(currentCommandType==RETURN_PROCESSED_MOTION_DATA) {
		//printf("Coming in\n");
		for(int i = 0; i < rodInfos.size(); i++) {
			ProcessedAccelerometerData accel;
			//printf("Trying for rod %d\n", i);
			
			if(tryToRead((unsigned char*)&accel, sizeof(accel))) {
				//printf("Reading\n");
				if(rodInfos.find(accel.id)!=rodInfos.end()) {
					//printf("Found rod in question\n");
					if(accel.id==1) {
						printf("%d\n", accel.tip);
					}
					rodInfos[accel.id].setProcessedData(accel);

				} else {
					printf("Can't find the rod with that id (%d)\n", accel.id);
					
				}
			} else {
				printf("Can't read accelerometer - no data\n");
			}
		}
	}	
}


// reads the data back from the lasers
void ForestSerialPort::request() {
	
	unsigned char cmd[] = {
		0xFF, // frame start marker
		0x23, // command length
		0x02, // command number
		0x00, // ID (broadcast)
		0x00, // data type, will be set after allocation
		0x00, // param1
		0x00, // param2
		0x00, // param3
		0x00, // tip over filter time constant 0-31, 31 = slowest
		0x00, // Tip-over threshold (typ approx 40-50), arbitary units, not degrees!
		0x0A, // laser timeout value. Laser will blank if now new commands recieved
			  // before timeout. units of 2.048 ms
		0xC0, // number of lasers specified in following bitmap.
			  // not currently used - for upward compatability
		
		0x0C, // bitmap of laser states for IDs 8 (bit 7) to 1 (bit 0). 1=On
			  // (ID 0 does not exist so laser IDs start at 1)
		0x0D,
		0x0E,
		0x0F,
		0x10,
		0x11,
		0x12,
		0x13,
		0x14,
		0x15,
		0x16,
		0x17,
		0x18,
		0x19,
		0x1A,
		0x1B,
		0x1C,
		0x1D,
		0x1E,
		0x1F,
		0x20,
		0x21,
		0x22,
		0x23 // laser 192 - 185
	};
	cmd[5] = param1;
	cmd[6] = param2;
	cmd[7] = param3;
	cmd[8] = tipOverTimeConstant; // [0-31], 31 = slowest
	cmd[9] = tipThreshold; // typically approx 40-50 - arbitary units
	cmd[10] = laserTimeoutValue; // units of 2.048 ms
	
	
	
	cmd[4] = currentCommandType;
	memcpy(&cmd[12], laserBitmap, LASER_BITMAP_SIZE);
	
	serial.write(cmd, sizeof(cmd));
}

void ForestSerialPort::draw(int x, int y) {

	int pos = 0;
	float width = 50;
	ofSetHexColor(0xFFFFFF);
	
	ofDrawBitmapString(serialNo, x, y);
	map<int,RodInfo>::iterator it = rodInfos.begin();

	for( ; it != rodInfos.end(); it++) {
		ofSetHexColor(0x666666);
		ofRectangle r(x + width * pos, y+23, width-10, 50);
		// make the rectangle's origin at the bottom left
		r.y += r.height;
		r.height *= -1;
		ofRect(r);
		
		ofRectangle meter = r;
		meter.width /= 3;
		if(currentCommandType==RETURN_PROCESSED_MOTION_DATA) {

			meter.height = r.height * (*it).second.motion/255;
			ofSetHexColor(0x999900);
			ofRect(meter);
			
			meter.x += meter.width;
			meter.height = r.height*(*it).second.motionSpare/255;
			ofSetHexColor(0x009999);
			ofRect(meter);
			
			meter.x += meter.width;
			meter.height = r.height*(*it).second.tip/255;
			ofSetHexColor(0x990099);
			ofRect(meter);
			
		} else if(currentCommandType==RETURN_RAW_ACCELEROMETER) {
			
			
			meter.y -= meter.height/2;
			ofSetHexColor(0x990000);
			meter.height = r.height*(*it).second.rawData.x/90.f;
			ofRect(meter);
			
			ofSetHexColor(0x009900);
			meter.x += meter.width;
			meter.height = r.height*(*it).second.rawData.y/90.f;
			ofRect(meter);
			
			ofSetHexColor(0x000099);
			meter.x += meter.width;
			meter.height = r.height*(*it).second.rawData.z/90.f;
			ofRect(meter);
		}

		
		
		
		
		
		//printf("[%d] = %f\n", (*it).second.id, (*it).second.motion);
		ofSetHexColor(0xFFFFFF);
		string status = " ";
		if((*it).second.getStatus(ROD_STATUS_BAD_ACCELEROMETER)) {
			status += "A";
		} else {
			status += " ";
		}
		
		if((*it).second.getStatus(ROD_STATUS_RESET)) {
			status += "R";
		} else {
			status += " ";
		}
		
		if((*it).second.getStatus(ROD_STATUS_TIPPED)) {
			status += "T";
		} else {
			status += " ";
		}
		
		ofDrawBitmapString(ofToString((int)(*it).second.id)+status, r.x, r.y+14);
		
		pos++;
	}
}

// close the serial port
void ForestSerialPort::close() {
	serial.close();
}

int ForestSerialPort::getRodCount() {
	return rodInfos.size();
}