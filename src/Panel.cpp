//
//  Panel.cpp
//  OF
//
//  Created by James Bentley on 1/30/15.
//
//

#include "Panel.h"

Panel::Panel() {
    bounds = ofRectangle(0, 0, 1, 1);
    rotSpeed = 0.0f;
    lastSpeed = 0.0f;
    maxSpeed = 100.0f;
    rotAcc = 1.0f;
    rot = 0;
    bFlipping = false;
    bNeedsUpdate = false;
    
    ofEnableSmoothing();
}

Panel::~Panel() {
    
}

void Panel::init(ofRectangle inputBounds, float inputRotThreshold) {
    bounds = inputBounds;
    rotThreshold = inputRotThreshold;
}

void Panel::setTexs(ofTexture *currTex, ofTexture *nextTex) {
    
    curr = currTex;
    next = nextTex;
    if(bNeedsUpdate) {
        bNeedsUpdate = false;
    }
}

void Panel::display() {
    if(bFlipping) {
        if(curr != NULL && next != NULL) {
            ofPushStyle();
            ofEnableDepthTest();
            ofSetColor(255);
            ofPushMatrix();
            ofTranslate(bounds.getX(), bounds.getY());
            next->drawSubsection(0, 0, bounds.getWidth(), bounds.getHeight()/2, 0, 0);
            curr->drawSubsection(0, bounds.getHeight()/2, bounds.getWidth(), bounds.getHeight()/2, 0, bounds.getHeight()/2);
            if(rot > rotThreshold) {
                ofPushMatrix();
                ofTranslate(0, bounds.getHeight()/2);
                ofRotateX(rot);
                ofTranslate(0, -bounds.getHeight()/2);
                curr->drawSubsection(0, 0, bounds.getWidth(), bounds.getHeight()/2, 0, 0);
                ofPopMatrix();
            }
            else {
                ofPushMatrix();
                ofTranslate(0, bounds.getHeight()/2);
                ofRotateX(180 + rot);
                next->drawSubsection(0, 0, bounds.getWidth(), bounds.getHeight()/2, 0, bounds.getHeight()/2);
                ofPopMatrix();
            }
            ofPopMatrix();
            ofDisableDepthTest();
            ofPopStyle();
        }
        else {
            ofLogWarning("Panel::display()", "one or more of you images have not been set");
        }
    }
    else {
        if(curr != NULL && next != NULL) {
            ofEnableDepthTest();
            ofPushMatrix();
            ofSetColor(255);
            ofTranslate(bounds.getX(), bounds.getY());
            curr->drawSubsection(0, 0, bounds.getWidth(), bounds.getHeight(), 0, 0);
            //            cout<<bounds.getWidth()<<endl;
            ofPopMatrix();
            ofDisableDepthTest();
        }
        else {
            ofLogWarning("Panel::display()", "one of more of your images have not been set");
        }
    }
}

void Panel::displayDebug() {
    //    currTopImg->draw(0, 0, bounds.getWidth(), bounds.getHeight()/2);
    //    currBotImg->draw(0, bounds.getHeight()/2, bounds.getWidth(), bounds.getHeight()/2);
    //    ofPopMatrix();
}

void Panel::update() {
    if(bFlipping) {
        rotSpeed += rotAcc;
        rotSpeed = min(rotSpeed, maxSpeed);
        rot-=rotSpeed;
        if(rot < -180){
            bFlipping = false;
            bNeedsUpdate = true;
            rot = 0;
            lastSpeed = rotSpeed;
            rotSpeed = 0.0f;
        }
    }
}

void Panel::setFlipping(bool newFlipping) {
    bFlipping = newFlipping;
}

void Panel::flip(float newSpeed) {
    if(!bFlipping) {
        bFlipping = true;
        rot = -1;
        rotSpeed = newSpeed;
    }
}

float Panel::getRot() {
    return rot;
}

bool Panel::needsUpdate() {
    return bNeedsUpdate;
}

float Panel::getLastSpeed() {
    return lastSpeed;
}