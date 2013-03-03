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
int ForestSerialPort::laserHoldoff = 10; // ??
// end: laser parameters

set<int> ForestSerialPort::foundDeviceIds;

ofRectangle ForestSerialPort::bgRect;
float ForestSerialPort::ampGain = 1;

bool ForestSerialPort::forceLasersOn = false;

map<int,RodInfo*> ForestSerialPort::allRodInfos;

ofTrueTypeFont ForestSerialPort::font;


bool hasInitedLaserBitmap = false;

unsigned char ForestSerialPort::laserBitmap[LASER_BITMAP_SIZE];


ForestSerialPort::ForestSerialPort() {
	
	currentCommandType = RETURN_RAW_ACCELEROMETER;
//	currentCommandType = RETURN_PROCESSED_MOTION_DATA;
	
	progress = 0;
	if(!hasInitedLaserBitmap) {
		memset(laserBitmap, 0, LASER_BITMAP_SIZE);
		hasInitedLaserBitmap = true;
	}
}

void ForestSerialPort::setLaser(int laserId, bool on) {
//	if(laserId>129 && laserId<140)
	//	printf("Set laser: %d %d\n", laserId, on);
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
		ofSleepMillis(10);
		// this is supposed to be good
		serial.setLatencyTimer(2);
		ofSleepMillis(10);
		int a = serial.available();
		if(a) printf("found %d still in buffer\n", a);
		unsigned char buff[256];
		while(a>0) {
			serial.read(buff, MAX(a, 256));
			a = serial.available();
		}
		ofSleepMillis(10);
	}
}

string padMe(int num, int space) {
	string out = ofToString(num);
	int numSpaces = space - out.size();
	for(int i = 0; i < numSpaces; i++) {
		out += " ";
	}
	return out;
}

// discover lasers on the network and give them timeslots
void ForestSerialPort::discover() {
	rodInfos.clear();
	int slotId = 1;
	progress = 0;
	float progressIncrement = 1.f/MAX_BOARDS_PER_NETWORK;
	for(int id = 1; id <= MAX_BOARDS_PER_NETWORK; id++) {

        if(foundDeviceIds.find(id)==foundDeviceIds.end()) {
            bool success = setTimeslot(id, slotId);
            
            if(success) {
                printf("DEVICE ID: %d - TIMESLOT: %d\n", id, slotId);
                foundDeviceIds.insert(id);
                slotId++;
            }
            
            
            
            progress += progressIncrement;
            ofSleepMillis(3);
        }
	}
	report = "=================================\n";
	report += "Forest Serial Port " + serialNo + "\n";
	report += "Scanned for " +ofToString(MAX_BOARDS_PER_NETWORK) + " boards, found: " + ofToString(rodInfos.size()) + "\n";

	string top =    "DEVICE ID: ";
	string bottom = "TIMESLOT:  ";
	map<int,RodInfo>::iterator it = rodInfos.begin();
	
	for( ; it != rodInfos.end(); it++) {
	
		top += padMe((*it).second.id, 5);
		bottom += padMe((*it).second.timeslot, 5);
	}
	report += top + "\n" + bottom + "\n";
	report += "=================================\n";
	printf("%s\n", report.c_str());
	printf("There are %d rodInfos\n", rodInfos.size());
}




bool ForestSerialPort::tryToRead(unsigned char *buff, int length, int timeout) {
	// wait for buffer to fill up, or timeout
	while(serial.available()<length && --timeout>0) usleep(50);
	if(serial.available()>=length) {
		if(serial.read(buff, length)==length) {
			return true;
		} else {
			printf("Error, read returned number not equal to available\n");
			
		}
	} else {
		//printf("Error, read timed out\n");
        return false;
	}
	return false;
}


void ForestSerialPort::reset() {
    int a = serial.available();
	unsigned char s[256];
	if(a) {
		printf("Found stuff in the buffer (%d bytes)\n", a);
		serial.read(s, MIN(a, 256));
		a = serial.available();
	}
}

bool ForestSerialPort::setTimeslot(int boardId, int timeslot, bool set) {
	unsigned char buff[] = {0xFF, 0x06, 0x01, 0x01, 0x00, 0x00, 0x00};
	buff[3] = boardId;
	
	int a = serial.available();
	unsigned char s[256];
	if(a) {
		printf("Found stuff in the buffer (%d bytes)\n", a);
		serial.read(s, MIN(a, 256));
		a = serial.available();
	}
	
	ofSleepMillis(5);
	serial.write(buff, 7);
	ofSleepMillis(5);
	RodIdentity ident;
	
	
	// read the current identity packet of the rod
	if(tryToRead((unsigned char *)&ident, sizeof(ident))) {

		if(set) {
			// now set its timeslot
			buff[4] = timeslot;
			serial.write(buff, 7);
			
			// read the acknowledgement
			if(tryToRead((unsigned char *)&ident, sizeof(ident))) {
				rodInfos[ident.deviceId] = RodInfo(ident.deviceId, ident.timeslot);
				allRodInfos[ident.deviceId] = &rodInfos[ident.deviceId];
				return true;
			} else {
				printf("Didn't get a response from rod with id %d\n", boardId);
			}
		} else {
			return true;
		}
		
		
	} else {
		//printf("Couldn't find rod with id %d\n", boardId);
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
			if(tryToRead((unsigned char*)&accel, sizeof(accel), 300)) {
				if(rodInfos.find(accel.id)!=rodInfos.end()) {
					if(accel.x!=0x40) {
						rodInfos[accel.id].setRawData(accel);
					} else {
//						printf("'%s' Rod id %d accelerometer is faulty\n", serialNo.c_str(), accel.id);	// TODO: re-enable
					}
				}
			} else {
                for(map<int,RodInfo>::iterator it = rodInfos.begin(); it != rodInfos.end(); it++) {
                    if((*it).second.timeslot==i+1) {
                        (*it).second.notifyTimeout();
                        break;
                    }
                }
            }
		}
	} else if(currentCommandType==RETURN_PROCESSED_MOTION_DATA) {
		for(int i = 0; i < rodInfos.size(); i++) {
			ProcessedAccelerometerData accel;
			
			if(tryToRead((unsigned char*)&accel, sizeof(accel))) {
				if(rodInfos.find(accel.id)!=rodInfos.end()) {
					rodInfos[accel.id].setProcessedData(accel);

				} else {
					printf("'%s' Can't find the rod with that id (%d)\n", serialNo.c_str(), accel.id);
					
				}
			} else {
				printf("'%s' Can't read accelerometer - no data\n", serialNo.c_str());
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
		0xC0, // Time laser must be vertical before turning on after tip-over condition cleared, units of 16mS
		
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
	cmd[11] = laserHoldoff;
	
	
	cmd[4] = currentCommandType;
	if(forceLasersOn) {
		
		for(int i = 0; i < LASER_BITMAP_SIZE; i++) {
			cmd[12+i] = 0xFF;
		}
	} else {
		memcpy(&cmd[12], laserBitmap, LASER_BITMAP_SIZE);
	}
	
//	for(int i = 0; i < LASER_BITMAP_SIZE; i++) {
//		cout<<ofToString(i)<< ": "  <<ofToBinary(cmd[12+i])<<endl;
//	}
	serial.write(cmd, sizeof(cmd));
}

void ForestSerialPort::draw(int x, int y) {
    if(bgRect.x==0) {
        bgRect.x = x;
        bgRect.y = y;
    }
    
	int pos = 0;
	float width = 50;
	ofSetHexColor(0xFFFFFF);
	
	drawString(serialNo, x, y);//, ofColor(0, 0, 0, 120), ofColor(255, 255, 255, 255));
    
	map<int,RodInfo>::iterator it = rodInfos.begin();

	for( ; it != rodInfos.end(); it++) {

        float cc = 0.2 + 0.8*(*it).second.timeout;
        
		ofRectangle r(x + width * pos, y+13, width-10, 40);
		// make the rectangle's origin at the bottom left
		r.y += r.height;
		r.height *= -1;
		
		glColor4f(cc, 0.2, 0.2, 0.9);
        ofRect(r);
        
        //bgRect.growToInclude(r);
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

		
		//ofDrawBitmapStringHighlight(<#string text#>, <#const ofPoint &position#>)
		

		
		//printf("[%d] = %f\n", (*it).second.id, (*it).second.motion);
		glColor4f(1, 1, 1, 0.6);
        ofNoFill();
        ofRect(r);
        ofFill();
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

		drawString(ofToString((int)(*it).second.id)+status, r.x, r.y+7);//, ofColor(0, 0, 0, 120), ofColor(255, 255, 255, 255));
		
		pos++;
	}
}

void ForestSerialPort::drawString(string s, int x, int y) {
		
    if(font.isLoaded()) {
        glColor4f(0,0,0,0.7);
        ofRectangle r = font.getStringBoundingBox(s, x, y);
        int pad = 5;
        r.width += pad*2;
        r.height += pad*2;
        x += pad;
        y += pad;
        ofRect(r);
        ofSetHexColor(0xFFFFFF);
        font.drawString(s, x, y);

    } else {
        ofDrawBitmapStringHighlight(s,x,y, ofColor(0, 0, 0, 120), ofColor(255, 255, 255, 255));
    }
}

// close the serial port
bool ForestSerialPort::close() {
	return serial.close();
}

int ForestSerialPort::getRodCount() {
	return rodInfos.size();
}
