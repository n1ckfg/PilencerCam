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

    triggerThreshold = settings.getValue("settings:trigger_threshold", 5);
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

    motionVal = 0;
    counterOn = 0;
    markTriggerTime = 0;
    trigger = false;
    isMoving = false;

    numPixels = width * height;
    backgroundPixels[numPixels];
}

void ofApp::update() {
    frame = cam.grab();
    unsigned char * pixels = frame.getPixels();

    if (!frame.empty()) {
        // * Background Subtraction by Golan Levin. 
        // Difference between the current frame and the stored background
        presenceSum = 0;
        for (int i = 0; i < numPixels; i++) { // For each pixel in the video frame...
          // Fetch the current color in that location, and also the color
          // of the background in that spot
          color currColor = pixels[i];
          color bkgdColor = backgroundPixels[i];
          // Extract the red, green, and blue components of the current pixel's color
          int currR = (currColor >> 16) & 0xFF;
          int currG = (currColor >> 8) & 0xFF;
          int currB = currColor & 0xFF;
          // Extract the red, green, and blue components of the background pixel's color
          int bkgdR = (bkgdColor >> 16) & 0xFF;
          int bkgdG = (bkgdColor >> 8) & 0xFF;
          int bkgdB = bkgdColor & 0xFF;
          // Compute the difference of the red, green, and blue values
          int diffR = abs(currR - bkgdR);
          int diffG = abs(currG - bkgdG);
          int diffB = abs(currB - bkgdB);
          // Add these differences to the running tally
          presenceSum += diffR + diffG + diffB;
          // Render the difference image to the screen
          pixels[i] = color(diffR, diffG, diffB);
          // The following line does the same thing much faster, but is more technical
          //pixels[i] = 0xFF000000 | (diffR << 16) | (diffG << 8) | diffB;
        }
        cout << presenceSum << endl; // Print out the total amount of movement

        isMoving = presenceSum > triggerThreshold;
        std::cout << "val: " << presenceSum << " motion: " << isMoving << endl;

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
            // https://stackoverflow.com/questions/16137953/is-there-a-function-to-copy-an-array-in-c-c
            std::copy(pixels, pixels+numPixels, backgroundPixels);
        }

        sendOsc();
    }
}

void ofApp::draw() {   
    if (debug) {
    	ofSetColor(255);
    	ofBackground(0);

        if(!frame.empty()) {
    	    drawMat(frame,0, 0);
    	}

        stringstream info;
        info << "FPS: " << ofGetFrameRate() << endl;
        ofDrawBitmapStringHighlight(info.str(), 10, 10, ofColor::black, ofColor::yellow);
    }
}


void ofApp::sendOsc() {
	ofxOscMessage msg;

    msg.setAddress("/pilencer");
    msg.addStringArg(compname);
    msg.addIntArg((int) trigger);

    // if you're only detecting motion, leave this off to save bandwidth
    if (sendMotionInfo) {
        msg.addIntArg(presenceSum); // total motion, always positive
    }  

    sender.sendMessage(msg);
    std:cout << "*** SENT: " << trigger << " ***\n";
}
