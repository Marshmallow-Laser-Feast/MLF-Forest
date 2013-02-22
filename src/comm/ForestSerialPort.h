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
 *  ForestSerialPort.h, created by Marek Bereza on 21/02/2013.
 */

#pragma once

#include "ofMain.h"
#include "D2xxSerial.h"
#include "rodStructs.h"

// 250k baud for the forest
#define SERIAL_PORT_SPEED 250000
#define MAX_BOARDS_PER_NETWORK 10 // should be 255
#define LASER_BITMAP_SIZE 24

class ForestSerialPort {
public:
	ForestSerialPort();
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
	
	int getRodCount();
	
	// draw some debug info
	void draw(int x, int y);
	void setLaser(int laserId, bool on);
	
	static int param1;
	static int param2;
	static int param3;
	
	static int tipOverTimeConstant; // [0-31], 31 = slowest
	static int tipThreshold; // typically approx 40-50 - arbitary units
	static int laserTimeoutValue; // units of 2.048 ms
	static int laserHoldoff;
	
private:
	
	bool setTimeslot(int boardId, int timeslot);
	bool tryToRead(unsigned char *buff, int length, int timeout = 100);
	
	D2xxSerial serial;
	string serialNo;

	
	map<int,RodInfo> rodInfos;

	
	enum LaserCommandType {
		RETURN_NOTHING = 0,
		RETURN_RAW_ACCELEROMETER = 1,
		RETURN_PROCESSED_MOTION_DATA  = 2
	};
	
	
	
	LaserCommandType currentCommandType;
	
	static unsigned char laserBitmap[LASER_BITMAP_SIZE];
};
