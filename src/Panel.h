//
//  Panel.h
//  OF
//
//  Created by James Bentley on 1/30/15.
//
//

#pragma once

#include "ofMain.h"

class Panel {
public:
    Panel();
    ~Panel();
    Panel(ofRectangle, int);
    void setTexs(ofTexture *currTex, ofTexture *nextTex);
    void init(ofRectangle, float);
    void update();

    void display();
    void displayDebug();
    void flip(float newSpeed);
    void setFlipping(bool newFlipping);
    float getRot();
    float getLastSpeed();
    
    bool needsUpdate();
protected:
    ofRectangle bounds;
    
    ofTexture *curr;
    ofTexture *next;
    
    float rotThreshold;
    
    bool bFlipping, bNeedsUpdate;
    
    float rotSpeed;
    float lastSpeed;
    float rotAcc;
    float maxSpeed;
    float rot;
};