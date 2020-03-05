#include "plugin.hpp"
#include "barkComponents.hpp"
#include "dependancies/dsp/simdLFO.hpp"

using namespace barkComponents;

struct TrimLFO : Module {
	enum ParamIds {
		OFFSET1_PARAM, 
		OFFSET2_PARAM,	
		OFFSET_PARAM, 
		INVERT_PARAM, 
		FREQ_PARAM, 
		FINE_PARAM, 
		FM1_PARAM, 
		FM2_PARAM, 
		PW_PARAM, 
		PWM_PARAM, 
		WAVEMIX_PARAM, 
		RESET_PARAM,	
		SETsin_PARAM,
		SETsaw_PARAM,
		SETtri_PARAM,
		SETsqr_PARAM,
		SETbi_PARAM,
		SETuni_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		FM1_INPUT, 
		FM2_INPUT, 
		RESET_INPUT, 
		PW_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT, 
		OUT2_OUTPUT, 
		OUT1a_OUTPUT, 
		OUT2a_OUTPUT, 
		SIN_OUTPUT, 
		TRI_OUTPUT, 
		SAW_OUTPUT, 
		SQR_OUTPUT, 
		trimLFO_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		PHASE_POS_LIGHT, 
		PHASE_NEG_LIGHT,
		NUM_LIGHTS
	};

	LowFrequencyOscillator<float_4> oscillators[4];
	dsp::ClockDivider lightDivider;
	float volts1 = 0.f;
	float volts2 = 0.f;
	float freqHz = 1.f;
	
	TrimLFO() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(FREQ_PARAM, -16.f, 4.f, 1.f, "Freq", " Hz", 2.f, 1.f * std::pow(2.f, params[FREQ_PARAM].getValue()));
		//TODO: fix the scale of finetune%
		configParam(FINE_PARAM, -0.06798301f, 0.06798301f, 0.f, "Fine Tune", "%", 0.f, 18.9702f, 0.f);
		// negatiuve value difference == 0.01645129630197%
		configParam(OFFSET1_PARAM, -10.f, 10.f, 0.f, "Offset 1", "v");
		configParam(OFFSET2_PARAM, -10.f, 10.f, 10.f, "Offset 2", "v");
		configParam(PW_PARAM, 0.01f, .99f, 0.5f, "Pulse Width", "%", 0.f, 100.f, -50.f);
		configParam(FM1_PARAM, 0.f, 1.f, 0.f, "Freq Mod 1", "%", 0.f, 100.f);
		configParam(FM2_PARAM, 0.f, 1.f, 0.f, "Freq Mod 2", "%", 0.f, 100.f);
		configParam(PWM_PARAM, 0.f, 1.f, 0.f, "Pulse Width Mod", "%", 0.f, 100.f);
		configParam<tpWave>(WAVEMIX_PARAM, 0.f, 3.f, 0.f, "Wave ");
		configParam<tpPolarVal>(OFFSET_PARAM, 0.f, 1.f, 1.f, "Type", "Polar");	
		configParam<tpPhaseVal>(INVERT_PARAM, 0.f, 1.f, 1.f, "Phase", "°");
		configParam(RESET_PARAM, 0.f, 1.f, 0.f, "Reset Phase");
		configParam(SETsin_PARAM, 0.f, 1.f, 0.f, "Top Output Sine");
		configParam(SETsaw_PARAM, 0.f, 1.f, 0.f, "Top Output Saw");
		configParam(SETtri_PARAM, 0.f, 1.f, 0.f, "Top Output Triangle");
		configParam(SETsqr_PARAM, 0.f, 1.f, 0.f, "Top Output Square");
		configParam(SETbi_PARAM, 0.f, 1.f, 0.f, "Set Offsets Bipolar");
		configParam(SETuni_PARAM, 0.f, 1.f, 0.f, "Set Offsets Unipolar");
		lightDivider.setDivision(256);
	}

	void process(const ProcessArgs &args) override {
		float pwKnob = params[PW_PARAM].getValue(), pwmKnob = params[PWM_PARAM].getValue();
		float out1a = simd::clamp(params[OFFSET1_PARAM].getValue(), -10.f, 10.f),
			out2a = simd::clamp(params[OFFSET2_PARAM].getValue(), -10.f, 10.f),
			fineTune = 4 * dsp::quadraticBipolar(params[FINE_PARAM].getValue());		//TODO: pow this
		float_4 sinValue, sawValue, triValue, sqrValue;

		if (outputs[OUT1a_OUTPUT].isConnected()) { outputs[OUT1a_OUTPUT].setVoltage(out1a); }
		if (outputs[OUT2a_OUTPUT].isConnected()) { outputs[OUT2a_OUTPUT].setVoltage(out2a); }

		//simdLFO
		for (int i = 0; i < 4; i += 4) {
			auto *oscillator = &oscillators[i];
			//frequency
			float_4 pitch = params[FREQ_PARAM].getValue() + fineTune;
			pitch += params[FM1_PARAM].getValue() * inputs[FM1_INPUT].getVoltageSimd<float_4>(i);
			pitch += params[FM2_PARAM].getValue() * inputs[FM2_INPUT].getVoltageSimd<float_4>(i);
			oscillator->setPitch(pitch);
			//pulse width
			float_4 pw = pwKnob + inputs[PW_INPUT].getVoltageSimd<float_4>(i) / 10.f * pwmKnob;
			oscillator->setPulseWidth(pw);

			oscillator->polarPat = (params[OFFSET_PARAM].getValue() < 1.f);
			oscillator->invert = (params[INVERT_PARAM].getValue() < 1.f);

			oscillator->step(args.sampleTime);

			float_4 resetPhase = params[RESET_PARAM].getValue();
			//setReset(internal, external)
			oscillator->setReset(resetPhase, inputs[RESET_INPUT].getVoltageSimd<float_4>(i));

			//initialise oscilators
			sinValue = 5.f * oscillator->sin();
			sawValue = 5.f * oscillator->saw();
			triValue = 5.f * oscillator->tri();
			sqrValue = 5.f * oscillator->sqr();

			float_4 out1 = simd::clamp(params[OFFSET1_PARAM].getValue(), -10.f, 10.f) + oscillator->sin();
			float_4	out2 = simd::clamp(params[OFFSET2_PARAM].getValue(), -10.f, 10.f) + oscillator->sqr();

			outputs[OUT1_OUTPUT].setVoltageSimd(out1, i);
			outputs[OUT2_OUTPUT].setVoltageSimd(out2, i);
			///LFO outputs----
			if (outputs[SIN_OUTPUT].isConnected()) { outputs[SIN_OUTPUT].setVoltageSimd(sinValue, i); }
			if (outputs[SAW_OUTPUT].isConnected()) { outputs[SAW_OUTPUT].setVoltageSimd(sawValue, i); }
			if (outputs[TRI_OUTPUT].isConnected()) { outputs[TRI_OUTPUT].setVoltageSimd(triValue, i); }
			if (outputs[SQR_OUTPUT].isConnected()) { outputs[SQR_OUTPUT].setVoltageSimd(sqrValue, i); }
			///TRIM LFO output----
			float wavemixParamVal = params[WAVEMIX_PARAM].getValue();
			float_4 waveMixParam = simd::clamp(params[WAVEMIX_PARAM].getValue(), 0.f, 2.99999f), xFade;
			///sin.saw----
			if (wavemixParamVal < 1.f) {// 0.0f sin
				xFade = simd::crossfade(sinValue, sawValue, waveMixParam);
				outputs[trimLFO_OUTPUT].setVoltageSimd(simd::fmax(out1a, simd::fmin(out2a, xFade)), i);
			}
			///saw.tri----
			else if (wavemixParamVal < 2.f) {	//1.0f saw
				xFade = simd::crossfade(sawValue, triValue, waveMixParam - 1.f);
				outputs[trimLFO_OUTPUT].setVoltageSimd(simd::fmax(out1a, simd::fmin(out2a, xFade)), i);
			}
			///tri.sqr----
			else if (wavemixParamVal < 3.f) { //2.0f tri
				xFade = simd::crossfade(triValue, sqrValue, waveMixParam - 2.f);//2.f
				outputs[trimLFO_OUTPUT].setVoltageSimd(simd::fmax(out1a, simd::fmin(out2a, xFade)), i);
			}
			///sqr----
			else if (wavemixParamVal == 3.f) { //3.0f sqr
				outputs[trimLFO_OUTPUT].setVoltageSimd(simd::fmax(out1a, simd::fmin(out2a, xFade)), i);
			}
		}
		//----------------	DISPLAY Hz	--------------------------
		float frq = params[FREQ_PARAM].getValue() + params[FM1_PARAM].getValue() * inputs[FM1_INPUT].getVoltage() +
			params[FM2_PARAM].getValue() * inputs[FM2_INPUT].getVoltage(),
			fine = fineTune;
		volts1 = simd::clamp(params[OFFSET1_PARAM].getValue(), -10.f, 10.f);
		volts2 = simd::clamp(params[OFFSET2_PARAM].getValue(), -10.f, 10.f);
		frq = simd::clamp(frq, -16.f, 16.f);
		freqHz = 1.f * simd::pow(2.f, frq + fine);
		//----------------	DISPLAY Hz	--------------------------

		//set param on click
		if (params[SETsin_PARAM].getValue() > 0.f) {
			params[WAVEMIX_PARAM].setValue(0.f);
		}
		if (params[SETsaw_PARAM].getValue() > 0.f) {
			params[WAVEMIX_PARAM].setValue(1.f);
		}
		if (params[SETtri_PARAM].getValue() > 0.f) {
			params[WAVEMIX_PARAM].setValue(2.f);
		}
		if (params[SETsqr_PARAM].getValue() > 0.f) {
			params[WAVEMIX_PARAM].setValue(2.99999f);
		}

		//set Trim params to uni/bi
		if (params[SETbi_PARAM].getValue() > 0.f) {
			params[OFFSET1_PARAM].setValue(-5.f);
			params[OFFSET2_PARAM].setValue(5.f);
			params[OFFSET_PARAM].setValue(0.f);
		} else if (params[SETuni_PARAM].getValue() > 0.f) {
			params[OFFSET1_PARAM].setValue(0.f);
			params[OFFSET2_PARAM].setValue(10.f);
			params[OFFSET_PARAM].setValue(1.f);
		}

		if (lightDivider.process()) {
			bool isUni = params[OFFSET_PARAM].getValue();
			if (isUni) {
				//lights[PHASE_POS_LIGHT].setSmoothBrightness(oscillatorLight, args.sampleTime * lightDivider.getDivision());
				lights[PHASE_POS_LIGHT].setBrightness(outputs[trimLFO_OUTPUT].getVoltage() / 10);
			} else {
				//lights[PHASE_POS_LIGHT].setSmoothBrightness(oscillatorLight, args.sampleTime * lightDivider.getDivision());
				lights[PHASE_POS_LIGHT].setBrightness(outputs[trimLFO_OUTPUT].getVoltage() / 5);
				//lights[PHASE_NEG_LIGHT].setSmoothBrightness(-oscillatorLight, args.sampleTime * lightDivider.getDivision());
				lights[PHASE_NEG_LIGHT].setBrightness(-outputs[trimLFO_OUTPUT].getVoltage() / 5);
			}
		}
		
		
	}
};
////---------------------------------------------------------------------------------------------------------------------------
struct FreqDisplayWidget : TransparentWidget {
	float *freqHz;
	std::shared_ptr<Font> font;

	FreqDisplayWidget() {
		font = FONT;
	}

	void draw(const DrawArgs &freqDisp) override {
		constexpr float spacer = 40.f;
		NVGcolor backgroundColor = nvgRGB(26, 26, 36);		//CreamyRed 97, 54, 57
		NVGcolor borderColor = nvgRGB(0, 0, 0);
		NVGcolor gradStartCol = nvgRGBA(255, 255, 244, 17);
		NVGcolor gradEndCol = nvgRGBA(0, 0, 0, 15);
		NVGcolor textColor = nvgRGB(63, 154, 0);
		nvgBeginPath(freqDisp.vg);
		nvgRoundedRect(freqDisp.vg, 0.0, 0.0, box.size.x, box.size.y, 0.75);
		nvgFillColor(freqDisp.vg, backgroundColor);
		nvgFill(freqDisp.vg);
		nvgStrokeWidth(freqDisp.vg, 0.75);
		nvgStrokeColor(freqDisp.vg, borderColor);
		nvgStroke(freqDisp.vg);
		nvgTextAlign(freqDisp.vg, 1 << 1);
		nvgFontSize(freqDisp.vg, FONT_SIZE);
		nvgFontFaceId(freqDisp.vg, font->handle);
		nvgTextLetterSpacing(freqDisp.vg, LETTER_SPACING/* / 10*/);
		Vec textPos = Vec(85.798f / 2.0f - 8.0f, TEXT_POS_Y);
		//----- "Hz"
		nvgFillColor(freqDisp.vg, nvgTransRGBA(textColor, 255));
		char display_stringHz[11];
		snprintf(display_stringHz, sizeof(display_stringHz), "%0.7f", *freqHz);
		nvgText(freqDisp.vg, textPos.x + spacer, textPos.y, "Hz", NULL);
		nvgText(freqDisp.vg, textPos.x, textPos.y, display_stringHz, NULL);		
		nvgFillColor(freqDisp.vg, textColor);
		//---------Gradient Screen
		nvgRoundedRect(freqDisp.vg, 0.f, 0.f, box.size.x, box.size.y, .75f);
		//(startX,startY)-(endX,endY) should be the reverse of inkscape coordinates 
		float gradHeight = 13.673f;
		nvgFillPaint(freqDisp.vg, nvgLinearGradient(freqDisp.vg, 71.5f, gradHeight - 4.98f, 70.61f, gradHeight - 10.11f, gradStartCol, gradEndCol));
		nvgFill(freqDisp.vg);
	}
};
///-------------------------------------------------------------------------------------------------------------------------------------
struct VoltsDisplayWidget : TransparentWidget {
	TrimLFO *module;
	float *value;
	std::shared_ptr<Font> font;

	VoltsDisplayWidget() {
		font = FONT;
	}

	void draw(const DrawArgs &voltDisp) override {
		NVGcolor backgroundColor = nvgRGB(26, 26, 36);
		NVGcolor borderColor = nvgRGB(0, 0, 0);
		NVGcolor gradStartCol = nvgRGBA(255, 255, 244, 17);
		NVGcolor gradEndCol = nvgRGBA(0, 0, 0, 15);
		NVGcolor textColor = nvgRGB(63, 154, 0);
		nvgBeginPath(voltDisp.vg);
		nvgRoundedRect(voltDisp.vg, 0.0, 0.0, box.size.x, box.size.y, 0.75);
		nvgFillColor(voltDisp.vg, backgroundColor);
		nvgFill(voltDisp.vg);
		nvgStrokeWidth(voltDisp.vg, 0.75);
		nvgStrokeColor(voltDisp.vg, borderColor);
		nvgStroke(voltDisp.vg);
		nvgTextAlign(voltDisp.vg, 1 << 1);
		nvgFontSize(voltDisp.vg, FONT_SIZE);
		nvgFontFaceId(voltDisp.vg, font->handle);
		nvgTextLetterSpacing(voltDisp.vg, LETTER_SPACING);
		char display_string[8];
		sprintf(display_string, "%0.4f", *value);
		Vec textPos = Vec(25.364f, TEXT_POS_Y);
		nvgFillColor(voltDisp.vg, nvgTransRGBA(nvgRGB(0xdf, 0xd2, 0x2c), 16));
		nvgText(voltDisp.vg, textPos.x, textPos.y, "$$$$", NULL);
		nvgFillColor(voltDisp.vg, nvgTransRGBA(nvgRGB(0xda, 0xe9, 0x29), 11));
		nvgText(voltDisp.vg, textPos.x, textPos.y, "##.##", NULL);
		nvgFillColor(voltDisp.vg, textColor);
		nvgText(voltDisp.vg, textPos.x, textPos.y, display_string, NULL);
		//---------Gradient Screen
		nvgRoundedRect(voltDisp.vg, 0.f, 0.f, box.size.x, box.size.y, .75f);
		//(startX,startY)-(endX,endY) should be the reverse of inkscape coordinates 
		float gradHeight = 12.728f;
		nvgFillPaint(voltDisp.vg, nvgLinearGradient(voltDisp.vg, 71.5f, gradHeight - 4.98f, 70.61f, gradHeight - 10.11f, gradStartCol, gradEndCol));
		nvgFill(voltDisp.vg);
	}
};
////---------------------------------------------------------------------------------------------------------------------------
struct TrimLFOWidget : ModuleWidget {
	TrimLFOWidget(TrimLFO *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BarkTrimLFO.svg")));
		
		///Ports---
		//Out---
		addOutput(createOutput<BarkOutPort350>(Vec(13.28f, rackY - 52.35f), module, TrimLFO::SIN_OUTPUT));
		addOutput(createOutput<BarkOutPort350>(Vec(46.58f, rackY - 52.35f), module, TrimLFO::SAW_OUTPUT));
		addOutput(createOutput<BarkOutPort350>(Vec(79.68f, rackY - 52.35f), module, TrimLFO::TRI_OUTPUT));
		addOutput(createOutput<BarkOutPort350>(Vec(113.245f, rackY - 52.35f), module, TrimLFO::SQR_OUTPUT));
		addOutput(createOutput<BarkOutPort350>(Vec(14.57f, rackY - 275.08f), module, TrimLFO::OUT1_OUTPUT));	
		addOutput(createOutput<BarkOutPort350>(Vec(112.09f, rackY - 275.08f), module, TrimLFO::OUT2_OUTPUT));
		addOutput(createOutput<BarkOutPort350>(Vec(42.11f + 0.35f, rackY - 275.08f), module, TrimLFO::OUT1a_OUTPUT));
		addOutput(createOutput<BarkOutPort350>(Vec(84.18f, rackY - 275.08f), module, TrimLFO::OUT2a_OUTPUT));
		addOutput(createOutput<BarkPatchPortOut>(Vec(63.35f, rackY - 332.02f), module, TrimLFO::trimLFO_OUTPUT));
		//In---
		addInput(createInput<BarkInPort350>(Vec(22.7f, rackY - 82.70f), module, TrimLFO::FM1_INPUT));
		addInput(createInput<BarkInPort350>(Vec(63.f, rackY - 82.70f), module, TrimLFO::FM2_INPUT));
		addInput(createInput<BarkInPort350>(Vec(103.3f, rackY - 82.70f), module, TrimLFO::PW_INPUT));
		addInput(createInput<BarkInPort350>(Vec(119.89f, rackY - 164.05f), module, TrimLFO::RESET_INPUT));
		//Knobs---
		addParam(createParam<BarkKnob_60>(Vec(45.12f, rackY - 217.87f), module, TrimLFO::FREQ_PARAM));
		addParam(createParam<BarkScrew01>(Vec(box.size.x - 13, 367.2f), module, TrimLFO::FINE_PARAM));
		addParam(createParam<BarkKnob_40>(Vec(20.38f, rackY - 329.78f), module, TrimLFO::OFFSET1_PARAM));
		addParam(createParam<BarkKnob_40>(Vec(89.6f, rackY - 329.78f), module, TrimLFO::OFFSET2_PARAM));
		addParam(createParam<BarkKnob_30>(Vec(4.08f, rackY - 170.f), module, TrimLFO::PW_PARAM));
		addParam(createParam<BarkKnob_30>(Vec(20.31f, rackY - 124.64f), module, TrimLFO::FM1_PARAM));
		addParam(createParam<BarkKnob_30>(Vec(60.499f, rackY - 124.64f), module, TrimLFO::FM2_PARAM));
		addParam(createParam<BarkKnob_30>(Vec(100.909f, rackY - 124.64f), module, TrimLFO::PWM_PARAM));
		addParam(createParam<BarkSlide1>(Vec(25.41f, rackY - 57.f), module, TrimLFO::WAVEMIX_PARAM));
		//Switch---
		addParam(createParam<BarkSwitch>(Vec(8.67f, rackY - 217.06f), module, TrimLFO::OFFSET_PARAM));
		addParam(createParam<BarkSwitch>(Vec(117.57f, rackY - 217.06f), module, TrimLFO::INVERT_PARAM));
		///quick access / hidden params
		addParam(createParam<BarkButton1>(Vec(121.54f, rackY - 140.91f), module, TrimLFO::RESET_PARAM));
		addParam(createParam<BarkButton1>(Vec(14.91f, rackY - 31.2f), module, TrimLFO::SETsin_PARAM));
		addParam(createParam<BarkButton1>(Vec(48.21f, rackY - 31.2f), module, TrimLFO::SETsaw_PARAM));
		addParam(createParam<BarkButton1>(Vec(81.52f, rackY - 31.2f), module, TrimLFO::SETtri_PARAM));
		addParam(createParam<BarkButton1>(Vec(114.91f, rackY - 31.2f), module, TrimLFO::SETsqr_PARAM));
		addParam(createParam<BarkButton1>(Vec(10.55f, rackY - 191.09f), module, TrimLFO::SETbi_PARAM));
		addParam(createParam<BarkButton1>(Vec(10.55f, rackY - 228.33f), module, TrimLFO::SETuni_PARAM));
		//Screw---
		addChild(createWidget<BarkScrew3>(Vec(2, 3)));		//pos1
		//Light---
		addChild(createLight<LessBigLight<greenRedLight>>(Vec(71.87f, rackY - 152.63f), module, TrimLFO::PHASE_POS_LIGHT));
		//------------------------------
		//if not NULL i.e. in the browser don't draw display's
		if (module != NULL) {
			VoltsDisplayWidget *display1 = createWidget<VoltsDisplayWidget>(Vec(15.009f, 31.05f));
			display1->box.size = Vec(50.728f, 13.152f);
			display1->value = &module->volts1;
			addChild(display1);
			VoltsDisplayWidget *display2 = createWidget<VoltsDisplayWidget>(Vec(84.228f, 31.05f));
			display2->box.size = Vec(50.728f, 13.152f);
			display2->value = &module->volts2;
			addChild(display2);
		////------------------------------
			FreqDisplayWidget *display3 = createWidget<FreqDisplayWidget>(Vec(32.23f, 237.31f));
			display3->box.size = Vec(85.798f, 13.673f);
			display3->freqHz = &module->freqHz;
			addChild(display3);
		////------------------------------
		}

	}

};

Model *modelTrimLFO = createModel<TrimLFO, TrimLFOWidget>("TrimLFO");
//**-------------------------------------------------------------------------------------------------------------------------------------------------
//-----bpmTrimLFO------------------------------------------------------------------------------------------------------------------------------------
//**-------------------------------------------------------------------------------------------------------------------------------------------------
struct bpmTrimLFO : Module {

	enum ParamIds {
		OFFSET1_PARAM,
		OFFSET2_PARAM,
		OFFSET_PARAM,
		INVERT_PARAM,
		FREQ_PARAM,
		FINE_PARAM,
		BPM_PARAM,
		FM1_PARAM,
		FM2_PARAM,
		PW_PARAM,
		PWM_PARAM,
		WAVEMIX_PARAM,
		RESET_PARAM,
		SETsin_PARAM,
		SETsaw_PARAM,
		SETtri_PARAM,
		SETsqr_PARAM,
		SETbi_PARAM,
		SETuni_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		FM1_INPUT,
		FM2_INPUT,
		RESET_INPUT,
		PW_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		OUT1a_OUTPUT,
		OUT2a_OUTPUT,
		SIN_OUTPUT,
		TRI_OUTPUT,
		SAW_OUTPUT,
		SQR_OUTPUT,
		trimLFO_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		PHASE_POS_LIGHT,
		PHASE_NEG_LIGHT,
		NUM_LIGHTS
	};

	LowFrequencyOscillator<float_4> oscillators[4];
	dsp::ClockDivider lightDivider;
	float volts1bpm = 0.f;
	float volts2bpm = 0.f;
	float freqHz = 1.f;

	bpmTrimLFO() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(FREQ_PARAM, -16.f, 4.f, 1.f, "Freq", " BPM", 2.f, 1.f * simd::pow(2.f, params[FREQ_PARAM].getValue()) * 60);
		configParam(FINE_PARAM, -0.06798301f, 0.06798301f, 0.f, "Fine Tune", "%", 0.f, 18.9702f, 0.f);
		configParam(BPM_PARAM, -1.0136f / 2, 0.952f / 2, 0.f, "Fine Tune", "%", 0.f, 100.f, 0.f);
		configParam(OFFSET1_PARAM, -10.f, 10.f, 0.f, "Offset 1", "v");
		configParam(OFFSET2_PARAM, -10.f, 10.f, 10.f, "Offset 2", "v");
		configParam(PW_PARAM, 0.01f, .99f, 0.5f, "Pulse Width", "%", 0.f, 100.f, -50.f);
		configParam(FM1_PARAM, 0.f, 1.f, 0.f, "Freq Mod 1", "%", 0.f, 100.f);
		configParam(FM2_PARAM, 0.f, 1.f, 0.f, "Freq Mod 2", "%", 0.f, 100.f);
		configParam(PWM_PARAM, 0.f, 1.f, 0.f, "Pulse Width Mod", "%", 0.f, 100.f);
		configParam<tpWave>(WAVEMIX_PARAM, 0.f, 2.99999f, 0.f, "Wave ");
		configParam<tpPolarVal>(OFFSET_PARAM, 0.f, 1.f, 1.f, "Type", "Polar");
		configParam<tpPhaseVal>(INVERT_PARAM, 0.f, 1.f, 1.f, "Phase", "°");
		configParam(RESET_PARAM, 0.f, 1.f, 0.f, "Reset Phase");
		configParam(SETsin_PARAM, 0.f, 1.f, 0.f, "Top Output Sine");
		configParam(SETsaw_PARAM, 0.f, 1.f, 0.f, "Top Output Saw");
		configParam(SETtri_PARAM, 0.f, 1.f, 0.f, "Top Output Triangle");
		configParam(SETsqr_PARAM, 0.f, 1.f, 0.f, "Top Output Square");
		configParam(SETbi_PARAM, 0.f, 1.f, 0.f, "Set Offsets Bipolar");
		configParam(SETuni_PARAM, 0.f, 1.f, 0.f, "Set Offsets Unipolar");
		lightDivider.setDivision(256);
	}

	void process(const ProcessArgs &args) override {		
		float pwKnob = params[PW_PARAM].getValue(), pwmKnob = params[PWM_PARAM].getValue();
		float out1a = simd::clamp(params[OFFSET1_PARAM].getValue(), -10.f, 10.f),
			out2a = simd::clamp(params[OFFSET2_PARAM].getValue(), -10.f, 10.f),
			fineTune = 4.f * dsp::quadraticBipolar(params[FINE_PARAM].getValue()) + 
			3.f * dsp::quadraticBipolar(params[BPM_PARAM].getValue());		//TODO: pow this
		float_4 sinValue, sawValue, triValue, sqrValue;

		if (outputs[OUT1a_OUTPUT].isConnected()) { outputs[OUT1a_OUTPUT].setVoltage(out1a); }
		if (outputs[OUT2a_OUTPUT].isConnected()) { outputs[OUT2a_OUTPUT].setVoltage(out2a); }

		//simdLFO
		for (int i = 0; i < 4; i += 4) {
			auto *oscillator = &oscillators[i];
			//frequency
			float_4 pitch = params[FREQ_PARAM].getValue() + fineTune;
			pitch += params[FM1_PARAM].getValue() * inputs[FM1_INPUT].getVoltageSimd<float_4>(i);
			pitch += params[FM2_PARAM].getValue() * inputs[FM2_INPUT].getVoltageSimd<float_4>(i);
			oscillator->setPitch(pitch);
			//pulse width
			float_4 pw = pwKnob + inputs[PW_INPUT].getVoltageSimd<float_4>(i) / 10.f * pwmKnob;
			oscillator->setPulseWidth(pw);

			oscillator->polarPat = (params[OFFSET_PARAM].getValue() < 1.f);
			oscillator->invert = (params[INVERT_PARAM].getValue() < 1.f);

			oscillator->step(args.sampleTime);

			float_4 resetPhase = params[RESET_PARAM].getValue();
			//setReset(internal, external)
			oscillator->setReset(resetPhase, inputs[RESET_INPUT].getVoltageSimd<float_4>(i));

			//initialise oscilators
			sinValue = 5.f * oscillator->sin();
			sawValue = 5.f * oscillator->saw();
			triValue = 5.f * oscillator->tri();
			sqrValue = 5.f * oscillator->sqr();

			float_4 out1 = simd::clamp(params[OFFSET1_PARAM].getValue(), -10.f, 10.f) + oscillator->sin();
			float_4	out2 = simd::clamp(params[OFFSET2_PARAM].getValue(), -10.f, 10.f) + oscillator->sqr();

			outputs[OUT1_OUTPUT].setVoltageSimd(out1, i);
			outputs[OUT2_OUTPUT].setVoltageSimd(out2, i);
			///LFO outputs----
			if (outputs[SIN_OUTPUT].isConnected()) { outputs[SIN_OUTPUT].setVoltageSimd(sinValue, i); }
			if (outputs[SAW_OUTPUT].isConnected()) { outputs[SAW_OUTPUT].setVoltageSimd(sawValue, i); }
			if (outputs[TRI_OUTPUT].isConnected()) { outputs[TRI_OUTPUT].setVoltageSimd(triValue, i); }
			if (outputs[SQR_OUTPUT].isConnected()) { outputs[SQR_OUTPUT].setVoltageSimd(sqrValue, i); }
			///TRIM LFO output----
			float wavemixParamVal = params[WAVEMIX_PARAM].getValue();
			float_4 waveMixParam = simd::clamp(params[WAVEMIX_PARAM].getValue(), 0.f, 2.99999f), xFade;
			///sin.saw----
			if (wavemixParamVal < 1.f) {// 0.0f sin
				xFade = simd::crossfade(sinValue, sawValue, waveMixParam);
				outputs[trimLFO_OUTPUT].setVoltageSimd(simd::fmax(out1a, simd::fmin(out2a, xFade)), i);
			}
			///saw.tri----
			else if (wavemixParamVal < 2.f) {	//1.0f saw
				xFade = simd::crossfade(sawValue, triValue, waveMixParam - 1.f);
				outputs[trimLFO_OUTPUT].setVoltageSimd(simd::fmax(out1a, simd::fmin(out2a, xFade)), i);
			}
			///tri.sqr----
			else if (wavemixParamVal < 3.f) { //2.0f tri
				xFade = simd::crossfade(triValue, sqrValue, waveMixParam - 2.f);//2.f
				outputs[trimLFO_OUTPUT].setVoltageSimd(simd::fmax(out1a, simd::fmin(out2a, xFade)), i);
			}
			///sqr----
			else if (wavemixParamVal == 3.f) { //3.0f sqr
				outputs[trimLFO_OUTPUT].setVoltageSimd(simd::fmax(out1a, simd::fmin(out2a, xFade)), i);
			}
		}
		//----------------	DISPLAY BPM	--------------------------
		float frq = params[FREQ_PARAM].getValue() + params[FM1_PARAM].getValue() * inputs[FM1_INPUT].getVoltage() +
			params[FM2_PARAM].getValue() * inputs[FM2_INPUT].getVoltage(),
			fine = fineTune;
		volts1bpm = simd::clamp(params[OFFSET1_PARAM].getValue(), -10.f, 10.f);
		volts2bpm = simd::clamp(params[OFFSET2_PARAM].getValue(), -10.f, 10.f);
		frq = simd::clamp(frq, -16.f, 16.f);
		freqHz = 1.f * simd::pow(2.f, frq + fine);
		//----------------	DISPLAY BPM	--------------------------

		//set param on click
		if (params[SETsin_PARAM].getValue() > 0.f) {
			params[WAVEMIX_PARAM].setValue(0.f);
		}
		if (params[SETsaw_PARAM].getValue() > 0.f) {
			params[WAVEMIX_PARAM].setValue(1.f);
		}
		if (params[SETtri_PARAM].getValue() > 0.f) {
			params[WAVEMIX_PARAM].setValue(2.f);
		}
		if (params[SETsqr_PARAM].getValue() > 0.f) {
			params[WAVEMIX_PARAM].setValue(2.99999f);
		}

		//set Trim params to uni/bi
		if (params[SETbi_PARAM].getValue() > 0.f) {
			params[OFFSET1_PARAM].setValue(-5.f);
			params[OFFSET2_PARAM].setValue(5.f);
			params[OFFSET_PARAM].setValue(0.f);
		} else if (params[SETuni_PARAM].getValue() > 0.f) {
			params[OFFSET1_PARAM].setValue(0.f);
			params[OFFSET2_PARAM].setValue(10.f);
			params[OFFSET_PARAM].setValue(1.f);
		}


		if (lightDivider.process()) {
			bool isUni = params[OFFSET_PARAM].getValue();
			if (isUni) {
				//lights[PHASE_POS_LIGHT].setSmoothBrightness(oscillatorLight, args.sampleTime * lightDivider.getDivision());
				lights[PHASE_POS_LIGHT].setBrightness(outputs[trimLFO_OUTPUT].getVoltage() / 10);
			} else {
				//lights[PHASE_POS_LIGHT].setSmoothBrightness(oscillatorLight, args.sampleTime * lightDivider.getDivision());
				lights[PHASE_POS_LIGHT].setBrightness(outputs[trimLFO_OUTPUT].getVoltage() / 5);
				//lights[PHASE_NEG_LIGHT].setSmoothBrightness(-oscillatorLight, args.sampleTime * lightDivider.getDivision());
				lights[PHASE_NEG_LIGHT].setBrightness(-outputs[trimLFO_OUTPUT].getVoltage() / 5);
			}
		}
		
	}
};

////---------------------------------------------------------------------------------------------------------------------------
struct bpmFreqDisplayWidget : TransparentWidget {
	float *freqHz;
	std::shared_ptr<Font> font;

	bpmFreqDisplayWidget() {
		font = FONT;
	}

	void draw(const DrawArgs &freqDisp) override {
		constexpr float spacer = 40.f;
		NVGcolor backgroundColor = nvgRGB(26, 26, 36);
		NVGcolor borderColor = nvgRGB(0, 0, 0);
		NVGcolor gradStartCol = nvgRGBA(255, 255, 244, 17);
		NVGcolor gradEndCol = nvgRGBA(0, 0, 0, 15);
		NVGcolor textColor = nvgRGB(63, 154, 0);
		nvgBeginPath(freqDisp.vg);
		nvgRoundedRect(freqDisp.vg, 0.f, 0.f, box.size.x, box.size.y, .75f);
		nvgFillColor(freqDisp.vg, backgroundColor);
		nvgFill(freqDisp.vg);
		nvgStrokeWidth(freqDisp.vg, .75f);
		nvgStrokeColor(freqDisp.vg, borderColor);
		nvgStroke(freqDisp.vg);
		nvgTextAlign(freqDisp.vg, NVG_ALIGN_CENTER);
		nvgFontSize(freqDisp.vg, FONT_SIZE);
		nvgFontFaceId(freqDisp.vg, font->handle);
		nvgTextLetterSpacing(freqDisp.vg, LETTER_SPACING);
		Vec textPos = Vec(85.798f / 2.f - 8.f, TEXT_POS_Y);
		//----- "BPM"
		nvgFillColor(freqDisp.vg, nvgTransRGBA(textColor, 255));
		char display_stringBPM[11];
		snprintf(display_stringBPM, sizeof(display_stringBPM), "%0.5f", (*freqHz) * 60);
		nvgText(freqDisp.vg, textPos.x - 5, textPos.y, display_stringBPM, NULL);
		nvgTextLetterSpacing(freqDisp.vg, LETTER_SPACING * .2f);
		nvgText(freqDisp.vg, textPos.x + spacer - 8.5f, textPos.y, "BP", NULL);
		nvgText(freqDisp.vg, textPos.x + spacer + 2.3f, textPos.y, "M", NULL);
		//textColor = nvgRGB(68, 255, 78);
		nvgFillColor(freqDisp.vg, textColor);
		//---------Gradient Screen
		nvgRoundedRect(freqDisp.vg, 0.f, 0.f, box.size.x, box.size.y, .75f);
		//(startX,startY)-(endX,endY) should be the reverse of inkscape coordinates 
		float gradHeight = 12.728f;
		nvgFillPaint(freqDisp.vg, nvgLinearGradient(freqDisp.vg, 71.5f, gradHeight - 4.98f, 70.61f, gradHeight - 10.11f, gradStartCol, gradEndCol));
		nvgFill(freqDisp.vg);

	}
};
//-------------------------------------------------------------------------------------------------------------------------------------
struct bpmVoltsDisplayWidget : TransparentWidget {
	bpmTrimLFO *module;
	float *value;
	std::shared_ptr<Font> font;

	bpmVoltsDisplayWidget() {
		font = FONT;
	}

	void draw(const DrawArgs &voltDisp) override {
		NVGcolor backgroundColor = nvgRGB(26, 26, 36);
		NVGcolor borderColor = nvgRGB(0, 0, 0);
		NVGcolor gradStartCol = nvgRGBA(255, 255, 244, 17);
		NVGcolor gradEndCol = nvgRGBA(0, 0, 0, 15);
		NVGcolor textColor = nvgRGB(63, 154, 0);
		nvgBeginPath(voltDisp.vg);
		nvgRoundedRect(voltDisp.vg, 0.0, 0.0, box.size.x, box.size.y, 0.75);
		nvgFillColor(voltDisp.vg, backgroundColor);
		nvgFill(voltDisp.vg);
		nvgStrokeWidth(voltDisp.vg, 0.75);
		nvgStrokeColor(voltDisp.vg, borderColor);
		nvgStroke(voltDisp.vg);
		nvgTextAlign(voltDisp.vg, 1 << 1);
		nvgFontSize(voltDisp.vg, FONT_SIZE);
		nvgFontFaceId(voltDisp.vg, font->handle);
		nvgTextLetterSpacing(voltDisp.vg, LETTER_SPACING);
		char display_string[8];
		sprintf(display_string, "%0.4f", *value);
		Vec textPos = Vec(25.364f, TEXT_POS_Y);		//		box.size = Vec(50.728f, 13.152f);
		nvgFillColor(voltDisp.vg, nvgTransRGBA(nvgRGB(0xdf, 0xd2, 0x2c), 16));
		nvgText(voltDisp.vg, textPos.x, textPos.y, "$$$$", NULL);
		nvgFillColor(voltDisp.vg, nvgTransRGBA(nvgRGB(0xda, 0xe9, 0x29), 11));
		nvgText(voltDisp.vg, textPos.x, textPos.y, "##.##", NULL);
		nvgFillColor(voltDisp.vg, textColor);
		nvgText(voltDisp.vg, textPos.x, textPos.y, display_string, NULL);
		//---------Gradient Screen
		nvgRoundedRect(voltDisp.vg, 0.f, 0.f, box.size.x, box.size.y, .75f);
		//(startX,startY)-(endX,endY) should be the reverse of inkscape coordinates 
		float gradHeight = 12.728f;
		nvgFillPaint(voltDisp.vg, nvgLinearGradient(voltDisp.vg, 71.5f, gradHeight - 4.98f, 70.61f, gradHeight - 10.11f, gradStartCol, gradEndCol));
		nvgFill(voltDisp.vg);
	}
};
////---------------------------------------------------------------------------------------------------------------------------
struct bpmTrimLFOWidget : ModuleWidget {
	bpmTrimLFOWidget(bpmTrimLFO *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BarkTrimLFObpm.svg")));
		
		///Ports---
		//Out---
		addOutput(createOutput<BarkOutPort350>(Vec(13.28f, rackY - 52.35f), module, bpmTrimLFO::SIN_OUTPUT));
		addOutput(createOutput<BarkOutPort350>(Vec(46.58f, rackY - 52.35f), module, bpmTrimLFO::SAW_OUTPUT));
		addOutput(createOutput<BarkOutPort350>(Vec(79.68f, rackY - 52.35f), module, bpmTrimLFO::TRI_OUTPUT));
		addOutput(createOutput<BarkOutPort350>(Vec(113.245f, rackY - 52.35f), module, bpmTrimLFO::SQR_OUTPUT));
		addOutput(createOutput<BarkOutPort350>(Vec(14.57f, rackY - 275.08f), module, bpmTrimLFO::OUT1_OUTPUT));
		addOutput(createOutput<BarkOutPort350>(Vec(112.09f, rackY - 275.08f), module, bpmTrimLFO::OUT2_OUTPUT));	
		addOutput(createOutput<BarkOutPort350>(Vec(42.11f + 0.35f, rackY - 275.08f), module, bpmTrimLFO::OUT1a_OUTPUT));	
		addOutput(createOutput<BarkOutPort350>(Vec(84.18f, rackY - 275.08f), module, bpmTrimLFO::OUT2a_OUTPUT));	
		addOutput(createOutput<BarkPatchPortOut>(Vec(63.35f, rackY - 332.02f), module, bpmTrimLFO::trimLFO_OUTPUT));	
		//In---
		addInput(createInput<BarkInPort350>(Vec(22.7f, rackY - 82.70f), module, bpmTrimLFO::FM1_INPUT));
		addInput(createInput<BarkInPort350>(Vec(63.f, rackY - 82.70f), module, bpmTrimLFO::FM2_INPUT));
		addInput(createInput<BarkInPort350>(Vec(103.3f, rackY - 82.70f), module, bpmTrimLFO::PW_INPUT));
		addInput(createInput<BarkInPort350>(Vec(119.89f, rackY - 164.05f), module, bpmTrimLFO::RESET_INPUT));
		//Knobs---
		addParam(createParam<BarkKnob_60snap>(Vec(45.12f, rackY - 217.87f), module, bpmTrimLFO::FREQ_PARAM));
		addParam(createParam<BarkScrew01>(Vec(box.size.x - 13, 367.2f), module, bpmTrimLFO::FINE_PARAM));
		addParam(createParam<BarkScrew02>(Vec(2, 3), module, bpmTrimLFO::BPM_PARAM));
		addParam(createParam<BarkKnob_40>(Vec(20.38f, rackY - 329.78f), module, bpmTrimLFO::OFFSET1_PARAM));
		addParam(createParam<BarkKnob_40>(Vec(89.6f, rackY - 329.78f), module, bpmTrimLFO::OFFSET2_PARAM));
		addParam(createParam<BarkKnob_30>(Vec(4.08f, rackY - 170.f), module, bpmTrimLFO::PW_PARAM));
		addParam(createParam<BarkKnob_30>(Vec(20.31f, rackY - 124.64f), module, bpmTrimLFO::FM1_PARAM));
		addParam(createParam<BarkKnob_30>(Vec(60.499f, rackY - 124.64f), module, bpmTrimLFO::FM2_PARAM));
		addParam(createParam<BarkKnob_30>(Vec(100.909f, rackY - 124.64f), module, bpmTrimLFO::PWM_PARAM));
		addParam(createParam<BarkSlide1>(Vec(25.41f, rackY - 57.f), module, bpmTrimLFO::WAVEMIX_PARAM));
		//Switch---
		addParam(createParam<BarkSwitch>(Vec(8.67f, rackY - 217.06f), module, bpmTrimLFO::OFFSET_PARAM));
		addParam(createParam<BarkSwitch>(Vec(117.57f, rackY - 217.06f), module, bpmTrimLFO::INVERT_PARAM));
		///quick access - hidden params
		addParam(createParam<BarkButton1>(Vec(121.54f, rackY - 140.91f), module, bpmTrimLFO::RESET_PARAM));
		addParam(createParam<BarkButton1>(Vec(14.91f, rackY - 31.2f), module, bpmTrimLFO::SETsin_PARAM));
		addParam(createParam<BarkButton1>(Vec(48.21f, rackY - 31.2f), module, bpmTrimLFO::SETsaw_PARAM));
		addParam(createParam<BarkButton1>(Vec(81.52f, rackY - 31.2f), module, bpmTrimLFO::SETtri_PARAM));
		addParam(createParam<BarkButton1>(Vec(114.91f, rackY - 31.2f), module, bpmTrimLFO::SETsqr_PARAM));
		addParam(createParam<BarkButton1>(Vec(10.55f, rackY - 191.09f), module, bpmTrimLFO::SETbi_PARAM));
		addParam(createParam<BarkButton1>(Vec(10.55f, rackY - 228.33f), module, bpmTrimLFO::SETuni_PARAM));
		//Screw---
		//addChild(createWidget<BarkScrew3>(Vec(2, 3)));		//pos1
		//Light---
		addChild(createLight<LessBigLight<greenRedLight>>(Vec(71.87f, rackY - 152.63f), module, bpmTrimLFO::PHASE_POS_LIGHT));
		
		//------------------------------bpmTrimLFO
		if (module != NULL) {
			bpmVoltsDisplayWidget *display4 = createWidget<bpmVoltsDisplayWidget>(Vec(15.009f, 31.05f));
			display4->box.size = Vec(50.728f, 13.152f);
			display4->value = &module->volts1bpm;
			addChild(display4);
			bpmVoltsDisplayWidget *display5 = createWidget<bpmVoltsDisplayWidget>(Vec(84.228f, 31.05f));
			display5->box.size = Vec(50.728f, 13.152f);
			display5->value = &module->volts2bpm;
			addChild(display5);
			////------------------------------
			bpmFreqDisplayWidget *display6 = createWidget<bpmFreqDisplayWidget>(Vec(32.23f, 237.31f));
			display6->box.size = Vec(85.798f, 13.673f);
			display6->freqHz = &module->freqHz;
			addChild(display6);
			////------------------------------bpmTrimLFO
		}

	}

};

Model *modelbpmTrimLFO = createModel<bpmTrimLFO, bpmTrimLFOWidget>("bpmTrimLFO");
