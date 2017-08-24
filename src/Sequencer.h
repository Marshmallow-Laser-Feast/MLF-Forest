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
	


	vector<SequencerEvent> sequence;
    int currPreset = -1;
    float timeElapsedCurrPreset = 0;
    bool playing = false;
    
    
	void setPlaying(bool playing) {
        this->playing = playing;
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
                advance();
            }
            // advance playhead
            timeElapsedCurrPreset += delta;
            if(sequence[currPreset].duration<timeElapsedCurrPreset) {
                advance();
            }
        }
    }
    
    void advance() {
        currPreset = (currPreset + 1) % sequence.size();
        triggerPreset();
    }
    
    void triggerPreset() {
        timeElapsedCurrPreset = 0;
        // callback here
        if(loadPresetCallback) {
            loadPresetCallback(sequence[currPreset].presetName);
        }
    }
    
    function<void(string)> loadPresetCallback;
    
    bool addedListener = false;
    
	void load(string path, function<void(string)> loadPresetCallback) {
        if(!addedListener) {
            ofAddListener(ofEvents().update, this, &Sequencer::update);
            addedListener = true;
        }
        this->loadPresetCallback = loadPresetCallback;
		ofXml xml;
		sequence.clear();

		xml.load(path);

        auto seq = xml.getFirstChild();
        auto events = seq.getChildren("event");
        for(auto &event: events) {
            sequence.emplace_back(
                                  event.getAttribute("presetName").getValue(),
                                  event.getAttribute("durationMinutes").getFloatValue() * 60.f
            );
        }
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

