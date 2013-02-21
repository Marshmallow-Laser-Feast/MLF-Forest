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
 *  RodCommunicator.h, created by Marek Bereza on 21/02/2013.
 */

#pragma once

#include "ofMain.h"
#include "ForestSerialPort.h"

class RodCommunicator: public ofThread {
public:
	RodCommunicator();
	
	
	// call this in your setup
	void start();
	
	//
	bool doneDiscovering();
	
	
	float getProgress();
	
	
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
};
