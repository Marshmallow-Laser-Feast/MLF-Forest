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
 *  Description: The serial thread that reads and writes all the laser serial ports.
 *				 
 *  RodCommunicator.h, created by Marek Bereza on 21/02/2013.
 */

#pragma once

#include "ofMain.h"
#include "ForestSerialPort.h"

class RodCommunicator: public ofThread {
public:
	RodCommunicator();
	
	
	void reset();
	
	// call this in your setup
	void start();
	
	//
	bool doneDiscovering();
	
	
	float getProgress();
	
	void setLaser(int deviceId, bool on);
	
	float getAmplitude(int deviceId);
	
	
	// draws debug info
	void draw();
	
protected:
	void threadedFunction();
	
private:


	void discover();
	
	
	enum CommunicatorMode {
		DISCOVERING,
		RUNNING
	};
	vector<ForestSerialPort> ports;
	CommunicatorMode MODE;
	int totalRodCount;
	
	// reported update rate in hz
	float updateRate;
	bool running;
};
