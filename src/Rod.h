#pragma once

#include "ofMain.h"
#include "ScaleManager.h"

class Rod : public ofNode {
public:
    float height;           // real value (in cm)
    float heightNorm;       // normalized (0...1) based on min/max parameters
    
    float radius;           // real value (in cm)
    float radiusNorm;       // normalized (0...1) based on min/max parameters
    
    static int angleAmp;
    static float dampSpeed;
    
	//static bool showPitchIndex;
	//static bool showDeviceIds;
	
	enum IDDisplayType {
		DISPLAY_NONE,
		DISPLAY_PITCH_INDEX,
		DISPLAY_DEVICE_ID,
		DISPLAY_INDEX,
        DISPLAY_POLAR_COORDS,
        DISPLAY_ID
	};
	static IDDisplayType idDisplayType;
    static bool bDisplaySelectedId;
	
    static int heightMin;
    static int heightMax;
    static int diameterMin;
    static int diameterMax;
    static int color;
    
    static int laserHeight;
    static int laserDiameter;
    static bool bLaserAlwaysOn;
    static float laserAlphaThreshold;
    
	//--------------------------------------------------------------
    void setup() {
        heightNorm = ofRandomuf();
        radiusNorm = ofRandomuf();
        amp = 0;
        laserAlpha = 0;
        polarCoordinates.x = getPosition().length();
        polarCoordinates.y = ofMap(atan2(getPosition().z, getPosition().x), -PI, PI, 0, 360);
    }
    
    //--------------------------------------------------------------
    void updateId() {
        
    }
    
    //--------------------------------------------------------------
    void fadeAmp() {
        if(amp > 0.001) amp *= (1-dampSpeed);
        else amp = 0;
    }
    
    //--------------------------------------------------------------
    void setLaserBasedonAmp() {
		// decide whether the laser is on.
        float newLaserAlpha = bLaserAlwaysOn ? 1 : amp > laserAlphaThreshold;
        if(newLaserAlpha > laserAlpha) laserAlpha = newLaserAlpha;
        
        //		oldAmp = amp;
    }
    
    //--------------------------------------------------------------
    //    bool trigger(float retriggerThreshold, ScaleManager &scaleManager, float outputPitchMult, int maxNoteCount, float volumePitchMult, float volumeVariance, ofSoundPlayer *sound) {
    //        if(oldAmp < retriggerThreshold) amp = ofRandom(1 - volumeVariance, 1);
    //
    //        // if trigger
    //        if(oldAmp < amp/2) {
    //            if(sound) {
    //                float speedMult = scaleManager.currentMult(pitchIndex) * outputPitchMult;
    //                float volume = ofRandom(0.5, 1);
    //                if(maxNoteCount > 0) volume *= ofMap(pitchIndex, 0, maxNoteCount-1, 1, volumePitchMult);
    //                sound->setSpeed(speedMult);
    //                sound->setVolume(volume);
    //                sound->play();
    //                ofLogNotice() << "pitchIndex: " << pitchIndex << ", speedMult: " << speedMult;
    //            }
    //            return true;
    //        }
    //    }
    
    //--------------------------------------------------------------
    ofVec2f getPolarCoordinates() const {
        return polarCoordinates;
    }
    
    //--------------------------------------------------------------
    //    int getIndex() {
    //        return index;
    //    }
    
    //--------------------------------------------------------------
    int setDeviceId(int deviceId) {
        this->deviceId = deviceId;
    }
    
    //--------------------------------------------------------------
    void setPitchIndex(int pitchIndex) {
        this->pitchIndex = pitchIndex;
    }
    
    //--------------------------------------------------------------
    int getPitchIndex() const {
        return pitchIndex;
    }
    
    //--------------------------------------------------------------
    int getDeviceId() const {
        return deviceId;
    }
    
    
    //--------------------------------------------------------------
    void setAmp(float amp) {
        this->amp = amp;
    }
    
    //--------------------------------------------------------------
    float getAmp() const {
        return amp;
    }
    
    //--------------------------------------------------------------
    void setLaser(float laserAlpha) {
        this->laserAlpha = laserAlpha;
    }
    
    //--------------------------------------------------------------
    float getLaser() const {
        return laserAlpha;
    }
    
    
    //--------------------------------------------------------------
    void draw() {
        height = ofLerp(heightMin, heightMax, heightNorm);
        radius = ofLerp(diameterMin, diameterMax, radiusNorm)/2;
        
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
            
            if(bDisplaySelectedId == false || laserAlpha == 1) {
                if(idDisplayType==DISPLAY_DEVICE_ID) {
                    if(deviceId==0) ofSetHexColor(0xFF0000);
                    else ofSetHexColor(0x00FF00);
                    ofDrawBitmapString(ofToString(deviceId), 30, 0);
                } else if(idDisplayType==DISPLAY_INDEX) {
                    ofSetHexColor(0x0000FF);
                    ofDrawBitmapString(ofToString(index), 30, 0);
                } else if(idDisplayType==DISPLAY_PITCH_INDEX) {
                    ofSetColor(0, 100);
                    ofDrawBitmapString(ofToString(pitchIndex), 30, 0);
                } else if(idDisplayType==DISPLAY_ID) {
                    ofSetColor(0, 100);
                    ofDrawBitmapString(id, 30, 0);
                } else if(idDisplayType==DISPLAY_POLAR_COORDS) {
                    ofSetColor(0, 100);
                    ofDrawBitmapString(ofToString(polarCoordinates.x) + ", " + ofToString(polarCoordinates.y), 30, 0);
                }
            }
            
        } restoreTransformGL();
        ofPopStyle();
    }
    
    
private:
    int pitchIndex;
    
    float laserAlpha;
    
    float amp;      // 0...1 value of the amplitude of the rod
    
    //    float oldAmp;   // old value of the amp (to detect a trigger)
    
    // this is the amplitude of the rod
	// as returned by the serial interface
    //	float ampFromSerial;
    //	float ampFromMouse;
	
	// this is the id of the board
	// as programmed in its firmware.
	int deviceId;
	
	// this is the id of the rod in the vector of rods
	int index;
    
    // this is the string id of the rod (circle letter, number)
    string id;
    
    // each rod has polar coordinates, distance first, and then angle
    ofVec2f polarCoordinates;
};