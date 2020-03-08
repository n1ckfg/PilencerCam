#include "ofApp.h"

using namespace cv;
using namespace ofxCv;

void ofApp::setup() {
    settings.loadFile("settings.xml");

    ofSetVerticalSync(false);    
    framerate = settings.getValue("settings:framerate", 30); 
    width = settings.getValue("settings:width", 160); 
    height = settings.getValue("settings:height", 120); 
    ofSetFrameRate(framerate);

    host = settings.getValue("settings:host", "127.0.0.1"); 
    port = settings.getValue("settings:port", 7110);
    
    debug = (bool) settings.getValue("settings:debug", 1);
    firstRun = true;

    sender.setup(host, port);

    // ~ ~ ~   get a persistent name for this computer   ~ ~ ~
    compname = "RPi";
    file.open(ofToDataPath("compname.txt"), ofFile::ReadWrite, false);
    ofBuffer buff;
    if (file) { // use existing file if it's there
        buff = file.readToBuffer();
        compname = buff.getText();
    } else { // otherwise make a new one
        compname += "_" + ofGetTimestampString("%y%m%d%H%M%S%i");
        ofStringReplace(compname, "\n", "");
        ofStringReplace(compname, "\r", "");
        buff.set(compname.c_str(), compname.size());
        ofBufferToFile("compname.txt", buff);
    }
    std::cout << compname << endl;

    cam.setup(width, height, false); // color/gray;

    counterOn = 0;
    markTriggerTime = 0;
    trigger = false;
    isMoving = false;
    triggerThreshold = settings.getValue("settings:trigger_threshold", 0.2);  
    counterMax = settings.getValue("settings:trigger_frames", 3);
    timeDelay = settings.getValue("settings:time_delay", 5000);
    counterDelay = settings.getValue("settings:counter_reset", 1000);

    // ~ ~ ~   cam settings   ~ ~ ~
    camSharpness = settings.getValue("settings:sharpness", 0); 
    camContrast = settings.getValue("settings:contrast", 0); 
    camBrightness = settings.getValue("settings:brightness", 50); 
    camIso = settings.getValue("settings:iso", 300); 
    camExposureMode = settings.getValue("settings:exposure_mode", 0); 
    camExposureCompensation = settings.getValue("settings:exposure_compensation", 0); 
    camShutterSpeed = settings.getValue("settings:shutter_speed", 0);

    cam.setSharpness(camSharpness);
    cam.setContrast(camContrast);
    cam.setBrightness(camBrightness);
    cam.setISO(camIso);
    cam.setExposureMode((MMAL_PARAM_EXPOSUREMODE_T) camExposureMode);
    cam.setExposureCompensation(camExposureCompensation);
    cam.setShutterSpeed(camShutterSpeed);
    //cam.setFrameRate // not implemented in ofxCvPiCam
}

void ofApp::update() {
    frame = cam.grab();
    if(!frame.empty()) {
        if (firstRun) {
            imitate(previous, frame);
            imitate(diff, frame);
            firstRun = false;
        }

        absdiff(frame, previous, diff);
        diff.update();   
        copy(frame, previous);        
        diffMean = mean(toCv(diff));
    
        diffAvg = (diffMean[0] + diffMean[1] + diffMean[2]) / 3.0;

        isMoving = diffAvg > triggerThreshold;    

        int t = ofGetElapsedTimeMillis();
        
        // reset count if too much time has elapsed since the last change
        if (t > markCounterTime + counterDelay) counterOn = 0;

        // motion detection logic
        // 1. motion detected, but not triggered yet
        if (!trigger && isMoving) {
            if (counterOn < counterMax) { // start counting the ON frames
                counterOn++;
                markCounterTime = t;
            } else { // trigger is ON
                markTriggerTime = t;
                trigger = true;
            }  
        // 2. motion is triggered
        } else if (trigger && isMoving) { // keep resetting timer as long as motion is detected
            markTriggerTime = t;
        // 3. motion no longer detected
        } else if (trigger && !isMoving && t > markTriggerTime + timeDelay) {
            trigger = false;
        }

        sendOsc();
    }
}

void ofApp::draw() {   
    if (!frame.empty() && debug) {
        ofSetColor(255);
        ofBackground(0);

        diff.draw(0, 0, ofGetWidth(), ofGetHeight());      
        
        if (trigger) {
            ofSetColor(0, 255, 0);
        } else {
            ofSetColor(255, 0, 0);
        }
        ofDrawRectangle(0, 0, triggerThreshold*100, 10);
        
        if (trigger) {
            ofSetColor(0, 255, 0);
        } else {
            ofSetColor(255, 255, 0);          
        }
        ofDrawRectangle(0, 10, diffAvg*100, 10);
    }
}


void ofApp::sendOsc() {
	ofxOscMessage msg;

    msg.setAddress("/pilencer");
    msg.addStringArg(compname);
    msg.addIntArg((int) trigger);

    sender.sendMessage(msg);
    
    cout << "*** SENT: " << trigger << ", diff/thresh: " << diffAvg << " / " << triggerThreshold << " ***" << endl;
}
