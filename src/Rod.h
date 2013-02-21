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
    
    float value;
    float oldValue;
    
    int pitchIndex;
    
    float laserAlpha;
    
    static int angleAmp;
    static float dampSpeed;
    static bool showPitchIndex;
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
        //        color.set(ofRandom(255), ofRandom(255), ofRandom(255));
        value = 0;
    }
    
    //--------------------------------------------------------------
    void update() {
        if(value > 0.001) value *= (1-dampSpeed);
        else value = 0;
        oldValue = value;
        
        height = ofLerp(heightMin, heightMax, heightNorm);
        radius = ofLerp(diameterMin, diameterMax, radiusNorm)/2;
    }
    
    //--------------------------------------------------------------
    bool trigger(float retriggerThreshold, ScaleManager &scaleManager, float outputPitchMult, int maxNoteCount, float volumePitchMult, float volumeVariance, ofSoundPlayer *sound) {
        if(oldValue < retriggerThreshold) value = ofRandom(1 - volumeVariance, 1);
        
        // if trigger
        if(oldValue < value/2) {
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
        update();
        
        ofPushStyle();
        transformGL(); {
            ofPushMatrix(); {
                //            ofTranslate(pos.x, 0, pos.z);
                ofRotateX(ofRandom(-angleAmp, angleAmp) * value);
                ofRotateY(ofRandom(-angleAmp, angleAmp) * value);
                ofRotateZ(ofRandom(-angleAmp, angleAmp) * value);
                
                ofPushMatrix(); {
                    ofSetColor(color);
                    ofTranslate(0, height/2, 0);
                    ofScale(radius*2, height, radius*2);
                    ofBox(1);
                } ofPopMatrix();
                //            if(value > 0.01) {
                ofPushStyle();
                ofDisableLighting();
                if(bLaserAlwaysOn) laserAlpha = 1;
                else laserAlpha = value > laserAlphaThreshold;
                //                        laserAlpha = value;
                //                        laserAlpha = 1-laserAlpha;
                //                        laserAlpha *= laserAlpha * laserAlpha * laserAlpha;
                //                        laserAlpha = 1-laserAlpha;
                //                        ofSetColor(0, 255, 0, 255 * laserAlpha);
                //                    }
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
            if(showPitchIndex) {
                ofSetColor(0, 100);
                ofDrawBitmapString(ofToString(pitchIndex), 30, 0);
            }
        } restoreTransformGL();
        ofPopStyle();
    }
};