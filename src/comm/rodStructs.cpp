/**
 *  rodStructs.cpp
 *
 *  Created by Marek Bereza on 21/02/2013.
 */

#include "rodStructs.h"
#include "ForestSerialPort.h"


int MMA_XYar[64] = {
	0,3,5,8,11,14,16,19,22,25,28,31,34,38,41,45,49,53,58,63,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-80,-70,-63,-58,-53,-49,-45,-41,-38,-34,-31,-28,-25,-22,-19,-16,-14,-11,-8,-5,-3};


int MMA_Zar[64] = {
	90,87,84,82,79,76,74,70,68,65,62,59,56,52,49,45,41,37,32,27,20,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-10,-20,-27,-32,-37,-41,-45,-49,-52,-56,-59,-52,-65,-68,-71,-74,-76,-79,-82,-85,-87};



float lookupXY(int i) {

	return MMA_XYar[i];
}


float lookupZ(int i) {

	return MMA_Zar[i];
}

int RodInfo::dcFilterStrength = 1;

// how long to hold amplitude once we've seen a peak
int RodInfo::holdAmt = 20;

// threshold for the noise gate ~ [0-8000]
int RodInfo::threshold = 1000;

float RodInfo::attackSmoothing = 4;

int RodInfo::attackThreshold = 1000;


int RodInfo::processRawAccelerometerData(int x, int z) {


	
    x = MMA_XYar[x];
    
	z = MMA_Zar[z];
	// wrap the z-output
	if(z<0) {
		z += 180;
	}
	
	z -= 90;
	
	lastX = x;
	lastZ = z;
	
	// scale up to get better resolution
	int out = MAX(x, z)*100;
	
	// dc filter
	avg *= (100-dcFilterStrength);
	avg /= 100;
	avg += out * dcFilterStrength;
	out -= avg/100;
	
	// rectify
	out = ABS(out);

	// peak follower
	if(out>curr) { // attack
		if(curr<attackThreshold) {
            curr = out;
			//printf("jumping\n");
			
		} else {
            curr += (float)(out-curr)/(float)attackSmoothing;
			//printf("Not jumping\n");
		}
		
		cnt = 0;
		
	} else { // decay
		
		if(cnt>holdAmt) {
			curr-=(cnt-holdAmt)*3;
		}
		cnt++;
	}
	out = curr;
	
	
	
	// gate it
	out -= threshold;
	if(out<0) out = 0;
	// scale back down
	out /= 100;
	
	return out;
}






void RodInfo::setRawData(const RawAccelerometerData &data) {
	
    
	//		data.printDebug();
	if(data.x==0x40) return;
	// this is the raw data, normalized and wrapped.
	rawData.set(lookupXY(data.x), lookupZ(data.y), lookupXY(data.z));//lookupXY(data.x), lookupXY(data.y), lookupZ(data.z));
	
	if(rawData.y<0) {
		rawData.y += 180;
	}
	rawData.y -= 90;
	
    
    // decay timeout indicator.
	timeout *= 0.99;
	
	motion = processRawAccelerometerData(data.x, data.y);

	if(motion!=motion || motion>100) resetMotion();

	//ForestSerialPort::setLaser(id, motion>0);
	rawData.z = motion;
	status = data.status;
	// do yer filterin' ere innit.
}


float RodInfo::getAmplitude() {
    float signal = rawData.z/90.f;
    signal *= ForestSerialPort::ampGain;
    signal = ofClamp(signal, 0, 1);
    return signal;
}
