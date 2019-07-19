#include "plugin.hpp"
#include "barkComponents.hpp"
#include "dependancies/filt/biquad.cpp"
#include "dependancies/filt/lp24.cpp"

using namespace barkComponents;

struct tpEQstatus : ParamQuantity {
	std::string getDisplayValueString() override {
		if (getValue() < 1.f)
			return "On";
		else
			return "Off";
	}
};

struct tpEQprocess : ParamQuantity {
	std::string getDisplayValueString() override {
		if (getValue() > 0.f)
			return "Listen";
		else
			return "Process";
	}
};

struct tpSwapLR : ParamQuantity {
	std::string getDisplayValueString() override {
		if (getValue() < 1.f)
			return "L/R";
		else
			return "R/L";
	}
};

struct tpGainVal : ParamQuantity {
	std::string getDisplayValueString() override {
		if (getValue() < 1.f)
			return "Pre";
		else
			return "Post";
	}
};

struct OneBand : Module {
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
	Biquad *parametricEQL = new Biquad();
	Biquad *parametricEQR = new Biquad();
	LadderFilter lpf24;		//smoothing

	OneBand() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		//Knob---
		configParam(EQGAIN_PARAM, -6.5f, 6.5f, 0.f, "Band Gain", "dB", params[EQGAIN_PARAM].getValue(), 5.f);
		configParam(EQFREQ_PARAM, .459435f, 10.f, 3.459432f, "Freq", " Hz", 2, 20);	// .01375f
		configParam(EQBANDWIDTH_PARAM, .1f, 40.f, 15.f, "Q Factor");
		configParam(OUTGAIN_PARAM, 0.f, 7.f, 2.f, "Output Gain", "dB", -10, 20, -13.f);	//needs diff offset, TODO: show 0dB by default? or meter dB?
		//Switch---
		configParam<tpEQstatus>(EQBYPASS_PARAM, 0.f, 1.f, 0.f, "EQ");
		configParam<tpGainVal>(PREPOST_PARAM, 0.f, 1.f, 0.f, "Meter", " Gain");
		configParam<tpSwapLR>(SWAPLR_PARAM, 0.f, 1.f, 0.f, "Output");
		configParam<tpEQprocess>(LISTEN_PARAM, 0.f, 1.f, 0.f, "EQ");
		//Lights---
		volUnitIndicatorPEAK.lambda = 1 / 0.1f;
		vuDivider.setDivision(16);
		lightDivider.setDivision(256);
	}

	void process(const ProcessArgs &args) override {
		bool Listen = params[LISTEN_PARAM].getValue(), swapLR = params[SWAPLR_PARAM].getValue();
		float inL = 0.0f, inR = 0.0f, outLdBVU = 0.0f, outRdBVU = 0.0f,
			outL = clamp(outputs[OUTL_OUTPUT].getVoltage(), -9.9f, 9.9f), outR = clamp(outputs[OUTR_OUTPUT].getVoltage(), -9.9f, 9.9f),
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
				volUnitIndicatorPEAK.process(args.sampleTime * vuDivider.getDivision(), inL / 10.f);
			} else if (params[PREPOST_PARAM].value == 1.0f && !inputs[INR_INPUT].active) {//post
				volUnitIndicatorPEAK.process(args.sampleTime * vuDivider.getDivision(), outLdBVU / 10.f);
			}
			//post EQ---
			if (params[PREPOST_PARAM].value == 0.0f && inputs[INR_INPUT].active) {
				//volUnitIndicatorPEAK.setValue(((inL / 10.f) + (inR / 10.f)) / 2.f);
				volUnitIndicatorPEAK.process(args.sampleTime * vuDivider.getDivision(), (inL / 10.f) + (inR / 10.f) / 2.f);
			} else if (params[PREPOST_PARAM].value == 1.0f && inputs[INR_INPUT].active) {
				//volUnitIndicatorPEAK.setValue(((outLdBVU / 10.f) + (outRdBVU / 10.f)) / 2.f);
				volUnitIndicatorPEAK.process(args.sampleTime * vuDivider.getDivision(), (outLdBVU / 10.f) + (outRdBVU / 10.f) / 2.f);
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
		parametricEQL->setBiquad(bq_type_peak, biquadFreq / sampRate, biquadQ, biquadGain);
		parametricEQR->setBiquad(bq_type_peak, biquadFreq / sampRate, biquadQ, biquadGain);
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

		int rackY = 380;
		float floatyMcFloatFace = 16.11f, lightXpos = 45.5f, offsetKnobs = 0.47f;

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
		addParam(createParam<BarkKnob26>(Vec(10.2f - offsetKnobs, rackY - 349.73f), module, OneBand::EQGAIN_PARAM));
		addParam(createParam<BarkKnob26>(Vec(24.95f - offsetKnobs, rackY - 291.2f), module, OneBand::EQFREQ_PARAM));
		addParam(createParam<BarkKnob26>(Vec(10.68f - offsetKnobs, rackY - 231.51f), module, OneBand::EQBANDWIDTH_PARAM));
		addParam(createParam<BarkKnob30b>(Vec(8.29f, rackY - 107.46f), module, OneBand::OUTGAIN_PARAM));
		//Switch---
		addParam(createParam<BarkSwitchSmall>(Vec(41.29f, rackY - 355.97f), module, OneBand::EQBYPASS_PARAM));
		addParam(createParam<BarkSwitchSmall>(Vec(11.26f, rackY - 136.57f), module, OneBand::PREPOST_PARAM));
		addParam(createParam<BarkSwitchSmallSide>(Vec(21.89f, rackY - 161.23f), module, OneBand::SWAPLR_PARAM));
		addParam(createParam<BarkSwitchSmall>(Vec(40.4f, rackY - 212.39f), module, OneBand::LISTEN_PARAM));
		//TODO: Screw Positions
		addChild(createWidget<BarkScrew1>(Vec(box.size.x - 13, 3)));			//pos2
		addChild(createWidget<BarkScrew2>(Vec(2, 367.2f)));						//pos3
		//Light---
		addChild(createLight<SmallerLightFA<ParamInLight>>(Vec(floatyMcFloatFace, rackY - 280.05f), module, OneBand::FreqParamOn));
		addChild(createLight<SmallerLightFA<ParamInLight>>(Vec(floatyMcFloatFace, rackY - 261.72f), module, OneBand::FreqParamOff));
		addChild(createLight<BiggerLight<clipLight>>(Vec(lightXpos - 1.f, rackY - 134.65f - 12.8f), module, OneBand::dBpeak_LIGHT + 0));
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
