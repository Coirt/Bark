#include "plugin.hpp"
#include "barkComponents.hpp"
#include "dependancies/filt/biquad.h"

using namespace barkComponents;

//lowpass(lp, 82.41Hz, 0.707, Gain)
//mud = (lp, 2139.6Hz, 0.71182, Gain) (peak1, 233.08Hz, 0.1Q, 6.9599) (peak2, 55.343Hz, 0.1Q, 1.4344)
//highpass(hp, 192Hz, 0.19597, Gain)
///3.8% 0.89us no difference
/*
#define lowLP_Fc 82.41
#define lowQ 0.707
#define highHP_Fc 192.0
#define highQ 0.19597
#define mudLP_Fc 2139.6
#define mudQ 0.71182
#define peak1_Fc 233.08
#define peak1_Gain 6.9599
#define peak2_Fc 55.343
#define peak2_Gain 1.4344
#define peak_Q 0.1
*/
struct LMH : Module {
	enum ParamIds {
		LOW_PARAM,
		MUD_PARAM,
		HIGH_PARAM,
		INV_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Biquad *low = new Biquad();
	Biquad *high = new Biquad();
	Biquad *mud = new Biquad();
	Biquad *peak1 = new Biquad();
	Biquad *peak2 = new Biquad();
	dsp::SlewLimiter clickFilter;
	dsp::ClockDivider step;

	double sR = APP->engine->getSampleRate();
	float gLow;
	float gHigh;
	float gMud;

	LMH() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		/*
		configParam(LOW_PARAM, .459435f, 10.f, 3.459432f, "Freq", "Hz", 2, 20);
		configParam(MUD_PARAM, 0.01f, 40.f, .707f, "Q");
		configParam(HIGH_PARAM, 0.f, 1.f, .0f, "Gain");
		*/
		
		configParam(LOW_PARAM, 0.0, M_SQRT2, 1.0, "Low", " dB", -10, 40);
		configParam(MUD_PARAM, 0.0, M_SQRT1_2, .5, "Mud", " dB", -10, 40, 6.0206 + 4.7684e-07);
		configParam(HIGH_PARAM, 0.0, M_SQRT2, 1.0, "High", " dB", -10, 40);
		configParam<tpOnOffBtn>(INV_PARAM, 0.f, 1.f, 0.f, "Mud");
		clickFilter.rise = 400.f; // Hz
		clickFilter.fall = 400.f; // Hz
		step.setDivision(16);
	}

	void process(const ProcessArgs &args) override {
		//sR = args.sampleRate;
		double low_Fc = 82.41 / sR, high_Fc = 192.0 / sR, mud_Fc = 2139.6 / sR;
		double peak1_Fc = 233.08 / sR, peak2_Fc = 55.343 / sR;
		
		gLow = params[LOW_PARAM].getValue();
		gHigh = params[HIGH_PARAM].getValue();
		gMud = params[MUD_PARAM].getValue();
		/*
		Freq = std::pow(2.f, params[LOW_PARAM].getValue()) * 20;
		Q = params[MUD_PARAM].getValue();
		Gain = params[HIGH_PARAM].getValue();
		*/

		//double Freq, Q, Gain = 0.;
		
		/*
		You could write a `ClickFilter` class with an `insertClick(float x)` 
		method that is called when you make a jump from - x to 0 or from 0 to x.
		When you call its `process(float deltaTime)` method, it applies a 
		filter to that impulse, such as cheap Euler exponential decay, 
		and returns the value.You then add that value to your output, 
		and it will cancel out the clicks.

		In other words, you are inserting a discontinuous jump in
		the opposite direction the is filtered on the "right" and 
		unfiltered on the "left".
		*/

		bool inv = params[INV_PARAM].getValue();
		float in = inputs[IN_INPUT].getVoltage(), out;
		clickFilter.process(args.sampleTime, params[INV_PARAM].getValue());

		if (step.process()) {
			if (gLow > 0.f) {
				low->setBiquad(bq_type_lowpass, low_Fc, 0.707, gLow);
			}

			if (gHigh > 0.f) {
				high->setBiquad(bq_type_highpass, high_Fc, 0.19597, gHigh);
			}

			if (gMud > 0.f) {
				peak1->setBiquad(bq_type_peak, peak1_Fc, 0.1, 6.9599);
				peak2->setBiquad(bq_type_peak, peak2_Fc, 0.1, 1.4344);
				mud->setBiquad(bq_type_lowpass, mud_Fc, 0.71182, gMud);
			}
		}

		out = low->process(in * gLow);

		out += high->process(in * gHigh);

		float invertMud = inv ? -in : in;
		out += peak1->process(peak2->process(mud->process(invertMud * gMud)));
		
		outputs[OUT_OUTPUT].setVoltage(out);
	}
};

struct LMHWidget : ModuleWidget {
	LMHWidget(LMH *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BarkLMH.svg")));


		box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		constexpr float portX = 10.387f, knbX = 9.536f;
		constexpr float Y[5] = { 319.82f, 254.704f, 188.488f, 115.392f, 55.191f };

		///Ports---
		//Out---
		addOutput(createOutput<BarkOutPort350>(Vec(portX, Y[0]), module, LMH::OUT_OUTPUT));
		//In---
		addInput(createInput<BarkInPort350>(Vec(portX, Y[1]), module, LMH::IN_INPUT));
		//Knob---
		addParam(createParam<BarkKnob_26>(Vec(knbX, Y[2]), module, LMH::LOW_PARAM));
		addParam(createParam<BarkKnob_26>(Vec(knbX, Y[3]), module, LMH::MUD_PARAM));
		addParam(createParam<BarkKnob_26>(Vec(knbX, Y[4]), module, LMH::HIGH_PARAM));
		//Switch---
		addParam(createParam<BarkPushButton2>(Vec(knbX, 155.767f), module, LMH::INV_PARAM));
		//screw---
		//screw---
		addChild(createWidget<BarkScrew3>(Vec(2.7f, 2.7f)));					//pos1
		addChild(createWidget<BarkScrew1>(Vec(box.size.x - 12.3f, 367.7f)));			//pos4
	}
};

Model *modelLMH = createModel<LMH, LMHWidget>("LMH");
