#pragma once

#include "ofMain.h"
#include "ofxJSON.h"
#include "ofxTextSuite.h"
#include "ofxFontStash.h"
#include "Clock.h"
#include "Panel.h"

class ofApp : public ofBaseApp{

	public:
    
        struct rawPost {
            std::string title, text, email, type, imageLink, portraitLink, logoLink, qrLink, venue, date, shortCode;
        };

		void setup();
		void update();
		void draw();
        void keyPressed(int key);

    
        void updatePosts(rawPost newRawPost);
        void generatePostImages(rawPost rawPost, map<string, ofFbo> panelBuffers);
        vector<ofApp::rawPost> fetchRawPosts(std::string newURL);
        void drawSquareImage(float x, float y, ofImage img, int targetWidth);
        void drawZoomedImage(float x, float y, ofImage img, int targetWidth, int targetHeight);
    
        std::string intToMonth(int num);
        std::string appendOrdinal(int num);
    
        ofTexture renderDate();
    
    /*variables from canvas*/
        ofxJSONElement json;
        ofxJSONElement settings;
        std::string url;
        
        vector<rawPost> rawPosts;
        deque< map<string, ofTexture> > posts;
    
        int timer;
        int rawIterator;
        int today;
    
        bool bNeedsFetch;
        bool nextPanelReady;
        bool bInternal, bExternal;
        bool bSetup;
    
        ofxTextBlock todayText;
        ofxFontStash font;
    
        map<string, ofRectangle> panels;
    
        ofImage eventImage, housekeepingImage, informationImage, blankImage;
    
        ofTexture dateTex;
    
        map<string, ofFbo> panelBuffers1;
        map<string, ofFbo> panelBuffers2;
        map<string, ofTexture> returnMap;
    
        ofImage image, portrait, qrCode, logo;

    
        ofVideoPlayer housekeepingVid, eventVid, informationVid;
        bool eVidIsPlaying, hVidIsPlaying, iVidIsPlaying;

        int titleIndex;
        int textIndex;
        int portIndex;
        int typeIndex;
        int dateIndex;
                
        Panel titlePanel;
        Panel textPanel;
        Panel portPanel;
        Panel typePanel;
        Panel datePanel;
        
        Clock clock;
};

