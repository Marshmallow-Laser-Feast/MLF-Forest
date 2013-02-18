#include "testApp.h"
#include "ofxMSAControlFreakGui.h"
#include "Rod.h"
#include "Performer.h"
#include "ScaleManager.h"
#include "ofxOpenCv.h"
#include "ofxAssimpModelLoader.h"
#include "ofxOsc.h"

/*
 TODO:
 - load layout from 3D file
 
 */

msa::controlfreak::ParameterGroup params;
msa::controlfreak::gui::Gui gui;

vector<Rod> rods;
vector<Performer> performers;

ScaleManager scaleManager;

vector<ofCamera*> cameras;
ofEasyCam easyCam;

vector<ofLight*> lights;
ofFbo fbo;


ofSoundPlayer sound;
ofVec3f mouse3d;
ofxAssimpModelLoader venueModel;

ofImage layoutImage;
ofxCvContourFinder layoutImageContours;

ofVideoPlayer animationVideo;
ofxCvContourFinder animationVideoContours;


ofVec3f installationSize;
//ofVec3f rodCount;

ofxOscSender *oscSender = NULL;



//--------------------------------------------------------------
ofVec3f windowToWorld(float x, float y) {
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY, winZ;
    GLdouble posX, posY, posZ;
    
    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    glGetDoublev( GL_PROJECTION_MATRIX, projection );
    glGetIntegerv( GL_VIEWPORT, viewport );
    
    winX = (float)x;
    winY = (float)viewport[3] - (float)y;
    glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
    
    gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
    
    return ofVec3f(posX, posY, posZ);
}

//--------------------------------------------------------------
void updateFilesGroup(string paramPath, string filePath, bool bAddNone) {
    msa::controlfreak::ParameterNamedIndex &paramFiles = params.get<msa::controlfreak::ParameterNamedIndex>(paramPath);
    ofDirectory dir;
    dir.listDir(filePath);
    paramFiles.clearLabels();
    if(bAddNone) paramFiles.addLabel("<NONE>");
    for(int i=0; i<dir.size(); i++) {
        paramFiles.addLabel(dir.getPath(i));
    }
}

void sendRodOsc(bool bForce = false);


//--------------------------------------------------------------
void testApp::setup() {
    ofSetVerticalSync(true);
    ofSetFrameRate(30);
    
    // lights
    lights.resize(2);
    for(int i=0; i<lights.size(); i++) {
        lights[i] = new ofLight();
        lights[i]->setSpecularColor(ofColor(0, 0, 0));
    }
//    lights[0]->setGlobalPosition(-1294, 41, 4199);
//    lights[0]->setDiffuseColor(ofFloatColor(0.7, 0.7, 0.7));
//    lights[1]->setGlobalPosition(294, 133, 7176);
//    lights[1]->setDiffuseColor(ofFloatColor(0.4, 0.4, 0.4));
//    lights[2]->setGlobalPosition(1716, 836, -1773);
//    lights[2]->setDiffuseColor(ofFloatColor(0.9, 0.9, 0.9));
    
    
//    // sound
//    sound.loadSound("audio/vibe2.wav");
//    //    sound.loadSound("audio/sinfade.wav");
////    sound.loadSound("audio/sinfade2.wav");
//    sound.setMultiPlay(true);
//    sound.setLoop(false);
    scaleManager.setup();
    
    params.setName("Laser Forest");
    
    params.startGroup("fbo").close(); {
        params.addBool("enabled");
        params.addInt("overSampling").setRange(1, 4).setClamp(true).setSnap(true).set(1);
        params.addInt("numSamples").setRange(1, 16).setClamp(true);
        params.addNamedIndex("internalFormat").setLabels(3, "GL_RGBA", "GL_RGBA16F", "GL_RGBA32F");
    } params.endGroup();
    
    params.startGroup("display").close(); {
        params.addInt("backgroundColor").setClamp(true).set(60);
        params.addInt("floorColor").setClamp(true).set(60);
        params.addBool("showPitchIndex").trackVariable(&Rod::showPitchIndex);
        params.startGroup("lighting"); {
            params.addBool("enabled");
            for(int i=0; i<lights.size(); i++) {
                params.startGroup(ofToString(i)); {
                    params.addFloat("brightness").setClamp(true);
                    params.addInt("x").setRange(-5000, 5000).setClamp(true);
                    params.addInt("y").setRange(-5000, 5000).setClamp(true);
                    params.addInt("z").setRange(-5000, 5000).setClamp(true);
                } params.endGroup();
            }
        } params.endGroup();
    } params.endGroup();
    
    params.startGroup("camera").close(); {
        params.addNamedIndex("view").setLabels(3, "perspective", "top", "first person");
        //            params.addBang("top view");
        //            params.addBool("orthogonal");
        params.addInt("distance").setRange(0, 10000).setClamp(true).set(5000);
        params.addInt("fov").setRange(1, 100).setClamp(true).set(40);
        params.addInt("trackPerson").setClamp(true);
        params.addInt("rotx").setRange(-360, 360).setClamp(true);
        params.addInt("roty").setRange(-360, 360).setClamp(true);
        params.addInt("rotz").setRange(-360, 360).setClamp(true);
        //            params.startGroup("pos").close(); {
        //                params.addFloat("x");//.trackVariable(&cam.xRot);
        //                params.addFloat("y");//.trackVariable(&cam.yRot);
        //                params.addFloat("z");//.trackVariable(&cam.zRot);
        //            } params.endGroup();
    } params.endGroup();
    
    params.startGroup("layout").close(); {
        params.addInt("floorWidth").setTooltip("total floor width (cm)").setClamp(true).setRange(100, 10000).set(6000);
        params.addInt("floorLength").setTooltip("total floor length (cm)").setClamp(true).setRange(100, 10000).set(3000);
        params.addFloat("installationWidth").setTooltip("installation width (cm)").setClamp(true).setRange(100, 10000).set(3000).trackVariable(&installationSize.x);
        params.addFloat("installationLength").setTooltip("installation length (cm)").setClamp(true).setRange(100, 10000).set(1500).trackVariable(&installationSize.z);
//        params.addBool("useImage").setTooltip("use a black & white image for rod layout (rodCountWidth & rodCountLength will be ignored)");
        params.addInt("rodCountWidth").setTooltip("number of rods in width (ignored if an image is used)").setClamp(true).setRange(1, 60).set(20);//.trackVariable(&rodCount.x);
        params.addInt("rodCountLength").setTooltip("number of rods in length (ignored if an image is used)").setClamp(true).setRange(1, 60).set(10);//.trackVariable(&rodCount.z);
        params.addInt("randomness").setTooltip("amount of randomness in position (cm)").setRange(0, 1000).setClamp(true);
        params.addNamedIndex("image").setTooltip("use a black & white image for rod layout (rodCountWidth & rodCountLength will be ignored)");
    } params.endGroup();
    params.startGroup("rods").close(); {
        params.addInt("heightMin").setTooltip("minimum rod height (cm)").setRange(1, 1000).setClamp(true).set(180).trackVariable(&Rod::heightMin);
        params.addInt("heightMax").setTooltip("maximum rod height (cm)").setRange(1, 1000).setClamp(true).set(300).trackVariable(&Rod::heightMax);
        params.addInt("diameterMin").setTooltip("minimum rod diamater (cm)").setRange(1, 50).setClamp(true).set(3).trackVariable(&Rod::diameterMin);
        params.addInt("diameterMax").setTooltip("maximum rod diamater (cm)").setRange(1, 50).setClamp(true).set(10).trackVariable(&Rod::diameterMax);;
        params.addInt("color").setRange(0, 255).setClamp(true).set(60).trackVariable(&Rod::color);
        params.addInt("angleAmp").setRange(0, 90).setClamp(true).trackVariable(&Rod::angleAmp);
        params.addFloat("fadeSpeed").setClamp(true).trackVariable(&Rod::fadeSpeed);
        params.addInt("laserHeight").setRange(0, 10000).setClamp(true).trackVariable(&Rod::laserHeight);
        params.addInt("laserDiameter").setRange(1, 50).setClamp(true).trackVariable(&Rod::laserDiameter);
    } params.endGroup();
    
    //    params.startGroup("ssao"); {
    //        params.addBool("enabled");
    //        params.addFloat("weight").setRange(0, 100).setClamp(true).set(1);
    //        params.addFloat("radius").setRange(0, 500).setClamp(true).set(100);
    //        params.addFloat("maxThreshold").setRange(0, 1).setClamp(true).set(0.1);
    //        params.addFloat("minThreshold").setRange(0, 1).setClamp(true).set(0.01);
    //        params.addFloat("exponent").setRange(0, 20).setClamp(true).set(1);
    //        params.addInt("numSamples").setRange(0, 250).setClamp(true).set(24);
    //        params.addBool("rayReflection");
    //    } params.endGroup();
    
    params.startGroup("performers"); {
        params.addInt("count").setTooltip("number of performers").setRange(0, 30).setClamp(true);
        params.addInt("heightMin").setTooltip("minimum performer height (cm)").setRange(1, 200).setClamp(true).set(100);
        params.addInt("heightMax").setTooltip("maximum performer height (cm)").setRange(1, 200).setClamp(true).set(150);
        params.addInt("speedMin").setTooltip("minimum performer speed (cm/s)").setRange(1, 500).setClamp(true).set(10);
        params.addInt("speedMax").setTooltip("maximum performer speed (cm/s)").setRange(1, 500).setClamp(true).set(20);
        params.addInt("color").setClamp(true).set(60);
        params.addFloat("affectRadius").setRange(0, 2).setClamp(true);
        params.addFloat("noiseAmount").setRange(0, 20).setClamp(true).trackVariable(&Performer::noiseAmount);
        params.addFloat("noiseFreq").setRange(0, 0.1).setClamp(true).trackVariable(&Performer::noiseFreq);
    } params.endGroup();
    
    params.startGroup("animation"); {
        params.startGroup("laser"); {
            params.addNamedIndex("file").setTooltip("use a black & white PJPG quicktime for laser animation");
            params.addBool("loop");
            params.addFloat("speed").setRange(0, 4).setClamp(true).setSnap(true);
        } params.endGroup();
        params.addNamedIndex("performance").setTooltip("use a black & white PJPG quicktime for performance animation");
        params.addInt("blurAmount").setClamp(true).setRange(1, 33).setIncrement(2).setSnap(true);
        params.addInt("threshold").setRange(0, 255).setClamp(true);
    } params.endGroup();
    
//    params.startGroup("sweeps"); {
//        params.addNamedIndex("direction").setLabels(5, "radial in", "radial out", "left to right", "right to left", "front to back", "back to front");
//        params.addBang("go");
//        params.addFloat("speed").setRange(0, 5).setClamp(true);
////        params.addBang("radial in");
////        params.addFloat("radial in speed").setRange(0, 5).setClamp(true);
////        params.addBang("radial out");
////        params.addFloat("radial out speed").setRange(0, 5).setClamp(true);
////        params.addBang("left to right");
////        params.addFloat("left to right speed").setRange(0, 5).setClamp(true);
////        params.addBang("right to left");
////        params.addFloat("right to left speed").setRange(0, 5).setClamp(true);
////        params.addBang("front to back");
////        params.addFloat("front to back speed").setRange(0, 5).setClamp(true);
////        params.addBang("back to front");
////        params.addFloat("back to front speed").setRange(0, 5).setClamp(true);
//    } params.endGroup();
    
    params.startGroup("sound"); {
        params.addFloat("volumeVariance").setClamp(true).set(1);
        params.addFloat("retriggerThreshold").setClamp(true).set(1);
        params.startGroup("local"); {
            params.addBool("enabled").set(true);
            params.addNamedIndex("file").setTooltip("select file");
            params.addFloat("outputPitchMult").setRange(0, 2).setClamp(true).setIncrement(0.005).setSnap(true).set(1);
        } params.endGroup();
        params.startGroup("osc"); {
            params.addBool("enabled").set(true);
            params.addInt("port").setRange(0, 100000).setClamp(true).set(57120);
            params.addFloat("outputPitchMult").setRange(0, 2).setClamp(true).setIncrement(0.005).setSnap(true).set(1);
            params.addInt("volumePower").setRange(1, 8).setClamp(true).set(1);
        } params.endGroup();
        
    } params.endGroup();
    params.startGroup("tuning"); {
        params.addNamedIndex("scale").setLabels(scaleManager.scaleNames)/*.setMode(msa::controlfreak::ParameterNamedIndex::kList)*/.trackVariable(&scaleManager.currentIndex);
        params.addInt("noteCountWidth").setTooltip("how many notes are mapped from center to outer edge on width").setRange(0, 128).setClamp(true).set(8);
        params.addInt("noteCountLength").setTooltip("how many notes are mapped from center to outer edge on length").setRange(0, 128).setClamp(true).set(8);
        params.addInt("noteCountRadial").setTooltip("how many notes are mapped around the circumference").setRange(0, 128).setClamp(true).set(8);
        params.addInt("noteCountDistance").setTooltip("how many notes are mapped to the distance from center").setRange(0, 128).setClamp(true).set(8);
        params.addInt("inputPitchOffset").setRange(0, 16).setClamp(true).set(0);
        params.addInt("maxNoteCount").setRange(0, 50).setClamp(true).set(0);
        params.addFloat("volumePitchMult").setTooltip("make higher sounds lower volume").setClamp(true);
        params.addBool("invert").setTooltip("invert pitch relationship from out to in");
        
    } params.endGroup();

    
    updateFilesGroup("sound.local.file", "audio", false);
    updateFilesGroup("layout.image", "layout", true);
    updateFilesGroup("animation.laser.file", "animations/laser", true);
    updateFilesGroup("animation.performance", "animations/performance", true);

//    updateSoundFiles();
//    updatelayout();
//    updateLaserAnimations();
    
    params.loadXmlValues();
    
    gui.addPage(params);
    gui.setDefaultKeys(false);
    
    // camera
    cameras.resize(params.get<msa::controlfreak::ParameterNamedIndex>("camera.view").size());
    
    easyCam.disableMouseInput();
    cameras[0] = &easyCam;
    
    cameras[1] = new ofCamera;
    cameras[1]->setGlobalPosition(0, 2000, 0);
    cameras[1]->lookAt(ofVec3f(0, 0, 0));
    cameras[1]->setFov(10);
    //    cameras[1]->enableOrtho();
    
    cameras[2] = new ofCamera;
    
    
    
    venueModel.loadModel("3d/venue.dae");
    venueModel.setScaleNomalization(false);
    venueModel.setScale(1, -1, 1);
    
    sendRodOsc(true);
}


//--------------------------------------------------------------
//void updateRods(bool bForceUpdate = false) {
//    msa::controlfreak::ParameterGroup &paramsRods = params.getGroup("rods");
//    if(bForceUpdate || paramsRods.hasChanged()) {
//        ofLogNotice() << "updateRods at frame " << ofGetFrameNum();
//        
////        float heightMin = paramsRods["heightMin"];
////        float heightMax = paramsRods["heightMax"];
////        float diameterMin = paramsRods["diameterMin"];
////        float diameterMax = paramsRods["diameterMax"];
//        float color = paramsRods["color"].getMappedTo(0, 255);
//        
//        Rod::fadeSpeed = paramsRods["fadeSpeed"];
//        Rod::angleAmp = paramsRods["angleAmp"];
//        Rod::laserHeight = paramsRods["laserHeight"];
//        Rod::laserDiameter = paramsRods["laserDiameter"];
//        
//        for(int i=0; i<rods.size(); i++) {
//            Rod &r = rods[i];
////            r.height = ofLerp(heightMin, heightMax, r.heightNorm);
////            r.radius = ofLerp(diameterMin, diameterMax, r.radiusNorm)/2;
//            r.color = ofColor(color);
//        }
//    }
//}


//--------------------------------------------------------------
void updateRodLayout(bool bForceUpdate = false) {
    msa::controlfreak::ParameterGroup &paramsLayout = params.getGroup("layout");
    if(bForceUpdate || paramsLayout.hasChanged())  {
        ofLogNotice() << "updateRodLayout at frame " << ofGetFrameNum();
        
//        updatelayout();
        
        int installationWidth = paramsLayout["installationWidth"];
        int installationLength = paramsLayout["installationLength"];
        int rodCountWidth = paramsLayout["rodCountWidth"];
        int rodCountLength = paramsLayout["rodCountLength"];
        float randomness = paramsLayout["randomness"];
//        bool useImage = paramsLayout["useImage"];
        int useImageIndex = paramsLayout["image"];
        
        if(useImageIndex > 0) {
//            layoutImage.loadImage("layout/" + paramsLayout.get<msa::controlfreak::ParameterNamedIndex>("layoutImage").getSelectedLabel());
            layoutImage.loadImage(paramsLayout.get<msa::controlfreak::ParameterNamedIndex>("image").getSelectedLabel());
            
            ofxCvColorImage colorImage;
            colorImage.allocate(layoutImage.getWidth(), layoutImage.getHeight());
            colorImage.setFromPixels(layoutImage);
            
            ofxCvGrayscaleImage greyImage;
            greyImage.allocate(layoutImage.getWidth(), layoutImage.getHeight());
            greyImage = colorImage;
            
            greyImage.threshold(50);
            
            layoutImageContours.findContours(greyImage, 0, greyImage.getWidth() * greyImage.getHeight(), 100000, false);
            
//            rods.resize(layoutImageContours.blobs.size());
            rods.clear();
            for(int i=0; i<layoutImageContours.nBlobs; i++) {
                rods.push_back(Rod());
                Rod &r = rods[i];
                ofxCvBlob &blob = layoutImageContours.blobs[i];
                float x = ofMap(blob.centroid.x, 0, greyImage.getWidth(), -installationWidth/2, installationWidth/2);
                float z = ofMap(blob.centroid.y, 0, greyImage.getHeight(), -installationLength/2, installationLength/2);
                r.setGlobalPosition(x, 0, z);
            }
            
        } else {
            layoutImage.clear();
            
//            rods.resize(rodCountWidth * rodCountLength);
            rods.clear();
            for(int i=0; i<rodCountWidth; i++) {
                for(int j=0; j<rodCountLength; j++) {
                    rods.push_back(Rod());
                    Rod &r = rods[j*rodCountWidth + i];
                    float x = ofMap(i, 0, rodCountWidth-1, -installationWidth/2, installationWidth/2);
                    float z = ofMap(j, 0, rodCountLength-1, -installationLength/2, installationLength/2);
                    r.setGlobalPosition(x, 0, z);
                }
            }
        }
        
        for(int i=0; i<rods.size(); i++) {
            Rod &r = rods[i];
            r.setup();
            r.move(randomness * ofVec3f(ofRandomf(), 0, ofRandomf()));
        }
        
//        updateRods(true);
        
        Performer::worldMin.set(-installationWidth/2, 0, -installationLength/2);
        Performer::worldMax.set( installationWidth/2, 0, installationLength/2);
    }
}


//--------------------------------------------------------------
void updatePerformers(bool bForceUpdate = false) {
    msa::controlfreak::ParameterGroup &paramsPerformers = params.getGroup("performers");
    if(bForceUpdate || paramsPerformers.hasChanged()) {
        ofLogNotice() << "updatePerformers at frame " << ofGetFrameNum();
        
        float heightMin = paramsPerformers["heightMin"];
        float heightMax = paramsPerformers["heightMax"];
        float speedMin = paramsPerformers["speedMin"];
        float speedMax = paramsPerformers["speedMax"];
        float color = paramsPerformers["color"].getMappedTo(0, 255);
        float affectRadius = paramsPerformers["affectRadius"];
        
        
        bool doVel = paramsPerformers["speedMin"].hasChanged() || paramsPerformers["speedMax"];
        bool doSetup = paramsPerformers["count"].hasChanged();
        if(bForceUpdate || doSetup) {
//            performers.resize(paramsPerformers["count"]); // for some reason resize screws up the init somehow!
            performers.clear();
            int count = paramsPerformers["count"];
            for(int i=0; i<count; i++) performers.push_back(Performer());
        }
        
        for(int i=0; i<performers.size(); i++) {
            Performer &p = performers[i];
            if(bForceUpdate || doSetup) p.setup();
            p.height = ofLerp(heightMin, heightMax, p.heightNorm);
            if(doVel) {
                p.speed = ofLerp(speedMin, speedMax, p.speedNorm);
            }
            p.color = ofColor(color);
            p.affectRadiusNorm = affectRadius;
        }
    }
}

//--------------------------------------------------------------
void updateFbo(bool bForceUpdate = false) {
    msa::controlfreak::ParameterGroup &paramsFbo = params.getGroup("fbo");
    if(bForceUpdate || paramsFbo.hasChanged()) {
        ofLogNotice() << "updateFbo at frame " << ofGetFrameNum();
        
        float overSampling = params["fbo.overSampling"];
        int internalFormat;
        switch((int)params["fbo.internalFormat"]) {
            case 1: internalFormat = GL_RGBA16F; break;
            case 2: internalFormat = GL_RGBA32F; break;
                
            case 0:
            default: internalFormat = GL_RGBA; break;
        }
        fbo.allocate(ofGetWidth() * overSampling, ofGetHeight() * overSampling, internalFormat, params["fbo.numSamples"]);
    }
}

//--------------------------------------------------------------
void updateCamera() {
    msa::controlfreak::ParameterGroup &paramsCamera = params.getGroup("camera");
    
    if(params["performers.count"].hasChanged()) paramsCamera["trackPerson"].setRange(1, (int)params["performers.count"]);
    
    if(paramsCamera["trackPerson"].hasChanged()) cameras[2]->setParent(performers[paramsCamera["trackPerson"]]);
    
    //    if(paramsCamera["top view"]) {
    //        cam.setGlobalPosition(0, 100, 0);
    //        //        arcball.curRot = ofQuaternion();
    //    }
    //
    //    if(paramsCamera["orthogonal"].hasChanged()) {
    //        if(paramsCamera["orthogonal"]) {
    //            cam.enableOrtho();
    //            //            cam.setGlobalPosition(-ofGetWidth()/2, paramsCamera["distance"],-ofGetHeight()/2);
    //        } else {
    //            cam.disableOrtho();
    //            //            cam.setGlobalPosition(0, paramsCamera["distance"], 0);
    //        }
    //        //        cam.lookAt(ofVec3f(0, 0, 0));
    //    }
    
    easyCam.setDistance(paramsCamera["distance"]);
    easyCam.setFov(paramsCamera["fov"]);
    
    //    cam.rotateAround(paramsCamera["yrot"], ofVec3f(0, 1, 0), ofVec3f(0, 0, 0));
    //    cam.rotateAround(paramsCamera["xrot"], ofVec3f(1, 0, 0), ofVec3f(0, 0, 0));
    
    //    cam.move(paramsCamera["x"], paramsCamera["y"], 0);
    
    //    ofTranslate(ofGetWidth()/2, ofGetHeight()/2, -(float)paramsCamera["distance"]);
    //    ofTranslate(0, 0, -(float)paramsCamera["distance"]);
}

//--------------------------------------------------------------
//void updateSSAO() {
//    ssao.setWeight(params["ssao.weight"]);
//    ssao.setRadius(params["ssao.radius"]);
//    ssao.setMaxThreshold(params["ssao.maxThreshold"]);
//    ssao.setMinThreshold(params["ssao.minThreshold"]);
//    ssao.setExponent(params["ssao.exponent"]);
//    ssao.setNumSamples(params["ssao.numSamples"]);
//    ssao.setRayReflection(params["ssao.rayReflection"]);
//}

//--------------------------------------------------------------
void checkRodCollisions(ofVec3f p, float radius) {
    float outputPitchMult = params["sound.local.outputPitchMult"];
    float volumeVariance = params["sound.volumeVariance"];
    float retriggerThreshold = params["sound.retriggerThreshold"];
    int maxNoteCount = params["tuning.maxNoteCount"];
    float volumePitchMult = params["tuning.volumePitchMult"];
    ofSoundPlayer *psound = params["sound.local.enabled"] ? &sound : NULL;
    
    for(int i=0; i<rods.size(); i++) {
        Rod &r = rods[i];
        if((p - r.getGlobalPosition()).lengthSquared() < radius * radius) r.trigger(retriggerThreshold, scaleManager, outputPitchMult, maxNoteCount, volumePitchMult, volumeVariance, psound);
    }
//    return bRet;
}


//--------------------------------------------------------------
void updateLaserAnimation() {
    msa::controlfreak::ParameterNamedIndex &paramNamedIndex = params.get<msa::controlfreak::ParameterNamedIndex>("animation.laser.file");
    if(paramNamedIndex.hasChanged()) {
        if((int)paramNamedIndex == 0) {
            animationVideo.close();
        } else {
            params["animation.performance"] = 0;
            params["animation.performance"].clearChanged();
            params["performers.count"] = 0;
            animationVideo.loadMovie(paramNamedIndex.getSelectedLabel());
            animationVideo.play();
        }
    }
    
    
    // if video is loaded and a it's a laser animation
    if(animationVideo.isLoaded() && (int)paramNamedIndex > 0) {
        if(params["animation.laser.loop"].hasChanged()) animationVideo.setLoopState(params["animation.laser.loop"] ? OF_LOOP_NORMAL : OF_LOOP_NONE);
        if(params["animation.laser.speed"].hasChanged()) animationVideo.setSpeed(params["animation.laser.speed"]);

        animationVideo.update();
        
        float outputPitchMult = params["sound.local.outputPitchMult"];
        float volumeVariance = params["sound.volumeVariance"];
        float retriggerThreshold = params["sound.retriggerThreshold"];
        int maxNoteCount = params["tuning.maxNoteCount"];
        float volumePitchMult = params["tuning.volumePitchMult"];
        ofSoundPlayer *psound = params["sound.local.enabled"] ? &sound : NULL;

        ofPixelsRef pixels = animationVideo.getPixelsRef();
        for(int i=0; i<rods.size(); i++) {
            Rod &r = rods[i];
            ofVec2f imagePos;
            imagePos.x = ofMap(r.getX(), -installationSize.x/2, installationSize.x/2, 0, animationVideo.getWidth());
            imagePos.y = ofMap(r.getZ(), -installationSize.z/2, installationSize.z/2, 0, animationVideo.getHeight());
            if(pixels.getColor(imagePos.x, imagePos.y).r > 0) r.trigger(retriggerThreshold, scaleManager, outputPitchMult, maxNoteCount, volumePitchMult, volumeVariance, psound);
            else r.value = 0;
        }
    }
}


//--------------------------------------------------------------
void updatePerformanceAnimation() {
    msa::controlfreak::ParameterNamedIndex &paramNamedIndex = params.get<msa::controlfreak::ParameterNamedIndex>("animation.performance");
    if(paramNamedIndex.hasChanged()) {
        if((int)paramNamedIndex == 0) {
            animationVideo.close();
        } else {
            params["animation.laser.file"] = 0;
            params["animation.laser.file"].clearChanged();
            params["performers.count"] = 0;

            //            animationVideo.loadMovie("animations/laser/" + paramNamedIndex.getSelectedLabel());
            animationVideo.loadMovie(paramNamedIndex.getSelectedLabel());
            animationVideo.setLoopState(OF_LOOP_NORMAL);
            animationVideo.play();
        }
    }
    
    
    // if video is loaded and a it's a performance animation
    if(animationVideo.isLoaded() && (int)paramNamedIndex > 0) {
        Performer::updateFromAnimation = true;
        
        animationVideo.update();

        ofxCvColorImage colorImage;
        colorImage.allocate(animationVideo.getWidth(), animationVideo.getHeight());
        colorImage.setFromPixels(animationVideo.getPixelsRef());
        
        ofxCvGrayscaleImage greyImage;
        greyImage.allocate(animationVideo.getWidth(), animationVideo.getHeight());
        greyImage = colorImage;
        
        greyImage.blur(params["animation.blurAmount"]);
        greyImage.threshold(params["animation.threshold"]);
        
        animationVideoContours.findContours(greyImage, 0, greyImage.getWidth() * greyImage.getHeight(), 100000, false);
        
        int numBlobs = animationVideoContours.blobs.size();
        if((int)params["performers.count"] != numBlobs) {
            params["performers.count"] = numBlobs;
            updatePerformers(true);
        }
        float heightMin = params["performers.heightMin"];
        float heightMax = params["performers.heightMax"];

        for(int i=0; i<numBlobs; i++) {
            Performer &p = performers[i];
            ofxCvBlob &blob = animationVideoContours.blobs[i];
            float x = ofMap(blob.centroid.x, 0, greyImage.getWidth(), -installationSize.x/2, installationSize.x/2);
            float z = ofMap(blob.centroid.y, 0, greyImage.getHeight(), -installationSize.z/2, installationSize.z/2);
            p.setGlobalPosition(x, 0, z);
            p.heightNorm = 0.5;
            p.height = ofLerp(heightMin, heightMax, p.heightNorm);

        }
    } else {
        Performer::updateFromAnimation = false;
    }
}


//--------------------------------------------------------------
void updateSound() {
    //    sound.loadSound("audio/vibe2.wav");
    //    //    sound.loadSound("audio/sinfade.wav");
    ////    sound.loadSound("audio/sinfade2.wav");
    //    sound.setMultiPlay(true);
    //    sound.setLoop(false);

    
    msa::controlfreak::ParameterNamedIndex &paramNamedIndex = params.get<msa::controlfreak::ParameterNamedIndex>("sound.local.file");
    if(paramNamedIndex.hasChanged()) {
            //            animationVideo.loadMovie("animations/laser/" + paramNamedIndex.getSelectedLabel());
        sound.loadSound(paramNamedIndex.getSelectedLabel());
        sound.setMultiPlay(true);
        sound.setLoop(false);
    }
    
}

//--------------------------------------------------------------
void updateRodTuning() {
//    ofVec3f installationSize(params["layout.installationWidth"], 0, params["layout.installationLength"]);
    ofVec3f noteCount(params["tuning.noteCountWidth"], 0, params["tuning.noteCountLength"]);
    int noteCountRadial = params["tuning.noteCountRadial"];
    int noteCountDistance = params["tuning.noteCountDistance"];
    bool invert = params["tuning.invert"];
    int maxNoteCount = params["tuning.maxNoteCount"];
    int inputPitchOffset = params["tuning.inputPitchOffset"];
    float halfInstallationLength =  installationSize.x/2;//length()/2;  // TODO: hack?
    
    bool bRet = false;
    for(int i=0; i<rods.size(); i++) {
        Rod &r = rods[i];
//        ofVec3f normPos = r.getGlobalPosition()  / (installationSize/2);
//        if(invert) {
//            normPos.x = ofSign(normPos.x) * (1 - fabsf(normPos.x));
//            normPos.y = ofSign(normPos.y) * (1 - fabsf(normPos.y));
//            normPos.z = ofSign(normPos.z) * (1 - fabsf(normPos.z));
//        }
//        ofVec3f vpitchIndex = normPos * noteCount;
        r.pitchIndex = 0;
//        r.pitchIndex += fabs(vpitchIndex.x) + fabs(vpitchIndex.z);
        float distRatio = r.getGlobalPosition().length() / halfInstallationLength;
        if(invert) distRatio = 1-distRatio;
        r.pitchIndex += distRatio * noteCountDistance;
        float angle = atan2(r.getX(), r.getZ());
        angle = fabsf(angle);
        if(angle > PI/2) angle = PI - angle;
        if(r.getGlobalPosition().length() > 100) {  // hack to include radial only in rods not in center
            r.pitchIndex += ofMap(angle, 0, PI, 0, noteCountRadial);
        }
        if(maxNoteCount > 0) {
            r.pitchIndex %= 2 * maxNoteCount;
            if(r.pitchIndex >= maxNoteCount) r.pitchIndex = 2 * maxNoteCount - 1 - r.pitchIndex;  // mirror mod
        }
        r.pitchIndex += inputPitchOffset;
        if(maxNoteCount > 0) {
            r.heightNorm = ofNormalize(r.pitchIndex, 0, maxNoteCount);
        }
    }
}


//--------------------------------------------------------------
void resetAll() {
    updateRodLayout(true);
    //    updateRods(true);
    updatePerformers(true);
    updateFbo(true);
}

//--------------------------------------------------------------
void sendRodOsc(bool bForce) {
    if(params["sound.osc.enabled"].hasChanged()) {
        bForce = true;
        updateRodLayout(true);  // force zero if enabled state has changed
    }
    
    
    
    if(bForce || params["sound.osc.enabled"]) {
        if(params["sound.osc.port"].hasChanged() || oscSender == NULL) {
            if(oscSender) delete oscSender;
            oscSender = new ofxOscSender;
            oscSender->setup("127.0.0.1", params["sound.osc.port"]);
        }
        ofxOscBundle b;
        float outputPitchMult = params["sound.osc.outputPitchMult"];
        
        bool doSendTuning = params["tuning"].hasChanged() || params["sound.osc.outputPitchMult"].hasChanged();

        int volumePower = params["sound.osc.volumePower"];
        for(int i=0; i<rods.size(); i++){
            Rod &r = rods[i];
            
            ofxOscMessage m;
            if(bForce || doSendTuning) {
                m.setAddress("/forestFreq");
                m.addIntArg(i);
                m.addFloatArg(scaleManager.currentFreq(r.pitchIndex) * outputPitchMult);
                b.addMessage(m);
            }
            
            m.clear();
            m.setAddress("/forestAmp");
            m.addIntArg(i);
            float amp = 1;
            for(int i=0; i<volumePower; i++) {
                amp *= r.value;
                
            }
            m.addFloatArg(amp);
            b.addMessage(m);
        }
        oscSender->sendBundle(b);
    }
}


//--------------------------------------------------------------
void testApp::update(){
    msa::controlfreak::update();
    
    updateRodLayout();
//    updateRods();
    updatePerformers();
    updateFbo();
    updateSound();
    updateRodTuning();

    updateLaserAnimation();
    updatePerformanceAnimation();
    
    
    // check for performer-rod collision, and do sound
    for(int j=0; j<performers.size(); j++) {
        Performer &p = performers[j];
        checkRodCollisions(p.getGlobalPosition(), p.affectRadius);
    }
    
    sendRodOsc();
    //    updateSSAO();
}


//--------------------------------------------------------------
void drawFloor() {
    int floorWidth = params["layout.floorWidth"];
    int floorLength = params["layout.floorLength"];
    ofSetColor((int)params["display.floorColor"].getMappedTo(0, 255));
    ofBeginShape();
    ofVertex(-floorWidth/2, 0, -floorLength/2);
    ofVertex(floorWidth/2, 0, -floorLength/2);
    ofVertex(floorWidth/2, 0, floorLength/2);
    ofVertex(-floorWidth/2, 0, floorLength/2);
    ofEndShape(true);
    
    venueModel.drawFaces();
}

//--------------------------------------------------------------
void testApp::draw() {
    glEnable(GL_DEPTH_TEST);
    ofEnableAlphaBlending();
    ofClear((int)params["display.backgroundColor"].getMappedTo(0, 255));
    
    //    bool doSSAO = params["ssao.enabled"];
    bool doFbo = params["fbo.enabled"];
    
    //    if(doSSAO) ssao.begin();
    if(doFbo) {
        fbo.begin(false);
        ofClear((int)params["display.backgroundColor"].getMappedTo(0, 255));
    }
    
    if(params["display.lighting.enabled"]) {
        for(int i=0; i<lights.size(); i++) {
            msa::controlfreak::ParameterGroup &p = params.getGroup("display.lighting." + ofToString(i));
            ofLight &light = *lights[i];
            light.setPosition(p["x"], p["y"], p["z"]);
            float brightness = p["brightness"];
            light.setDiffuseColor(ofFloatColor(brightness, brightness, brightness));
            light.enable();
        }
    } else {
        ofDisableLighting();
    }
    
    
    ofCamera &cam = *cameras[ (int)params["camera.view"] ];
    cam.setNearClip(500);
    updateCamera();
    //    params["display.camera.pos.x"] = cam.getGlobalPosition().x;
    //    params["display.camera.pos.y"] = cam.getGlobalPosition().y;
    //    params["display.camera.pos.z"] = cam.getGlobalPosition().z;
    //    if(params["display.camera.pos"].hasChanged()) {
    //        cam.setGlobalPosition(params["display.camera.pos.x"], params["display.camera.pos.y"], params["display.camera.pos.z"]);
    //    }
    cam.begin();
    
    //    arcball.begin();
    ofTranslate(0, -100, 0);
    ofRotateX(params["camera.rotx"]);
    ofRotateY(params["camera.roty"]);
    ofRotateZ(params["camera.rotz"]);
    
    ofPushStyle();
    
    drawFloor();
    for(int i=0; i<rods.size(); i++) rods[i].draw();
    for(int i=0; i<performers.size(); i++) {
        performers[i].draw();
        ofVec3f v = performers[i].getOrientationEuler();
    }
    for(int i=0; i<lights.size(); i++) lights[i]->draw();

    ofPopStyle();
    
    
    // draw mouse cursor
    {
        float r = 75;
        mouse3d = windowToWorld(ofGetMouseX(), ofGetMouseY());
        //        if(ofGetMousePressed())
        checkRodCollisions(mouse3d, r);
        
        ofPushStyle();
        ofSetColor(100, 0, 0, 50);
        ofSphere(mouse3d, r);
        ofPopStyle();
    }
    
    //    arcball.end();
    cam.end();
    
    //    if(doSSAO) {
    //        ssao.end();
    //        ssao.draw();
    //    }
    if(doFbo) {
        fbo.end();
        ofSetColor(255);
        fbo.draw(0, ofGetHeight(), ofGetWidth(), -ofGetHeight());
    }
    
    {
        ofDisableLighting();
        glDisable(GL_DEPTH_TEST);

        int w = 256;
        if(layoutImage.isAllocated()) {
            ofSetColor(255);
            layoutImage.draw(ofGetWidth()-w, 0, w, w);
            layoutImageContours.draw(ofGetWidth()-w, 0, w, w);
        }
        if(animationVideo.isLoaded()) {
            ofSetColor(255);
            animationVideo.draw(ofGetWidth()-w, w, w, w);
            animationVideoContours.draw(ofGetWidth()-w, w, w, w);
        }

    }
    ofSetColor(255);
    ofDrawBitmapString(ofToString(ofGetFrameRate(), 2), ofGetWidth() - 100, 30);
    
    
}

//--------------------------------------------------------------
void testApp::exit() {
    resetAll();
    sendRodOsc(true);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    switch(key) {
        case 's':
            params.saveXmlValues();
            break;
            
        case 'l':
            params.loadXmlValues();
            break;
            
        case 'f':
            ofToggleFullscreen();
            break;
            
        case ' ':
            //            arcball.enabled = true;
            easyCam.enableMouseInput();
            break;
            
        case 'r':
            resetAll();
            break;
            
    }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
    switch(key) {
        case ' ':
            easyCam.disableMouseInput();
            break;
            
    }
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
    //    arcball.mouseDragged(x, ofGetHeight()-y, button);
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
    //    arcball.mousePressed(x, ofGetHeight()-y, button);
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
    updateFbo(true);
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){
    
}