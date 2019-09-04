#pragma once

#include "ofMain.h"
#include "ofxRPiCameraVideoGrabber.h"
#include "ofxOsc.h"
#include "ofxXmlSettings.h"

class ofApp : public ofBaseApp {

    public:
        void setup();
		void update();
		void draw();

		char * text;
		int width;
		int height;
		int threshold;
		int xlen;
		int ylen;
		float fader;
		
		int threshold;
		
		// grid
		int gridData[256];
		int lastGridData[256];
		float gridHistory[256];
		bool grid[256];
		int gridWidth;
		int gridHeight;
		
		unsigned char * dataGray;
		unsigned char * dataGrayCurrent;
		unsigned char * dataGrayPast;
		unsigned char * dataGrayDiff;
		unsigned char * dataGrayThreshold;
		
		ofxRPiCameraVideoGrabber vidGrabber;
    	OMXCameraSettings omxCameraSettings;
		ofTexture vidTexture;

		ofxOscSender sender;
		void sendOsc(int slot, int value);

};
