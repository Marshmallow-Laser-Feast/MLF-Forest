// Forest project - SC code

/*
NOTE: This can receive the following OSC messages (type & ranges in brackets):
Message: '/forestAmp', rodNo (int 0-149), value (float 0.0-1.0)
Message: '/forestFreq', rodNo (int 0-149), value (float 0.0-20000.0)
Message: '/forestPos', rodNo (int 0-149), value (float 0.0-1.0)
Message: '/forestCentre', rodNo (int 0-149), value (float 0.0-1.0)

e.g message: ['/forestAmp", 15, 0.33] means set the amp of rod 15 to 0.33
*/


/*
NOTE
new synthesis fields to add:
	amplitudeVariation, // perlin
	frequencyVariation,

*/


(
	// initialise
	~rodCount = 150;
	~globalVolumeInit = 0.25;
	~windowTitle = "Forest - stereo sine waves";

	// sound functions - add new rows at end to create more sound options (see template row)
	~arrSoundFuncs = [
		["Sine + octave", {arg freq, shape; (1 - shape) * SinOsc.ar(freq, 0) + (shape * SinOsc.ar(freq * 2, 0));}],
		["Sine + wobble", {arg freq, shape; Mix.new(SinOsc.ar([freq, freq * (1 + (shape * 0.01))], 0));}],
		["Dust Ringz + density", {arg freq, shape;  Ringz.ar(Dust.ar(0.5 + (shape * 7), 0.5), freq, 2) }],
		["Sine Ringz + wobble", {arg freq, shape;  Ringz.ar(SinOsc.ar([freq, freq * (1 + (shape * 0.01))] * 0.6, 0, 0.03), freq, 0.3) }],
		["Klank Impulses + speed", {arg freq, shape;
			Klank.ar(`[[freq, freq * 3.355, 5.765, freq * 8.615], nil, [1, 1, 1, 1]], Impulse.ar((LFBrownNoise2.kr(0.1) * (1 + (shape * 20))), 0, 0.5)) }],
		["Klank noise 1", {arg freq, shape;  Klank.ar(`[[freq, freq * 3.355, 5.765, freq * 8.615],
			nil, [1, 1, 1, 1]], PinkNoise.ar(0.07)) }],
		["Klank noise 2", {arg freq, shape;  Klank.ar(`[ (1..13)*freq, 1/(1..13), nil ], PinkNoise.ar(0.07)) }],
		["Klank noise 3", {arg freq, shape;  Klank.ar(`[ (1,3..13)*freq, 1/(1,3..13), nil ], PinkNoise.ar(0.07)) }],
		["Klank noise 4", {arg freq, shape;
			Klank.ar(`[ freq/440 *[ 445, 2156.52, 1501.12, 2882.94, 946.15, 879.51, 418.82, 1445.69, 1303.3, 1455.97, 406.44, 1223.28 ],
				nil, nil ], PinkNoise.ar(0.07)) }],
		["Klank noise 5", {arg freq, shape;
			Klank.ar(`[ freq/440 *[ 886.91, 1297.96, 431.38, 2197.68, 1058.26, 364.2, 1308.45, 1880.13, 1466.36, 685.48, 1212.76, 1426.61 ],
				nil, nil ], PinkNoise.ar(0.07)) }],
		["Klank noise 6", {arg freq, shape;
			Klank.ar(`[ freq/440 *[ 234.24, 237.11, 2930.37, 581.05, 3265.09, 3064.39, 3465.8, 508.14, 3761.15, 210.13, 301.71, 2586.58 ],
				nil, nil ], PinkNoise.ar(0.07)) }],
		["Pulse + width", {arg freq, shape; Pulse.ar(freq, 0.01 + (shape * 0.48))}],
		["Sine -> Sawtooth + mix", {arg freq, shape; (((1-shape) * SinOsc.ar(freq)) + (shape * Saw.ar(freq)));}],
		["Sine -> Square + mix", {arg freq, shape;  (((1-shape) * SinOsc.ar(freq)) + (shape * Pulse.ar(freq, 0.5))); }],
		["Sawtooth -> Square wave + mix", {arg freq, shape; (((1-shape) * Saw.ar(freq)) + (shape * Pulse.ar(freq, 0.5))); }],
		["FM 1 + depth", {arg freq, shape; 1/3 * Mix.ar(PMOsc.ar(freq, freq*[2, 1.5], [0.1 + (shape*pi),0.1 + (shape*pi)], 0)); }],
		["FM 2 + depth", {arg freq, shape; 1/3 * Mix.ar(PMOsc.ar(freq, freq*[1.5, 3.5], [0.1 + (shape*pi),0.1 + (shape*pi)], 0)); }],
		["FM 3 + depth", {arg freq, shape; 1/3 * Mix.ar(PMOsc.ar(freq, freq*[1.0001, 1.4999], [0.1 + (shape*pi),0.1 + (shape*pi)], 0)); }],
		["FM 4 + depth", {arg freq, shape; 1/3 * Mix.ar(PMOsc.ar(freq, freq*[1.0001, 1.9999], [0.1 + (shape*pi),0.1 + (shape*pi)], 0)); }],
		["FM 5 + depth", {arg freq, shape; 1/3 * PMOsc.ar(freq, freq * 1.5, 0.1 + (shape*pi), SinOsc.ar(freq * 3.5, 0, 0.1 + (shape*pi))); }],

		// templates for adding new functions:
		//	["Pulse + width", {arg freq, shape; Pulse.ar(freq, 0.01 + (shape * 0.48))}],
	];


	// start server
	s.boot;
	//GUI.qt;
	~synthsRunning = false;
	~globalVolumeBus = Bus.control(s, 1);
	~globalSoundShapeBus = Bus.control(s, 2);

	"".postln;
	("===  Listening to OSC messages on port: " ++ NetAddr.langPort).postln;
	"".postln;

	// for testing
	~loopBackNetAddr = NetAddr("127.0.0.1", NetAddr.langPort); // loopback

	// ~freqs = {(50 + 60.rand).midicps} ! ~rodCount;
	~freqs = {(48 + [0,12,24].choose + [0, 7, 12, 0,4,7,9,10].choose).midicps} ! ~rodCount;

	~positions = ~rodCount.collect({ arg i; (i / ~rodCount); });

	~centres = ~rodCount.collect({ arg i; 0; });

	~amps = ~rodCount.collect({ arg i; 0; });

	~soundSelectionIndex = 0;

	// define functions

	~funcCreateSynthDef = {
		SynthDef(\forest, { |outbus = 0, freq = 440, amp = 0.0, pos = 0.5, centre = 0.0|
			var oscillator, panOut;
			var globalVol = ~globalVolumeBus.kr * 0.1;
			var globalShape = ~globalSoundShapeBus.kr;
			// oscillator =  SinOsc.ar(freq, 0, globalVol);
			oscillator =  ~arrSoundFuncs[~soundSelectionIndex][1].value(freq, globalShape);
			panOut = PanAz.ar(2, oscillator, pos-0.5, amp * globalVol);
		    Out.ar(outbus, panOut);
		}).add;
	};

	~funcStartSystem = {
		fork{
			~funcCreateSynthDef.value;
			s.sync;

			"Forest Synth Def has been added.".postln;
			"".postln;

			~globalVolumeBus.set(~globalVolumeInit);

			~funcBuildSynths.value;
			~funcStartOSC.value;
		};
	};

	~funcBuildSynths = {
		if (~synthsRunning == false) {
			~synthsRunning = true;
			~synths = ~rodCount.collect({ arg i;
				x = Synth.new("forest");
				x.set("freq", ~freqs[i], "out", 0, "pos", ~positions[i], "amp", ~amps[i]);
				x;
			});
			"Synths have been built.".postln;
			"".postln;
		};
	};


	~funcStartOSC = {
				~ampFunc = { arg msg, time;
			    		// "time: % message: %\n".postf(time, msg);
					n = msg[1]; // synth no
					a = msg[2]; // amp
					~amps[n] = a;
					~synths[n].set ("amp", a);
				};
				~oscForestAmp = OSCFunc.new(~ampFunc, '/forestAmp');
				~freqFunc = { arg msg, time;
			    		// "time: % message: %\n".postf(time, msg);
					n = msg[1]; // synth no
					a = msg[2]; // freq
					~freqs[n] = a;
					~synths[n].set ("freq", a);
				};
				~oscForestFreq = OSCFunc.new(~freqFunc, '/forestFreq');
				~posFunc = { arg msg, time;
			    		// "time: % message: %\n".postf(time, msg);
					n = msg[1]; // synth no
					a = msg[2]; // pos
					~positions[n] = a;
					~synths[n].set ("pos", a);
				};
				~oscForestPos = OSCFunc.new(~posFunc, '/forestPos');
				~centreFunc = { arg msg, time;
			    		// "time: % message: %\n".postf(time, msg);
					n = msg[1]; // synth no
					a = msg[2]; // centre
					~centres[n] = a;
					~synths[n].set ("centre", a);
				};
				~oscForestCentre = OSCFunc.new(~centreFunc, '/forestCentre');

				"Forest system has been built.".postln;
				"".postln;
				"=== OSC Message Types (ranges in brackets): ".postln;
				"Message: '/forestAmp', rodNo (int 0-149), value (float 0.0-1.0)".postln;
				"Message: '/forestFreq', rodNo (int 0-149), value (float 0.0-20000.0)".postln;
				"Message: '/forestPos', rodNo (int 0-149), value (float 0.0-1.0)".postln;
				"Message: '/forestCentre', rodNo (int 0-149), value (float 0.0-1.0)".postln;
				"(N.B. /forestCentre has no effect on Stereo Panning version of Forest)".postln;
				"".postln;
	};

	~funcQuit = {
		~funcKillSynths.value;
		~funcKillOsc.value;
		s.quit;
	};

	~funcKillSynths = {
		if (~synthsRunning == true) {
			~rodCount.do({arg item, i;
				~synths[i].free;
			});
			~synthsRunning = false;
			"All synths have been killed.".postln;
			"".postln;
		};
	};
	~funcKillOsc = {
			~oscForestAmp.remove(~ampFunc);
			~oscForestFreq.remove(~freqFunc);
			~oscForestPos.remove(~posFunc);
			~oscForestCentre.remove(~centreFunc);
	};


	~funcRebuildSynths = {
		fork {
			~funcKillSynths.value;
			~funcCreateSynthDef.value;
			s.sync;
			~funcBuildSynths.value;
		};
	};



	~funcRandomiseAmps = {
		~rodCount.do({arg item, i;
			q = 1.0.rand;
			if (q < 0.8) {r = 0;} {r = 0.99.rand;};
			~loopBackNetAddr.sendMsg("/forestAmp", i, r);
		});

		"All amplitudes have been randomised.".postln;
		"".postln;
	};

	~funcZeroAmps = {
		~rodCount.do({arg item, i;
			~loopBackNetAddr.sendMsg("/forestAmp", i, 0);
		});

		d.value_(0);
		"All amplitudes have been set to zero.".postln;
		"".postln;
	};


	////////////////////////////////////////////////////////////

	// ~funcStartSystem.value;

	// GUI SETUP

	w = Window.new(~windowTitle, Rect( 300, 300, 400, 500));
	w.onClose_({

		"User has Quit".postln;
		~funcQuit.value;
	});
	w.view.decorator = FlowLayout( w.view.bounds, 10@10, 20@5 );

	// clear post window
	//Document.listener.string = " ";

	a = Button(w, Rect(0, 0, 380, 30))
        .states_([
            ["Start system", Color.black, Color.green],
        ])
        .action_({
			~funcStartSystem.value;
		});

	b = Button(w, Rect(0, 0, 380, 30))
        .states_([
            ["Randomize all amplitudes", Color.black, Color.blue],
        ])
        .action_({
			~funcRandomiseAmps.value;
		});

	c = Button(w, Rect(0, 0, 380, 30))
        .states_([
            ["Zero all amplitudes", Color.black, Color.grey],
        ])
        .action_({
			~funcZeroAmps.value;
		});

	w.front;

	m = PopUpMenu(w,Rect(10, 10, 380, 30));
	m.items = ~arrSoundFuncs.collect({arg item, i; item[0];});
	m.background_(Color.white);
	m.action = ({ arg view;
		~soundSelectionIndex = view.value;
		~funcRebuildSynths.value;
	});

	j = EZSlider( w,         // parent
		Rect(0, 0, 380, 20), // bounds
      	"Variation", // label
    		\unipolar, // ControlSpec
	     { arg ez;
			~globalSoundShapeBus.value = ez.value;
		});
	j.value = 0;


	g = EZSlider( w,         // parent
		Rect(0, 0, 380, 20), // bounds
      	"Volume", // label
    		\amp, // controlSpec
	     { arg ez;
			~globalVolumeBus.value = ez.value;
		});
	g.value = ~globalVolumeInit;

	l = StaticText(w, Rect(0, 0, 380, 20));
	l = StaticText(w, Rect(0, 0, 380, 20));
	l.string = "For testing Rod 0 controls:";


	d = EZSlider( w,         // parent
		Rect(0, 0, 380, 20), // bounds
      	"Amp 0", // label
    		\amp, // controlSpec
	     { arg ez;
			~loopBackNetAddr.sendMsg("/forestAmp", 0, ez.value);
		});
	e = EZSlider( w,         // parent
		Rect(0, 0, 380, 20), // bounds
      	" Freq 0", // label
    		ControlSpec(48.midicps, 96.midicps, \exp, 0, 440, units: " Hz"), // controlSpec
	     { arg ez;
			~loopBackNetAddr.sendMsg("/forestFreq", 0, ez.value);
		});
	f = EZSlider( w,         // parent
		Rect(0, 0, 380, 20), // bounds
      	"Pos 0", // label
    		ControlSpec(0, 1, \lin, 0, 0.5), // controlSpec
	     { arg ez;
			~loopBackNetAddr.sendMsg("/forestPos", 0, ez.value);
		});

	h = EZSlider( w,         // parent
		Rect(0, 0, 380, 20), // bounds
      	"Centre 0", // label
    		ControlSpec(0, 1, \lin, 0, 0.0), // controlSpec
	     { arg ez;
			~loopBackNetAddr.sendMsg("/forestCentre", 0, ez.value);
		});

	l = StaticText(w, Rect(0, 0, 380, 20));

	c = Button(w, Rect(0, 0, 380, 30))
        .states_([
            ["Quit", Color.black, Color.red],
        ])
        .action_({
			w.close;
		});

	w.front;
)

/* PANNING NOTES

SplayAz spreads an array of channels across a ring of channels. Optional spread and center controls, and levelComp(ensation) (equal power).

*ar (numChans: 4, inArray, spread: 1, level: 1, width: 2, center: 0, orientation: 0.5,levelComp: true)


numChans - Number of output channels.

orientation - Should be zero if the front is a vertex of the polygon. The first speaker will be directly in front. Should be 0.5 if the front bisects a side of the polygon. Then the first speaker will be the one left of center.


*/



