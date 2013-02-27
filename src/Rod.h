//  _  _   __   ____  ____  _  _  _  _   __   __    __     __   _  _    __     __   ____  ____  ____    ____  ____   __   ____  ____
// ( \/ ) / _\ (  _ \/ ___)/ )( \( \/ ) / _\ (  )  (  )   /  \ / )( \  (  )   / _\ / ___)(  __)(  _ \  (  __)(  __) / _\ / ___)(_  _)
// / \/ \/    \ )   /\___ \) __ (/ \/ \/    \/ (_/\/ (_/\(  O )\ /\ /  / (_/\/    \\___ \ ) _)  )   /   ) _)  ) _) /    \\___ \  )(
// \_)(_/\_/\_/(__\_)(____/\_)(_/\_)(_/\_/\_/\____/\____/ \__/ (_/\_)  \____/\_/\_/(____/(____)(__\_)  (__)  (____)\_/\_/(____/ (__)
//
//
//  Created by Memo Akten, www.memo.tv
//  Marshmallow Laser Feast
//  Forest
//

#pragma once

#include "ofMain.h"
#include "ScaleManager.h"

class Rod : public ofNode {
public:
    ofColor color;
    
    float height;           // real value (in cm)
    float heightNorm;       // normalized (0...1) based on min/max parameters
    
    float radius;           // real value (in cm)
    float radiusNorm;       // normalized (0...1) based on min/max parameters
    
    float volume;
    
    static int angleAmp;
    static float dampSpeed;
    
	enum IDDisplayType {
		DISPLAY_NONE,
		DISPLAY_PITCH_INDEX,
		DISPLAY_DEVICE_ID,
		DISPLAY_INDEX,
        DISPLAY_POLAR_COORDS,
        DISPLAY_POLAR_COORDS_NORM,
        DISPLAY_RADIUS,
        DISPLAY_ANGLE,
        DISPLAY_NAME
	};
	static IDDisplayType idDisplayType;
    //    static bool bDisplaySelectedId;
	
    static int heightMin;
    static int heightMax;
    static int diameterMin;
    static int diameterMax;
    //    static int brightness;
    
    static bool displayLaser;
    static int laserHeight;
    static int laserDiameter;
    static bool bLaserAlwaysOn;
    static float laserCutoffThreshold;
    static float laserTriggerThreshold;
    
    static map<int,Rod*> deviceIdToRod;
    
    
	//--------------------------------------------------------------
    void setup(float installationRadius) {
        heightNorm = ofRandomuf();
        radiusNorm = ofRandomuf();
        pitchIndexOffset = ofRandomf();
        amp = 0;
        laserAlpha = 0;
        polarCoordinates.x = getPosition().length();
        polarCoordinates.y = ofMap(atan2(getPosition().z, getPosition().x), -PI, PI, 0, 360);
        
        polarCoordinatesNorm.x = polarCoordinates.x / installationRadius;
        polarCoordinatesNorm.y = polarCoordinates.y / 360.0f;
    }
    
    //--------------------------------------------------------------
    void updateName() {
        if(index == 0) name = "A";
        else if(index <= 10) name = "B" + ofToString(index);
        else if(index <= 30) name = "C" + ofToString(index - 10);
        else if(index <= 60) name = "D" + ofToString(index - 30);
        else if(index <= 90) name = "E" + ofToString(index - 60);
        else if(index <= 120) name = "F" + ofToString(index - 90);
        else if(index <= 150) name = "G" + ofToString(index - 120);
        else name = "NA";
    }
    
    //--------------------------------------------------------------
    void fadeAmp() {
        oldAmp = amp;
        if(amp > 0.001) amp *= (1-dampSpeed);
        else amp = 0;
    }
    
    //--------------------------------------------------------------
    void setLaserBasedonAmp() {
		// decide whether the laser is on.
//        float newLaserAlpha;
        if(bLaserAlwaysOn) laserAlpha = 1;
        else {
            // if laser is off, and amp is greater than trigger theshold, and amp is rising -> switch it on
            if(laserAlpha == 0 && amp > laserTriggerThreshold && amp > oldAmp) laserAlpha = 1;

            
            // if laser is on, and amp is less than cutoff threshold, and amp is falling
            else if(laserAlpha == 1 && amp < laserCutoffThreshold && amp <= oldAmp) laserAlpha = 0;
            //        if(newLaserAlpha > laserAlpha)
//            laserAlpha = newLaserAlpha;
        }
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
    ofVec2f getPolarCoordinatesNorm() const {
        return polarCoordinatesNorm;
    }
    
    //--------------------------------------------------------------
    float getSortScore() const {
        return floor(polarCoordinates.x / 100.0) * 1000000 + round(polarCoordinates.y);
    }
    
    
    //--------------------------------------------------------------
    void setIndex(int index) {
        this->index = index;
        updateName();
    }
    
    //--------------------------------------------------------------
    int getIndex() {
        return index;
    }
    
    //--------------------------------------------------------------
    int setDeviceId(int deviceId) {
        this->deviceId = deviceId;
        deviceIdToRod[deviceId] = this;
        printf("Rod::setDeviceId : %i for index %i\n", deviceId, index);
    }
    
    //--------------------------------------------------------------
    int getDeviceId() const {
        return deviceId;
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
    float getPitchIndexOffset() const {
        return pitchIndexOffset;
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
    string getInfoStr() const {
        string s = "";
        s += "index: " + ofToString(index) + "\n";
        s += "deviceId: " + ofToString(deviceId) + "\n";
        s += "name: " + name + "\n";
        s += "pitchIndex: " + ofToString(pitchIndex) + "\n";
        s += "polarCoordinates: " + ofToString(polarCoordinates.x, 2) + ", " + ofToString(polarCoordinates.y, 2) + "\n";
        s += "polarCoordinatesNorm: " + ofToString(polarCoordinatesNorm.x, 2) + ", " + ofToString(polarCoordinatesNorm.y, 2) + "\n";
        s += "sortScore: " + ofToString(getSortScore(), 2) + "\n";
        s += "amp: " + ofToString(getAmp()) + "\n";
        return s;
    }
    
    
    static void loadDeviceIdToRodMap(vector<Rod> &rods);
    static void saveDeviceIdToRodMap(vector<Rod> &rods);
    
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
                
                
                if(laserAlpha>0 && displayLaser) {
                    ofPushMatrix(); {
                        ofSetColor(0, 255, 0, 255 * laserAlpha);
                        ofTranslate(0, laserHeight/2, 0);
                        ofScale(laserDiameter, laserHeight, laserDiameter);
                        ofBox(1);
                    } ofPopMatrix();
                    ofPopStyle();
                }
            } ofPopMatrix();
            
            //            if(bDisplaySelectedId == false || laserAlpha == 1) {
            if(idDisplayType==DISPLAY_DEVICE_ID) {
                if(deviceId==0) ofSetHexColor(0xFF0000);
                else ofSetHexColor(0x00FF00);
                ofDrawBitmapString(ofToString(deviceId), 30, 0);
            } else if(idDisplayType==DISPLAY_INDEX) {
                ofSetColor(255);
                ofDrawBitmapString(ofToString(index), 30, 0);
            } else if(idDisplayType==DISPLAY_PITCH_INDEX) {
                ofSetColor(0, 100);
                ofDrawBitmapString(ofToString(pitchIndex), 30, 0);
            } else if(idDisplayType==DISPLAY_NAME) {
                ofSetColor(255);
                ofDrawBitmapString(name, 30, 0);
            } else if(idDisplayType==DISPLAY_POLAR_COORDS) {
                ofSetColor(0, 100);
                ofDrawBitmapString(ofToString(polarCoordinates.x, 0) + ", " + ofToString(polarCoordinates.y, 0), 30, 0);
            } else if(idDisplayType==DISPLAY_POLAR_COORDS_NORM) {
                ofSetColor(0, 100);
                ofDrawBitmapString(ofToString(polarCoordinatesNorm.x, 2) + ", " + ofToString(polarCoordinatesNorm.y, 2), 30, 0);
            } else if(idDisplayType==DISPLAY_RADIUS) {
                ofSetColor(0, 100);
                ofDrawBitmapString(ofToString(polarCoordinates.x, 0), 30, 0);
            } else if(idDisplayType==DISPLAY_ANGLE) {
                ofSetColor(0, 100);
                ofDrawBitmapString(ofToString(polarCoordinates.y, 0), 30, 0);
            }
            //            }
            
        } restoreTransformGL();
        ofPopStyle();
    }
    
    
    //--------------------------------------------------------------
    bool operator< (const Rod &rhs) const {
        return getSortScore() < rhs.getSortScore();
    }
    
    
private:
    int pitchIndex;
    float pitchIndexOffset; // -1...1 random offset for pitch note (multiplied by global)
    
    float laserAlpha;
    
    float amp;      // 0...1 value of the amplitude of the rod
    float oldAmp;
    
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
    
    // this is the string name of the rod (circle letter, number, e.g. A6)
    string name;
    
    // each rod has polar coordinates, distance first, and then angle
    ofVec2f polarCoordinates;
    ofVec2f polarCoordinatesNorm;   // normalized
};