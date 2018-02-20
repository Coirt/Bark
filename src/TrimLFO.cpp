#include "Bark.hpp"
#include "dsp/digital.hpp"
//#include <math.h>

struct LowFrequencyOscillator {
	float phase = 0.0;
	float pw = 0.5;
	float freq = 1.0;
	bool offset = false;
	bool invert = false;
	SchmittTrigger resetTrigger;
	LowFrequencyOscillator() {
		resetTrigger.setThresholds(0.0, 0.01);
	}
	void setPitch(float pitch) {
		pitch = fminf(pitch, 8.0);
		freq = powf(2.0, pitch);
	}
	void setPulseWidth(float pw_) {
		const float pwMin = 0.01;
		pw = clampf(pw_, pwMin, 1.0 - pwMin);
	}
	void setReset(float reset) {
		if (resetTrigger.process(reset)) {
			phase = 0.0;
		}
	}
	void step(float dt) {
		float deltaPhase = fminf(freq * dt, 0.5);
		phase += deltaPhase;
		if (phase >= 1.0)
			phase -= 1.0;
	}
	float sin() {
		if (offset)
			return 1.0 - cosf(2 * M_PI * phase) * (invert ? -1.0 : 1.0);
		else
			return sinf(2 * M_PI * phase) * (invert ? -1.0 : 1.0);
	}
	float tri(float x) {
		return 4.0 * fabsf(x - roundf(x));
	}
	float tri() {
		if (offset)
			return tri(invert ? phase - 0.5 : phase);
		else
			return -1.0 + tri(invert ? phase - 0.25 : phase - 0.75);
	}
	float saw(float x) {
		return 2.0 * (x - roundf(x));
	}
	float saw() {
		if (offset)
			return invert ? 2.0 * (1.0 - phase) : 2.0 * phase;
		else
			return saw(phase) * (invert ? -1.0 : 1.0);
	}
	float sqr() {
		float sqr = (phase < pw) ^ invert ? 1.0 : -1.0;
		return offset ? sqr + 1.0 : sqr;
	}
	float light() {
		return sinf(2 * M_PI * phase);
	}
};

struct TrimLFO : Module {
	enum ParamIds
	{
		//Offset part
		OFFSET1_PARAM,
		OFFSET2_PARAM,
		//Internal Param for Logic?
		//LFO Part
		OFFSET_PARAM,
		INVERT_PARAM,
		FREQ_PARAM,
		FM1_PARAM,
		FM2_PARAM,
		PW_PARAM,
		PWM_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN1_INPUT,  //Code needs to link Lfo In to Logic 1A, OFFSET_1 to Logic 1B, > Max to Logic 2A > OFFSET_2 to Logic 2B Min to Lfo out Port, Internally different waves need to link to Logic.
		FM1_INPUT,
		FM2_INPUT,
		RESET_INPUT,
		PW_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT, //Offset L
		OUT2_OUTPUT, //Offset R
		OUT3_OUTPUT, //LFO Out
		SIN_OUTPUT,
		TRI_OUTPUT,
		SAW_OUTPUT,
		SQR_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		PHASE_POS_LIGHT,
		PHASE_NEG_LIGHT,
		NUM_LIGHTS
	};

	LowFrequencyOscillator oscillator;

	TrimLFO() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

};


void TrimLFO::step() {
	float out1 = params[OFFSET1_PARAM].value;
	float out2 = params[OFFSET2_PARAM].value;
	out1 = clampf(out1, -10.0, 10.0);
	out2 = clampf(out2, -10.0, 10.0);
	outputs[OUT1_OUTPUT].value = out1;
	outputs[OUT2_OUTPUT].value = out2;
	oscillator.setPitch(params[FREQ_PARAM].value + params[FM1_PARAM].value * inputs[FM1_INPUT].value + params[FM2_PARAM].value * inputs[FM2_INPUT].value);
	oscillator.setPulseWidth(params[PW_PARAM].value + params[PWM_PARAM].value * inputs[PW_INPUT].value / 10.0);
	oscillator.offset = (params[OFFSET_PARAM].value > 0.0);
	oscillator.invert = (params[INVERT_PARAM].value <= 0.0);
	oscillator.step(1.0 / engineGetSampleRate());
	oscillator.setReset(inputs[RESET_INPUT].value);

	///OFFSET- TRIM ----
	//float offsetMin
	//float offsetMax
	//offsetMin = params[OFFSET1_PARAM].value;
	//offsetMax = params[OFFSET2_PARAM].value;


	outputs[SIN_OUTPUT].value = 5.0 * oscillator.sin();
	outputs[TRI_OUTPUT].value = 5.0 * oscillator.tri();
	outputs[SAW_OUTPUT].value = 5.0 * oscillator.saw();
	outputs[SQR_OUTPUT].value = 5.0 * oscillator.sqr();

	lights[PHASE_POS_LIGHT].setBrightnessSmooth(fmaxf(0.0, oscillator.light()));
	lights[PHASE_NEG_LIGHT].setBrightnessSmooth(fmaxf(0.0, -oscillator.light()));
}


TrimLFOWidget::TrimLFOWidget() {
	TrimLFO *module = new TrimLFO();
	setModule(module);
	box.size = Vec(150, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/TrimLFO.svg")));
		addChild(panel);
	}
	////////////
	//components
	////////////
//Ports---
	//Out---
	addOutput(createOutput<BarkOutPort350>(Vec(12.93 + 0.35, 380 - 52 + 0.35), module, TrimLFO::SIN_OUTPUT));
	addOutput(createOutput<BarkOutPort350>(Vec(43.38 + 2.85 + 0.35, 380 - 52 + 0.35), module, TrimLFO::SAW_OUTPUT));
	addOutput(createOutput<BarkOutPort350>(Vec(76.48 + 2.85 + 0.35, 380 - 52 + 0.35), module, TrimLFO::TRI_OUTPUT));
	addOutput(createOutput<BarkOutPort350>(Vec(109.87 + 2.85 + 0.525, 380 - 52 + 0.35), module, TrimLFO::SQR_OUTPUT));
	addOutput(createOutput<BarkOutPort350>(Vec(14.22 + 0.35, 380 - 274.73), module, TrimLFO::OUT1_OUTPUT));
	addOutput(createOutput<BarkOutPort350>(Vec(111.74 + 0.35, 380 - 274.73), module, TrimLFO::OUT2_OUTPUT));

	//In---
	//addInput(createInput<BarkInPort350>(Vec(12.93 + 0.35, 380 - 82 - 0.35), module, TrimLFO::FM1_INPUT));
	addInput(createInput<BarkInPort350>(Vec(26.71 + 0.35, 380 - 82 + 0.35), module, TrimLFO::FM1_INPUT));
	//addInput(createInput<BarkInPort350>(Vec(46.23 + 0.35, 380 - 82 - 0.35), module, TrimLFO::FM2_INPUT));
	addInput(createInput<BarkInPort350>(Vec(62.90 + 0.35, 380 - 82 + 0.35), module, TrimLFO::FM2_INPUT));
	//addInput(createInput<BarkInPort350>(Vec(76.48 + 2.85 + 0.35, 380 - 82 - 0.35), module, TrimLFO::PW_INPUT));
	addInput(createInput<BarkInPort350>(Vec(99.31 + 0.35, 380 - 82 + 0.35), module, TrimLFO::PW_INPUT));
	//addInput(createInput<BarkInPort350>(Vec(109.87 + 2.85 + 0.7, 380 - 82 - 0.35), module, TrimLFO::RESET_INPUT));
	addInput(createInput<BarkInPort350>(Vec(119.54 + 0.35, 380 - 163 + 0.35), module, TrimLFO::RESET_INPUT));

//Knobs---
	addParam(createParam<BarkKnob92>(Vec(40.01, 380 - 217.01), module, TrimLFO::FREQ_PARAM, -8.0, 6.0, -1.0));
	addParam(createParam<BarkKnob40>(Vec(20.98 - 0.5, 380 - 329.18 - 1), module, TrimLFO::OFFSET1_PARAM, -10.0, 10.0, 10.0));
	addParam(createParam<BarkKnob40>(Vec(90.2 - 0.5, 380 - 329.18 - 1), module, TrimLFO::OFFSET2_PARAM, -10.0, 10.0, -10.0));
	addParam(createParam<BarkKnob26>(Vec(5.54 - 0.35, 380 - 167.6), module, TrimLFO::PW_PARAM, 0.0, 1.0, 0.5));
	addParam(createParam<BarkKnob26>(Vec(25.67 - 0.35, 380 - 122.3), module, TrimLFO::FM1_PARAM, 0.0, 1.0, 0.0));
	addParam(createParam<BarkKnob26>(Vec(62 - 0.35, 380 - 122.3), module, TrimLFO::FM2_PARAM, 0.0, 1.0, 0.0));
	addParam(createParam<BarkKnob26>(Vec(98.41 - 0.35, 380 - 122.3), module, TrimLFO::PWM_PARAM, 0.0, 1.0, 0.0));

//Switch---
	addParam(createParam<BarkSwitch>(Vec(8.67, 380 - 223.06 +6), module, TrimLFO::OFFSET_PARAM, 0.0, 1.0, 1.0));
	addParam(createParam<BarkSwitch>(Vec(117.57, 380 - 223.06 +6), module, TrimLFO::INVERT_PARAM, 0.0, 1.0, 1.0));

//Screw---
	addChild(createScrew<BarkScrew3>(Vec(2, 3)));							//pos1
	addChild(createScrew<BarkScrew1>(Vec(box.size.x - 13, 367.2)));			//pos4

	//Light---
	addChild(createLight<SmallLight<GreenRedLight>>(Vec(71.93, 380 - 230.22), module, TrimLFO::PHASE_POS_LIGHT));
}