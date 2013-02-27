// Forest project - SC code

//  ======= Notes re: version SC_Forest_14
//
// changed the definition of ~globalSoundShapeBus:
// OLD:  ~globalSoundShapeBus = Bus.control(s, 2); // wrongly asks for 2 channels
// NEW:  ~globalSoundShapeBus = Bus.control(s, 1);
// - changed ~arrSoundFuncs: the 2 "Sine Ringz" functions needed Mix.new to mix them to mono.
// - changed ~arrPanFuncs as follows:
//    - changed the Stereo function to use Pan2, not SplayAz
//    - changed Surround function by replacing "oscillator" with "oscillator ! 2" as SplayAz needs an array of inputs.
// ======= end

/*
NOTE: This can receive the following OSC messages (type & ranges in brackets):
Message: '/forestAmp', rodNo (int 0-149), value (float 0.0-1.0)
Message: '/forestFreq', rodNo (int 0-149), value (float 0.0-20000.0)
Message: '/forestPos', rodNo (int 0-149), value (float 0.0-1.0)
Message: '/forestCentre', rodNo (int 0-149), value (float 0.0-1.0)

e.g message: ['/forestAmp", 15, 0.33] means set the amp of rod 15 to 0.33
*/

(
// initialise
~rodCount = 151;
~surroundSpeakerCount = 8;

// Compressor controls
~compressorThreshold = 0.5;
~compressorRatio = 4;

~recordingsFolder = "~/Desktop";
~recChannels = 2;  // default is stereo recording
~recSampleFormat = "int24";   // can also be "int16" or "float"

~globalSoundShapeInit = 0.2;
~globalVolumeInit = 0.5;

~windowTitle = "Forest";


~soundSelectionIndex = 2;
~panSelectionIndex = 0;
~synthsRunning = false;
~globalVolumeBus = Bus.control(s, 1);
~globalSoundShapeBus = Bus.control(s, 1);



// set server outputs
q = s.options;
q.numOutputBusChannels = ~surroundSpeakerCount;
s.recSampleFormat = ~recSampleFormat;
s.recChannels = ~recChannels;
// start server
    "INITING0".postln;

s.waitForBoot({~funcStartSystem.value}, 100, {"couldn't boot".postln});

// sound functions - add new rows at end to create more sound options (see template row)
~arrSoundFuncs = [
    ["Sine", {arg freq, shape; SinOsc.ar(freq, 0) + (0 * shape);}],
    ["Sine + Ringz(noise)", {arg freq, shape;  (1 - shape) * SinOsc.ar(freq, 0) + Ringz.ar(PinkNoise.ar(shape), freq); }],
    ["Sine + octave + Ringz(noise)", {arg freq, shape;  ((1 - shape) * (SinOsc.ar(freq) + (0.5 * SinOsc.ar(freq * 2)))) + Ringz.ar(PinkNoise.ar(shape), freq); }],
	["Sine + octave", {arg freq, shape; (1 - shape) * SinOsc.ar(freq, 0) + (shape * SinOsc.ar(freq * 2, 0));}],
	["Sine + wobble", {arg freq, shape; Mix.new(SinOsc.ar([freq, freq * (1 + (shape * 0.01))], 0));}],
	["Dust Ringz + density", {arg freq, shape;  Ringz.ar(Dust.ar(0.5 + (shape * 7), 0.5), freq, 2, 2) }],
	["Sine Ringz + wobble", {arg freq, shape;  Mix.new( Ringz.ar(SinOsc.ar([freq, freq * (1 + (shape * 0.01))] * 0.6, 0, 0.03), freq, 0.3) )}],
	["Sine Ringz + noise", {arg freq, shape;  Mix.new( Ringz.ar(SinOsc.ar([freq, freq * 1.003] * 0.5, 0, 0.03) + (shape * PinkNoise.ar(0.2)), freq, 0.3) )}],
	["Klank Impulses + speed", {arg freq, shape;
		DynKlank.ar(`[[freq, freq * 3.355, freq * 5.765, freq * 8.615], nil, [1, 1, 1, 1]], Impulse.ar((LFNoise1.kr(1,0.5,1) * (1 + (shape * 5))), 0, 0.5)) }],
	["Klank noise 1 + highs", {arg freq, shape;  DynKlank.ar(`[[freq, freq * 3.355, freq * 5.765, freq * 8.615],
		nil, [1, 1, 1, 1]], PinkNoise.ar(0.07) + (shape * WhiteNoise.ar(0.03))) }],
	["Klank noise 2 + highs", {arg freq, shape;  DynKlank.ar(`[ (1,3..13)*freq, 1/(1,3..13), nil ], PinkNoise.ar(0.07) + (shape * WhiteNoise.ar(0.07))) }],
	// ["Klank noise 3 + highs", {arg freq, shape;  DynKlank.ar(`[ (1..13)*freq, 1/(1..13), nil ], PinkNoise.ar(0.07) + (shape * WhiteNoise.ar(0.07))) }],
	// ["Klank noise 4 + highs", {arg freq, shape;
	// 	0.5 * DynKlank.ar(`[ freq/440 *[ 445, 2156.52, 1501.12, 2882.94, 946.15, 879.51, 418.82, 1445.69, 1303.3, 1455.97, 406.44, 1223.28 ],
	// nil, nil ], PinkNoise.ar(0.07) + (shape * WhiteNoise.ar(0.07))) }],
	// ["Klank noise 5 + highs", {arg freq, shape;
	// 	0.5 * DynKlank.ar(`[ freq/440 *[ 886.91, 1297.96, 431.38, 2197.68, 1058.26, 364.2, 1308.45, 1880.13, 1466.36, 685.48, 1212.76, 1426.61 ],
	// nil, nil ], PinkNoise.ar(0.07) + (shape * WhiteNoise.ar(0.07))) }],
	// ["Klank noise 6 + highs", {arg freq, shape;
	// 	0.5 * DynKlank.ar(`[ freq/440 *[ 234.24, 237.11, 2930.37, 581.05, 3265.09, 3064.39, 3465.8, 508.14, 3761.15, 210.13, 301.71, 2586.58 ],
	// nil, nil ], PinkNoise.ar(0.07) + (shape * WhiteNoise.ar(0.07))) }],
	["Pulse + width", {arg freq, shape; Pulse.ar(freq, 0.01 + (shape * 0.48))}],
	["Sine -> Sawtooth + mix", {arg freq, shape; (((1-shape) * SinOsc.ar(freq)) + (shape * Saw.ar(freq)));}],
	["Sine -> Square + mix", {arg freq, shape;  (((1-shape) * SinOsc.ar(freq)) + (shape * Pulse.ar(freq, 0.5))); }],
	["Sawtooth -> Square wave + mix", {arg freq, shape; (((1-shape) * Saw.ar(freq)) + (shape * Pulse.ar(freq, 0.5))); }],
	["FM 1 + depth", {arg freq, shape; 1/3 * Mix.ar(PMOsc.ar(freq, freq*[2, 1.5], [0.1 + (shape*pi),0.1 + (shape*pi)], 0)); }],
	["FM 2 + depth", {arg freq, shape; 1/3 * Mix.ar(PMOsc.ar(freq, freq*[1.5, 3.5], [0.1 + (shape*pi),0.1 + (shape*pi)], 0)); }],
	["FM 3 + depth", {arg freq, shape; 1/3 * Mix.ar(PMOsc.ar(freq, freq*[1.0001, 1.4999], [0.1 + (shape*pi),0.1 + (shape*pi)], 0)); }],
	["FM 4 + depth", {arg freq, shape; 1/3 * Mix.ar(PMOsc.ar(freq, freq*[1.0001, 1.9999], [0.1 + (shape*pi),0.1 + (shape*pi)], 0)); }],
	["FM 5 + depth", {arg freq, shape; 1/3 * PMOsc.ar(freq, freq * 1.5, 0.1 + (shape*pi), SinOsc.ar(freq * 3.5, 0, 0.1 + (shape*pi))); }],

	// template for adding new functions:
	//	["Pulse + width", {arg freq, shape; Pulse.ar(freq, 0.01 + (shape * 0.48))}],
];

~arrPanFuncs = [
	["Stereo (Position used, Centre ignored)", {arg oscillator, amp, pos, centre; Pan2.ar( oscillator, ((pos - 0.5) * 2), amp);}],
	["Surround (Position & Centre used)", {arg oscillator, amp, pos, centre; SplayAz.ar(~surroundSpeakerCount, oscillator ! 2, spread: centre * 0.5, level: amp, width: 2, center: (pos * 2) - 1, orientation: 0.5, levelComp: true)}],
];

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

// define functions

~funcCreateSynthDef = {
	SynthDef(\forest, { |outbus = 0, freq = 440, amp = 0.0, pos = 0.5, centre = 0.0|
		var oscillator, panOut;
		var globalVol = ~globalVolumeBus.kr * 0.1;
		var globalShape = ~globalSoundShapeBus.kr;
		// oscillator =  SinOsc.ar(freq, 0, globalVol);
		oscillator =  ~arrSoundFuncs[~soundSelectionIndex][1].value(freq, globalShape);
		// panOut = PanAz.ar(2, oscillator, pos-0.5, amp * globalVol);
		panOut =  ~arrPanFuncs[~panSelectionIndex][1].value(oscillator, amp * globalVol, pos, centre);
		Out.ar(outbus, panOut);
	}).add;
};

~funcCreateCompressorSynthDef = {
	SynthDef(\masterCompressor, { |inbus = 0, outbus = 0, threshold = 0.5, ratio = 4|
		var inArray, compressors;
		inArray =  In.ar (bus: 0, numChannels: ~surroundSpeakerCount);
		compressors = Compander.ar (in: inArray, control: Mix.new(inArray), thresh: threshold, slopeBelow: 1, slopeAbove: ratio.reciprocal, clampTime: 0.01, relaxTime: 0.1);
		// use ReplaceOut so input busses are overwritten with compressed signals
		ReplaceOut.ar(outbus, compressors);
	}).add;
};

~funcBuildSynths = {
	if (~synthsRunning == false) {
		~synthsRunning = true;
		~synths = ~rodCount.collect({ arg i;
			x = Synth.new("forest");
			x.set("freq", ~freqs[i], "out", 0, "pos", ~positions[i], "amp", ~amps[i]);
			x;
		});
		// ~masterCompressorSynth added to tail of synth group after all other synths
		~masterCompressorSynth = Synth.new("masterCompressor", addAction: 'addToTail');
		~masterCompressorSynth.set("threshold", ~compressorThreshold, "ratio", ~compressorRatio);
		"Synths have been built.".postln;
		"".postln;
	};
};


~funcStartOSC = {
	~ampFunc = { arg msg, time;
		// "time: % message: %\n".postf(time, msg);
		n = msg[1]; // synth no
		if ((n >= 0) && (n <~rodCount)) {
			a = msg[2].min(1).max(0); // amp
			~amps[n] = a;
			~synths[n].set ("amp", a);
		} {
			"Invalid OSC Message. Synth No out of range. Message address: /forestAmp".postln;
		};
	};
	~oscForestAmp = OSCFunc.new(~ampFunc, '/forestAmp');

	~freqFunc = { arg msg, time;
		// "time: % message: %\n".postf(time, msg);
		n = msg[1]; // synth no
		if ((n >= 0) && (n <~rodCount)) {
			a = msg[2].min(20000).max(0); // freq
			~freqs[n] = a;
			~synths[n].set ("freq", a);
		} {
			"Invalid OSC Message: Synth No out of range. Message address: /forestFreq ".postln;
		};
	};
	~oscForestFreq = OSCFunc.new(~freqFunc, '/forestFreq');

	~posFunc = { arg msg, time;
		// "time: % message: %\n".postf(time, msg);
		n = msg[1]; // synth no
		if ((n >= 0) && (n <~rodCount)) {
			a = msg[2].min(1).max(0); // pos
			~positions[n] = a;
			~synths[n].set ("pos", a);
		} {
			"Invalid OSC Message. Synth No out of range. Message address: /forestPos".postln;
		};
	};
	~oscForestPos = OSCFunc.new(~posFunc, '/forestPos');

	~centreFunc = { arg msg, time;
		// "time: % message: %\n".postf(time, msg);
		n = msg[1]; // synth no
		if ((n >= 0) && (n <~rodCount)) {
			a = msg[2].min(1).max(0); // centre
			~centres[n] = a;
			~synths[n].set ("centre", a);
		} {
			"Invalid OSC Message. Synth No out of range. Message address: /forestCentre".postln;
		};
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

~funcStartSystem = {
	fork{
		s.sync;

		~funcCreateSynthDef.value;
		~funcCreateCompressorSynthDef.value;

		s.sync;
		"Forest Synth Def has been added.".postln;
		"".postln;

		~globalVolumeBus.set(~globalVolumeInit);
        ~globalSoundShapeBus.set(~globalSoundShapeInit);
		~funcBuildSynths.value;
		s.sync;
		~funcStartOSC.value;
		s.sync;
	};
};

~funcQuit = {
	~funcStopRecording.value;
	~funcKillSynths.value;
	~funcKillOsc.value;
	if (~meter.notNil) {~meter.window.close;  ~meter = nil; "Meters stopped".postln; };
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

	~rod0AmpSlider.value_(0);
	"All amplitudes have been set to zero.".postln;
	"".postln;
};

// Recording funcs
~funcStartRecording = {
	var path;
	path = path = ~recordingsFolder.standardizePath +/+ "SC_Forest_" ++ Date.localtime.stamp ++ ".aiff";
	s.record(path);
	~recordingStatusText.string = "Recording: " ++ path;
};

~funcStopRecording = {
	s.stopRecording;
	~recordingStatusText.string = "Status: Not Recording";
};

////////////////////////////////////////////////////////////

// GUI

w = Window.new(~windowTitle, Rect( 300, 300, 400, 620));
w.onClose_({
	"User has Quit".postln;
	~funcQuit.value;
});
w.view.decorator = FlowLayout( w.view.bounds, 10@10, 20@5 );
w.front;

Button(w, Rect(0, 0, 380, 30))
.states_([
	["Start system", Color.white, Color.green(0.7)],
])
.action_({
	~funcStartSystem.value;
});

Button(w, Rect(0, 0, 380, 30))
.states_([
	["Show Meters", Color.white, Color.magenta(0.7)],
])
.action_({
	if (~meter.isNil) {	~meter = s.meter;};
});

Button(w, Rect(0, 0, 380, 30))
.states_([
	["Randomize all amplitudes", Color.white, Color.blue],
])
.action_({
	~funcRandomiseAmps.value;
});

Button(w, Rect(0, 0, 380, 30))
.states_([
	["Zero all amplitudes", Color.white, Color.grey],
])
.action_({
	~funcZeroAmps.value;
});


m = PopUpMenu(w,Rect(10, 10, 380, 30));
m.items = ~arrSoundFuncs.collect({arg item, i; item[0];});
m.background_(Color.white);
m.action = ({ arg view;
	~soundSelectionIndex = view.value;
	~funcRebuildSynths.value;
});
m.value = ~soundSelectionIndex;

a = EZSlider( w,         // parent
	Rect(0, 0, 380, 20), // bounds
	"Variation", // label
	\unipolar, // ControlSpec
	{ arg ez;
		~globalSoundShapeBus.value = ez.value;
});
a.value = ~globalSoundShapeInit;


a = EZSlider( w,         // parent
	Rect(0, 0, 380, 20), // bounds
	"Volume", // label
	\amp, // controlSpec
	{ arg ez;
		~globalVolumeBus.value = ez.value;
});
a.value = ~globalVolumeInit;

p = PopUpMenu(w,Rect(10, 10, 380, 30));
p.items = ~arrPanFuncs.collect({arg item, i; item[0];});
p.background_(Color.white);
p.action = ({ arg view;
	~panSelectionIndex = view.value;
	~funcRebuildSynths.value;
});
p.value = ~panSelectionIndex;




StaticText(w, Rect(0, 0, 380, 20));

Button(w, Rect(0, 0, 380, 30))
.states_([
	["Start Recording", Color.white, Color.yellow(0.8)],
])
.action_({
	~funcStartRecording.value;
});

Button(w, Rect(0, 0, 380, 30))
.states_([
	["Stop Recording", Color.white, Color.grey],
])
.action_({
	~funcStopRecording.value;
});

~recordingStatusText = StaticText(w, Rect(0, 0, 380, 20));
~recordingStatusText.font = Font("Monaco", 9);
~recordingStatusText.string = "Status: Not Recording";

StaticText(w, Rect(0, 0, 380, 20));

l = StaticText(w, Rect(0, 0, 380, 20));
l.string = "For testing Rod 0 controls:";

~rod0AmpSlider = EZSlider( w,         // parent
	Rect(0, 0, 380, 20), // bounds
	"Amp 0", // label
	\amp, // controlSpec
	{ arg ez;
		~loopBackNetAddr.sendMsg("/forestAmp", 0, ez.value);
});
EZSlider( w,         // parent
	Rect(0, 0, 380, 20), // bounds
	" Freq 0", // label
	ControlSpec(48.midicps, 96.midicps, \exp, 0, 440, units: " Hz"), // controlSpec
	{ arg ez;
		~loopBackNetAddr.sendMsg("/forestFreq", 0, ez.value);
});
EZSlider( w,         // parent
	Rect(0, 0, 380, 20), // bounds
	"Pos 0", // label
	ControlSpec(0, 1, \lin, 0, 0.5), // controlSpec
	{ arg ez;
		~loopBackNetAddr.sendMsg("/forestPos", 0, ez.value);
});

EZSlider( w,         // parent
	Rect(0, 0, 380, 20), // bounds
	"Centre 0", // label
	ControlSpec(0, 1, \lin, 0, 0.0), // controlSpec
	{ arg ez;
		~loopBackNetAddr.sendMsg("/forestCentre", 0, ez.value);
});

StaticText(w, Rect(0, 0, 380, 20));

Button(w, Rect(0, 0, 380, 30))
.states_([
	["Quit", Color.white, Color.red(0.7)],
])
.action_({
	w.close;
});

)
