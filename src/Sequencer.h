class Sequencer {
public:

	class SequencerEvent {
	public:
		string presetName;
		float duration; // in seconds

		SequencerEvent(string presetName = "", float duration = 0) {
			this->presetName = presetName;
			this->duration = duration;
		}
	};
    class Button: public ofRectangle {
    public:
        
        bool down = false;
        bool over = false;
        
        string text = "*";
        void draw() {
            ofSetColor(50);
            ofDrawRectangle(*this);
            ofSetColor(255);
            ofDrawBitmapString(text, x, getBottom() - 10);
        }
        
        void enableEvents() {
            ofAddListener(ofEvents().mouseMoved, this, &Button::mouseMoved);
            ofAddListener(ofEvents().mousePressed, this, &Button::mousePressed);
            ofAddListener(ofEvents().mouseDragged, this, &Button::mouseDragged);
            ofAddListener(ofEvents().mouseReleased, this, &Button::mouseReleased);
        }
        
        void mouseMoved(ofMouseEventArgs &m) {
            over = inside(m);
            down = false;
        }
        
        void mousePressed(ofMouseEventArgs &m) {
            down = inside(m);
        }
        
        void mouseDragged(ofMouseEventArgs &m) {
            down = inside(m);
        }
        
        void mouseReleased(ofMouseEventArgs &m) {
            down = false;
            if(inside(m)) {
                if(action) {
                    action();
                }
            }
        }
        function<void()> action;
    };
    
    
    Sequencer() {
        playButton.text = " || ";
        nextButton.text = " >> ";
        prevButton.text = " << ";
    }
    Button playButton;
    Button nextButton;
    Button prevButton;
    
    ofRectangle rect;
    ofRectangle textRect;
    void draw(ofEventArgs &args) {
        
        float h = 30;
        int padding = 4;
        rect.width = 460;
        rect.height = h + padding * 2;
        
        rect.x = ofGetWidth() - rect.width;
        rect.y = ofGetHeight() - rect.height;
        
        textRect = rect;
        textRect.x += padding;
        textRect.y += padding;
        textRect.height -= padding * 2;
        textRect.width = rect.width - padding * 2 - (h+padding) * 3;
        
        playButton.width = playButton.height = prevButton.width = prevButton.height = nextButton.width = nextButton.height = h;
        prevButton.x = textRect.getRight() + padding;
        playButton.x = prevButton.getRight() + padding;
        nextButton.x = playButton.getRight() + padding;
        prevButton.y = playButton.y = nextButton.y = textRect.y;
        
        ofSetColor(0);
        ofDrawRectangle(rect);
        
        
        ofSetColor(50);
        ofDrawRectangle(textRect);
    
        ofSetColor(100);
        auto progressRect = textRect;
        progressRect.width *= getProgress();
        ofDrawRectangle(progressRect);
    
        ofSetColor(255);
        ofDrawBitmapString(statusString(), textRect.x + 3, textRect.getBottom() - 10);
        
        playButton.draw();
        prevButton.draw();
        nextButton.draw();
        
        // play/pause button
        // current preset
        // prev / next
    }
    
    float getProgress() {
        return timeElapsedCurrPreset / sequence[currPreset].duration;
    }
    
    string timeStr() {
        char a [64];
        
        float t = getTimeLeftForCurrPreset();
        float fraction = (t - (int)t)*100.f;
        int ds = fraction;
        int s = ((int) t) % 60;
        int m = ((int) t) / 60;
        int h = ((int) t) / 3600;
        sprintf(a, "%02d:%02d:%02d:%02d", h, m, s, ds);
        //sprintf(a, "%05.2f", getTimeLeftForCurrPreset());
        
        return string(a);
    }
    string statusString() {
        string paused = "";
        if(!playing) {
            paused = " (paused)";
        }
        
        return getCurrPresetName() + ": -"+timeStr() + paused;
    }


	vector<SequencerEvent> sequence;
    int currPreset = -1;
    float timeElapsedCurrPreset = 0;
    bool playing = false;
    
    
	void setPlaying(bool playing) {
        this->playing = playing;
        
        if(!playing) {
            playButton.text = " |> ";
        } else {
            playButton.text = " || ";
            
        }
        
        save();
	}
    float lastTime = 0;
    
    string getCurrPresetName() {
        return sequence[currPreset].presetName;
    }
    float getTimeLeftForCurrPreset() {
        return sequence[currPreset].duration - timeElapsedCurrPreset;
    }
    void update(ofEventArgs &args) {
        float currTime = ofGetElapsedTimef();
        float delta = currTime - lastTime;
        
        
        update(delta);
        
        
        lastTime = currTime;
    }
    void update(float delta) {
        if(playing) {
            if(currPreset==-1) {
                next();
            }
            // advance playhead
            timeElapsedCurrPreset += delta;
            if(sequence[currPreset].duration<timeElapsedCurrPreset) {
                next();
            }
        }
    }
    
    void next() {
        currPreset = (currPreset + 1) % sequence.size();
        triggerPreset();
    }
    void prev() {
        currPreset--;
        if(currPreset<0) currPreset += sequence.size();
        triggerPreset();
    }
    void playPause() {
        
        playing = !playing;
        setPlaying(playing);
    }
    
    void save() {
        // save the preset and save whether playing
        ofXml xml;
        xml.load(path);
        
        ofXml seq = xml.findFirst("sequence");
        
        seq.setAttribute("playing", playing);
        seq.setAttribute("currPreset", currPreset);
        
        ofLogNotice() << xml.toString();
        ofstream myfile;
        myfile.open (ofToDataPath(path));
        myfile << xml.toString();
        myfile.close();

        
        
        //xml.save(path);
        
    }
    
    
    void triggerPreset() {
        timeElapsedCurrPreset = 0;
        // callback here
        if(loadPresetCallback) {
            if(currPreset>=sequence.size() || currPreset<0) currPreset = 0;
            loadPresetCallback(sequence[currPreset].presetName);
        }
        save();
    }
    
    function<void(string)> loadPresetCallback;
    
    bool addedListener = false;
    string path;
	void load(string path, function<void(string)> loadPresetCallback) {
        this->path = path;
        if(!addedListener) {
            ofAddListener(ofEvents().update, this, &Sequencer::update);
            ofAddListener(ofEvents().draw, this, &Sequencer::draw);
            
            playButton.enableEvents();
            nextButton.enableEvents();
            prevButton.enableEvents();
            
            addedListener = true;
        }
        
        nextButton.action = bind(&Sequencer::next, this);
        prevButton.action = bind(&Sequencer::prev, this);
        playButton.action = bind(&Sequencer::playPause, this);
        
        this->loadPresetCallback = loadPresetCallback;
		ofXml xml;
		sequence.clear();

		xml.load(path);

        auto seq = xml.getFirstChild();
        
        playing = seq.getAttribute("playing").getBoolValue();
        currPreset = seq.getAttribute("currPreset").getIntValue();
        
        
        
        auto events = seq.getChildren("event");
        for(auto &event: events) {
            sequence.emplace_back(
                                  event.getAttribute("presetName").getValue(),
                                  event.getAttribute("durationMinutes").getFloatValue() * 60.f
            );
        }
        triggerPreset();
        print();
	}
    
    void print() {
        printf("--------------------------------------\nSEQUENCE: \n");
        for(int i = 0; i < sequence.size(); i++) {
            printf("[%d]  %s (%.0f seconds)\n", i, sequence[i].presetName.c_str(), sequence[i].duration);
        }
        printf("--------------------------------------\n");
    }
};

