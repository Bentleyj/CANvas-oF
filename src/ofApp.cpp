#include "ofApp.h"

#define BUFFER 20.0f
#define MARGIN 48.0f
#define TEXTBUFFER 40
#define LINESPACING 0.7f
#define QRSIZE 120

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetWindowPosition(0, 0);
    ofSetRectMode(OF_RECTMODE_CORNER);
    ofBackground(0);
    ofTrueTypeFont::setGlobalDpi(72);
    
    ofHideCursor();
    
    today = ofGetWeekday();
    
    bool parsingSuccessful = settings.open("settings.json");
    if(parsingSuccessful) {
        bInternal = settings["Internal"].asBool();
        bExternal = settings["External"].asBool();
        url = settings["URL"].asString();
        minDuration = settings["minDuration"].asInt();
    } else {
        ofLog() << "[" << ofGetTimestampString() << "]" << "Failed to Parse settings json, Setting Default values...";
        bInternal = true;
        bExternal = true;
        url = "http://canvas.can-online.org.uk";
        minDuration = 30;
    }
    
    if(bInternal && !bExternal) ofLog() << "[" << ofGetTimestampString() << "]" << "This is an internal screen and will only display internal posts";
    if(bExternal && !bInternal) ofLog() << "[" << ofGetTimestampString() << "]" << "This is an external screen and will only display external posts";
    if(bExternal && bInternal) ofLog() << "[" << ofGetTimestampString() << "]" << "This is both and internal and external screen and will display everything";
    if(!bInternal && !bExternal) ofLog() << "[" << ofGetTimestampString() << "]" << "This is neither an internal nor an external screen and will display nothing";
    
    bSetup = false;
    
    rawIterator = 0;
    panelBufferIterator = 0;
    numPanelBuffers = 5;
    
    housekeepingImage.loadImage("House-Icon.jpg");
    eventImage.loadImage("Event-Icon.jpg");
    informationImage.loadImage("Information-Icon.jpg");
    blankImage.loadImage("blank-canvas.jpg");
    
    panels["title"] = ofRectangle(0, 0, 1080, 1185);
    panels["text"] = ofRectangle(442.75, 1209, 596.25, 669);
    panels["portrait"] = ofRectangle(24, 1488, 389, 408);
    panels["clock"] = ofRectangle(6, 1209, 222, 266.625);
    panels["type"] = ofRectangle(241.75, 1209, 188.75, 151);
    panels["date"] = ofRectangle(241.75, 1373.125, 188.75, 102.5);
    
    titlePanel.init(panels["title"], -100);
    textPanel.init(panels["text"], -70);
    portPanel.init(panels["portrait"], -66);
    typePanel.init(panels["type"], -100);
    datePanel.init(panels["date"], -70);
    
    titleIndex = 0;
    textIndex = 0;
    portIndex = 0;
    typeIndex = 0;
    dateIndex = 0;
    
    clock = Clock(panels["clock"]);
    clock.setImgs("Clock.jpg","Hour-Hand.png","Minute-Hand.png","Half-Clock.png");
    
    font.setup("TradeGothicLTStd-Bold.ttf");
    
    todayText.init("TradeGothicLTStd-Bold.otf", 60);
    
    rawPosts = vector<ofApp::rawPost>();
    
    posts = deque< map<string, ofTexture> >();
    
    rawPosts = fetchRawPosts(url);
    
    for(int i=0; i<numPanelBuffers; i++) {
        panelBuffers[i]["title"] = ofFbo();
        panelBuffers[i]["title"].allocate(panels["title"].getWidth(), panels["title"].getHeight(), GL_RGBA);
        panelBuffers[i]["text"] = ofFbo();
        panelBuffers[i]["text"].allocate(panels["text"].getWidth(), panels["title"].getHeight(), GL_RGBA);
        panelBuffers[i]["portrait"] = ofFbo();
        panelBuffers[i]["portrait"].allocate(panels["portrait"].getWidth(), panels["title"].getHeight(), GL_RGBA);
        panelBuffers[i]["clock"] = ofFbo();
        panelBuffers[i]["type"].allocate(panels["type"].getWidth(), panels["title"].getHeight(), GL_RGBA);
    }
    
    image.allocate(panels["title"].getWidth(), panels["title"].getHeight(), OF_IMAGE_COLOR);
    portrait.allocate(panels["portrait"].getWidth(), panels["portrait"].getHeight(), OF_IMAGE_COLOR);
    qrCode.allocate(QRSIZE, QRSIZE, OF_IMAGE_COLOR); //don't allocate qr code or logo because we will need to resize them smaller
    logo.allocate(QRSIZE, QRSIZE, OF_IMAGE_COLOR);
    
    returnMap["title"].allocate(panels["title"].getWidth(), panels["title"].getHeight(), GL_RGBA);
    returnMap["text"].allocate(panels["text"].getWidth(), panels["text"].getHeight(), GL_RGBA);
    returnMap["portrait"].allocate(panels["portrait"].getWidth(), panels["portrait"].getHeight(), GL_RGBA);
    returnMap["type"].allocate(panels["type"].getWidth(), panels["type"].getHeight(), GL_RGBA);
    
    if(rawPosts.size() > 0) {
        updatePosts(rawPosts[rawIterator]);
        rawIterator++;
        rawIterator%=rawPosts.size();
        updatePosts(rawPosts[rawIterator]);
    }
    
    dateTex = renderDate();
    
    //    housekeepingVid.loadMovie("movies/House-Video.mov");
    //    eventVid.loadMovie("movies/Calendar-Video.mov");
    //    informationVid.loadMovie("movies/Information-Video.mov");
    
    //    eVidIsPlaying = false;
    //    hVidIsPlaying = false;
    //    iVidIsPlaying = false;
    postsNeedUpdate = false;
    flipsCompleted = true;
    useBuffer1 = true;
    
    //    informationVid.stop();
    //    eventVid.stop();
    //    housekeepingVid.stop();
    
    timer = ofGetElapsedTimeMillis();
    ofLogToFile("logFile.txt", true);
}

//--------------------------------------------------------------
void ofApp::update(){
    if(!bSetup) {
        if(bNeedsFetch) {
            rawPosts = fetchRawPosts(url);
            if(ofGetWeekday() != today) {
                today = ofGetWeekday();
                dateTex = renderDate();
            }
            bNeedsFetch = false;
        }
        
        clock.update(ofToInt(ofGetTimestampString("%S")), ofToInt(ofGetTimestampString("%M")), ofToInt(ofGetTimestampString("%H")));
        titlePanel.update();
        textPanel.update();
        portPanel.update();
        typePanel.update();
        datePanel.update();
        
        if(postsNeedUpdate && rawPosts.size() > 0) {
            rawIterator++;
            rawIterator%=rawPosts.size();
            updatePosts(rawPosts[rawIterator]);
            postsNeedUpdate = false;
        }
        
        if(ofGetElapsedTimeMillis() - timer > minDuration*1000 && posts.size() > 1 && flipsCompleted) {
            flipsCompleted = false;
            timer = ofGetElapsedTimeMillis();
            titlePanel.flip(0.0f);
        }
        if(titlePanel.needsUpdate()) {
            titleIndex++;
            titleIndex %= posts.size();
            textPanel.flip(titlePanel.getLastSpeed());
        }
        if(textPanel.needsUpdate()) {
            textIndex++;
            textIndex %= posts.size();
            portPanel.flip(textPanel.getLastSpeed());
        }
        if(portPanel.needsUpdate()) {
            portIndex++;
            portIndex %= posts.size();
            clock.flip(ofColor(0, 0, 0), portPanel.getLastSpeed());
        }
        if(clock.needsUpdate()) {
            typePanel.flip(clock.getLastSpeed());
        }
        if(typePanel.needsUpdate()) {
            typeIndex++;
            typeIndex %= posts.size();
            datePanel.flip(typePanel.getLastSpeed());
        }
        if(datePanel.needsUpdate()) {
            dateIndex++;
            dateIndex %= posts.size();
            nextPanelReady = false;
            bNeedsFetch = true;
            
            if(rawPosts.size() > 0) postsNeedUpdate = true;
            
            flipsCompleted = true;
        }
        if(!(rawPosts.size() > 0)) {
            ofLog() << "[" << ofGetTimestampString() << "]" << "rawPosts.size() is < 0";
            ofLog() << "[" << ofGetTimestampString() << "]" << "trying to fetch raw posts";
            rawPosts = fetchRawPosts(url);
            if(rawPosts.size() > 0) {
                ofLog() << "[" << ofGetTimestampString() << "]" << "fetch after failure was successful, filling posts anew";
                ofLog() << "[" << ofGetTimestampString() << "]" << "updating first post";
                updatePosts(rawPosts[rawIterator]);
                rawIterator++;
                rawIterator%=rawPosts.size();
                ofLog() << "[" << ofGetTimestampString() << "]" << "updating second posts";
                updatePosts(rawPosts[rawIterator]);
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
#ifdef __APPLE__
    ofScale(0.5, 0.5);
#endif
    if(posts.size() > 0) {
        ofBackground(0);
        
        ofPushStyle();
        ofSetColor(255);
        ofSetLineWidth(1);
        ofLine(24, 1203, 1056, 1203); //headline/clock/body/info line
        ofLine(235, 1215, 235, 1475); // clock/info/date line
        ofLine(246, 1366, 425, 1366); // type/date line
        ofLine(437, 1215, 437, 1908); //body/date/info/portrait line
        ofPopStyle();
        
        int nextTitleIndex = (titleIndex+1)%posts.size();
        int nextTextIndex = (textIndex+1)%posts.size();
        int nextPortIndex = (portIndex+1)%posts.size();
        int nextTypeIndex = (typeIndex+1)%posts.size();
        
        ofEnableDepthTest();
        titlePanel.setTexs(&posts[titleIndex]["title"], &posts[nextTitleIndex]["title"]);
        titlePanel.display();
        
        textPanel.setTexs(&posts[textIndex]["text"], &posts[nextTextIndex]["text"]);
        textPanel.display();
        
        portPanel.setTexs(&posts[portIndex]["portrait"], &posts[nextPortIndex]["portrait"]);
        portPanel.display();
        
        ofDisableDepthTest();
        clock.display();
        ofEnableDepthTest();
        
        typePanel.setTexs(&posts[typeIndex]["type"], &posts[nextTypeIndex]["type"]);
        typePanel.display();
        
        datePanel.setTexs(&dateTex, &dateTex);
        datePanel.display();
        ofDisableDepthTest();
        
        //        ofPushStyle();
        //        if(iVidIsPlaying) {
        //            ofSetColor(255);
        //            informationVid.draw(panels["type"].getX(), panels["type"].getY(), panels["type"].getWidth(), panels["type"].getHeight());
        //            ofSetColor(0);
        //            ofRect(panels["type"].getX() - 1, panels["type"].getY() + panels["type"].getHeight() - 1, panels["type"].getWidth() + 2, 3);
        //        }
        //        if(eVidIsPlaying) {
        //            ofSetColor(255);
        //            eventVid.draw(panels["type"].getX(), panels["type"].getY(), panels["type"].getWidth(), panels["type"].getHeight());
        //            ofSetColor(0);
        //            ofRect(panels["type"].getX() - 1, panels["type"].getY() + panels["type"].getHeight() - 1, panels["type"].getWidth() + 2, 3);
        //        }
        //        if(hVidIsPlaying) {
        //            ofSetColor(255);
        //            housekeepingVid.draw(panels["type"].getX(), panels["type"].getY(), panels["type"].getWidth(), panels["type"].getHeight());
        //            ofSetColor(0);
        //            ofRect(panels["type"].getX() - 1, panels["type"].getY() + panels["type"].getHeight() - 1, panels["type"].getWidth() + 2, 3);
        //        }
        //        ofPopStyle();
    } else {
        ofPushStyle();
        ofSetColor(255);
        blankImage.draw(0, 0, ofGetWidth(), ofGetHeight());
        ofPopStyle();
    }
    if(bSetup) {
        ofEnableAlphaBlending();
        ofSetColor(0, 0, 0, 200);
        ofTranslate(ofGetWidth()/4, ofGetHeight()/4);
        ofRect(0, 0, ofGetWidth()/2, ofGetHeight()/2);
        ofSetColor(255);
        int lineSpacing = 10;
        int leftMargin = 10;
        int textHeight = 10;
        ofDrawBitmapString("Setup Mode: Press 'Tab' again to return to the main app", leftMargin, textHeight);
        textHeight += lineSpacing*2;
        if(bInternal) ofDrawBitmapString("This screen is currently displaying internal posts", leftMargin, textHeight);
        else ofDrawBitmapString("This screen is not currently displaying internal posts", leftMargin, textHeight);
        textHeight += lineSpacing;
        ofDrawBitmapString("Press 'i' to toggle this option", leftMargin, textHeight);
        textHeight += lineSpacing*2;
        if(bExternal) ofDrawBitmapString("This screen is currently displaying external posts", leftMargin, textHeight);
        else ofDrawBitmapString("This screen is not currently displaying external posts",leftMargin, textHeight);
        textHeight += lineSpacing;
        ofDrawBitmapString("Press 'e' to toggle this option", leftMargin, textHeight);
        textHeight += lineSpacing*2;
        ofDrawBitmapString("Currently connecting to this URL: " + url, leftMargin, textHeight);
        textHeight += lineSpacing;
        ofDrawBitmapString("Change this by editting the URL field of the settings.json file within the bin/data/ folder", leftMargin, textHeight);
        textHeight += lineSpacing*2;
        ofDrawBitmapString("The minimum time a post stays active is " + ofToString(minDuration) + " seconds", leftMargin, textHeight);
        textHeight += lineSpacing;
        ofDrawBitmapString("Press '+' to increase the minimum duration and '-' to decrease it", leftMargin, textHeight);
        textHeight += lineSpacing;
        ofDrawBitmapString("Please note the next post will only be displayed when it has loaded from the internet", leftMargin, textHeight);
        ofDisableAlphaBlending();
    }
}

//--------------------------------------------------------------
void ofApp::generatePostImages(ofApp::rawPost rawPost, map<string, ofFbo> panelBuffers) {
    
    ofEnableAlphaBlending();
    
    std::string shortCode = rawPost.shortCode;
    std::string label;
    ofLog() << "[" << ofGetTimestampString() << "]" << "Generating Images For Post "<<shortCode<<" Synchronously.";
    
    
    //<------------------------------LOAD IMAGES---------------------------->
    
    image.loadImage(rawPost.imageLink);
    portrait.loadImage(rawPost.portraitLink);
    qrCode.loadImage(rawPost.qrLink);
    logo.loadImage(rawPost.logoLink);
    
    //<------------------------------DRAW MAIN PANEL ---------------------------->
    ofLog() << "[" << ofGetTimestampString() << "]" << "Drawing Main Panel";
    
    label = "title";
    ofLog() << "[" << ofGetTimestampString() << "]" << "allocating buffer";
    //buffer.allocate(panels[label].getWidth(), panels[label].getHeight(), GL_RGBA);
    ofLog() << "[" << ofGetTimestampString() << "]" << "starting buffer";
    panelBuffers["title"].begin();
    ofLog() << "[" << ofGetTimestampString() << "]" << "clearing";
    ofClear(255, 255, 255, 0);
    ofSetColor(255);
    //image.draw(panels[label]);
    ofLog() << "[" << ofGetTimestampString() << "]" << "drawing image";
    drawZoomedImage(0,  0, image, panels[label].getWidth(), panels[label].getHeight());
    ofLog() << "[" << ofGetTimestampString() << "]" << "setting font variables";
    float fontSize = 180.0f;
    int numLines = 0;
    bool wordsWereCropped;
    ofRectangle titleBox;
    std::string title = ofToUpper(rawPost.title);
    ofLog() << "[" << ofGetTimestampString() << "]" << "building title box";
    titleBox = font.drawMultiLineColumn(    title,
                                        fontSize,
                                        MARGIN+BUFFER, MARGIN+BUFFER + fontSize/2,
                                        panels[label].getWidth() - MARGIN*3 - BUFFER*3,
                                        numLines,
                                        true,
                                        5,
                                        true,
                                        &wordsWereCropped
                                        );
    
    ofSetColor(ofColor(0), 230);
    if(titleBox.getWidth() != 0) {
        ofRect(MARGIN, MARGIN, titleBox.getWidth() + 2*BUFFER, titleBox.getHeight() + 2*BUFFER);
        ofSetColor(255);
        font.drawMultiLineColumn(   title,
	                                180.0f,
	                                MARGIN+BUFFER, MARGIN+2*BUFFER + 180.0f/2,
	                                panels[label].getWidth() - MARGIN - BUFFER,
	                                numLines,
	                                false,
	                                4,
	                                true,
	                                &wordsWereCropped
                                 );
    }
    ofLog() << "[" << ofGetTimestampString() << "]" << "closing buffer";
    panelBuffers["title"].end();
    ofLog() << "[" << ofGetTimestampString() << "]" << "setting returnMap to the texture reference";
    returnMap[label] = panelBuffers["title"].getTextureReference(0);
    
    //<------------------------------DRAW TEXT PANEL ---------------------------->
    ofLog() << "[" << ofGetTimestampString() << "]" << "Drawing Text Panel";
    
    label = "text";
    //buffer.allocate(panels[label].getWidth(), panels[label].getHeight(), GL_RGBA);
    panelBuffers["text"].begin();
    ofClear(255, 255, 255, 0);
    ofSetColor(0);
    ofRect(0, 0, panels[label].getWidth(), panels[label].getHeight());
    ofSetColor(255);
    
    fontSize = 40.0f;
    numLines = 0;
    std::string text = rawPost.text;
    ofRectangle textBox;
    textBox = font.drawMultiLineColumn( text,
                                       fontSize,
                                       BUFFER, 2*BUFFER + fontSize/2,
                                       panels[label].getWidth() - 2*BUFFER,
                                       numLines,
                                       false,
                                       11,
                                       true,
                                       &wordsWereCropped
                                       );
    
    
    ofRectangle locBox;
    std::string loc = "Location: " + rawPost.venue;
    locBox = font.drawMultiLineColumn(  loc,
                                      fontSize,
                                      BUFFER, panels[label].getHeight() - BUFFER*2 - QRSIZE,
                                      panels[label].getWidth() - 2*BUFFER,
                                      numLines,
                                      false,
                                      3,
                                      true,
                                      &wordsWereCropped
                                      );
    
    vector<string> dateVec = ofSplitString(rawPost.date, "-");
    int dateInt = ofToInt(dateVec[1]);
    std::string month = intToMonth(dateInt);
    if(rawPost.type == "Event") {
        std::string date = "Event Date: " + month + " " + appendOrdinal(ofToInt(dateVec[2]));
        ofRectangle dateBox;
        dateBox = font.drawMultiLineColumn( date,
                                           fontSize,
                                           BUFFER, panels[label].getHeight() - BUFFER*3 - QRSIZE - locBox.getHeight(),
                                           panels[label].getWidth() - 2*BUFFER,
                                           numLines,
                                           false,
                                           1,
                                           true,
                                           &wordsWereCropped
                                           );
    }
    
    fontSize = 25.0f;
    ofRectangle contactBox;
    std::string contact = rawPost.email;
    contactBox = font.drawMultiLineColumn(  contact,
                                          fontSize,
                                          BUFFER, panels[label].getHeight() - QRSIZE/2,
                                          panels[label].getWidth() - 4*BUFFER - 2*QRSIZE,
                                          numLines,
                                          false,
                                          1,
                                          true,
                                          &wordsWereCropped
                                          );
    
    ofRectangle urlBox;
    urlBox = font.drawMultiLineColumn(  url,
                                      fontSize,
                                      BUFFER, panels[label].getHeight() - QRSIZE/2 + contactBox.getHeight() + BUFFER,
                                      panels[label].getWidth() - 4*BUFFER - 2*QRSIZE,
                                      numLines,
                                      false,
                                      1,
                                      true,
                                      &wordsWereCropped
                                      );
    
    qrCode.draw(panels[label].getWidth() - QRSIZE - BUFFER, panels[label].getHeight() - QRSIZE, QRSIZE, QRSIZE);
    drawSquareImage(panels[label].getWidth() - QRSIZE - QRSIZE - 2*BUFFER,  panels[label].getHeight() - QRSIZE, logo, QRSIZE);
    
    panelBuffers["text"].end();
    returnMap[label] = panelBuffers["text"].getTextureReference();
    
    //<------------------------------DRAW PORTRAIT PANEL ---------------------------->
    ofLog() << "[" << ofGetTimestampString() << "]" << "Drawing Portrait Panel";
    
    label = "portrait";
    //buffer.allocate(panels[label].getWidth(), panels[label].getHeight(), GL_RGBA);
    panelBuffers["portrait"].begin();
    ofClear(255, 255, 255, 0);
    ofSetColor(255);
    //    portrait.draw(0, 0, panels[label].getWidth(), panels[label].getHeight());
    drawZoomedImage(0,  0, portrait, panels[label].getWidth(), panels[label].getHeight());
    panelBuffers["portrait"].end();
    returnMap[label] = panelBuffers["portrait"].getTextureReference();
    
    
    //<------------------------------DRAW TYPE PANEL ---------------------------->
    ofLog() << "[" << ofGetTimestampString() << "]" << "Drawing Type Panel";
    
    label = "type";
    //buffer.allocate(panels[label].getWidth(), panels[label].getHeight(), GL_RGBA);
    panelBuffers["type"].begin();
    ofClear(255, 255, 255, 0);
    ofSetColor(255);
    if(rawPost.type == "Housekeeping") housekeepingImage.draw(0, 0, panels[label].getWidth(), panels[label].getHeight());
    if(rawPost.type == "Event") eventImage.draw(0, 0, panels[label].getWidth(), panels[label].getHeight());
    if(rawPost.type == "Information") informationImage.draw(0, 0, panels[label].getWidth(), panels[label].getHeight());
    panelBuffers["type"].end();
    returnMap[label] = panelBuffers["type"].getTextureReference();
    
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void ofApp::updatePosts(ofApp::rawPost newRawPost) {
    ofDisableDepthTest();
    
    if(posts.size()>1) posts.pop_front();
    generatePostImages(newRawPost, panelBuffers[panelBufferIterator]);
    panelBufferIterator++;
    panelBufferIterator%=numPanelBuffers;
    posts.push_back(returnMap);
    nextPanelReady = true;
    
    titleIndex = 0;
    textIndex = 0;
    portIndex = 0;
    typeIndex = 0;
    dateIndex = 0;
    
    ofEnableDepthTest();
}

//--------------------------------------------------------------
vector<ofApp::rawPost> ofApp::fetchRawPosts(std::string newURL) {
    ofLog() << "[" << ofGetTimestampString() << "]" << "fetch raw posts called";
    vector<rawPost> outputPosts;
    bool parsingSuccessful = json.open(newURL + "/data/data.json");
    
    if(parsingSuccessful) {
        //        ofLogNotice("ofApp::fetchRawPosts") << json.getRawString(true);
        for(unsigned int i = 0; i < json["posts"].size(); i++) {
            if(json["posts"][i]["active"].asBool()) {
                if((bInternal && bExternal) && (json["posts"][i]["internal"].asBool() || json["posts"][i]["external"].asBool())) {
                    rawPost tmpPost = {
                        json["posts"][i]["headlineText"].asString(),
                        json["posts"][i]["bodyText"].asString(),
                        json["posts"][i]["email"].asString(),
                        json["posts"][i]["postType"].asString(),
                        url + json["posts"][i]["imagePath"].asString() + "?dim=1080",
                        url + json["posts"][i]["portraitPath"].asString() + "?dim=400",
                        url + json["posts"][i]["logoPath"].asString() + "?dim=120",
                        url + "/data/posts/" + json["posts"][i]["shortCode"].asString() + "/" + "qr.png",
                        json["posts"][i]["venue"].asString(),
                        json["posts"][i]["date"].asString(),
                        json["posts"][i]["shortCode"].asString()
                    };
                    outputPosts.push_back(tmpPost);
                } else if(bInternal && json["posts"][i]["internal"].asBool()) {
                    rawPost tmpPost = {
                        json["posts"][i]["headlineText"].asString(),
                        json["posts"][i]["bodyText"].asString(),
                        json["posts"][i]["email"].asString(),
                        json["posts"][i]["postType"].asString(),
                        url + json["posts"][i]["imagePath"].asString() + "?dim=1080",
                        url + json["posts"][i]["portraitPath"].asString() + "?dim=400",
                        url + json["posts"][i]["logoPath"].asString()+ "?dim=120",
                        url + "/data/posts/" + json["posts"][i]["shortCode"].asString() + "/" + "qr.png",
                        json["posts"][i]["venue"].asString(),
                        json["posts"][i]["date"].asString(),
                        json["posts"][i]["shortCode"].asString()
                    };
                    outputPosts.push_back(tmpPost);
                } else if(bExternal && json["posts"][i]["external"].asBool()) {
                    rawPost tmpPost = {
                        json["posts"][i]["headlineText"].asString(),
                        json["posts"][i]["bodyText"].asString(),
                        json["posts"][i]["email"].asString(),
                        json["posts"][i]["postType"].asString(),
                        url + json["posts"][i]["imagePath"].asString() + "?dim=1080",
                        url + json["posts"][i]["portraitPath"].asString() + "?dim=400",
                        url + json["posts"][i]["logoPath"].asString() + "?dim=120",
                        url + "/data/posts/" + json["posts"][i]["shortCode"].asString() + "/" + "qr.png",
                        json["posts"][i]["venue"].asString(),
                        json["posts"][i]["date"].asString(),
                        json["posts"][i]["shortCode"].asString()
                    };
                    outputPosts.push_back(tmpPost);
                }
            }
        }
    }
    else {
        ofLogNotice("ofApp::fetchRawPosts()") << "failed to parse JSON";
    }
    return outputPosts;
}

//--------------------------------------------------------------
std::string ofApp::appendOrdinal(int num) {
    std::string suff;
    int ones = num % 10;
    int tens = (int)(num / 10) % 10;
    if(tens == 1) {
        suff = "th";
    } else {
        switch (ones) {
            case 1:
                suff = "st";
                break;
            case 2:
                suff = "nd";
                break;
            case 3:
                suff = "rd";
                break;
            default:
                suff = "th";
                break;
        }
    }
    return ofToString(num) + suff;
}

//--------------------------------------------------------------
std::string ofApp::intToMonth(int num) {
    switch (num) {
        case 1:
            return "January";
            break;
        case 2:
            return "February";
            break;
        case 3:
            return "March";
            break;
        case 4:
            return "April";
            break;
        case 5:
            return "May";
            break;
        case 6:
            return "June";
            break;
        case 7:
            return "July";
            break;
        case 8:
            return "August";
            break;
        case 9:
            return "September";
            break;
        case 10:
            return "October";
            break;
        case 11:
            return "November";
            break;
        case 12:
            return "December";
            break;
        default:
            return "input not a date";
            break;
    }
}

//--------------------------------------------------------------
ofTexture ofApp::renderDate(){
    ofTexture returnTex;
    returnTex.allocate(panels["date"].getWidth(), panels["date"].getHeight(), GL_RGB);
    
    vector<string> textLines;
    ofRectangle textBounds;
    
    std::string label;
    ofDisableDepthTest();
    
    //<------------------------------DRAW DATE PANEL ---------------------------->
    label = "date";
    ofFbo buffer;
    buffer = ofFbo();
    buffer.allocate(panels["date"].getWidth(), panels["title"].getHeight(), GL_RGBA);
    buffer.begin();
    ofClear(255, 255, 255, 0);
    ofSetColor(0, 0, 0);
    ofRect(0, 0, panels[label].getWidth(), panels[label].getHeight());
    ofSetColor(255);
    ofRectangle dateRect = ofRectangle(0, 0, panels[label].getWidth(), panels[label].getHeight());
    todayText.setText(ofGetTimestampString("%d"));
    todayText.setColor(255, 255, 255, 255);
    todayText.setLineHeight(0.0);
    todayText.draw(10, 20 + dateRect.getHeight()/2);
    todayText.setText("/");
    todayText.setColor(255, 255, 255, 255);
    todayText.setLineHeight(0.0);
    todayText.draw(80, 20 + dateRect.getHeight()/2);
    todayText.setText(ofGetTimestampString("%m"));
    todayText.setColor(255, 255, 255, 255);
    todayText.setLineHeight(0.0);
    todayText.draw(102, 20 + dateRect.getHeight()/2);
    buffer.end();
    returnTex = buffer.getTextureReference();
    ofEnableDepthTest();
    
    return returnTex;
}

//--------------------------------------------------------------
void ofApp::drawSquareImage(float x, float y, ofImage img, int targetWidth) {
    float width = img.getWidth();
    float height = img.getHeight();
    
    float maxDim = ( width >= height ) ? width : height;
    float scale = ( (float) targetWidth ) / maxDim;
    if(width >= height ) {
        img.draw(x, y + (targetWidth - height*scale)/2, targetWidth, height * scale);
    } else {
        img.draw(x + (targetWidth - width*scale)/2, y, width * scale, targetWidth);
    }
}

//--------------------------------------------------------------
void ofApp::drawZoomedImage(float x, float y, ofImage img, int targetWidth, int targetHeight) {
    float width = img.getWidth();
    float height = img.getHeight();\
    float scale = (width >= height) ? ( (float) targetHeight ) / height : ( (float) targetWidth ) / width ;
    if(width >= height ) {
        img.draw(x + (targetWidth - width*scale)/2, y, width * scale, targetHeight);
        //img.draw(x, y /*+ (targetHeight - height*scale)/2*/, targetWidth, height * scale);
    } else {
        img.draw(x, y + (targetHeight - height*scale)/2, targetWidth, height * scale);
        //img.draw(x + (targetWidth - width*scale)/2, y, width * scale, targetWidth);
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == OF_KEY_TAB) {
        bSetup = !bSetup;
    }
    if(bSetup) {
        if(key == 'i') {
            bInternal = !bInternal;
            settings["Internal"] = bInternal;
            settings.save("settings.json", false);
        }
        if(key == 'e'){
            bExternal = !bExternal;
            settings["External"] = bExternal;
            settings.save("settings.json", false);
        }
        if(key == '+') {
            minDuration++;
            minDuration = min(600, minDuration);
            settings["minDuration"] = minDuration;
            settings.save("settings.json", false);
        }
        if(key == '-') {
            minDuration--;
            minDuration = max(0, minDuration);
            settings["minDuration"] = minDuration;
            settings.save("settings.json", false);
        }
    }
}


