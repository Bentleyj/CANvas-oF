//
//  Clock.cpp
//  OF
//
//  Created by James Bentley on 1/29/15.
//
//

#include "Clock.h"

Clock::Clock() {
    bounds = ofRectangle(0, 0, 1, 1);
    rotSpeed = 0.0f;
    maxSpeed = 100.0f;
    lastSpeed = 0.0f;
    rotAcc = 1.0f;
    rot = 0.0f;
    bFlipping = false;
    bNeedsUpdate = false;
    currCol = ofColor(0);
    nextCol = ofColor(255, 0, 0);
    
    //Make everything looking nice and smooth.
    ofSetCircleResolution(100);
    ofEnableSmoothing();
    
    //Set size & position of our clock
    if( ofGetHeight() < ofGetWidth() )
        radius = (ofGetHeight()/2);
    else
        radius = (ofGetWidth()/2);
    
    top = (ofGetHeight()/2);
    left = (ofGetWidth()/2);
}

Clock::Clock(ofRectangle bounds) : bounds(bounds){
    rotSpeed = 0.0f;
    maxSpeed = 100.0f;
    lastSpeed = 0.0f;
    ofSetCircleResolution(100);
    ofEnableSmoothing();
    
    topImg.allocate(bounds.getWidth(), bounds.getHeight()/2, OF_IMAGE_GRAYSCALE);
    botImg.allocate(bounds.getWidth(), bounds.getHeight()/2, OF_IMAGE_GRAYSCALE);
    
    rotSpeed = 0.0f;
    maxSpeed = 100.0f;
    rotAcc = 1.0f;
    rot = 0.0f;
    bFlipping = false;
    bNeedsUpdate = false;
    currCol = ofColor(0);
    nextCol = ofColor(0);
    
    if( bounds.getHeight() < bounds.getWidth() )
        radius = (bounds.getHeight()/2) - 5;
    else
        radius = (bounds.getWidth()/2) - 5;
    
    top = bounds.getY() + bounds.getHeight()/2;
    left = bounds.getX() + bounds.getWidth()/2;
}

void Clock::setImgs(std::string clockImgLink, std::string hourHandLink, std::string minuteHandLink, std::string outsideImgLink) {
    clockImg.loadImage(clockImgLink);
    hourHand.loadImage(hourHandLink);
    minuteHand.loadImage(minuteHandLink);

    topImg.loadImage(outsideImgLink);
    botImg = topImg;
    botImg.mirror(true, false);
}

Clock::~Clock() {
    
}

void Clock::update( int sec, int minute, int hour ) {
    secondsAngle = 6 * sec;
    minutesAngle = 6 * minute;
    hoursAngle = 30 * hour + (minute / 2);
    if(bFlipping) {
        rotSpeed += rotAcc;
        rotSpeed = min(rotSpeed, maxSpeed);
        rot-=rotSpeed;
        if(rot < -170){
            bFlipping = false;
            bNeedsUpdate = true;
            rot = 0;
            lastSpeed = rotSpeed;
            rotSpeed = 0.0f;
            currCol = nextCol;
        }
    }
}

void Clock::display() {
    
    ofPushMatrix();
    ofPushStyle();
    ofSetRectMode(OF_RECTMODE_CENTER);
    //Set the coortinatesystem to the center of the clock.
    ofPoint circle_center = ofPoint( left, top);
    ofTranslate(circle_center);
    clockImg.draw(0, 0);
    ofSetRectMode(OF_RECTMODE_CORNER);

//    ofRotateZ(-90);
    
    
    
//    //Draw background of the clock
//    ofSetHexColor(0xbbbbbb);
//    ofFill();
//    ofCircle( ofPoint(0,0), radius );
//    
//    //Draw Outline of the clock
//    ofSetLineWidth( (radius/100)*4 );
//    ofNoFill();
//    ofSetHexColor(0x000000);
//    ofCircle( ofPoint(0,0), radius );
//    
//    //Draw the clock face
//    ofSetLineWidth(1);
//    
//    for(int i=0;i<12;i++){
//        ofRotateZ( 30 );
//        
//        ofFill();
//        ofRect( ofPoint( (radius/10)*8, -((radius/100)*2) ), (radius/100)*20, (radius/100)*4 );
//        ofNoFill();
//        ofRect( ofPoint( (radius/10)*8, -((radius/100)*2) ), (radius/100)*20, (radius/100)*4 );
//    }
    
    //Draw the hour hand
    ofPushMatrix();
    ofPushStyle();
    ofSetRectMode(OF_RECTMODE_CENTER);
    ofSetHexColor(0x000000);
    ofRotateZ( hoursAngle );
    hourHand.draw(0, 0);
//    ofFill();
//    ofRect( 0,-((radius/100)*5),(radius/10)*6, (radius/100)*10 );
//    ofNoFill();
//    ofRect( 0,-((radius/100)*5),(radius/10)*6, (radius/100)*10 );
    
    ofPopStyle();
    ofPopMatrix();
    
    //Draw the minute hand
    ofPushMatrix();
    ofPushStyle();
    ofSetRectMode(OF_RECTMODE_CENTER);
    
    ofSetHexColor(0x000000);
    ofRotateZ( minutesAngle );
    minuteHand.draw(0, 0);
    
//    ofFill();
//    ofRect( 0,-((radius/100)*4),(radius/10)*7, (radius/100)*8 );
//    ofNoFill();
//    ofRect( 0,-((radius/100)*4),(radius/10)*7, (radius/100)*8 );
    
    ofPopStyle();
    ofPopMatrix();
    
//    Draw the second hand
//    ofPushMatrix();
//    ofPushStyle();
//    
//    ofSetHexColor(0x000000);
//    ofRotateZ( secondsAngle );
//    
//    ofFill();
//    ofRect( -((radius/100)*2),-((radius/100)*2),(radius/10)*9, (radius/100)*4 );
//    ofNoFill();
//    ofRect( -((radius/100)*2),-((radius/100)*2),(radius/10)*9, (radius/100)*4 );
////
//    ofPopStyle();
//    ofPopMatrix();
    
//    ofFill();
//    ofSetHexColor(0x000000);
//    ofCircle( ofPoint(0,0), (radius/100)*6 );
    
    ofPopMatrix();
    
    ofPushMatrix();
    ofEnableDepthTest();
    ofSetColor(currCol);
//    ofSetColor(255, 0, 0, 255);
    botImg.draw(bounds.getX(), bounds.getY() + bounds.getHeight()/2, bounds.getWidth(), bounds.getHeight()/2);
    
    ofSetColor(nextCol);
//    ofSetColor(255, 0, 0, 255);
    topImg.draw(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight()/2);
    
    if(bFlipping) {
        ofTranslate(circle_center);
        ofRotateX(rot);
        ofTranslate(-circle_center);
        if(rot < -77) ofSetColor(nextCol);
        else ofSetColor(currCol);
        topImg.draw(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight()/2);
    }
    ofDisableDepthTest();
    ofPopMatrix();
    if(bNeedsUpdate) bNeedsUpdate = false;
    ofPopStyle();
}

void Clock::flip(ofColor newCol, float newSpeed) {
    bFlipping = true;
    nextCol = newCol;
    rot = -1;
    rotSpeed = newSpeed;
}