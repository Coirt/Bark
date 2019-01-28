#include "Bark.hpp"
#include "dsp/digital.hpp"
#include "util/math.hpp"
#include "barkComponents.hpp"
#include "dsp/functions.hpp"
#include <sstream>
#include <iomanip>


struct LowFrequencyOscillator {
	float phase = 0.0f;
	float pw = 0.5f;
	float freq = 1.0f;
	bool offset = false;
	bool invert = false;
	SchmittTrigger resetTrigger;
	LowFrequencyOscillator() {

	}

	void setPitch(float pitch) {
		pitch = fminf(pitch, 8.0f);
		freq = powf(2.0f, pitch);
	}
	void setPulseWidth(float pw_) {
		const float pwMin = 0.01f;
		pw = clamp(pw_, pwMin, 1.0f - pwMin);
	}
	void setReset(float reset) {
		if (resetTrigger.process(reset / 0.01f)) {
			phase = 0.0f;
		}
	}
	void step(float dt) {
		float deltaPhase = fminf(freq * dt, 0.5f);
		phase += deltaPhase;
		if (phase >= 1.0f)
			phase -= 1.0f;
	}
	float sin() {
		if (offset)
			return 1.0f - cosf(2 * M_PI * phase) * (invert ? -1.0f : 1.0f);
		else
			return sinf(2 * M_PI * phase) * (invert ? -1.0f : 1.0f);
	}
	float tri(float x) {
		return 4.0f * fabsf(x - roundf(x));
	}
	float tri() {
		if (offset)
			return tri(invert ? phase - 0.5f : phase);
		else
			return -1.0f + tri(invert ? phase - 0.25f : phase - 0.75f);
	}
	float saw(float x) {
		return 2.0f * (x - roundf(x));
	}
	float saw() {
		if (offset)
			return invert ? 2.0f * (1.0f - phase) : 2.0f * phase;
		else
			return saw(phase) * (invert ? -1.0f : 1.0f);
	}
	float sqr() {
		float sqr = (phase < pw) ^ invert ? 1.0f : -1.0f;
		return offset ? sqr + 1.0f : sqr;
	}
	float light() {
		return sinf(2 * M_PI * phase);
	}
};

struct TrimLFO : Module {
	enum ParamIds
	{
		//Offset
		OFFSET1_PARAM,
		OFFSET2_PARAM,
		//LFO
		OFFSET_PARAM,
		INVERT_PARAM,
		FREQ_PARAM,
		FINE_PARAM,
		FM1_PARAM,
		FM2_PARAM,
		PW_PARAM,
		PWM_PARAM,
		WAVEMIX_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		//TODO mod source for ±1v sin/sqr Offset out
		MODSRC_INPUT,		//Modulates Offset Params
		//IN1_INPUT,  //Code needs to link Lfo In to Logic 1A, OFFSET_1 to Logic 1B, > Max to Logic 2A > OFFSET_2 to Logic 2B Min to Lfo out Port, Internally different waves need to link to Logic.
		FM1_INPUT,
		FM2_INPUT,
		RESET_INPUT,
		PW_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT, //Offset L + sin Clamped to ±1v
		OUT2_OUTPUT, //Offset R + sqr Clamped to ±1v
		OUT1a_OUTPUT, //Normal Offset
		OUT2a_OUTPUT, //Normal Offset
		SIN_OUTPUT,
		TRI_OUTPUT,
		SAW_OUTPUT,
		SQR_OUTPUT,
		trimLFO_OUTPUT, //trimmed output
		NUM_OUTPUTS
	};
	enum LightIds {
		PHASE_POS_LIGHT,
		PHASE_NEG_LIGHT,
		NUM_LIGHTS
	};

	LowFrequencyOscillator oscillator;
	float volts1 = 0.000f;
	float volts2 = 0.000f;
	float freqHz = 1.0f;				 /** powf(0.25 / 8, 0.25 * 96)*/

										 //pitch += inputs[PITCH1_INPUT].value + inputs[PITCH2_INPUT].value;
										 //pitch += inputs[FM_INPUT].value / 4.0;
										 //float freqHz = 0.25 * powf(2.0, freqHz);
										 //freqHz = clamp(freqHz, 0.0f, 20000.0f);;

										 /*
										 // Compute the frequency from the pitch parameter and input
										 float pitch = params[PITCH_PARAM].value;
										 pitch += inputs[PITCH_INPUT].value;
										 pitch = clamp(pitch, -4.0f, 4.0f);
										 // The default pitch is C4
										 float freq = 261.626f * powf(2.0f, pitch);
										 */

	TrimLFO() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

};


void TrimLFO::step() {
	
	//volts1 = clamp(params[WAVEMIX_PARAM].value, -10.0f, 10.0f);		//test param value***
	volts1 = clamp(params[OFFSET1_PARAM].value, -10.0f, 10.0f);
	volts2 = clamp(params[OFFSET2_PARAM].value, -10.0f, 10.0f);
	//float out1 = params[OFFSET1_PARAM].value;
	
	float out1 = params[OFFSET1_PARAM].value + oscillator.sin(); //Attenuvert??? osc.sin
	float out2 = params[OFFSET2_PARAM].value + oscillator.sqr();
	float out1a = params[OFFSET1_PARAM].value;	//Normal +-10v
	float out2a = params[OFFSET2_PARAM].value;	//Normal +-10v
	float fineTune = 3.0f * quadraticBipolar(params[FINE_PARAM].value);		//finetune
	//TODO Mod source offset sin + sqr
	//float modsrc = params[OFFSET1_PARAM].value + params[OFFSET2_PARAM].value
	//Trim output/Logic
	//float trimLFOmax = (volts1) + 5.0f * oscillator.sin();
	//float trimLFOmin = (volts2) + 5.0f * oscillator.sin();
	
	//float mixsrc =
		
		//TODO Logic onboard
		//OUT1_OUTPUT, //Offset L + sin Clamped to ±1v offset
		//OUT2_OUTPUT, //Offset R + sqr Clamped to ±1v offset
		//outputs[MAX1_OUTPUT].value = fmaxf(inputs[LOGIC_A1_INPUT].value, inputs[LOGIC_B1_INPUT].value);
		//outputs[MIN1_OUTPUT].value = fminf(inputs[LOGIC_A1_INPUT].value, inputs[LOGIC_B1_INPUT].value);
		/*
			if (2v==true){
				out.sin = 2v
			}else if (10v==true) {
				out.sin = 10v clamped to offset
			}
		*/

	outputs[OUT1_OUTPUT].value = clamp(out1, -10.0f, 10.0f);
	outputs[OUT2_OUTPUT].value = clamp(out2, -10.0f, 10.0f);
	outputs[OUT1a_OUTPUT].value = clamp(out1a, -10.0f, 10.0f);
	outputs[OUT2a_OUTPUT].value = clamp(out2a, -10.0f, 10.0f);
	oscillator.setPitch(params[FREQ_PARAM].value + fineTune + params[FM1_PARAM].value * inputs[FM1_INPUT].value + params[FM2_PARAM].value * inputs[FM2_INPUT].value);
	oscillator.setPulseWidth(params[PW_PARAM].value + params[PWM_PARAM].value * inputs[PW_INPUT].value / 10.0f);
	oscillator.offset = (params[OFFSET_PARAM].value > 0.0f);
	oscillator.invert = (params[INVERT_PARAM].value <= 0.0f);
	oscillator.step(1.0 / engineGetSampleRate());
	oscillator.setReset(inputs[RESET_INPUT].value);

	//----------------	DISPLAY Hz	--------------------------
	//TODO: FM INPUT TO DISPLAY
	float frq = params[FREQ_PARAM].value + params[FM1_PARAM].value * inputs[FM1_INPUT].value + params[FM2_PARAM].value * inputs[FM2_INPUT].value;

	//float trimmedOutput = params[OFFSET1_PARAM].value + fmaxf(params[OFFSET1_PARAM].value, oscillator.sin()) * params[OFFSET2_PARAM].value * fminf(params[OFFSET2_PARAM], oscillator.sin());

	float fine = fineTune;
	frq = clamp(frq, -32.0f, 32.0f);
	//Default Frq is 1.0Hz == 60BPM			
	freqHz = 1.0f * powf(2.0f, frq + fine);
	//--------------------------------------------------------
	///TRIM LFO----
	//initilise oscillators
	float sinValue = 5.0f * oscillator.sin(), sawValue = 5.0f * oscillator.saw(),
		  triValue = 5.0f * oscillator.tri(), sqrValue = 5.0f * oscillator.sqr();
	//float fmaxValue, fminValue, tOffset, bOffset, a, b, c, d, sum1, sum2, minVal, newVal;		//pseudo
	float waveMixParam = clamp(params[WAVEMIX_PARAM].value, 0.0f, 3.0f), xFade;
	///Trim
	/*pseudo-------
	a = params[OFFSET1_PARAM].value;
	b = xFade;
	sum1 = a + b;
	minVal = fminf(a, b);
	c = params[OFFSET2_PARAM].value;
	d = minVal;
	maxVal = fmaxf(c, d);
	outputs[trimLFO_OUTPUT].value = maxVal;
	*///-----------
	///sin.saw----
	if (waveMixParam < 1.0f) {// 0.0f sin
		xFade = crossfade(sinValue, sawValue, waveMixParam);
		outputs[trimLFO_OUTPUT].value = fmaxf(params[OFFSET2_PARAM].value, fminf(params[OFFSET1_PARAM].value, xFade));
	}
	///saw.tri----
	else if (waveMixParam < 2.0f) {	//1.0f saw
		xFade = crossfade(sawValue, triValue, waveMixParam - 1.0f); ///some of the higher voltages get lost TODO: fix that - 1.2 maybe
		outputs[trimLFO_OUTPUT].value = fmaxf(params[OFFSET2_PARAM].value, fminf(params[OFFSET1_PARAM].value, xFade));
	}
	///tri.sqr----
	else if (waveMixParam < 3.0f) { //2.0f tri
		xFade = crossfade(triValue, sqrValue, waveMixParam - 1.95f); 
		outputs[trimLFO_OUTPUT].value = fmaxf(params[OFFSET2_PARAM].value, fminf(params[OFFSET1_PARAM].value, xFade));
	}
	///sqr----
	else if (waveMixParam >= 3.0f) { //3.0f sqr
		xFade = sqrValue;
		outputs[trimLFO_OUTPUT].value = fmaxf(params[OFFSET2_PARAM].value, fminf(params[OFFSET1_PARAM].value, xFade));
	}
	///LFO----
	outputs[SIN_OUTPUT].value = sinValue;
	outputs[SAW_OUTPUT].value = sawValue;
	outputs[TRI_OUTPUT].value = triValue;
	outputs[SQR_OUTPUT].value = sqrValue;

	/* Interesting shape----
	outputs[trimLFO_OUTPUT].value = fmaxf(fminf(params[OFFSET1_PARAM].value, sinValue), fmaxf(params[OFFSET2_PARAM].value, sinValue)) -
	fmodf(fminf(params[OFFSET1_PARAM].value, sinValue), fmaxf(params[OFFSET2_PARAM].value, sinValue));

	//----old
	/*
	fmaxValue = fmaxf(fminf(params[OFFSET1_PARAM].value, xFade), fmaxf(params[OFFSET2_PARAM].value, xFade));
	fminValue = fminf(fmaxf(params[OFFSET1_PARAM].value, xFade), fminf(params[OFFSET2_PARAM].value, xFade));
	////issue - not accounting for second offset!!! because it only outputs 1 value
	tOffset = fmaxf(fminValue, fmaxValue);
	bOffset = fminf(fmaxValue, fminValue);
	outputs[trimLFO_OUTPUT].value = fmaxf(tOffset, bOffset);
	//---old
	//outputs[trimLFO_OUTPUT].value = xFade;
	//fmaxValue = fmaxf(fminf(params[OFFSET1_PARAM].value, sinValue), fmaxf(params[OFFSET2_PARAM].value, sinValue));
	//fminValue = fminf(fmaxf(params[OFFSET2_PARAM].value, sinValue), fminf(params[OFFSET1_PARAM].value, sinValue));
	//tOffset = fminf(fmaxValue, fminValue);
	//bOffset = fmaxf(fminValue, fmaxValue);
	//outputs[trimLFO_OUTPUT].value = fmaxf(tOffset, bOffset);
	*/

	//outputs[trimLFO_OUTPUT].value = fminf(fmaxValue, fminValue) - params[OFFSET2_PARAM].value; //offset by param2

	lights[PHASE_POS_LIGHT].setBrightnessSmooth(fmaxf(0.0f, oscillator.light()));
	lights[PHASE_NEG_LIGHT].setBrightnessSmooth(fmaxf(0.0f, -oscillator.light()));
	//TODO: Split Lights???
}

struct FreqDisplayWidget : TransparentWidget {

	float *freqHz;
	std::shared_ptr<Font> font;

	FreqDisplayWidget() {
		font = Font::load(assetPlugin(plugin, "res/segoescb.ttf"));
	};

	void draw(NVGcontext *vg) override {

		float spacer = 40.0f;
		// Background
		//NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
		NVGcolor backgroundColor = nvgRGB(97, 54, 57);		//CreamyRed
		NVGcolor borderColor = nvgRGB(0, 0, 0);				//Black
		nvgBeginPath(vg);
		nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 0.75);
		nvgFillColor(vg, backgroundColor);
		nvgFill(vg);
		nvgStrokeWidth(vg, 0.75);
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);
		// text 
		//NVGalign = 1 << 1;
		nvgTextAlign(vg, 1 << 1);
		nvgFontSize(vg, 16);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -1.0);

		char display_string[10];
		snprintf(display_string, sizeof(display_string), "%0.520f", *freqHz);

		Vec textPos = Vec(85.798f / 2.0f - 8.0f, 10.673f);
		//----- "Hz"
		NVGcolor textColor = nvgRGB(93, 193, 57);		//97, 193, 57 == Green
		nvgFillColor(vg, nvgTransRGBA(textColor, 255));
		nvgText(vg, textPos.x + spacer, textPos.y, "Hz", NULL);
		//----- Background
		/*
		NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
		nvgFillColor(vg, nvgTransRGBA(textColor, 16));
		nvgText(vg, textPos.x, textPos.y, "~~~~~~", NULL);

		textColor = nvgRGB(0xda, 0xe9, 0x29);
		nvgFillColor(vg, nvgTransRGBA(textColor, 16));
		nvgText(vg, textPos.x, textPos.y, "//////////", NULL);*/

		textColor = nvgRGB(93, 193, 57);
		nvgFillColor(vg, textColor);
		nvgText(vg, textPos.x, textPos.y, display_string, NULL);
		//nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
	}
};

struct VoltsDisplayWidget : TransparentWidget {

	float *value;
	std::shared_ptr<Font> font;

	VoltsDisplayWidget() {
		font = Font::load(assetPlugin(plugin, "res/segoescb.ttf"));	//segoescb
	};

	void draw(NVGcontext *vg) override {
		// Background
		//NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
		NVGcolor backgroundColor = nvgRGB(97, 54, 57);
		NVGcolor borderColor = nvgRGB(0, 0, 0);
		nvgBeginPath(vg);
		nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 0.75);
		nvgFillColor(vg, backgroundColor);
		nvgFill(vg);
		nvgStrokeWidth(vg, 0.75);
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);
		// text 
		//NVGalign = 1 << 1;
		nvgTextAlign(vg, 1 << 1);
		nvgFontSize(vg, 18);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, 0.75);

		char display_string[10];
		sprintf(display_string, "%5.2f", *value);

		Vec textPos = Vec(25.0f, 10.0f);		//		85.798f, 13.673f 

		NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
		nvgFillColor(vg, nvgTransRGBA(textColor, 16));
		nvgText(vg, textPos.x, textPos.y, "$$$$$", NULL);

		textColor = nvgRGB(0xda, 0xe9, 0x29);
		nvgFillColor(vg, nvgTransRGBA(textColor, 11));
		nvgText(vg, textPos.x, textPos.y, "-00.00", NULL);

		textColor = nvgRGB(93, 193, 57);
		nvgFillColor(vg, textColor);
		nvgText(vg, textPos.x, textPos.y, display_string, NULL);
		//nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
	}
};


struct TrimLFOWidget : ModuleWidget {
	TrimLFOWidget(TrimLFO *module);
};

TrimLFOWidget::TrimLFOWidget(TrimLFO *module) : ModuleWidget(module) {
	int rackY = 380;

	setPanel(SVG::load(assetPlugin(plugin, "res/BarkTrimLFO.svg")));
	/*box.size = Vec(10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	{
	SVGPanel *panel = new SVGPanel();
	panel->box.size = box.size;
	panel->setBackground(SVG::load(assetPlugin(plugin, "res/BarkTrimLFO.svg")));
	addChild(panel);
	}
	////////////
	//components
	////////////
	//----
	//	ModuleLightWidget::create	== Light
	//	Widget::create 				== Screw
	//	ParamWidget::create 			== Knob
	//	Port::create					== Port
	//	    ""		<COMPONENT>(Vec(0, 0), (for port) = , Port::INPUT, or ,Port::OUTPUT , module, NAME::ENUM));
	// y = 380 == -up, +down
	//----*/
	//Ports---
	//Out---
	addOutput(Port::create<BarkOutPort350>(Vec(13.28f, rackY - 52.35f), Port::OUTPUT, module, TrimLFO::SIN_OUTPUT));
	addOutput(Port::create<BarkOutPort350>(Vec(46.58f, rackY - 52.35f), Port::OUTPUT, module, TrimLFO::SAW_OUTPUT));
	addOutput(Port::create<BarkOutPort350>(Vec(79.68f, rackY - 52.35f), Port::OUTPUT, module, TrimLFO::TRI_OUTPUT));
	addOutput(Port::create<BarkOutPort350>(Vec(113.245f, rackY - 52.35f), Port::OUTPUT, module, TrimLFO::SQR_OUTPUT));
	addOutput(Port::create<BarkOutPort350>(Vec(14.57f, rackY - 274.73f + 0.35f), Port::OUTPUT, module, TrimLFO::OUT1_OUTPUT));				//2v sin
	addOutput(Port::create<BarkOutPort350>(Vec(112.09f, rackY - 274.73f + 0.35f), Port::OUTPUT, module, TrimLFO::OUT2_OUTPUT));				//2v sqr
	addOutput(Port::create<BarkOutPort350>(Vec(42.11f + 0.35f, rackY - 274.73f + 0.35f), Port::OUTPUT, module, TrimLFO::OUT1a_OUTPUT));		//Offset1
	addOutput(Port::create<BarkOutPort350>(Vec(84.18f, rackY - 274.73f + 0.35f), Port::OUTPUT, module, TrimLFO::OUT2a_OUTPUT));				//Offset2
	addOutput(Port::create<BarkPatchPortOut>(Vec(63.35f, rackY - 332.02f), Port::OUTPUT, module, TrimLFO::trimLFO_OUTPUT));	//trimmed offset output 
	//In---
	addInput(Port::create<BarkInPort350>(Vec(27.06f, rackY - 82.35f + 0.35f), Port::INPUT, module, TrimLFO::FM1_INPUT));
	addInput(Port::create<BarkInPort350>(Vec(63.25f, rackY - 82.35f + 0.35f), Port::INPUT, module, TrimLFO::FM2_INPUT));
	addInput(Port::create<BarkInPort350>(Vec(99.66f, rackY - 82.35f + 0.35f), Port::INPUT, module, TrimLFO::PW_INPUT));
	addInput(Port::create<BarkInPort350>(Vec(119.89f, rackY - 163.35f + 0.7f), Port::INPUT, module, TrimLFO::RESET_INPUT));
	//addInput(Port::create<BarkPatchPortIn>(Vec(63.35f, rackY - 332.02f), Port::INPUT, module, TrimLFO::MODSRC_INPUT));
	//Knobs---
	addParam(ParamWidget::create<BarkKnob70>(Vec(40.01f - 0.35f, rackY - 217.01f), module, TrimLFO::FREQ_PARAM, -16.0f, 4.0f, -6.0f));
	addParam(ParamWidget::create<BarkScrew01>(Vec(box.size.x - 13, 367.2f), module, TrimLFO::FINE_PARAM, -0.06798301f, 0.06798301f, 0.0f));
	//--------------------------------		FINETUNE VALUES		-------------------------------------------¬
	/*
	±0 == added to minus value (e.g. minus starts counting from 9 instead of 0)

	0.1		== ±0.0210120, 0.01		== ±0.0002080, 0.001		== ±0.0000020
	0.15		== ±0.0478992, 0.015		== ±0.0004680, 0.0015	== ±0.0000046
	0.25		== ±0.1387885, 0.025		== ±0.0013004, 0.0025	== ±0.0000129
	0.5		== ±0.6817928, 0.0005	== 0.0000004
	*/
	//------------------------------------------------------------------------------------------------------
	addParam(ParamWidget::create<BarkKnob40>(Vec(20.48f, rackY - 328.18f - 1.4f), module, TrimLFO::OFFSET1_PARAM, -10.0f, 10.0f, 10.0f));
	addParam(ParamWidget::create<BarkKnob40>(Vec(89.7f, rackY - 328.18f - 1.4f), module, TrimLFO::OFFSET2_PARAM, -10.0f, 10.0f, 0.0f));
	//addParam(ParamWidget::create<BarkKnobLine>(Vec(71.77f, rackY - 308.03f), module, TrimLFO::WAVEMIX_PARAM, -3.0f, 3.0f, 0.0f));
	addParam(ParamWidget::create<BarkKnob26>(Vec(5.19f, rackY - 167.6f), module, TrimLFO::PW_PARAM, 0.0f, 1.0f, 0.5f));
	addParam(ParamWidget::create<BarkKnob26>(Vec(25.32f, rackY - 122.3f), module, TrimLFO::FM1_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<BarkKnob26>(Vec(61.65f, rackY - 122.3f), module, TrimLFO::FM2_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<BarkKnob26>(Vec(98.06f, rackY - 122.3f), module, TrimLFO::PWM_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<BarkSlide1>(Vec(25.41f, rackY - 57.0f), module, TrimLFO::WAVEMIX_PARAM, 0.0f, 3.0f, 0.0f));
	//Switch---
	addParam(ParamWidget::create<BarkSwitch>(Vec(8.67f, rackY - 229.06f + 12.0f), module, TrimLFO::OFFSET_PARAM, 0.0f, 1.0f, 1.0f));
	addParam(ParamWidget::create<BarkSwitch>(Vec(117.57f, rackY - 229.06f + 12.0f), module, TrimLFO::INVERT_PARAM, 0.0f, 1.0f, 1.0f));
	//Button---
	//addParam(ParamWidget::create<BarkSinActive>(Vec(14.96f, rackY - 38.06), module, TrimLFO::OFFSET_PARAM, 0.0f, 1.0f, 1.0f));
	//Screw---
	addChild(Widget::create<BarkScrew3>(Vec(2, 3)));									//pos1
																						//addChild(Widget::create<BarkScrew1>(Vec(box.size.x - 13, 367.2f)));			//pos4
																						//Light---
	addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(71.93f, rackY - 230.22f), module, TrimLFO::PHASE_POS_LIGHT));
	//------------------------------
	//ref, VOLTS DISPLAY : AS-TriggersMKI
	VoltsDisplayWidget *display1 = new VoltsDisplayWidget();
	display1->box.pos = Vec(15.009f, 33.05f);
	display1->box.size = Vec(50.728f, 13.152f);
	display1->value = &module->volts1;
	addChild(display1);
	VoltsDisplayWidget *display2 = new VoltsDisplayWidget();
	display2->box.pos = Vec(84.228f, 33.05f);
	display2->box.size = Vec(50.728f, 13.152f);
	display2->value = &module->volts2;
	addChild(display2);
	//------------------------------
	FreqDisplayWidget *display3 = new FreqDisplayWidget();
	display3->box.pos = Vec(32.23f, 237.31f);	//== -L/+R, +==D/-==U
	display3->box.size = Vec(85.798f, 13.673f);	//x, y == Lenght, Height
	display3->freqHz = &module->freqHz;
	addChild(display3);
	//------------------------------
}


Model *modelTrimLFO = Model::create<TrimLFO, TrimLFOWidget>("Bark", "TrimLFO", "Trim LFO", LFO_TAG, UTILITY_TAG, LOGIC_TAG, DUAL_TAG);
