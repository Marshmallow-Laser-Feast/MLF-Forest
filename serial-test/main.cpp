#include <ftd2xx.h>
#include <stdio.h>
#include <vector>
#include <string>
using namespace std;
#include "D2xxSerial.h"

D2xxSerial serial;


//The receive latency timer determines how long the device waits for more serial byte before sending 
//a packet over USB, and can have a very significant impact on performance of a 2-way 
//poll/response protocol. Values of 2 milliseconds (default is 16) are generally best.


struct IdentifyReturnData {
	
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


	
bool tryToRead(unsigned char *buff, int length, int timeout = 100) {
	// wait for buffer to fill up, or timeout
	while(serial.available()<length && --timeout>0) usleep(1000);
	if(serial.available()>=length) {
		if(serial.read(buff, length)==length) {
			return true;
		} else {
			printf("Error, read returned number not equal to available\n");

		}
	} else {
		printf("Error, read timed out\n");
	}
	return false;
}






#define STATUS_TIPPED 			1
#define STATUS_RESET 			2
#define STATUS_TIPPED_AND_RESET 3


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


enum LaserCommandType {
	RETURN_NOTHING = 0,
	RETURN_RAW_ACCELEROMETER = 1,
	RETURN_PROCESSED_MOTION_DATA  = 2
};


void LaserCommand(LaserCommandType cmdType = RETURN_PROCESSED_MOTION_DATA) {
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
	cmd[4] = cmdType;
	
	serial.write(cmd, sizeof(cmd));
	usleep(100);
	if(cmdType==RETURN_RAW_ACCELEROMETER) {
		for(int i = 0; i < 10; i++) {
			RawAccelerometerData accel;
			if(tryToRead((unsigned char*)&accel, sizeof(accel))) {
				accel.printDebug();
			}
		}
	} else if(cmdType==RETURN_PROCESSED_MOTION_DATA) {
		
		for(int i = 0; i < 10; i++) {
			ProcessedAccelerometerData accel;

			if(tryToRead((unsigned char*)&accel, sizeof(accel))) {
				accel.printDebug();
			}
		}
	}
	
	// don't do anything for RETURN_NOTHING
}

	

bool setTimeslot(int boardId, int timeslot) {
	unsigned char buff[] = {0xFF, 0x06, 0x01, 0x01, 0x00, 0x00, 0x00};
	buff[3] = boardId;
	serial.write(buff, 7);
	
	IdentifyReturnData ident;
	
	if(tryToRead((unsigned char *)&ident, sizeof(ident))) {
		ident.printDebug();
		buff[4] = timeslot;
		serial.write(buff, 7);
		if(tryToRead((unsigned char *)&ident, sizeof(ident))) {
			printf("Changed to:\n");
			ident.printDebug();
			return true;
		} else {
			printf("Didn't get a response from rod with id %d\n", boardId);
		}
	} else {
		printf("Couldn't find rod with id %d\n", boardId);
	}
	
	return false;
}


class ForestSerialPort {
public:
	
	~ForestSerialPort();
	
	// opens the serial communications port
	void open(string portSerialNumber);
	
	// discover lasers on the network and give them timeslots
	void discover();
	
	// sends out a packet requesting data from lasers
	// and also asks each laser to be on or off according
	// to a bitmap
	void request();
	
	
	// reads the data back from the lasers
	void retrieve();

	// close the serial port
	void close();
	
	
	// how much progress has been made in
	// discovering the lasers. used for startup
	float progress;	

	
private:

	D2xxSerial serial;

};



int main() {
	printf("Serial test\n");
	
	serial.listDevices();
	
	serial.open("FTVID08H", 250000);
	
	// begin identify code
	usleep(50*1000);
	
	
	
	int MAX_BOARDS = 10;
	int slotId = 1;
	
	for(int id = 1; id <= MAX_BOARDS; id++) {
		bool success = setTimeslot(id, slotId);
		if(success) {
			slotId++;
		}
		
	}
	
	LaserCommand(RETURN_PROCESSED_MOTION_DATA);
	
	
	return 0;
}