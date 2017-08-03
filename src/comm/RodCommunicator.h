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

#define MAX_NUM_SERIAL_PORTS 4

class RodCommunicator: public ofThread {
public:
	RodCommunicator();
	~RodCommunicator();
	
	void reset();
	
	// call this in your setup
	void start();
    void stop();
	string getValues();
	//
	bool doneDiscovering();
	
    int maxNumSerialPorts = MAX_NUM_SERIAL_PORTS;
    
	float getProgress();
	
	void setLaser(int deviceId, bool on);
	
	string getReport();
	
	float getAmplitude(int deviceId);
	
    int findRodWithBiggestAmplitude(float &outAmplitude);

	// draws debug info
	void draw();
	void inspect();
	
    bool paused;    
    
    bool stopped = true;
    bool checkStatus();
    
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
	float updateRate = 0;
	bool running;
	ofTrueTypeFont font;
    float smoothedUpdateRate = 0;
};
