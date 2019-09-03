#include "ofApp.h"

using namespace cv;
using namespace ofxCv;

void ofApp::setup() {
    settings.loadFile("settings.xml");

    //doDrawInfo  = true; 
    ofSetVerticalSync(false);    
    framerate = settings.getValue("settings:framerate", 90); 
    ofSetFrameRate(framerate);

    host = settings.getValue("settings:host", "127.0.0.1"); 
    port = settings.getValue("settings:port", 7110);
    
    debug = (bool) settings.getValue("settings:debug", 1);

    sender.setup(host, port);

    compname = "RPi";
    
    file.open(ofToDataPath("compname.txt"), ofFile::ReadWrite, false);
    ofBuffer buff;
    if (file) {
        buff = file.readToBuffer();
        compname = buff.getText();
    } else {
        compname += "_" + ofGetTimestampString("%y-%m-%d-%H-%M-%S-%i");
        ofStringReplace(compname, "-", "");
        ofStringReplace(compname, "\n", "");
        ofStringReplace(compname, "\r", "");
        buff.set(compname.c_str(), compname.size());
        ofBufferToFile("compname.txt", buff);
    }
    cout << compname;

    w = 160;
    h = 120;
    cam.setup(w, h, false); // color/gray;

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

    pyrScale = 0.5;   // 0 to 1
    levels = 4;   // 1 to 8
    winsize = 8;   // 4 to 64
    iterations = 2;   // 1 to 8
    polyN = 7;   // 5 to 10
    polySigma = 1.5;   // 1.1 to 2
    OPTFLOW_FARNEBACK_GAUSSIAN = false;
	
    useFarneback = true;
    winSize = 32;   // 4 to 64
    maxLevel = 3;   // 0 to 8
	
    maxFeatures = 200;   // 1 to 1000
    qualityLevel = 0.01;   // 0.001 to 0.02
    minDistance = 4;   // 1 to 16
}

void ofApp::update() {
    frame = cam.grab();

    if (!frame.empty()) {
        if (useFarneback) {
			curFlow = &farneback;
            farneback.setPyramidScale( pyrScale );
            farneback.setNumLevels( levels );
            farneback.setWindowSize( winsize );
            farneback.setNumIterations( iterations );
            farneback.setPolyN( polyN );
            farneback.setPolySigma( polySigma );
            farneback.setUseGaussian( OPTFLOW_FARNEBACK_GAUSSIAN );
		} else {
			curFlow = &pyrLk;
            pyrLk.setMaxFeatures( maxFeatures );
            pyrLk.setQualityLevel( qualityLevel );
            pyrLk.setMinDistance( minDistance );
            pyrLk.setWindowSize( winSize );
            pyrLk.setMaxLevel( maxLevel );
		}
        //check it out that that you can use Flow polymorphically
        curFlow->calcOpticalFlow(frame);
    }
}

void ofApp::draw() {
    ofSetColor(255);
    ofBackground(0);
    
    if(!frame.empty()) {
        if (debug) {
		    drawMat(frame,220,0,w*4,h*4);
		    curFlow->draw(220,0,w*4,h*4);

	        if (useFarneback) {
	        	std::cout << farneback.getTotalFlow() << "\n" << farneback.getAverageFlow();
		    } else {
		    	std::cout << pyrLk.getFeatures() << "\n" << pyrLk.getCurrent() << "\n" << pyrLk.getMotion();
		    }
    	}
    }

    if (debug) {
        stringstream info;
        info << "FPS: " << ofGetFrameRate() << "\n";
        //info << "Camera Resolution: " << cam.width << "x" << cam.height << " @ "<< "xx" <<"FPS"<< "\n";
        ofDrawBitmapStringHighlight(info.str(), 10, 10, ofColor::black, ofColor::yellow);
    }
   
}


void ofApp::sendOsc() {
	/*
    ofxOscMessage m;
    m.setAddress("/contour");
    m.addStringArg(compname);
    
    m.addIntArg(index);
    m.addBlobArg(contourColorBuffer);
    m.addBlobArg(contourPointsBuffer);

    sender.sendMessage(m);
    */
}

