#include "plugin.hpp"
#include "barkComponents.hpp"
#include "dependancies/filt/biquad.h"
#include "dependancies/filt/lp24.cpp"

using namespace barkComponents;

struct OneBand : Module {

	std::vector<std::string> vuLightDescription = { "Clipping +0dB",
							"-3dB to -0dB\n\nA white",
							"-5dB to -2dB\nEach",
							"-8dB to -6dB\nEach",
							"-11dB to -9dB\nEach",
							"-14dB to -12dB\nEach",
							"-17dB to -15dB\nEach",
							"-inf to -18dB\nEach"
	};

	enum ParamIds {
		EQGAIN_PARAM,
		EQFREQ_PARAM,
		EQBANDWIDTH_PARAM,
		EQBYPASS_PARAM,
		PREPOST_PARAM,
		OUTGAIN_PARAM,
		SWAPLR_PARAM,
		LISTEN_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		GAINMOD_INPUT,
		FREQMOD_INPUT,
		BWMOD_INPUT,
		INL_INPUT,
		INR_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTL_OUTPUT,
		OUTR_OUTPUT,
		devParamOutGain,
		devParamOutFrq,
		devParamOutQ,
		NUM_OUTPUTS
	};
	enum LightIds {
		FreqParamOn,
		FreqParamOff,
		dBpeak_LIGHT,
		NUM_LIGHTS = dBpeak_LIGHT + 8			//7 + Clipping Light
	};

	dsp::VuMeter2 volUnitIndicatorPEAK;
	dsp::ClockDivider vuDivider;
	dsp::ClockDivider lightDivider;
	dsp::ClockDivider step;
	Biquad *parametricEQL = new Biquad();
	Biquad *parametricEQR = new Biquad();
	LadderFilter lpf24;		//smoothing

	OneBand() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		//Knob---
		configParam(EQGAIN_PARAM, -6.5f, 6.5f, 0.f, "Band Gain", "dB", params[EQGAIN_PARAM].getValue(), 5.f);
		configParam(EQFREQ_PARAM, .459435f, 10.f, 3.459432f, "Freq", " Hz", 2, 20);	// .01375f
		configParam(EQBANDWIDTH_PARAM, .1f, 40.f, 15.f, "Q Factor");
		configParam(OUTGAIN_PARAM, 0.f, 8.f, 3.f, "Output Gain", " dB", -10, 20, -12.04119836);
		//Switch---
		configSwitch(EQBYPASS_PARAM, 0.f, 1.f, 0.f, "EQ", {"On", "Off"});
		configSwitch(PREPOST_PARAM, 0.f, 1.f, 1.f, "Meter", {"Pre Gain", "Post Gain" });
		configSwitch(SWAPLR_PARAM, 0.f, 1.f, 0.f, "Output", { "L/R","R/L" });
		configSwitch(LISTEN_PARAM, 0.f, 1.f, 0.f, "EQ", { "Process","Listen" });
		//Input---
		configInput(GAINMOD_INPUT, "Gain");
		configInput(FREQMOD_INPUT, "Frequency");
		inputInfos[FREQMOD_INPUT]->description = "\nWhen \"input\" connected\n\"freq\" knob is bypassed.";
		configInput(BWMOD_INPUT, "Band width");
		configInput(INL_INPUT, "Left");
		configInput(INR_INPUT, "Right");
		//Output---
		configOutput(OUTL_OUTPUT, "Left");
		configOutput(OUTR_OUTPUT, "Right");
		
		//Lights---
		for (int i = 0; i < 8; i++) {
			configLight(dBpeak_LIGHT + i, vuLightDescription[i]);
		}
		for (int i = 2; i < 8; i++) {	//start at 2[1]
			lightInfos[dBpeak_LIGHT + i]->description = " represents 3dB\n of Peak gain\nlight " + std::to_string(i) + ":";
		}
		lightInfos[dBpeak_LIGHT + 1]->description = " represents a\nclipping signal.\n\nlight 1:"; //clip/intersample peak light draws underneath

		volUnitIndicatorPEAK.lambda = 1 / 0.1f;
		vuDivider.setDivision(16);
		lightDivider.setDivision(256);
		step.setDivision(2048);

		//Route Bypass---
		configBypass(INL_INPUT, OUTL_OUTPUT);
		configBypass(INR_INPUT, OUTR_OUTPUT);
	}

	void process(const ProcessArgs &args) override {
		bool Listen = params[LISTEN_PARAM].getValue(), swapLR = params[SWAPLR_PARAM].getValue();
		float inL = 0.0f, inR = 0.0f, outLdBVU = 0.0f, outRdBVU = 0.0f,
			outL = outputs[OUTL_OUTPUT].getVoltage(), outR = outputs[OUTR_OUTPUT].getVoltage(),
			Gain = params[OUTGAIN_PARAM].getValue() / 2.0f;
		double eqGain = params[EQGAIN_PARAM].getValue() * 5.0 + (clamp(inputs[GAINMOD_INPUT].getVoltage(), -6.5f, 6.5f) * 5.0), modInput, eqFreq,
			eqQ = clamp(params[EQBANDWIDTH_PARAM].getValue(), .1f, 40.f) + (4.f * clamp(inputs[BWMOD_INPUT].getVoltage(), .1f, 40.f));
		double sampRate, biquadFreq, biquadQ, biquadGain = 0.;

		//dBmeter on input-------------------
		inL = inputs[INL_INPUT].getVoltage();
		inR = inputs[INR_INPUT].getVoltage();
		//dBmeter on output--------------------------
		outLdBVU = outputs[OUTL_OUTPUT].getVoltage();
		outRdBVU = outputs[OUTR_OUTPUT].getVoltage();

		//dB Peak Level Indicator-------------------------------------------------------  
		if (vuDivider.process()) {
			//pre EQ---
			if (params[PREPOST_PARAM].value == 0.0f && !inputs[INR_INPUT].active) {//pre
				volUnitIndicatorPEAK.process(args.sampleTime * vuDivider.getDivision(), (inL / 10.f) + 0.005f);
			} else if (params[PREPOST_PARAM].value == 1.0f && !inputs[INR_INPUT].active) {//post
				volUnitIndicatorPEAK.process(args.sampleTime * vuDivider.getDivision(), (outLdBVU / 10.f) + 0.005f);
			}
			//post EQ---
			if (params[PREPOST_PARAM].value == 0.0f && inputs[INR_INPUT].active) {
				//volUnitIndicatorPEAK.setValue(((inL / 10.f) + (inR / 10.f)) / 2.f);
				volUnitIndicatorPEAK.process(args.sampleTime * vuDivider.getDivision(), (((inL / 10.f) + (inR / 10.f)) / 2.f) + 0.005f);
			} else if (params[PREPOST_PARAM].value == 1.0f && inputs[INR_INPUT].active) {
				//volUnitIndicatorPEAK.setValue(((outLdBVU / 10.f) + (outRdBVU / 10.f)) / 2.f);
				volUnitIndicatorPEAK.process(args.sampleTime * vuDivider.getDivision(), (((outLdBVU / 10.f) + (outRdBVU / 10.f)) / 2.f) + 0.005f);
			}
		}
		if (lightDivider.process()) {
			lights[dBpeak_LIGHT + 0].setBrightness(volUnitIndicatorPEAK.getBrightness(0.f, 0.f));
			for (int light = 1; light < 8; light++) {
				//TODO: L/R unbalanced inputs will not engage clip light
				if (params[PREPOST_PARAM].value == 0.0f && !inputs[INR_INPUT].active) {//pre
					//volUnitIndicatorPEAK.setValue(inL / 10.f);
					lights[dBpeak_LIGHT + light].setBrightness(volUnitIndicatorPEAK.getBrightness(-3.f * light, -3.f * (light - 1)));
				} else if (params[PREPOST_PARAM].value == 1.0f && !inputs[INR_INPUT].active) {//post
					//volUnitIndicatorPEAK.setValue(outLdBVU / 10.f);
					lights[dBpeak_LIGHT + light].setBrightness(volUnitIndicatorPEAK.getBrightness(-3.f * light, -3.f * (light - 1)));
				}
				if (params[PREPOST_PARAM].value == 0.0f && inputs[INR_INPUT].active) {
					//volUnitIndicatorPEAK.setValue(((inL / 10.f) + (inR / 10.f)) / 2.f);
					lights[dBpeak_LIGHT + light].setBrightness(volUnitIndicatorPEAK.getBrightness(-3.f * light, -3.f * (light - 1)));
				} else if (params[PREPOST_PARAM].value == 1.0f && inputs[INR_INPUT].active) {
					//volUnitIndicatorPEAK.setValue(((outLdBVU / 10.f) + (outRdBVU / 10.f)) / 2.f);
					lights[dBpeak_LIGHT + light].setBrightness(volUnitIndicatorPEAK.getBrightness(-3.f * light, -3.f * (light - 1)));
				}
			}
		}

		if (inputs[FREQMOD_INPUT].isConnected()) {
			lights[FreqParamOn].setBrightness(0); lights[FreqParamOff].setBrightness(1);
			modInput = std::fminf(clamp((inputs[FREQMOD_INPUT].getVoltage() / 5.f), fabs(.022f), fabs(10.f)), std::fmaxf(fabs(.022f), fabs(10.f)));
			lpf24.setCutoff(27.5f);
			lpf24.process(modInput, args.sampleTime);
			float smooth = 5.f * lpf24.lowpass;
			eqFreq = clamp(smooth * 2, 1.f, 11.f) - .89f;
		} else if (!inputs[FREQMOD_INPUT].isConnected()) {
			lights[FreqParamOn].setBrightness(1); lights[FreqParamOff].setBrightness(0);
			eqFreq = std::fmax(std::pow(2.f, params[EQFREQ_PARAM].getValue()), std::fmin(fabs(1.1726039399558574), fabs(10.)));//min == .01375
		}
		sampRate = args.sampleRate;
		if (inputs[FREQMOD_INPUT].isConnected()) {
			biquadFreq = eqFreq * 2000.;	//lin
		} else if (!inputs[FREQMOD_INPUT].isConnected()) {
			biquadFreq = eqFreq * 20.;	//exp
		}
		
		//Q
		biquadQ = eqQ;
		//Bypass EQ
		params[EQBYPASS_PARAM].getValue() < 1.f ? biquadGain = eqGain : biquadGain = 0.0;
		//set Biquad Values
		if (step.process()) {
			parametricEQL->setBiquad(bq_type_peak, biquadFreq / sampRate, biquadQ, biquadGain);
			parametricEQR->setBiquad(bq_type_peak, biquadFreq / sampRate, biquadQ, biquadGain);
		}
		
		//Listen to EQ
		if (Listen == 1) {//invert input
			if (swapLR == 1) {
				outR = parametricEQL->process(inL * Gain) - inL;
				outL = parametricEQR->process(inR * Gain) - inR;
			} else {
				outL = parametricEQL->process(inL * Gain) - inL;
				outR = parametricEQR->process(inR * Gain) - inR;
			}
		} else if (Listen != 1) {
			if (swapLR == 1) {
				outR = parametricEQL->process(inL * Gain);
				outL = parametricEQR->process(inR * Gain);
			} else {
				outL = parametricEQL->process(inL * Gain);
				outR = parametricEQR->process(inR * Gain);
			}
		}

		outputs[OUTL_OUTPUT].setVoltage(outL);
		outputs[OUTR_OUTPUT].setVoltage(outR);
	}


};


struct OneBandWidget : ModuleWidget {

	OneBandWidget(OneBand *module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Bark1Band.svg")));

		//constexpr int rackY = 380;
		constexpr float floatyMcFloatFace = 16.11f, lightXpos = 45.5f;

		///Ports---
		//Out---
		addOutput(createOutput<BarkOutPort350>(Vec(4.05f, rackY - 174.04f - 13.74f), module, OneBand::OUTL_OUTPUT));
		addOutput(createOutput<BarkOutPort350>(Vec(31.38f, rackY - 174.04f - 13.74f), module, OneBand::OUTR_OUTPUT));
		//In---
		//Audio--
		addInput(createInput<BarkInPort350>(Vec(4.05f, rackY - 46.16f - 14.02f), module, OneBand::INL_INPUT));
		addInput(createInput<BarkInPort350>(Vec(31.38f, rackY - 46.16f - 14.02f), module, OneBand::INR_INPUT));
		//Mod--
		addInput(createInput<BarkPatchPortIn>(Vec(34.16f, rackY - 324.73f), module, OneBand::GAINMOD_INPUT));
		addInput(createInput<BarkPatchPortIn>(Vec(2.16f, rackY - 307.59f), module, OneBand::FREQMOD_INPUT));
		addInput(createInput<BarkPatchPortIn>(Vec(33.78f, rackY - 248.26f), module, OneBand::BWMOD_INPUT));
		//Knobs---
		addParam(createParam<BarkKnob_26>(Vec(10.21f, rackY - 349.76f), module, OneBand::EQGAIN_PARAM));
		addParam(createParam<BarkKnob_26>(Vec(25.09f, rackY - 291.05f), module, OneBand::EQFREQ_PARAM));
		addParam(createParam<BarkKnob_26>(Vec(10.82f, rackY - 231.36f), module, OneBand::EQBANDWIDTH_PARAM));
		addParam(createParam<BarkKnob_30>(Vec(8.77f, rackY - 106.79f), module, OneBand::OUTGAIN_PARAM));
		//Switch---
		addParam(createParam<BarkSwitchSmall>(Vec(41.29f, rackY - 355.97f), module, OneBand::EQBYPASS_PARAM));
		addParam(createParam<BarkSwitchSmall>(Vec(11.26f, rackY - 136.57f), module, OneBand::PREPOST_PARAM));
		addParam(createParam<BarkSwitchSmallSide>(Vec(21.89f, rackY - 161.23f), module, OneBand::SWAPLR_PARAM));
		addParam(createParam<BarkSwitchSmall>(Vec(40.4f, rackY - 212.39f), module, OneBand::LISTEN_PARAM));
		//TODO: Screw Positions
		addChild(createWidget<RandomRotateScrew>(Vec(box.size.x - 12.3f, 2.7f)));		//pos2
		addChild(createWidget<RandomRotateScrew>(Vec(2.7f, 367.7f)));				//pos3
		//Light---
		addChild(createLight<SmallerLightFA<ParamInLight>>(Vec(floatyMcFloatFace, rackY - 280.05f), module, OneBand::FreqParamOn));
		addChild(createLight<SmallerLightFA<ParamInLight>>(Vec(floatyMcFloatFace, rackY - 261.72f), module, OneBand::FreqParamOff));
		addChild(createLight<BigLight<clipLight>>(Vec(lightXpos, rackY - 133.66f - 12.8f), module, OneBand::dBpeak_LIGHT + 0));	//v2 clip light draw order
		//addChild(createLight<BiggerLight<clipLight>>(Vec(lightXpos - 1.f, rackY - 134.65f - 12.8f), module, OneBand::dBpeak_LIGHT + 0));
		addChild(createLight<BigLight<redLight>>(Vec(lightXpos, rackY - 133.66f - 12.8f), module, OneBand::dBpeak_LIGHT + 1));
		addChild(createLight<BigLight<orangeLight>>(Vec(lightXpos, rackY - 122.11f - 12.8f), module, OneBand::dBpeak_LIGHT + 2));
		addChild(createLight<BigLight<yellowLight2>>(Vec(lightXpos, rackY - 110.55f - 12.8f), module, OneBand::dBpeak_LIGHT + 3));
		addChild(createLight<BigLight<yellowLight1>>(Vec(lightXpos, rackY - 99.f - 12.8f), module, OneBand::dBpeak_LIGHT + 4));
		addChild(createLight<BigLight<greenLight>>(Vec(lightXpos, rackY - 87.45f - 12.8f), module, OneBand::dBpeak_LIGHT + 5));
		addChild(createLight<BigLight<greenLight>>(Vec(lightXpos, rackY - 75.9f - 12.8f), module, OneBand::dBpeak_LIGHT + 6));
		addChild(createLight<BigLight<greenLight>>(Vec(lightXpos, rackY - 64.35f - 12.8f), module, OneBand::dBpeak_LIGHT + 7));
	}
};
Model *modelOneBand = createModel<OneBand, OneBandWidget>("OneBand");
