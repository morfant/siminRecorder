#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    string logFileName = "log_siminrecorder_" + ofGetTimestampString("%Y%m%d_%H%M%S") + ".txt";
    ofLogToFile(logFileName, true);
    
    ofBackground(54);
    
    // Audio
    soundStream.getDeviceList();
    soundStream.setDeviceID(3);
//    soundStream.setDeviceID(0);
    
    int bufferSize = 512;
    
    left.assign(bufferSize, 0.0);
    right.assign(bufferSize, 0.0);
    volHistory.assign(400, 0.0);
    soundStream.setup(this, 0, 1, 44100, bufferSize, 4);
    
    recording=false;
    
    beginMessage.load("beginningGuide.wav");
    callingMessage.load("callingGuide.wav");
    endMessage.load("finishingGuide.wav");
    
    lasttime = "No one recorded yet.";
    
    
    // Serial
    serial.listDevices();
    vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();
    if (serial.setup("tty.usbmodem1431", 9600)){
        serialConnected = true;
        memset(bytesReadString, 0, 2);
        serial.flush();
        
        serialConnectionResult.open(_RESULT_PATH_, ofFile::WriteOnly);
        serialConnectionResult << ofGetTimestampString("%Y-%m-%d %H:%M:%S");
        serialConnectionResult << "\n\n센서와의 연결에 성공했습니다.";
        serialConnectionResult.close();

    } else {
        serialConnected = false;
        
        // serial connection failure notice
        serialConnectionResult.open(_RESULT_PATH_, ofFile::WriteOnly);
        serialConnectionResult << ofGetTimestampString("%Y-%m-%d %H:%M:%S");
        serialConnectionResult << "\n\n센서와의 연결에 실패했습니다. 연결을 확인하고 시스템을 재부팅해 주세요.";
        serialConnectionResult.close();
    }
    
    
    //GUI
    gui.setup("", "settings.xml", 576, sliderYpos);
    gui.setSize(400, 200);
    gui.add(volumeSlider.setup("Record Volume", 1.0, 0.0, 2.0));
    gui.add(callingIntervalSlider.setup("Calling Interval(min)", 10, 1, 60));
    volumeSlider.setSize(400, 30);
    callingIntervalSlider.setSize(400, 30);
    volumeSlider.loadFromFile("settings.xml");
    callingIntervalSlider.loadFromFile("settings.xml");
}

//--------------------------------------------------------------
void ofApp::update(){
    
    if (serial.available() > 0) {
        char ch = serial.readByte();
        
        if(ch == 'b' && !recording){
//            cout << "b" << endl;
            beginRecording();
        }
        
        else if (ch == 'f' && recording){
//            cout << "f" << endl;
            endRecording();
        }
        
    }

    if (!recording && ofGetElapsedTimef() > CALLING_INTERVAL) {
        calling();
    }
    
    
    if (recording && ofGetElapsedTimef() > 60.0)
        endRecording();
    
    ofDirectory dir(_VOICE_PATH_);
    dir.listDir();
    numOfVoice = (int)dir.size() - 1;
    
    
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
    ofDrawRectangle(0, 0, 512, 200);
    
    ofSetColor(245, 58, 135);
    ofSetLineWidth(3);
    
    ofBeginShape();
    for (int i = 0; i < left.size()-1; i++){
        ofVertex(i, 100 -left[i]*100.0f);
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
    ofDrawRectangle(0, 0, 512, 200);
    
    ofSetColor(245, 58, 135);
    ofSetLineWidth(3);
    
    ofBeginShape();
    for (int i = 0; i < right.size()-1; i++){
        ofVertex(i, 100 -right[i]*100.0f);
    }
    ofEndShape(false);
    
    ofPopMatrix();
    ofPopStyle();
    

    // Text
    ofSetColor(255);
    ofDrawBitmapString(ofGetTimestampString("%Y-%m-%d %H:%M:%S"), 576, 50);
    ofDrawBitmapString("We have "+ofToString(numOfVoice) + " voices", 576, 66);
    ofDrawBitmapString(lasttime, 576,82);
    
    if (recording) {
        ofDrawBitmapString("Recording......ElapsedTime: "+ofToString(ofGetElapsedTimef()), 576,98);
    } else {
        ofDrawBitmapString("IDLE.", 576, 98);
    }
    
    if (serialConnected) {
        ofSetColor(0, 255, 0);
        ofDrawBitmapString("tty.usbmodem1431 sucessfully connected.", 576, 300);
    } else {
        ofSetColor(255, 0, 0);
        ofDrawBitmapString("Serial connection failed.", 576, 300);
    }
    
    
    // Display amp value
//    ofDrawBitmapString("'+' Volume up / '-' Volume down", 576, 140);
//    ofDrawBitmapString("Record volume: " + ofToString(amp), 576, 150);
    ofSetColor(255);
    ofDrawBitmapString("*To save setting value, Click disk icon", 576, sliderYpos - 10);
    amp = volumeSlider;
    CALLING_INTERVAL = 60.0 * callingIntervalSlider;
    gui.draw();

}

//--------------------------------------------------------------
void ofApp::audioIn(float * input, int bufferSize, int nChannels){
    
    //lets go through each sample and calculate the root mean square which is a rough way to calculate volume
    for (int i = 0; i < bufferSize; i++){
        		left[i]		= input[i]*amp;
        		right[i]	= input[i+1]*amp;
    }

//    cout << "BEFORE" << *input << endl;
    
    // Record volume control with variable 'amp'
    for (int i = 0; i < bufferSize; i++){
        float ft = input[i] * amp;
        input[i] = ft;
    }

    
//    cout << "AFTER" << *input << endl;
    
    if(recording)
    {
        audioRecorder.addSamples(input, bufferSize*nChannels);
    }
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
    
    if (callingMessage.isPlaying()){
        callingMessage.stop();
    }
    
    static int audioCount = 0;
    
    beginMessage.play();
    while(beginMessage.isPlaying());
    
    string filename = getFilename();
    
    audioRecorder.setup(_VOICE_PATH_ + filename);
    audioRecorder.setFormat(SF_FORMAT_WAV | SF_FORMAT_PCM_16);
    recording = true;
    audioCount++;
    
    ofResetElapsedTimeCounter();
}


//--------------------------------------------------------------
void ofApp::calling() {
    
    callingMessage.play();
    ofResetElapsedTimeCounter();
//    while(callingMessage.isPlaying());
    
    
}

//--------------------------------------------------------------
void ofApp::endRecording() {
    
    long t = ofGetSystemTime();
    
    if( ofGetElapsedTimef() > 2.0) {
        
        cout << "stop recording\n";
        audioRecorder.finalize();
        audioRecorder.close();
        recording = false;
        
        lasttime = "Latest voice is recorded in "+ofGetTimestampString("%Y-%m-%d %H:%M:%S");
        
        endMessage.play();
        while(endMessage.isPlaying());
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
    else if (key == 'c' && !recording)
        calling();
    else if (key == 'a')
        cout << ofGetElapsedTimef() << "\n";
    else if (key == 'r')
        ofResetElapsedTimeCounter();
    else if (key == '+' || key == '=') {
        amp+=0.1;
        amp = min(2.0f, amp);
    }
    else if (key == '-' || key == '_') {
        amp-=0.1;
        amp = max(0.0f, amp);
    }
    

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
