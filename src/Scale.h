//
//  Scale.h
//  shm05
//
//  Created by Memo Akten on 23/08/2011.
//  Copyright 2011 MSA Visuals Ltd. All rights reserved.
//

#pragma once

#include "ofMain.h"

#define USE_SAMPLE
//#define USE_MIDI


class Scale {
public:
    vector<float> freqs;    // contains either direct frequences, or midi note indices
    string name;
    int numOctaves;        // how many octaves set in the scale?
    int len() { return freqs.size(); }
    
    
    //--------------------------------------------------------------
    // init with frequencies
    Scale(string s, float* f, int l) {
        name = s;
        for(int i=0; i<l; i++) {
            freqs.push_back(f[i]);
        }
        
//        // TODO: hack? adding same notes in reverse for smooth transition
//        for(int i=l-1; i>0; i--) {
//            freqs.push_back(f[i]);
//        }

        numOctaves = ceil(freqs[len()-1] / freqs[0]);
        printf("*** numOctaves for %s is %i\n", s.c_str(), numOctaves);
    }
    
    //--------------------------------------------------------------
    // init with midi note indices
    Scale(string s, int* f, int l) {
        name = s;
        for(int i=0; i<l; i++) {
            freqs.push_back(f[i]);
        }
        
//        // TODO: hack? adding same notes in reverse for smooth transition
//        for(int i=l-1; i>0; i--) {
//            freqs.push_back(f[i]);
//        }

        numOctaves = f[len()-1] / 12 + 1;
        printf("*** numOctaves for %s is %i\n", s.c_str(), numOctaves);
    }
};