#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(54);
    
    soundStream.listDevices();
    soundStream.setDeviceID(3);
    
    int bufferSize = 512;
    
    left.assign(bufferSize, 0.0);
    right.assign(bufferSize, 0.0);
    volHistory.assign(400, 0.0);
    soundStream.setup(this, 0, 1, 44100, bufferSize, 4);
    
    
    recording=false;
    
    beginMessage.loadSound("beginningGuide.wav");
    endMessage.loadSound("finishingGuide.wav");
    
    lasttime = "No one recorded yet.";
    
    serial.listDevices();
    vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();
    serial.setup("tty.usbmodem1431", 9600);
    memset(bytesReadString, 0, 2);
    serial.flush();
}

//--------------------------------------------------------------
void ofApp::update(){
    if (serial.available() > 0) {
        char ch = serial.readByte();
        
        if(ch == 'b' && !recording)
            beginRecording();
        else if (ch == 'f' && recording)
            endRecording();
    }
    
    if (recording && ofGetElapsedTimef() > 60.0)
        endRecording();
    
    ofDirectory dir(_VOICE_PATH_);
    dir.listDir();
    numOfVoice = (int)dir.size();
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofNoFill();
    
    // draw the left channel:
    ofPushStyle();
    ofPushMatrix();
    //    ofTranslate(32, 170, 0);
    ofTranslate(32, 32, 0);
    
    
    ofSetColor(225);
    ofDrawBitmapString("Left Channel", 4, 18);
    
    ofSetLineWidth(1);
    ofRect(0, 0, 512, 200);
    
    ofSetColor(245, 58, 135);
    ofSetLineWidth(3);
    
    ofBeginShape();
    for (int i = 0; i < left.size(); i++){
        ofVertex(i*2, 100 -left[i]*180.0f);
    }
    ofEndShape(false);
    
    ofPopMatrix();
    ofPopStyle();
    
    // draw the right channel:
    ofPushStyle();
    ofPushMatrix();
    ofTranslate(32, 232, 0);
    
    ofSetColor(225);
    ofDrawBitmapString("Right Channel", 4, 18);
    
    ofSetLineWidth(1);
    ofRect(0, 0, 512, 200);
    
    ofSetColor(245, 58, 135);
    ofSetLineWidth(3);
    
    ofBeginShape();
    for (int i = 0; i < right.size(); i++){
        ofVertex(i*2, 100 -right[i]*180.0f);
    }
    ofEndShape(false);
    
    ofPopMatrix();
    ofPopStyle();
    
    ofDrawBitmapString(ofGetTimestampString("%Y-%m-%d %H:%m:%s"), 576, 50);
    ofDrawBitmapString("We have "+ofToString(numOfVoice) + " voices", 576, 66);
    ofDrawBitmapString(lasttime, 576,82);
    
    if (recording) {
        ofDrawBitmapString("Recording......ElapsedTime: "+ofToString(ofGetElapsedTimef()), 576,98);
    } else {
        ofDrawBitmapString("IDLE.", 576, 98);
    }

}

//--------------------------------------------------------------
void ofApp::audioIn(float * input, int bufferSize, int nChannels){
    
    //lets go through each sample and calculate the root mean square which is a rough way to calculate volume
    for (int i = 0; i < bufferSize; i++){
        left[i] = input[i];
        //		left[i]		= input[i]*0.5;
        //		right[i]	= input[i+1]*0.5;
    }
    
    if(recording)
        audioRecorder.addSamples(input, bufferSize*nChannels);
}

string ofApp::getFilename(bool newFile) {
    
    static string name;
    
    if(newFile) {
        ofDirectory dir;
        dir.listDir(_VOICE_PATH_);
        
        string today = ofGetTimestampString("%Y%m%d");
        
        int seqCounter = 0;
        for (int i=0; i< (int)dir.size(); i++) {
            name = dir.getName(i);
            if( ofIsStringInString(name, today) )
                seqCounter++;
        }
        
        name = today + ofToString( seqCounter+1, 0, 3, '0') + ".wav";
    }
    
    return name;
}

//--------------------------------------------------------------
void ofApp::beginRecording() {
    
    static int audioCount = 0;
    
    beginMessage.play();
    while(beginMessage.getIsPlaying());
    
    string filename = getFilename();
    
    audioRecorder.setup(_VOICE_PATH_ + filename);
    audioRecorder.setFormat(SF_FORMAT_WAV | SF_FORMAT_PCM_16);
    recording = true;
    audioCount++;
    
    ofResetElapsedTimeCounter();
}

//--------------------------------------------------------------
void ofApp::endRecording() {
    
    long t = ofGetSystemTime();
    
    if( ofGetElapsedTimef() > 2.0) {
        
        cout << "stop recording\n";
        audioRecorder.finalize();
        audioRecorder.close();
        recording = false;
        
        lasttime = "Latest voice is recorded in "+ofGetTimestampString("%Y-%m-%d %H:%m:%s");
        
        endMessage.play();
        while(endMessage.getIsPlaying());
    } else {
        cout << "quit recording\n";
        audioRecorder.close();
        recording=false;
        
        string filename = getFilename(false);
        filename = _VOICE_PATH_ + filename;
        
        ofFile file(filename);
        if(file.exists())
            file.remove();
    }
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == 'b' && !recording)
        beginRecording();
    else if (key == 'f' && recording)
        endRecording();
    else if (key == 'a')
        cout << ofGetElapsedTimef() << "\n";
    else if (key == 'r')
        ofResetElapsedTimeCounter();
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
