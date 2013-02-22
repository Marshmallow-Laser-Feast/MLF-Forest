#include <ftd2xx.h>
#include "ofMain.h"

#pragma once

class D2xxSerial {

public:
	D2xxSerial();
	~D2xxSerial();
	
	static const char *getError(FT_STATUS err);
	static int getNumDevices();

	
	static vector<string> getDeviceSerialNumbers();
	
	bool listDevices();

	bool close();
	
	bool setBaudRate(int baudRate);
	
	bool open(int which = 0, int baudRate = 9600);
	
	bool open(string who, int baudRate = 9600);
	
	int available();
	
	
	int read(unsigned char *buffer, int length);
	
	
	int write(unsigned char *buffer, int length);
	
	int read();
	
	
	int getLatencyTimer();
	
	void setLatencyTimer(int ms);
		
private:
	bool opened;	
	FT_HANDLE handle;
	
};