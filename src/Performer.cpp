//
//  Performer.cpp
//  Forest Performance Simulator
//
//  Created by Memo Akten on 30/01/2013.
//
//

#include "Performer.h"


ofxAssimpModelLoader *Performer::model = NULL;
ofVec3f Performer::worldMin;
ofVec3f Performer::worldMax;
float Performer::noiseAmount;
float Performer::noiseFreq;
bool Performer::updateFromAnimation = false;
