//
//  Clock.h
//  OF
//
//  Created by James Bentley on 1/29/15.
//
//

#pragma once

#include "ofMain.h"
#include "Panel.h"

class Clock : public Panel {
public:
    Clock();
    Clock(ofRectangle);
    ~Clock();
    
    void update(int, int, int);
    void display();
    void flip(ofColor newCol, float newSpeed);
    void setImgs(std::string clockImgLink, std::string hourHandLink, std::string minuteHandLink, std::string outsideImgLink);
    
private:
    int radius;
    
    ofRectangle bounds;
    int top;
    int left;
    
    ofImage topImg, botImg;
    ofImage clockImg, hourHand, minuteHand;
    
    ofColor currCol;
    ofColor nextCol;
    
    float secondsAngle;
    float minutesAngle;
    float hoursAngle;
    
    bool bFlipping;
    
    float rotSpeed;
    float rotAcc;
    float maxSpeed;
    float rot;
};