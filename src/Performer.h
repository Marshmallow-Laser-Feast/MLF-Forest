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
#include "ofxAssimpModelLoader.h"

class Performer : public ofNode {
public:
    ofColor color;
    ofVec3f oldPos;
    
    float speed;           // real value
    float speedNorm;       // normalized (0...1) based on min/max parameters
    
    float height;           // real value
    float heightNorm;       // normalized (0...1) based on min/max parameters
    
    float affectRadius;
    float affectRadiusNorm;
    
    float seed;
    
    static ofxAssimpModelLoader *model;
    static ofVec3f worldMin;
    static ofVec3f worldMax;
    static float noiseAmount;
    static float noiseFreq;
    static bool updateFromAnimation;
    
    //--------------------------------------------------------------
    void setup() {
        ofLogNotice() << "Performer::setup";
        seed = ofRandomuf();
        
//        pan(ofRandom(360));
        
        speedNorm = ofRandomuf();
        
        heightNorm = ofRandomuf();
        setPosition(ofRandom(worldMin.x, worldMax.x), 0, ofRandom(worldMin.z, worldMax.z));
        
        if(model == NULL) {
            model = new ofxAssimpModelLoader();
            model->loadModel("3d/person.dae");
            model->setScaleNomalization(false);
            model->setScale(0.01, -0.01, 0.01);
        }
        
    }
    
    //--------------------------------------------------------------
    void update() {
        if(updateFromAnimation) {
            setGlobalOrientation(ofQuaternion());
            ofVec3f diff(getGlobalPosition() - oldPos);
            if(diff.lengthSquared()>1) {
                float targetRotY = ofRadToDeg(atan2(diff.x, diff.z));
//                float curRot = getHeading();
                float rotY = targetRotY;//curRot + ofAngleDifferenceDegrees(curRot, targetRotY) * 0.1;
//                printf("%f\n", rotY);
                setGlobalOrientation(ofQuaternion(rotY, ofVec3f(0, 1, 0)));
            }
        } else {
            dolly(speed * ofGetLastFrameTime());
            
            if(getPosition().x > worldMax.x) {
                pan(150);
                setPosition(worldMax.x-1, getY(), getZ());
            } else if(getPosition().x < worldMin.x) {
                pan(150);
                setPosition(worldMin.x+1, getY(), getZ());
            }
            
            if(getPosition().z > worldMax.z) {
                pan(150);
                setPosition(getX(), getY(), worldMax.z-1);
            } else if(getPosition().z < worldMin.z) {
                pan(150);
                setPosition(getX(), getY(), worldMin.z+1);
            }
            
            pan(ofSignedNoise(getPosition().x * noiseFreq, getPosition().z * noiseFreq) * noiseAmount);
        }
        
        setScale(height);
        affectRadius = height * affectRadiusNorm;
        
        oldPos = getGlobalPosition();
    }
    
    
    //--------------------------------------------------------------
    void draw() {
        ofPushStyle();
        transformGL(); {
            ofPushMatrix(); {
                //                ofRotateY(90 + atan2(vel.z, vel.x) * RAD_TO_DEG);
                ofRotateY(90);
                //                ofScale(height, height, height);
                ofSetColor(color);
                model->drawFaces();
            } ofPopMatrix();
            
            ofRotateX(90);
            ofFill();
            ofSetColor(50, 0, 0, 30);
            ofCircle(0, 0, -1.0f/height, affectRadiusNorm);
            
        } restoreTransformGL();
        ofPopStyle();
    }
    
};