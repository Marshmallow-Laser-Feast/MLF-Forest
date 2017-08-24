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
#include "ofxMSAControlFreakGui.h"
//#include "ofxMidi.h"
#include "ofxAssimpModelLoader.h"
#include "ofxOsc.h"
class testApp : public ofBaseApp {
//, public ofxMidiListener {

public:
    void setup();
    void update();
    void draw();
    void exit();
    
    void keyPressed  (int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void receiveOsc();
//    virtual void newMidiMessage(ofxMidiMessage& msg);
    
    ofxAssimpModelLoader venueModel;
    
    void drawFloor();
    
    msa::controlfreak::ParameterGroup params;
    msa::controlfreak::gui::Gui gui;
    
    ofxOscReceiver oscReceiver;

    void updateFilesGroup(string paramPath, string filePath, bool bAddNone);
    void checkAndInitRodLayout(bool bForceUpdate = false);
    void updateRodLaserAnimation();
    void checkAndInitPerformers(bool bForceUpdate);
    void checkAndInitFbo(bool bForceUpdate = false);
    void updateCamera();
    void updateRandomMusic();
    void updatePerformanceAnimation();
    void updateRodTuning();
    void resetAll();
    void sendRodOsc(bool bForce);
};
