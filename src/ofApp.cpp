#include "ofApp.h"

using namespace cv;
using namespace ofxCv;

void ofApp::setup() {
    settings.loadFile("settings.xml");

    //doDrawInfo  = true; 
    ofSetVerticalSync(false);    
    width = ofGetWidth(); 
    height = ofGetHeight(); 
    framerate = settings.getValue("settings:framerate", 60); 
    ofSetFrameRate(framerate);

    host = settings.getValue("settings:host", "127.0.0.1"); 
    port = settings.getValue("settings:port", 7110);
    
    debug = (bool) settings.getValue("settings:debug", 1);

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
    std::cout << compname << "\n";

    cam.setup(width, height, true); // color/gray;

    triggerThreshold = settings.getValue("settings:trigger_threshold", 10);
    counterMax = settings.getValue("settings:trigger_frames", 3);
    timeDelay = settings.getValue("settings:time_delay", 5000);

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

    // ~ ~ ~   contour settings   ~ ~ ~
    thresholdValue = settings.getValue("settings:threshold", 127); 
    contourThreshold = 2.0;
    contourMinAreaRadius = 1.0;
    contourMaxAreaRadius = 250.0;
    contourFinder.setMinAreaRadius(contourMinAreaRadius);
    contourFinder.setMaxAreaRadius(contourMaxAreaRadius);
    //contourFinder.setInvert(true); // find black instead of white
    trackingColorMode = TRACK_COLOR_RGB;

    avgMotion = 0;
    counter = 0;
    markTime = 0;
    trigger = false;
    isMoving = false;
}

void ofApp::update() {
    frame = cam.grab();
}

void ofApp::draw() { 
    if (!frame.empty()) { 
        if (debug) {
            ofSetLineWidth(2);
            ofNoFill();
        }

        contourFinder.setThreshold(h);
        contourFinder.findContours(frame);
        if (debug) contourFinder.draw();            

        int contourCounter = contourFinder.size();   

        isMoving = contourCounter > triggerThreshold;

        int t = ofGetElapsedTimeMillis();

        if (!trigger && isMoving) { // motion detected, but not triggered yet
                if (counter < counterMax) { // start counting frames
                    counter++;
                } else { // motion frames have reached trigger threshold
                    markTime = t;
                    trigger = true;
                }  
        } else if (trigger && isMoving) { // triggered, reset timer as long as motion is detected
            markTime = t;
        } else if (trigger && !isMoving && t > markTime + timeDelay) { // triggered, timer has run out
            trigger = false;
            counter = 0;
        }

        if (trigger) {
            sendOsc(1);    
        } else {
            sendOsc(0);
        }

        if (debug) std::cout << "contours: " << contourCounter << "   isMoving: " << isMoving << "   trigger: " << trigger << "\n";
    }

    if (debug) {
        stringstream info;
        info << "FPS: " << ofGetFrameRate() << "\n";
        //info << "Camera Resolution: " << cam.width << "x" << cam.height << " @ "<< "xx" <<"FPS"<< "\n";
        ofDrawBitmapStringHighlight(info.str(), 10, 10, ofColor::black, ofColor::yellow);
    }
}


void ofApp::sendOsc(int _trigger) {
	ofxOscMessage m;
    m.setAddress("/pilencer");
    m.addStringArg(compname);
    m.addIntArg(_trigger);

    sender.sendMessage(m);
    std:cout << "*** SENT: " << _trigger << " ***\n";
}


