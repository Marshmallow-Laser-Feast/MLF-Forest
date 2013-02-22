#pragma once

#include "ofMain.h"
#include "ScaleManager.h"

class Rod : public ofNode {
public:
    //    ofVec3f pos;
    //    ofColor color;
    
    float height;           // real value (in cm)
    float heightNorm;       // normalized (0...1) based on min/max parameters
    
    float radius;           // real value (in cm)
    float radiusNorm;       // normalized (0...1) based on min/max parameters
    
    float amp;              // the current amplitude of vibration (0...1)
    float oldAmp;
    
    int pitchIndex;
    
    float laserAlpha;
    
    static int angleAmp;
    static float dampSpeed;
    
	//static bool showPitchIndex;
	//static bool showDeviceIds;
	
	enum IDDisplayType {
		DISPLAY_NONE,
		DISPLAY_PITCH_INDEX,
		DISPLAY_DEVICE_ID,
		DISPLAY_BLOB_ID
	};
	static IDDisplayType idDisplayType;
	
    static int heightMin;
    static int heightMax;
    static int diameterMin;
    static int diameterMax;
    static int color;
    
    static int laserHeight;
    static int laserDiameter;
    static bool bLaserAlwaysOn;
    static float laserAlphaThreshold;
    
	
	// this is the amplitude of the rod
	// as returned by the serial interface
	float ampFromSerial;
	
	float ampFromMouse;
	
	// this is the id of the board
	// as programmed in its firmware.
	int deviceId;
	
	// this is the id of the opencv
	// blob from the layout file.
	// we need this to be able to
	// assign it a device id.
	int blobId;
	
	//--------------------------------------------------------------
    void setup() {
		ampFromSerial = 0;
        heightNorm = ofRandomuf();
        radiusNorm = ofRandomuf();
        //        color.set(ofRandom(255), ofRandom(255), ofRandom(255));
        amp = 0;
		ampFromMouse;
    }
    
    //--------------------------------------------------------------
    void update() {
        if(ampFromMouse > 0.001) ampFromMouse *= (1-dampSpeed);
        else ampFromMouse = 0;

		oldAmp = ampFromMouse;
		
		
		// add the amplitude from serial instead of replacing it
		// so both are interactive
		amp = MAX(ampFromMouse, ampFromSerial);
		
		// decide whether the laser is on.
		if(bLaserAlwaysOn) laserAlpha = 1;
		else laserAlpha = amp > laserAlphaThreshold;
		

        
        height = ofLerp(heightMin, heightMax, heightNorm);
        radius = ofLerp(diameterMin, diameterMax, radiusNorm)/2;
    }
    
    //--------------------------------------------------------------
    bool trigger(float retriggerThreshold, ScaleManager &scaleManager, float outputPitchMult, int maxNoteCount, float volumePitchMult, float volumeVariance, ofSoundPlayer *sound) {
        if(oldAmp < retriggerThreshold) ampFromMouse = ofRandom(1 - volumeVariance, 1);
        
        // if trigger
        if(oldAmp < ampFromMouse/2) {
            if(sound) {
                float speedMult = scaleManager.currentMult(pitchIndex) * outputPitchMult;
                float volume = ofRandom(0.5, 1);
                if(maxNoteCount > 0) volume *= ofMap(pitchIndex, 0, maxNoteCount-1, 1, volumePitchMult);
                sound->setSpeed(speedMult);
                sound->setVolume(volume);
                sound->play();
                ofLogNotice() << "pitchIndex: " << pitchIndex << ", speedMult: " << speedMult;
            }
            return true;
        }
    }
    
    //--------------------------------------------------------------
    void draw() {
        // update();
        
        ofPushStyle();
        transformGL(); {
            ofPushMatrix(); {
                //            ofTranslate(pos.x, 0, pos.z);
                ofRotateX(ofRandom(-angleAmp, angleAmp) * amp);
                ofRotateY(ofRandom(-angleAmp, angleAmp) * amp);
                ofRotateZ(ofRandom(-angleAmp, angleAmp) * amp);
                
                ofPushMatrix(); {
                    ofSetColor(color);
                    ofTranslate(0, height/2, 0);
                    ofScale(radius*2, height, radius*2);
                    ofBox(1);
                } ofPopMatrix();
                //            if(amp > 0.01) {
                ofPushStyle();
                ofDisableLighting();
                
            
                if(laserAlpha>0) {
                    ofPushMatrix(); {
                        ofSetColor(0, 255, 0, 255 * laserAlpha);
                        ofTranslate(0, laserHeight/2, 0);
                        ofScale(laserDiameter, laserHeight, laserDiameter);
                        ofBox(1);
                    } ofPopMatrix();
                    ofPopStyle();
                }
            } ofPopMatrix();
			if(idDisplayType==DISPLAY_DEVICE_ID) {
				if(deviceId==0) ofSetHexColor(0xFF0000);
				else ofSetHexColor(0x00FF00);
				ofDrawBitmapString(ofToString(deviceId), 30, 0);
            } else if(idDisplayType==DISPLAY_BLOB_ID) {
				ofSetHexColor(0x0000FF);
				ofDrawBitmapString(ofToString(blobId), 30, 0);
				
			} else if(idDisplayType==DISPLAY_PITCH_INDEX) {
                ofSetColor(0, 100);
                ofDrawBitmapString(ofToString(pitchIndex), 30, 0);
            }
        } restoreTransformGL();
        ofPopStyle();
    }
};