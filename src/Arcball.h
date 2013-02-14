#pragma once

#include "ofMain.h"

class Arcball {
public:
    ofQuaternion curRot;
	
    //a place to store the mouse position so we can measure incremental change
    ofVec2f lastMouse;
	
	//slows down the rotation 1 = 1 degree per pixel
	float dampen;
    
    bool enabled;
    
    //--------------------------------------------------------------
    Arcball() {
        dampen = .4;
        enabled = false;
    }

    void begin() {
        ofPushMatrix();
        ofVec3f axis;
        float angle;
        curRot.getRotate(angle, axis);
        
        //apply the quaternion's rotation to the viewport and draw the sphere
        ofRotate(angle, axis.x, axis.y, axis.z);
    }

    //--------------------------------------------------------------
    void end() {
        ofPopMatrix();
    }

    //--------------------------------------------------------------
    void mouseDragged(int x, int y, int button){
        if(!enabled) return;
        
        //every time the mouse is dragged, track the change
        //accumulate the changes inside of curRot through multiplication
        ofVec2f mouse(x,y);
        ofQuaternion yRot((x-lastMouse.x)*dampen, ofVec3f(0,1,0));
        ofQuaternion xRot((y-lastMouse.y)*dampen, ofVec3f(-1,0,0));
        curRot *= yRot*xRot;
        lastMouse = mouse;
    }
    
    //--------------------------------------------------------------
    void mousePressed(int x, int y, int button){
        //store the last mouse point when it's first pressed to prevent popping
        if(!enabled) return;
        
        lastMouse = ofVec2f(x,y);
    }
};