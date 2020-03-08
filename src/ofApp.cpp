#include "ofApp.h"

using namespace cv;
using namespace ofxCv;

void ofApp::setup() {
    settings.loadFile("settings.xml");

    ofSetVerticalSync(false);    
    framerate = settings.getValue("settings:framerate", 60); 
    width = settings.getValue("settings:width", 160); 
    height = settings.getValue("settings:height", 120); 
    ofSetFrameRate(framerate);

    host = settings.getValue("settings:host", "127.0.0.1"); 
    port = settings.getValue("settings:port", 7110);
    
    debug = (bool) settings.getValue("settings:debug", 1);
    sendMotionInfo = (bool) settings.getValue("settings:sendMotionInfo", 0);

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
    imitate(previous, cam);
    imitate(diff, cam);

    triggerThreshold = settings.getValue("settings:trigger_threshold", 0.05);  

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

    // ~ ~ ~   optical flow settings   ~ ~ ~
    isMoving = false;
}

void ofApp::update() {
    cam.update();
    if(cam.isFrameNew()) {
        absdiff(cam, previous, diff);
        diff.update();   
        copy(cam, previous);        
        diffMean = mean(toCv(diff));
        diffMean *= Scalar(50);
    
        diffAvg = (diffMean[0] + diffMean[1] + diffMean[2]) / 3.0;

        trigger = diffAvg > trigger_threshold;
    }
}

void ofApp::draw() {   
    if (debug) {
        ofSetColor(255);
        cam.draw(0, 0);
        diff.draw(320, 0);      
        
        ofSetColor(255, 255, 0);
        ofDrawRectangle(0, 0, diffAvg, 10);
    }
}


void ofApp::sendOsc() {
	ofxOscMessage msg;

    msg.setAddress("/pilencer");
    msg.addStringArg(compname);
    msg.addIntArg((int) trigger);

    // if you're only detecting motion, leave this off to save bandwidth
    if (sendMotionInfo) {
        msg.addFloatArg(motionVal); // total motion, always positive
    }  

    sender.sendMessage(msg);
    std:cout << "*** SENT: " << trigger << " ***\n";
}
