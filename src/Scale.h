//  _  _   __   ____  ____  _  _  _  _   __   __    __     __   _  _    __     __   ____  ____  ____    ____  ____   __   ____  ____
// ( \/ ) / _\ (  _ \/ ___)/ )( \( \/ ) / _\ (  )  (  )   /  \ / )( \  (  )   / _\ / ___)(  __)(  _ \  (  __)(  __) / _\ / ___)(_  _)
// / \/ \/    \ )   /\___ \) __ (/ \/ \/    \/ (_/\/ (_/\(  O )\ /\ /  / (_/\/    \\___ \ ) _)  )   /   ) _)  ) _) /    \\___ \  )(
// \_)(_/\_/\_/(__\_)(____/\_)(_/\_)(_/\_/\_/\____/\____/ \__/ (_/\_)  \____/\_/\_/(____/(____)(__\_)  (__)  (____)\_/\_/(____/ (__)
//
//
//  Created by Memo Akten, www.memo.tv
//  Marshmallow Laser Feast
//  Forest
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