#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
    // basic values
    ofSeedRandom();
    width = ofGetWidth();
    height = ofGetHeight();

    gridWidth  = 5;
    gridHeight = 5;
    //gridThreshold = 10;
    
    fader   = 1.618033; // fade with PHI

    xlen = (int) width / gridWidth;
    ylen = (int) height / gridHeight;

    // komplexere datenstrukturen, groessen initialisieren
    dataGrayCurrent = new unsigned char[width*height]; // grarray
    dataGrayPast = new unsigned char[width*height]; // grarray
    dataGrayDiff = new unsigned char[width*height]; // grarray
    dataGrayThreshold = new unsigned char[width*height]; // grarray
    
    omxCameraSettings.width = width;
    omxCameraSettings.height = height;
    omxCameraSettings.framerate = 30;
    omxCameraSettings.enablePixels = true;
    vidGrabber.setup(omxCameraSettings);
    vidTexture.allocate(width,height, GL_RGB);

    sender.setup("BrazilV.local", 7110);
}


//--------------------------------------------------------------
void ofApp::update() {
    int channels = 3;
    int totalBytes = width * height * channels;
    int counter = 0;

    // set background black
    ofBackground(0,0,0);
      
    // motion dedection action
    if (vidGrabber.isFrameNew()) {
        unsigned char * pixels = vidGrabber.getPixels();

        // convert to gray with supersimple-average-all-channels geayscale converter
        for (int i = 0; i < totalBytes; i += channels){
            int ave = 0;
            for (int curByte =0;curByte < 3; curByte++) {
                ave += pixels[i + curByte];
            }
            ave = (ave / 3);
            dataGrayCurrent[counter] = ave;
            counter++;
        }
        
    
        // get the difference between this frame and the last frame
        for (int i = 0; i < width*height; i++){
            dataGrayDiff[i] = abs( dataGrayCurrent[i] - dataGrayPast[i] );
        }

        //fade grid
        for (int i = 0; i < gridHeight * gridWidth; i++) {
            if (gridData[i] > 0) gridData[i] = gridData[i] / fader;
        }
        
        // thresholding of the motion data
        int threshold = 127;
        //int threshold = (int) (thresholdKey * 2);
        int row = 0;
        int col = 0;

        // weigh gridData Elements against threshold
        for (int i=0; i<height; i++){
            for (int j=0; j<width; j++){
                row = (int) ((height - i) * gridHeight / height);
                col = (int) ((width - j) * gridWidth / width);

                if(dataGrayDiff[width * i + j] < threshold)
                    dataGrayThreshold[width * i + j] = 0;
                else {
                    dataGrayThreshold[width * i + j] = 255;
                    gridData[row * gridWidth + col]++;
                    //diffSum++;
                }
            }
        }
    
        vidTexture.loadData(dataGrayThreshold, width,height, GL_LUMINANCE);
                
        // calculate mean to dedect sudden movements
        //diffMean     = diffSum / width * height;
        //historyMean  = (lastDiffMean + diffMean) / 2;
        //lastDiffMean = diffMean;
    }
    
    //update dataGrayPast
    memcpy(dataGrayPast, dataGrayCurrent, width*height);
}

//--------------------------------------------------------------
void ofApp::draw(){
    int row = 0;
    int xpos = 0;
    int ypos = height;

//  vidTexture.draw(360,20,width,height);


    for (int i = 0; i < gridHeight * gridWidth; i++ ) {
        if (i % gridWidth == 0) row++;
//      cout << (xlen * ylen * (movementThreshold / 100.0f)) << " " << endl;
        if (gridData[i] > 0 ) {
        //cout << i << " - " << gridData[i] << " " << endl;
            xpos = (int) (i % gridWidth) * xlen;
            ypos = (int) (height - row * ylen);
            ofSetColor(0, 0, gridData[i] / 2);
            ofRect( xpos, ypos, xlen, ylen);
        }

        sendOsc(i, gridData[i]);
    }

}

void ofApp::sendOsc (int slot, int value) {
    ofxOscMessage msg;

    msg.setAddress("/ctrl");
    msg.addIntArg(slot);
    msg.addIntArg(value);

    std::cout << slot << " " << value << endl;

    sender.sendMessage(msg);
}