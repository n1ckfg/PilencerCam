#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxCvPiCam.h"
#include "ofxOsc.h"
#include "ofxXmlSettings.h"

class ofApp : public ofBaseApp {

    public:
        void setup();
		void update();
		void draw();

		string compname;
		string host; // hostname;
		int port; // default 7110;

		bool debug; // draw to local screen, default true

		ofFile file;
		ofxXmlSettings settings;

	    ofxCv::FlowFarneback farneback;
	    //ofxCv::FlowPyrLK pyrLk;
	    ofxCv::Flow* curFlow;
	    int w,h;
	    float pyrScale;
	    int levels;
	    int winsize;
	    int iterations;
	    int polyN;
	    float polySigma;
	    bool OPTFLOW_FARNEBACK_GAUSSIAN;
	    //bool useFarneback;
	    int winSize;
	    int maxLevel;
	    int maxFeatures;
	    float qualityLevel;
	    int minDistance;

	    float triggerThreshold;
	    int timeDelay;
	    int markTime;
	    bool trigger;
	    float avgMotion;

		ofxCvPiCam cam;
		cv::Mat frame;

		// for more camera settings, see:
		// https://github.com/orgicus/ofxCvPiCam/blob/master/example-ofxCvPiCam-allSettings/src/testApp.cpp

        int camShutterSpeed; // 0 to 330000 in microseconds, default 0
    	int camSharpness; // -100 to 100, default 0
    	int camContrast; // -100 to 100, default 0
    	int camBrightness; // 0 to 100, default 50
		int camIso; // 100 to 800, default 300
		int camExposureCompensation; // -10 to 10, default 0;

		// 0 off, 1 auto, 2 night, 3 night preview, 4 backlight, 5 spotlight, 6 sports, 7, snow, 8 beach, 9 very long, 10 fixed fps, 11 antishake, 12 fireworks, 13 max
		int camExposureMode; // 0 to 13, default 0

		//string oscAddress;
		int framerate;
		//bool doDrawInfo;

		ofxOscSender sender;
		void sendOsc(int _trigger);

};
