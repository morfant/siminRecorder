#pragma once

#include "ofMain.h"
#include "ofxLibsndFileRecorder.h"

//#define _VOICE_PATH_    "/Users/siminrecorder/Dropbox/voices/"
#define _VOICE_PATH_    "/Users/giy/Desktop/siminTemp/"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    void beginRecording();
    void endRecording();
    void audioIn(float * input, int bufferSize, int nChannels);
    ofSoundStream soundStream;
    
    vector <float> left;
    vector <float> right;
    vector <float> volHistory;
    
    bool recording;
    ofxLibsndFileRecorder audioRecorder;
    
    ofSerial serial;
    char bytesReadString[2];
    
    ofSoundPlayer beginMessage;
    ofSoundPlayer endMessage;
    
    string getFilename(bool newFile=true);
    int numOfVoice;
    string lasttime;

		
};
