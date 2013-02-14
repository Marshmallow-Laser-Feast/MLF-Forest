#pragma once


#include "Scale.h"

class ScaleManager {
public:
    
    // statics
    int currentIndex;
    vector<Scale> scales;
    vector<string> scaleNames;
    
    //--------------------------------------------------------------
    void add(string s, float* f, int l) {
        scales.push_back(Scale(s, f, l));
    }
    
    //--------------------------------------------------------------
    void add(string s, int* f, int l) {
        scales.push_back(Scale(s, f, l));
    }
    
    //--------------------------------------------------------------
    Scale& currentScale() {
        return scales[currentIndex];
    }
    
    //--------------------------------------------------------------
    float currentFreq(int noteIndex) {
//        Scale &scale = currentScale();
//        return scale.freqs[noteIndex % scale.len()];
        Scale &scale = currentScale();
        float octave = 1;
        while(noteIndex >= scale.len()) {
            noteIndex -= scale.len();
            octave *= 2;//2 * scale.numOctaves;//pow(2.0f, scale.numOctaves);
        }
        return scale.freqs[noteIndex] * octave;
    }
    
    //--------------------------------------------------------------
    float currentMult(int noteIndex) {
        Scale &scale = currentScale();
//        float mult = 1;
//        while(noteIndex >= scale.len()) {
//            noteIndex -= scale.len();
//            mult *= 2;//2 * scale.numOctaves;//pow(2.0f, scale.numOctaves);
//        }
//        mult *= scale.freqs[noteIndex] / scale.freqs[0];
//        return mult;
        return currentFreq(noteIndex) / scale.freqs[0];
    }
    
    //--------------------------------------------------------------
    int currentMidiIndex(int noteIndex, int rootNote=0, int maxOctaves=0) {
        Scale &scale = currentScale();
        
        int midiIndex   = rootNote + currentFreq(noteIndex) + floor(noteIndex / scale.len()) * scale.numOctaves * 12;
        
        //		midiNoteNum				= (octave * 12 + scales[currentScaleIndex].intervals[noteInOctave]);
        return midiIndex;
    }
    
    
    
    //--------------------------------------------------------------
    void setup() {
        scaleNames.push_back("hicaz kar");
        scaleNames.push_back("Gong-Diao/Pentatonic");
        scaleNames.push_back("Zhi-Diao/Pentatonic");
        scaleNames.push_back("iwato");
        scaleNames.push_back("dorian");
        scaleNames.push_back("chromatic");
        scaleNames.push_back("weird");
        
#ifdef USE_SAMPLE
        static float scale1[] = { 220.000, 293.664, 311.126, 369.994, 391.995, 440.000, 466.163, 554.365, 587.329 }; // hicaz kar
        static float scale2[] = { 195.997, 261.625, 293.664, 329.627, 391.995, 440.000, 523.251, 587.329, 659.255 }; // Gong-Diao/Pentatonic
        static float scale3[] = { 174.614, 261.625, 293.665, 349.228, 391.995, 440.000, 523.251, 587.329, 698.456 }; // Zhi-Diao/Pentatonic
        static float scale4[] = { 174.614, 183.942, 232.571, 261.625, 277.182, 349.228, 369.994, 466.163, 523.251, 554.365 }; // iwato
        static float scale5[] = { 195.997, 261.625, 293.664, 311.126, 349.228, 391.995, 440.223, 466.163, 523.251 }; //dorian
        static float scale6[] = { 195.998, 207.652, 220.000, 233.082, 246.942, 261.626, 277.183, 293.665, 311.127, 329.628, 349.228 }; // chromatic
        static float scale7[] = { 200, 200+200*1/8, 200+200*1/4, 200+200*1/2 }; // weird
        
        add("weird", scale7, 4);
        add("chromatic", scale6, 11);
        add("dorian", scale5, 9);
        add("iwato", scale4, 10);
        add("Gong-Diao/Pentatonic", scale2, 9);
        add("Zhi-Diao/Pentatonic", scale3, 9);
        add("hicaz kar", scale1, 9);
#endif
        
        
#ifdef USE_MIDI
        midiOut.listPorts();
        midiOut.openPort();
        
        static int iiPentatonic[]		= {	0, 3, 5, 7, 10 };
        static int iiChromatic[]        = {	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
        static int iiDiminished[]		= {	0, 3, 6, 9 };							// diminished
        static int iiPentaChromatic[]	= {	0, 3, 5, 6, 7, 10, 11};					// Pentatonic + chromatics
        static int iiMajor[]			= {	0, 2, 4, 5, 7, 9, 11 };						// Major
        static int iiMinor[]			= {	0, 2, 3, 5, 7, 8, 10 };						// Minor
        static int iiZHicaz[]			= {	0, 1, 4, 5, 7, 8, 11   };                     // Zirguleli Hicaz
        
        add("iiPentatonic", iiPentatonic, 5);
        add("iiChromatic", iiChromatic, 12);
        add("iiDiminished", iiDiminished, 4);
        add("iiPentaChromatic", iiPentaChromatic, 7);
        add("iiMajor", iiMajor, 7);
        add("iiMinor", iiMinor, 7);
        add("iiZHicaz", iiZHicaz, 7);
#endif
    }
    
    
    //--------------------------------------------------------------
    void play(ofSoundPlayer &sound, int sampleIndex) {
#ifdef USE_SAMPLE
        float speedMult = currentMult(sampleIndex) * 0.4;
        //                float speedMult = pow(2, (balls.size()-b.index) * 1.0f/12.0f * 2) * 0.5;
        //                float speedMult = ofMap(b.index, 0, balls.size()-1, 0.5, 2);
        sound.setSpeed(speedMult);
        sound.setVolume(ofRandom(0.5, 1) * ofMap(speedMult, 0, 4, 1, 0.75, true));
        sound.play();
        //            printf("SpeedMult: %f\n", speedMult);
#endif
        
#ifdef USE_MIDI
        int rootNote = 20;
        int soundIndex = currentMidiIndex(i);
        soundIndex = (soundIndex % (8*12)) + rootNote;
        //            printf("soundIndex: %i\n", soundIndex);
        //            midiOut.sendNoteOff(1, i, 0);
        midiOut.sendNoteOn(1, soundIndex, ofRandom(30, 100));//ofMap(i, 0, count, 127, 50));
        midiTimers[soundIndex] = kMidiHoldSeconds * fps;
#endif
    }

};