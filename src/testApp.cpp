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
#define DOING_SERIAL



#include "testApp.h"

#include "Rod.h"
#include "Performer.h"
#include "ScaleManager.h"
#include "ofxOpenCv.h"
//#include "ofxAssimpModelLoader.h"
#include "ofxOsc.h"
//#include "ofxMidi.h"


#ifdef DOING_SERIAL
#include "RodCommunicator.h"
#include "RodMapper.h"
#endif

vector<Rod*> rods;

vector<Performer> performers;

ScaleManager scaleManager;

vector<ofCamera*> cameras;
ofEasyCam easyCam;

vector<ofLight*> lights;
ofFbo fbo;


//ofSoundPlayer sound;

ofVec3f mouse3d;    // 3d coordinates of mouse
float mouseRadius = 50; // radius of mouse cursor
Rod *selectedRod = NULL; // rod the mouse is currently hitting

//ofxAssimpModelLoader venueModel;

ofImage layoutImage;
ofxCvContourFinder layoutImageContours;

ofVideoPlayer *animationVideo = NULL;
//vector<ofVideoPlayer*> laserAnimations;

ofxCvContourFinder animationVideoContours;

ofSoundPlayer messageAudio;


ofVec3f installationSize;

ofxOscSender *oscSender = NULL;
bool bSendRodPositionsOsc = false;
bool bSendRodTuningOsc = false;

#ifdef DOING_SERIAL
RodCommunicator *rodCommunicator;
bool showRodGui = true;
RodMapper rodMapper;

#endif


int animation_nextChangeMillis = 0;
int randomMusic_nextChangeMillis = 0;



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


#include <sys/stat.h>
#include <fstream>
#include <dirent.h>
#include <errno.h>


void listDirectory(string dir, vector<string> &files) {
    dir = ofToDataPath(dir);
    if(dir.rfind("/")!=dir.size()-1) dir += "/";

    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        cout << "Error(" << errno << ") opening " << dir << endl;
        return;
    }
    
    
    while ((dirp = readdir(dp)) != NULL) {
        string s = dirp->d_name;
        if(s.size()>0 && s[0]!='.') {
            files.push_back(dir + dirp->d_name);
        }
        
    }
    closedir(dp);
    for(auto &f: files) {
        printf("FILE: %s\n", f.c_str());
    }
    return;
}



//--------------------------------------------------------------
void testApp::updateFilesGroup(string paramPath, string filePath, bool bAddNone) {
    msa::controlfreak::ParameterNamedIndex &paramFiles = params.get<msa::controlfreak::ParameterNamedIndex>(paramPath);
    //ofDirectory dir;
    //printf("Listing %s\n", filePath.c_str());
    
    vector<string> dir;
    
    listDirectory(filePath, dir);
    //dir.listDir(filePath);
    //printf("Done listing\n");
    paramFiles.clearLabels();
    if(bAddNone) paramFiles.addLabel("<NONE>");
    //for(int i=0; i<dir.size(); i++) {
     //   paramFiles.addLabel(dir.getPath(i));
    //}
    for(auto d: dir) {
        paramFiles.addLabel(d);
    }
}

void sendRodOsc(bool bForce = false);


//--------------------------------------------------------------
void testApp::setup() {
	// TODO: remove log level set here
#pragma warning we'll want to get rid of this line
	ofSetLogLevel(OF_LOG_ERROR);
#ifdef DOING_SERIAL
	rodCommunicator = new RodCommunicator();
	rodCommunicator->start();
#endif
	
    ofSetVerticalSync(true);
    ofSetFrameRate(30);
    
    // lights
    lights.resize(2);
    for(int i=0; i<lights.size(); i++) {
        lights[i] = new ofLight();
        lights[i]->setSpecularColor(ofColor(0, 0, 0));
    }
    
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
        //        params.addBool("showPitchIndex").trackVariable(&Rod::showPitchIndex);
		params.addNamedIndex("idDisplayType").setLabels(9, "None", "Pitch Index", "Device ID", "Index", "Polar Coordinates", "Polar Coordinates Norm", "Radius", "Angle", "Name").trackVariable(&Rod::idDisplayType);
        params.addBool("bDisplaySelectedId");//.trackVariable(&Rod::bDisplaySelectedId);
		
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
        params.addInt("fov").setRange(1, 100).setClamp(true).set(40);
        params.addInt("trackPerson").setClamp(true);
        params.addInt("rotx").setRange(-360, 360).setClamp(true);
        params.addInt("roty").setRange(-360, 360).setClamp(true);
        params.addInt("rotz").setRange(-360, 360).setClamp(true);
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
        params.addFloat("angleOffset").setRange(-1, 1).setClamp(true);
        params.addBool("angleFlip");
    } params.endGroup();
    params.startGroup("rods").close(); {
        params.addInt("heightMin").setTooltip("minimum rod height (cm)").setRange(1, 1000).setClamp(true).set(180).trackVariable(&Rod::heightMin);
        params.addInt("heightMax").setTooltip("maximum rod height (cm)").setRange(1, 1000).setClamp(true).set(300).trackVariable(&Rod::heightMax);
        params.addInt("diameterMin").setTooltip("minimum rod diamater (cm)").setRange(1, 50).setClamp(true).set(3).trackVariable(&Rod::diameterMin);
        params.addInt("diameterMax").setTooltip("maximum rod diamater (cm)").setRange(1, 50).setClamp(true).set(10).trackVariable(&Rod::diameterMax);;
        params.addInt("color").setRange(0, 255).setClamp(true).set(60);//.trackVariable(&Rod::brightness);
        params.addInt("angleAmp").setRange(0, 90).setClamp(true).trackVariable(&Rod::angleAmp);
        params.addFloat("dampSpeed").setClamp(true).trackVariable(&Rod::dampSpeed);
        params.addInt("showRodAmp").setRange(0, 200).setClamp(true);
        params.addFloat("selectedRodAmp").setClamp(true);
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
    
    params.startGroup("laser").close(); {
        params.addBool("displayLaser").trackVariable(&Rod::displayLaser);
        params.addInt("height").setRange(0, 10000).setClamp(true).trackVariable(&Rod::laserHeight);
        params.addInt("diameter").setRange(1, 50).setClamp(true).trackVariable(&Rod::laserDiameter);
        params.addBool("alwaysOn").trackVariable(&Rod::bLaserAlwaysOn);
        params.addBool("random").trackVariable(&Rod::bLaserRandom);
        params.addInt("laserRandomSkipFrame").setClamp(true).set(2).trackVariable(&Rod::laserRandomSkipFrame);
        params.addFloat("laserRandomProbability").setClamp(true).set(0.3).trackVariable(&Rod::laserRandomProbability);
        params.addFloat("triggerThreshold").setTooltip("turn laser on when amp increases above this").trackVariable(&Rod::laserTriggerThreshold).setClamp(true).set(0.1);
        params.addFloat("cutoffThreshold").setTooltip("cut laser off when amp falls below this").trackVariable(&Rod::laserCutoffThreshold).setClamp(true).set(0.5);
    } params.endGroup();
    
    params.startGroup("performers").close(); {
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
    
    params.startGroup("randomMusic"); {
        params.addBool("enabled");
        params.addInt("randomChangeTimeMin").setTooltip("Change random animation after min this many seconds").setClamp(true);
        params.addInt("randomChangeTimeMax").setTooltip("Change random animation after max this many seconds").setClamp(true);;
        params.addFloat("randomAmpMin").setClamp(true);
        params.addFloat("randomAmpMax").setClamp(true);
        params.addBool("triggerOnMouse");
    } params.endGroup();
    
    params.startGroup("animation"); {
        params.startGroup("laser"); {
            params.addNamedIndex("file").setTooltip("use a black & white PJPG quicktime for laser animation");
            params.addBool("loop");
            params.addFloat("speed").setRange(0, 4).setClamp(true).setSnap(true);
            params.addBool("playRandom");
            params.addInt("randomChangeTimeMin").setTooltip("Change random animation after min this many seconds").setClamp(true);
            params.addInt("randomChangeTimeMax").setTooltip("Change random animation after max this many seconds").setClamp(true);;
        } params.endGroup();
        params.startGroup("performance"); {
            params.addNamedIndex("file").setTooltip("use a black & white PJPG quicktime for performance animation");
            params.addInt("blurAmount").setClamp(true).setRange(1, 33).setIncrement(2).setSnap(true);
            params.addInt("threshold").setRange(0, 255).setClamp(true);
            params.addFloat("speed").setRange(0, 4).setClamp(true).setSnap(true);
            params.addBool("play");
            params.addFloat("time").setIncrement(1).setClamp(true);
        } params.endGroup();
    } params.endGroup();
    
    params.startGroup("sound"); {
        params.addFloat("volumeVariance").setClamp(true).set(1);
        params.addFloat("retriggerThreshold").setClamp(true).set(1);
        params.startGroup("local").close(); {
            params.addBool("enabled").set(true);
            params.addNamedIndex("file").setTooltip("select file");
            params.addFloat("outputPitchMult").setRange(0, 2).setClamp(true).setIncrement(0.005).setSnap(true).set(1);
        } params.endGroup();
        params.startGroup("osc"); {
            params.addBool("enabled").set(true);
            params.addInt("port").setRange(0, 100000).setClamp(true).set(57120);
            params.addFloat("outputPitchMult").setTooltip("multiply final tunings by this much").setRange(0, 2).setClamp(true).setIncrement(0.005).setSnap(true).set(1);
            params.addInt("volumePower").setTooltip("mapping curve between amp of rod and volume").setRange(1, 8).setClamp(true).set(1);
            params.addBang("forceSend").setTooltip("send full osc of all rod tunings and positions");
            params.addInt("sendFullFrameCount").setTooltip("if this is non-zero, send full OSC every this many seconds").setRange(0, 60*10).setClamp(true);
        } params.endGroup();
        params.startGroup("compression").close(); {
            params.addFloat("totalVolume");
            params.addFloat("avgVolume");
        } params.endGroup();
    } params.endGroup();
    
    params.startGroup("tuning").close(); {
        params.addNamedIndex("scale").setLabels(scaleManager.scaleNames)/*.setMode(msa::controlfreak::ParameterNamedIndex::kList)*/.trackVariable(&scaleManager.currentIndex);
        params.addBool("useIndex");
        params.addFloat("indexMultipler").setRange(0, 10).setClamp(true).setIncrement(0.1).setSnap(true);
        params.addInt("noteCountWidth").setTooltip("how many notes are mapped from center to outer edge on width").setRange(0, 128).setClamp(true).set(8);
        params.addInt("noteCountLength").setTooltip("how many notes are mapped from center to outer edge on length").setRange(0, 128).setClamp(true).set(8);
        params.addInt("noteCountRadial").setTooltip("how many notes are mapped around the circumference").setRange(0, 128).setClamp(true).set(8);
        params.addInt("noteCountDistance").setTooltip("how many notes are mapped to the distance from center").setRange(0, 128).setClamp(true).set(8);
        params.addInt("inputPitchOffset").setRange(0, 16).setClamp(true).set(0);
        params.addInt("maxNoteCount").setRange(0, 50).setClamp(true).set(0);
        params.addInt("subbassDivider").setRange(1, 8).setClamp(true);
        params.addFloat("volumePitchMult").setTooltip("make higher sounds lower volume").setClamp(true);
        params.addBool("invert").setTooltip("invert pitch relationship from out to in");
        params.addInt("noteRandomness").setTooltip("amount of randomness in note number").setClamp(true).setRange(0, 20);
        params.addInt("detuneAmount").setTooltip("amount of randomness in frequency (cents)").setClamp(true).setRange(0, 200);
    } params.endGroup();
    
    
#ifdef DOING_SERIAL
    params.startGroup("comms"); {
        params.addBool("learnMode");
        params.addFloat("learnAmpThreshold").setClamp(true).set(0.5);
        
		params.addBool("forceLasersOn").set(false).trackVariable(&ForestSerialPort::forceLasersOn);
		
		params.addInt("param1")
		.setTooltip("Haven't got info on this from Mike yet")
		.setRange(0, 255).setClamp(true).trackVariable(&ForestSerialPort::param1);
		
		params.addInt("param2")
		.setTooltip("Haven't got info on this from Mike yet")
		.setRange(0, 255).setClamp(true).trackVariable(&ForestSerialPort::param2);
		
		params.addInt("param3")
		.setTooltip("Haven't got info on this from Mike yet")
		.setRange(0, 255).setClamp(true).trackVariable(&ForestSerialPort::param3);
        
		params.addInt("tipOverTimeConstant")
		.setTooltip("Tip-over filter time constant. 0-31, 31 = slowest")
		.setRange(0, 255).setClamp(true)
		.trackVariable(&ForestSerialPort::tipOverTimeConstant);
        
		params.addInt("tipThreshold")
		.setTooltip("Tip-over threshold (typ approx 40-50), arbitary units,not degrees!")
		.setRange(0, 255)
		.setClamp(true)
		.trackVariable(&ForestSerialPort::tipThreshold);
        
		params.addInt("laserTimeoutValue")
		.setTooltip("Laser timeout value. Laser will blank if no new command is received before timeout. Units of 2.048mS")
		.setRange(0, 255)
		.setClamp(true)
		.trackVariable(&ForestSerialPort::laserTimeoutValue);
		
		params.addInt("laserHoldoff")
		.setTooltip("Time laser must be vertical before turning on after tip-over condition cleared, units of 16mS")
		.setRange(0, 255)
		.setClamp(true)
		.trackVariable(&ForestSerialPort::laserHoldoff);
        
        params.addFloat("ampGain")
		.setTooltip("how much to amplify the amplitude signal before clipping")
		.setRange(0, 10)
		.setClamp(true)
		.trackVariable(&ForestSerialPort::ampGain);
        
        params.addInt("gateThreshold")
		.setTooltip("threshold of the gate")
		.setRange(500, 3000)
		.setClamp(true)
		.trackVariable(&RodInfo::threshold);
        
        params.addInt("attackThreshold")
		.setTooltip("threshold of the attack")
		.setRange(0, 3000)
		.setClamp(true)
		.trackVariable(&RodInfo::attackThreshold);
        
        
        params.addFloat("attackSmoothing")
		.setTooltip("how smoothed over the attack is")
		.setRange(1, 6)
		.setClamp(true)
		.trackVariable(&RodInfo::attackSmoothing);
        
        
        
        
	} params.endGroup();
#endif
    
    updateFilesGroup("sound.local.file", "audio", false);
    updateFilesGroup("layout.image", "layout", true);
    updateFilesGroup("animation.laser.file", "animations/laser", true);
    updateFilesGroup("animation.performance.file", "animations/performance", true);
    
    params.loadXmlValues();
    
    gui.addPage(params);
    gui.setDefaultKeys(false);
    
    // camera
    cameras.resize(params.get<msa::controlfreak::ParameterNamedIndex>("camera.view").size());
    
    easyCam.disableMouseInput();
    easyCam.setPosition(0, 1000, 0);
    easyCam.setDistance(4400);
    cameras[0] = &easyCam;
    
    cameras[1] = new ofCamera;
    cameras[1]->setGlobalPosition(0, 2000, 0);
    cameras[1]->lookAt(ofVec3f(0, 0, 0));
    cameras[1]->setFov(10);
    //    cameras[1]->enableOrtho();
    
    cameras[2] = new ofCamera;
    
    messageAudio.loadSound("message.wav");
    
    venueModel.loadModel("3d/venue.dae");
    venueModel.setScaleNormalization(false);
    venueModel.setScale(1, -1, 1);
    
    sendRodOsc(true);
    checkAndInitRodLayout(true);
}


//--------------------------------------------------------------
void testApp::checkAndInitRodLayout(bool bForceUpdate) {
    msa::controlfreak::ParameterGroup &paramsLayout = params.getGroup("layout");
    if(bForceUpdate || paramsLayout.hasChanged())  {
        ofLogNotice() << "checkAndInitRodLayout at frame " << ofGetFrameNum();
        
        bSendRodPositionsOsc = true;
        
        int installationWidth = paramsLayout["installationWidth"];
        int installationLength = paramsLayout["installationLength"];
        int rodCountWidth = paramsLayout["rodCountWidth"];
        int rodCountLength = paramsLayout["rodCountLength"];
        float randomness = paramsLayout["randomness"];
        int useImageIndex = paramsLayout["image"];
        
        if(useImageIndex > 0) {
            layoutImage.loadImage(paramsLayout.get<msa::controlfreak::ParameterNamedIndex>("image").getSelectedLabel());
            
            ofxCvColorImage colorImage;
            colorImage.allocate(layoutImage.getWidth(), layoutImage.getHeight());
            colorImage.setFromPixels(layoutImage);
            
            ofxCvGrayscaleImage greyImage;
            greyImage.allocate(layoutImage.getWidth(), layoutImage.getHeight());
            greyImage = colorImage;
            
            greyImage.threshold(10);
            
            layoutImageContours.findContours(greyImage, 0, greyImage.getWidth() * greyImage.getHeight(), 100000, false);
            
            //            rods.resize(layoutImageContours.blobs.size());
            rods.clear();
            
            printf("found %d rods in layout\n", layoutImageContours.nBlobs);
            for(int i=0; i<layoutImageContours.nBlobs; i++) {
                rods.push_back(new Rod());
                Rod *r = rods[i];
                ofxCvBlob &blob = layoutImageContours.blobs[i];
                
                float x = ofMap(blob.centroid.x, 0, greyImage.getWidth(), -installationWidth/2, installationWidth/2);
                float z = ofMap(blob.centroid.y, 0, greyImage.getHeight(), -installationLength/2, installationLength/2);
                
                r->setGlobalPosition(x, 0, z);
                // printf("[%d]    %3.3f,   %3.3f      =>       %3.3f, %3.3f              =>    %3.3f, %3.3f\n",i,  blob.centroid.x, blob.centroid.y, x, z, r->getX(), r->getZ());
            }
            
        } else {
            layoutImage.clear();
            
            //            rods.resize(rodCountWidth * rodCountLength);
            rods.clear();
            for(int i=0; i<rodCountWidth; i++) {
                for(int j=0; j<rodCountLength; j++) {
                    rods.push_back(new Rod());
                    Rod *r = rods[j*rodCountWidth + i];
                    float x = ofMap(i, 0, rodCountWidth-1, -installationWidth/2, installationWidth/2);
                    float z = ofMap(j, 0, rodCountLength-1, -installationLength/2, installationLength/2);
                    r->setGlobalPosition(x, 0, z);
                }
            }
        }
        
    
        float installationRadius = installationSize.x/2;//sqrt(installationSize.x * installationSize.x + installationSize.z * installationSize.z)/2;
        for(int i=0; i<rods.size(); i++) {
            Rod *r = rods[i];
            r->move(randomness * ofVec3f(ofRandomf(), 0, ofRandomf()));
            r->setup(installationRadius);
        }
        
        sort(rods.begin(), rods.end(), [](const Rod *a, const Rod *b) {
            return b->getSortScore() < a->getSortScore();
        });

        for(int i = 0; i < rods.size(); i++) {
            rods[i]->setIndex(i);
            rods[i]->setDeviceId(i); // HACK: replace this with correct map
        }
        
        Rod::loadDeviceIdToRodMap(rods);
        
        Performer::worldMin.set(-installationWidth/2, 0, -installationLength/2);
        Performer::worldMax.set( installationWidth/2, 0, installationLength/2);
    }
}


//--------------------------------------------------------------
void testApp::checkAndInitPerformers(bool bForceUpdate = false) {
    msa::controlfreak::ParameterGroup &paramsPerformers = params.getGroup("performers");
    if(bForceUpdate || paramsPerformers.hasChanged()) {
        ofLogNotice() << "checkAndInitPerformers at frame " << ofGetFrameNum();
        
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
void testApp::checkAndInitFbo(bool bForceUpdate) {
    msa::controlfreak::ParameterGroup &paramsFbo = params.getGroup("fbo");
    if(bForceUpdate || paramsFbo.hasChanged()) {
        ofLogNotice() << "checkAndInitFbo at frame " << ofGetFrameNum();
        
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
void testApp::updateCamera() {
    msa::controlfreak::ParameterGroup &paramsCamera = params.getGroup("camera");
    if(params["performers.count"].hasChanged()) paramsCamera["trackPerson"].setRange(1, (int)params["performers.count"]);
   // if(paramsCamera["trackPerson"].hasChanged()) cameras[2]->setParent(performers[paramsCamera["trackPerson"]]);
    easyCam.setFov(paramsCamera["fov"]);
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
vector<Rod*> checkRodCollisions(ofVec3f p, float radius, bool setAmp = true) {
    vector<Rod*> hitRods;
    
    //    float outputPitchMult = params["sound.local.outputPitchMult"];
    //    float volumeVariance = params["sound.volumeVariance"];
    //    float retriggerThreshold = params["sound.retriggerThreshold"];
    //    int maxNoteCount = params["tuning.maxNoteCount"];
    //    float volumePitchMult = params["tuning.volumePitchMult"];
    //    ofSoundPlayer *psound = params["sound.local.enabled"] ? &sound : NULL;
    
    for(int i=0; i<rods.size(); i++) {
        Rod *r = rods[i];
        if((p - r->getGlobalPosition()).lengthSquared() < radius * radius) {
            if(setAmp) r->setAmp(1);
            hitRods.push_back(r);
        }
        //            r.trigger(retriggerThreshold, scaleManager, outputPitchMult, maxNoteCount, volumePitchMult, volumeVariance, psound);
    }
    
    return hitRods;
}


//--------------------------------------------------------------
void testApp::updateRodLaserAnimation() {
    msa::controlfreak::ParameterNamedIndex &paramNamedIndex = params.get<msa::controlfreak::ParameterNamedIndex>("animation.laser.file");
    static int videoNumber = 0;
    if(paramNamedIndex.hasChanged()) {
        if((int)paramNamedIndex == 0) {
            //            animationVideo->close();
            if(animationVideo) {
                delete animationVideo;
                animationVideo = NULL;
            }
        } else {
            params["animation.performance.file"] = 0;
            params["animation.performance.file"].clearChanged();
//            params["performers.count"] = 0;
            
            if(animationVideo) {
                videoNumber++;
                printf("deleting video %i %s\n", videoNumber, animationVideo->getMoviePath().c_str());
                delete animationVideo;
                animationVideo = NULL;
            }

            animationVideo = new ofVideoPlayer;
            animationVideo->loadMovie(paramNamedIndex.getSelectedLabel());
            animationVideo->setLoopState(params["animation.laser.loop"] ? OF_LOOP_NORMAL : OF_LOOP_NONE);
            animationVideo->setSpeed(params["animation.laser.speed"]);
            animationVideo->play();
        }
    }
    
    
    // if video is loaded and a it's a laser animation
    if(animationVideo && animationVideo->isLoaded() && (int)paramNamedIndex > 0) {
        if(params["animation.laser.loop"].hasChanged()) animationVideo->setLoopState(params["animation.laser.loop"] ? OF_LOOP_NORMAL : OF_LOOP_NONE);
        if(params["animation.laser.speed"].hasChanged()) animationVideo->setSpeed(params["animation.laser.speed"]);
        
        animationVideo->update();
        
        //        float outputPitchMult = params["sound.local.outputPitchMult"];
        //        float volumeVariance = params["sound.volumeVariance"];
        //        float retriggerThreshold = params["sound.retriggerThreshold"];
        //        int maxNoteCount = params["tuning.maxNoteCount"];
        //        float volumePitchMult = params["tuning.volumePitchMult"];
        //        ofSoundPlayer *psound = params["sound.local.enabled"] ? &sound : NULL;
        
        ofPixelsRef pixels = animationVideo->getPixelsRef();
        for(int i=0; i<rods.size(); i++) {
            Rod *r = rods[i];
            ofVec2f imagePos;
            imagePos.x = ofMap(r->getX(), -installationSize.x/2, installationSize.x/2, 0, animationVideo->getWidth());
            imagePos.y = ofMap(r->getZ(), -installationSize.z/2, installationSize.z/2, 0, animationVideo->getHeight());
            //            if(pixels.getColor(imagePos.x, imagePos.y).r > 0) r.trigger(retriggerThreshold, scaleManager, outputPitchMult, maxNoteCount, volumePitchMult, volumeVariance, psound);
            //            else r.amp = 0;
//            r.setLaser(pixels.getColor(imagePos.x, imagePos.y).r / 255.0);
            if(pixels.getColor(imagePos.x, imagePos.y).r > 50) r->setLaser(1);
        }
    }
    
    
    if(params["animation.laser.playRandom"]) {
        if(ofGetElapsedTimeMillis() >= animation_nextChangeMillis) {
            animation_nextChangeMillis = ofGetElapsedTimeMillis() + 1000 * ofRandom(params["animation.laser.randomChangeTimeMin"], params["animation.laser.randomChangeTimeMax"]);
            paramNamedIndex = ofRandom(1, paramNamedIndex.size());
        }
    }
}


//--------------------------------------------------------------
void testApp::updateRandomMusic() {
    if(params["randomMusic.enabled"]) {
        if(ofGetElapsedTimeMillis() >= randomMusic_nextChangeMillis) {
            randomMusic_nextChangeMillis = ofGetElapsedTimeMillis() + 1000 * ofRandom(params["randomMusic.randomChangeTimeMin"], params["randomMusic.randomChangeTimeMax"]);
            int randomRodIndex = ofRandom(floor(rods.size()));
            rods[randomRodIndex]->setAmp(ofRandom(params["randomMusic.randomAmpMin"], params["randomMusic.randomAmpMax"]));
        }
    }
}

//--------------------------------------------------------------
void testApp::updatePerformanceAnimation() {
    msa::controlfreak::ParameterNamedIndex &paramNamedIndex = params.get<msa::controlfreak::ParameterNamedIndex>("animation.performance.file");
    if(paramNamedIndex.hasChanged()) {
        if((int)paramNamedIndex == 0) {
            //            animationVideo->close();
            if(animationVideo) {
                delete animationVideo;
                animationVideo = NULL;
            }
        } else {
            params["animation.laser.file"] = 0;
            params["animation.laser.file"].clearChanged();
            params["performers.count"] = 0;
            
            animationVideo = new ofVideoPlayer;
            //            animationVideo->loadMovie("animations/laser/" + paramNamedIndex.getSelectedLabel());
            animationVideo->loadMovie(paramNamedIndex.getSelectedLabel());
            animationVideo->setLoopState(OF_LOOP_NONE);
            animationVideo->setSpeed(params["animation.performance.speed"]);
            
            if(params["animation.performance.play"]) animationVideo->play();
            params["animation.performance.time"].setRange(0, animationVideo->getDuration());
        }
    }
    
    
    // if video is loaded and a it's a performance animation
    if(animationVideo && animationVideo->isLoaded() && (int)paramNamedIndex > 0) {
        Performer::updateFromAnimation = true;
        
        if(params["animation.performance.speed"].hasChanged()) animationVideo->setSpeed(params["animation.performance.speed"]);
        
        if(params["animation.performance.play"].hasChanged()) {
            if(params["animation.performance.play"]) animationVideo->play();
            else animationVideo->stop();
        }
        
        if(params["animation.performance.play"]) {
            params["animation.performance.time"] = animationVideo->getPosition() * animationVideo->getDuration();
        } else {
            animationVideo->setPosition((float)params["animation.performance.time"] / animationVideo->getDuration());
        }
        
        animationVideo->update();
        //        params["animation.performance.time"].clearChanged();
        //        if(params["animation.performance.time"]
        
        ofxCvColorImage colorImage;
        colorImage.allocate(animationVideo->getWidth(), animationVideo->getHeight());
        colorImage.setFromPixels(animationVideo->getPixelsRef());
        
        ofxCvGrayscaleImage greyImage;
        greyImage.allocate(animationVideo->getWidth(), animationVideo->getHeight());
        greyImage = colorImage;
        
        //        greyImage.blur(params["animation.performance.blurAmount"]);
        greyImage.threshold(params["animation.performance.threshold"]);
        
        animationVideoContours.findContours(greyImage, 0, greyImage.getWidth() * greyImage.getHeight(), 100000, false);
        
        int numBlobs = animationVideoContours.blobs.size();
        if((int)params["performers.count"] != numBlobs) {
            params["performers.count"] = numBlobs;
            checkAndInitPerformers(true);
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
            p.color = animationVideo->getPixelsRef().getColor(blob.centroid.x, blob.centroid.y);
            //            p.color *= 255;
            
        }
    } else {
        Performer::updateFromAnimation = false;
    }
}


//--------------------------------------------------------------
void checkAndInitSoundFile() {
    //    msa::controlfreak::ParameterNamedIndex &paramNamedIndex = params.get<msa::controlfreak::ParameterNamedIndex>("sound.local.file");
    //    if(paramNamedIndex.hasChanged()) {
    //        sound.loadSound(paramNamedIndex.getSelectedLabel());
    //        sound.setMultiPlay(true);
    //        sound.setLoop(false);
    //    }
}

//--------------------------------------------------------------
void testApp::updateRodTuning() {
    bool useIndex = params["tuning.useIndex"];
    float indexMultipler = params["tuning.indexMultipler"];
    ofVec3f noteCount(params["tuning.noteCountWidth"], 0, params["tuning.noteCountLength"]);
    int noteCountRadial = params["tuning.noteCountRadial"];
    int noteCountDistance = params["tuning.noteCountDistance"];
    bool invert = params["tuning.invert"];
    int maxNoteCount = params["tuning.maxNoteCount"];
    int inputPitchOffset = params["tuning.inputPitchOffset"];
    int noteRandomness = params["tuning.noteRandomness"];
    //    float halfInstallationLength =  installationSize.x/2;//length()/2;  // TODO: hack?
    
    bool bRet = false;
    for(int i=0; i<rods.size(); i++) {
        Rod *r = rods[i];
        float distanceToCenter = r->getPolarCoordinatesNorm().x;
        float angle = r->getPolarCoordinatesNorm().y;
        
        int pitchIndex = 0;
        if(noteRandomness) pitchIndex += noteRandomness * r->getPitchIndexOffset();
        if(useIndex) pitchIndex += r->getIndex() * indexMultipler;
        float distRatio = distanceToCenter;
        if(invert) distRatio = 1-distRatio;
        pitchIndex += distRatio * noteCountDistance;
        if(angle > 0.5) angle = 1.0 - angle;
        if(distanceToCenter > 0.01) {  // hack to include radial only in rods not in center
            pitchIndex += angle * noteCountRadial;
        }
        if(maxNoteCount > 0) {
            if(pitchIndex < 0) pitchIndex += maxNoteCount;
            pitchIndex %= 2 * maxNoteCount;
            if(pitchIndex >= maxNoteCount) pitchIndex = 2 * maxNoteCount - pitchIndex;  // mirror mod
        } else {
            if(pitchIndex < 0) pitchIndex = 0;
        }
        pitchIndex += inputPitchOffset;
        if(maxNoteCount > 0) {
            r->heightNorm = ofNormalize(pitchIndex, 0, maxNoteCount);
        }
        r->setPitchIndex(pitchIndex);
    }
}


//--------------------------------------------------------------
void testApp::resetAll() {
    checkAndInitRodLayout(true);
    checkAndInitPerformers(true);
    checkAndInitFbo(true);
    selectedRod = NULL;
    params["animation.laser.file"] = 0;
}
float modAngle(float angle, float offset, bool flip) {
    if(flip) {
        angle = 1 - angle;
    }
    
    angle += offset;
    while(angle<0) angle += 1;
    while(angle>1) angle -= 1;
    return angle;
}
//--------------------------------------------------------------
void testApp::sendRodOsc(bool bForce) {
    if(params["sound.osc.enabled"].hasChanged()) {
        bForce = true;
        checkAndInitRodLayout(true);  // force zero if enabled state has changed
    }
    if(bForce) ofLogNotice() << "sending full osc";
    
    if(bForce || params["sound.osc.enabled"]) {
        if(params["sound.osc.port"].hasChanged() || oscSender == NULL) {
            if(oscSender) delete oscSender;
            oscSender = new ofxOscSender;
            oscSender->setup("127.0.0.1", params["sound.osc.port"]);
        }
        float outputPitchMult = params["sound.osc.outputPitchMult"];
        
        bSendRodTuningOsc = bSendRodTuningOsc || bForce || params["tuning"].hasChanged() || params["sound.osc.outputPitchMult"].hasChanged();
        bSendRodPositionsOsc = bSendRodPositionsOsc || bForce;
        
        ofxOscBundle b;
        for(int i=0; i<rods.size(); i++) {
            Rod *r = rods[i];
            ofxOscMessage m;
            m.clear();
            m.setAddress("/forestAmp");
            m.addIntArg(i);
            m.addFloatArg(r->volume);
            b.addMessage(m);
        }
        oscSender->sendBundle(b);
        
        float offset = params["layout.angleOffset"];
        bool flip = params["layout.angleFlip"];
        
        if(bSendRodPositionsOsc) {
            b.clear();
            for(int i=0; i<rods.size(); i++) {
                Rod *r = rods[i];
                float distanceToCenter = r->getPolarCoordinatesNorm().x;
                float angle = r->getPolarCoordinatesNorm().y;
                
                ofxOscMessage m;
                m.setAddress("/forestPos");
                m.addIntArg(i);
                m.addFloatArg(modAngle(angle, offset, flip));
                b.addMessage(m);
                
                m.clear();
                m.setAddress("/forestCentre");
                m.addIntArg(i);
                m.addFloatArg(0);//1.0 - distanceToCenter);
                b.addMessage(m);
            }
            oscSender->sendBundle(b);
        }
        
        if(bSendRodTuningOsc) {
            b.clear();
            float detuneAmount = 1 + (0.059 * 0.01 * (float)params["tuning.detuneAmount"]);
            for(int i=0; i<rods.size(); i++) {
                Rod *r = rods[i];
                ofxOscMessage m;
                m.setAddress("/forestFreq");
                m.addIntArg(i);
                float freq = scaleManager.currentFreq(r->getPitchIndex()) * outputPitchMult;
                if(detuneAmount != 1) freq *= ofRandom(1/detuneAmount, 1*detuneAmount);
                if(i==0) freq /= (float)params["tuning.subbassDivider"];
                m.addFloatArg(freq);
                b.addMessage(m);
            }
            oscSender->sendBundle(b);
        }
        
    }
    
    bSendRodPositionsOsc = false;
    bSendRodTuningOsc = false;
}


//--------------------------------------------------------------
void testApp::update() {
//    msa::controlfreak::update();
    
    checkAndInitRodLayout();
    checkAndInitPerformers();
    checkAndInitFbo();
    checkAndInitSoundFile();
    
    updateRodTuning();
    
    
    // clear all laser values and fade rod Amps
    for(int i = 0; i < rods.size(); i++) {
        Rod *r = rods[i];
//        r.setLaser(0);
        r->fadeAmp();
        r->color = ofColor((float)params["rods.color"]);
    }
    
    // update positions of virtual performers based on animation
    // this simply moves the performers around
    updatePerformanceAnimation();
    
    // update performers generative animation
    // this simply moves the performers around
    for(int i=0; i<performers.size(); i++) performers[i].update();
    
	updateRandomMusic();
    
    // serial comms should simply set (overwrite) the value of Amp for each rod
#ifdef DOING_SERIAL
	// don't talk to the lasers until
	// the forest has been scanned.
    //	if(rodCommunicator->doneDiscovering()) {
    //		rodMapper.update(rodCommunicator, rods);
    //	}
	if(rodCommunicator->doneDiscovering()) {
        if(params["comms.learnMode"] && selectedRod) {
            float currentHighestAmp;
            int deviceIdForHighestAmp = rodCommunicator->findRodWithBiggestAmplitude(currentHighestAmp);
            if(currentHighestAmp > (float)params["comms.learnAmpThreshold"]) {
                selectedRod->setDeviceId(deviceIdForHighestAmp);
                // TODO: save deviceIdmap
            }
        }
        
		for(int i=0; i<rods.size(); i++) {
			Rod *r = rods[i];
			r->setAmp(MAX(r->getAmp(), rodCommunicator->getAmplitude( r->getDeviceId() )));
		}
	}
#endif
    
    
    // check rod collisions
    // this sets the Amp of the rod if collision detected
    {
        // check for performer-rod collision
        for(int j=0; j<performers.size(); j++) checkRodCollisions(performers[j].getGlobalPosition(), performers[j].affectRadius);
        
        // check mouse-rod collision
        vector<Rod*> hitRods = checkRodCollisions(mouse3d, mouseRadius, params["randomMusic.triggerOnMouse"]);
        if(hitRods.size()) {
            if(ofGetMousePressed() && !ofGetKeyPressed()) selectedRod = hitRods[0];
            else hitRods[0]->color.set(255, 0, 0);
        } else {
            if(ofGetMousePressed() && !ofGetKeyPressed()) {
                selectedRod = NULL;
            }
        }
        
    }
 	
	// set lasers based on amp
    int volumePower = params["sound.osc.volumePower"];
    float totalVolume = 0;
	for(int i = 0; i < rods.size(); i++) {
        Rod *r = rods[i];
        r->setLaserBasedonAmp();
        r->volume = pow(r->getAmp(), volumePower);
        totalVolume += r->volume;
    }
    params["sound.compression.totalVolume"] = totalVolume;
    params["sound.compression.avgVolume"] = totalVolume / rods.size();
    
    
    // update rod laser values based on animation pixel values (if animation loaded)
    updateRodLaserAnimation();

    
    if(selectedRod) {
        selectedRod->color.set(255, 255, 0);
        selectedRod->setLaser(1);
    }
    
    
    // send laser value back down serial
#ifdef DOING_SERIAL
	for(int i=0; i<rods.size(); i++) {
		rodCommunicator->setLaser(rods[i]->getDeviceId(), rods[i]->getLaser());
	}
	
#endif
    
    {
        int showRodAmp = MIN((int)params["rods.showRodAmp"], rods.size()-1);
        params["rods.selectedRodAmp"] = rods[showRodAmp]->getAmp();
    }
    
	// send OSC
    bool bForce = params["sound.osc.forceSend"];
    if(ofGetFrameNum() < 5) bForce = true; // force true for first 5 frames to fix osc init issue
    int sendFullFrameCount = params["sound.osc.sendFullFrameCount"];
    if(sendFullFrameCount && (ofGetFrameNum() % (sendFullFrameCount * 30) == 0)) bForce = true;
    sendRodOsc(bForce);  // send OSC, force if nessecary
    //    updateSSAO();
}


//--------------------------------------------------------------
void testApp::drawFloor() {
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
    
    ofSetColor(100, 0, 0);
    ofLine(-floorWidth/2, 5, 0, floorWidth/2, 5, 0);
    ofSetColor(0, 0, 100);
    ofLine(0, 5, -floorLength/2, 5, 0, floorLength/2);
}

#ifdef DOING_SERIAL
void drawSerialProgress() {
	if(!rodCommunicator->doneDiscovering()) {
		ofSetHexColor(0);
		ofRectangle r(495, ofGetHeight()-20, ofGetWidth()-495, 20);
		ofRect(r);
		ofSetHexColor(0x990000);
		r.width *= rodCommunicator->getProgress();
		ofRect(r);
		ofSetHexColor(0xFFFFFF);
		ofDrawBitmapString(ofToString ((int)(rodCommunicator->getProgress()*100.f))+ "% done discovering nodes", r.x+5, r.y+15);
	}
}
#endif

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
    cam.begin();
    
    ofTranslate(0, -100, 0);
    ofRotateX(params["camera.rotx"]);
    ofRotateY(params["camera.roty"]);
    ofRotateZ(params["camera.rotz"]);
    
    ofPushStyle();
    
    drawFloor();
    for(int i=0; i<rods.size(); i++) rods[i]->draw();
    for(int i=0; i<performers.size(); i++) {
        performers[i].draw();
        ofVec3f v = performers[i].getOrientationEuler();
    }
    for(int i=0; i<lights.size(); i++) lights[i]->draw();
    
    ofPopStyle();
    
    
    // draw mouse cursor
    {
        mouse3d = windowToWorld(ofGetMouseX(), ofGetMouseY());
        ofPushStyle();
        ofSetColor(100, 0, 0, 50);
        ofSphere(mouse3d, mouseRadius);
        ofPopStyle();
    }
    
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
        if(animationVideo && animationVideo->isLoaded()) {
            ofSetColor(255);
            animationVideo->draw(ofGetWidth()-w, w, w, w);
            animationVideoContours.draw(ofGetWidth()-w, w, w, w);
        }
        
    }
    ofSetColor(255);
    ofDrawBitmapString(ofToString(ofGetFrameRate(), 2), ofGetWidth() - 100, 30);
    
    if(selectedRod && params["display.bDisplaySelectedId"]) {
        ofDrawBitmapString(selectedRod->getInfoStr(), ofGetWidth() - 285, ofGetHeight()-130);
    }
    
#ifdef DOING_SERIAL
    if(params["comms.learnMode"]) {
        ofPushStyle();
        ofSetColor(255, 0, 0);
        ofNoFill();
        int w = 10;
        ofSetLineWidth(2*w);
        ofRect(w, w, ofGetWidth()-2*w, ofGetHeight()-2*w);
        ofPopStyle();
    }
    
    
    if(showRodGui) {
		rodCommunicator->draw();
	}
    
    drawSerialProgress();
    
#endif
//    ofRectangle debugR(200, 100, 400, 400);
//    ofSetColor(0, 0, 0);
//    ofDrawRectangle(debugR);
//    ofSetColor(200, 0, 0);
//    
//    for(int i = 0; i < rods.size(); i++) {
//        Rod *r = rods[i];
//        
//        glm::vec3 imagePos;
//        imagePos.x = ofMap(r->getX(), -installationSize.x/2, installationSize.x/2, debugR.getLeft(), debugR.getHeight());
//        imagePos.y = ofMap(r->getZ(), -installationSize.z/2, installationSize.z/2, debugR.getTop(), debugR.getBottom());
//        ofCircle(imagePos, 5);
//        //            if(pixels.getColor(imagePos.x, imagePos.y).r > 0) r.trigger(retriggerThreshold, scaleManager, outputPitchMult, maxNoteCount, volumePitchMult, volumeVariance, psound);
//        //            else r.amp = 0;
//        //            r.setLaser(pixels.getColor(imagePos.x, imagePos.y).r / 255.0);
//        //if(pixels.getColor(imagePos.x, imagePos.y).r > 50) r.setLaser(1);
//        
//       // printf("[%d]    %3.3f,   %3.3f      =>       %3.3f, %3.3f\n",i,  imagePos.x, imagePos.y, r.getX(), r.getZ());
//    }
    
	
}

//--------------------------------------------------------------
void testApp::exit() {
    resetAll();
    sendRodOsc(true);
    rodCommunicator->stop();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    switch(key) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            params["animation.laser.file"] = key-48;
            break;
   
        case 's':
            params.saveXmlValues();
            Rod::saveDeviceIdToRodMap(rods);
            break;
            
        case 'l':
            params.loadXmlValues();
            break;
            
        case 'f':
            ofToggleFullscreen();
            break;
            
        case ' ':
            easyCam.enableMouseInput();
            break;
            
        case '>':
        case '.':
            if(selectedRod &&  selectedRod->getIndex() < rods.size()-1) selectedRod = rods[selectedRod->getIndex()+1];
            else selectedRod = rods[0];
            break;
            
        case '<':
        case ',':
            if(selectedRod && selectedRod->getIndex() > 0) selectedRod = rods[selectedRod->getIndex()-1];
            else selectedRod = rods[rods.size()-1];
            break;
        case '/':
            selectedRod = NULL;
            break;
            
        case 'R':
            resetAll();
            break;
            
        case 'm':
            messageAudio.setPosition(0);
            messageAudio.play();
            break;
            
        case 'a':
            params["laser.alwaysOn"] = !params["laser.alwaysOn"];
            params["animation.laser.file"] = 0;
            break;
            
        case 'r':
            params["laser.random"] = !params["laser.random"];
            params["animation.laser.file"] = 0;
            break;
            
        case 'o':
            params["sound.osc.forceSend"] = true;
            break;
            
            
            
#ifdef DOING_SERIAL
        case 'c':
            if(rodCommunicator->checkStatus()) {
                printf("All rods good\n");
            }
            break;
		case '\t':
			showRodGui ^= true;
			break;
            
		case '=':
			rodCommunicator->reset();
			rodMapper.reset();
			break;
        case '\'':
            rodCommunicator->paused ^= true;
            break;
            
        case 'A':
            params["comms.forceLasersOn"] = !params["comms.forceLasersOn"];
            break;

        case 'L':
            params["comms.learnMode"] = ! (bool) params["comms.learnMode"];
            break;
            

#endif
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
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
    checkAndInitFbo(true);
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){
    
}


//--------------------------------------------------------------
//void testApp::newMidiMessage(ofxMidiMessage& msg) {
//    
//}
