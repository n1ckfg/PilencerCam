#include "ofApp.h"

using namespace cv;
using namespace ofxCv;

void ofApp::setup() {
    settings.loadFile("settings.xml");

    //doDrawInfo  = true; 
    ofSetVerticalSync(false);    
    framerate = settings.getValue("settings:framerate", 60); 
    ofSetFrameRate(framerate);

    host = settings.getValue("settings:host", "127.0.0.1"); 
    port = settings.getValue("settings:port", 7110);
    
    debug = (bool) settings.getValue("settings:debug", 1);
    sendPosition = (bool) settings.getValue("settings:send_position", 0);

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

    w = 160;
    h = 120;
    cam.setup(w, h, false); // color/gray;

    triggerThreshold = settings.getValue("settings:trigger_threshold", 0.5);
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

    // ~ ~ ~   optical flow settings   ~ ~ ~
    pyrScale = 0.5;   // 0 to 1, default 0.5
    levels = 4;   // 1 to 8, default 4
    winsize = 8;   // 4 to 64, default 8
    iterations = 2;   // 1 to 8, default 2
    polyN = 7;   // 5 to 10, default 7
    polySigma = 1.5;   // 1.1 to 2, default 
    OPTFLOW_FARNEBACK_GAUSSIAN = false; // default false
    //useFarneback = true;
    winSize = 32;   // 4 to 64, default 32
    maxLevel = 3;   // 0 to 8, default 3
    maxFeatures = 200;   // 1 to 1000, default 200
    qualityLevel = 0.01;   // 0.001 to 0.02, default 0.01
    minDistance = 4;   // 1 to 16, default 4

    avgMotion = 0;
    counter = 0;
    markTime = 0;
    trigger = false;
    isMoving = false;

    sendOsc(0);
}

void ofApp::update() {
    frame = cam.grab();

    if (!frame.empty()) {
        //if (useFarneback) {
		curFlow = &farneback;
        farneback.setPyramidScale( pyrScale );
        farneback.setNumLevels( levels );
        farneback.setWindowSize( winsize );
        farneback.setNumIterations( iterations );
        farneback.setPolyN( polyN );
        farneback.setPolySigma( polySigma );
        farneback.setUseGaussian( OPTFLOW_FARNEBACK_GAUSSIAN );
		//} 
		/*
		else {
			curFlow = &pyrLk;
            pyrLk.setMaxFeatures( maxFeatures );
            pyrLk.setQualityLevel( qualityLevel );
            pyrLk.setMinDistance( minDistance );
            pyrLk.setWindowSize( winSize );
            pyrLk.setMaxLevel( maxLevel );
		}
		*/
        //check it out that that you can use Flow polymorphically
        curFlow->calcOpticalFlow(frame);

        //if (useFarneback) {
        avgRaw = farneback.getAverageFlow();
        avgMotion = (abs(avgRaw.x) + abs(avgRaw.y)) / 2.0;
        isMoving = avgMotion > triggerThreshold;
        std::cout << "avg: " << avgMotion << " motion: " << isMoving << "\n";

        if (sendPosition) sendOscPosition(avgRaw.x, avgRaw.y);
    }
     
    int t = ofGetElapsedTimeMillis();

	if (!trigger && isMoving) { // motion detected, but not triggered yet
        	if (counter < counterMax) { // start counting frames
        		counter++;
        	} else { // motion frames have reached trigger threshold
                markTime = t;
	        	trigger = true;
	            sendOsc(1);    
	        }  
    } else if (trigger && isMoving) { // triggered, reset timer as long as motion is detected
        markTime = t;
	} else if (trigger && !isMoving && t > markTime + timeDelay) { // triggered, timer has run out
		trigger = false;
		counter = 0;
        sendOsc(0);
    }
}

void ofApp::draw() {   
    if (debug) {
    	ofSetColor(255);
    	ofBackground(0);

        if(!frame.empty()) {
    	    drawMat(frame,0, 0, w * 4, h * 4);
    	    curFlow->draw(0, 0, w * 4, h * 4);
    	}

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
    m.addFloatArg(avgMotion);

    sender.sendMessage(m);
    std:cout << "*** SENT: " << _trigger << " ***\n";
}

void ofApp::sendOscPosition(float x, float y) {
    ofxOscMessage m;
    m.setAddress("/position");
    m.addStringArg(compname);
    m.addFloatArg(x);
    m.addFloatArg(y);

    sender.sendMessage(m);
    std:cout << "SENT: " << x << " " << y << "\n";
}

