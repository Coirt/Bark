#include "Bark.hpp"
#include "dsp/digital.hpp"
#include "dsp/ringbuffer.hpp"
#include "dsp/vumeter.hpp"
#include <math.h>
#include <cfloat>

struct Gain : Module {
	enum ParamIds {
		MAKEUP_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN_L_INPUT,
		IN_R_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_L_OUTPUT,
		OUT_R_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		BLINK_LIGHT,
		NUM_LIGHTS
	};
	
	DoubleRingBuffer<float, 16384> vu_L_Buffer, vu_R_Buffer;
	DoubleRingBuffer<float, 512> rms_L_Buffer, rms_R_Buffer;
	float runningVU_L_Sum = 1e-6f, runningRMS_L_Sum = 1e-6f, rms_L = -96.3f, vu_L = -96.3f, peakL = -96.3f;
	float runningVU_R_Sum = 1e-6f, runningRMS_R_Sum = 1e-6f, rms_R = -96.3f, vu_R = -96.3f, peakR = -96.3f;
	float in_L_dBFS = 1e-6f;
	float in_R_dBFS = 1e-6f;
	float gain = 1, gaindB = 1;
	float makeup = 1, previousPostGain = 1.0f;
	int indexVU = 0, indexRMS = 0, lookAheadWriteIndex = 0;
	int maxIndexVU = 0, maxIndexRMS = 0, maxLookAheadWriteIndex = 0;
	//int lookAhead;
	float buffL[20000] = { 0 }, buffR[20000] = { 0 };


	float LEFT;
	float RIGHT;
	float out1;
	float out2;


	Gain() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}

	void step() override;
};

void Gain::step()
{

//VU Meter
	{
		//float readIndex;
		if (indexVU >= 16384) {
			runningVU_L_Sum -= *vu_L_Buffer.startData();
			runningVU_R_Sum -= *vu_R_Buffer.startData();
			vu_L_Buffer.startIncr(1);
			vu_R_Buffer.startIncr(1);
			indexVU--;
		}

		if (indexRMS >= 512) {
			runningRMS_L_Sum -= *rms_L_Buffer.startData();
			runningRMS_R_Sum -= *rms_R_Buffer.startData();
			rms_L_Buffer.startIncr(1);
			rms_R_Buffer.startIncr(1);
			indexRMS--;
		}

		indexVU++;
		indexRMS++;

		//#buffL[lookAheadWriteIndex] = inputs[IN_L_INPUT].value;
		//#buffR[lookAheadWriteIndex] = inputs[IN_R_INPUT].value;

		//if (!inputs[SC_L_INPUT].active && inputs[IN_L_INPUT].active)
		//	in_L_dBFS = max(20 * log10((abs(inputs[IN_L_INPUT].value) + 1e-6f) / 5), -96.3f);
		//else if (inputs[SC_L_INPUT].active)
		//	in_L_dBFS = max(20 * log10((abs(inputs[SC_L_INPUT].value) + 1e-6f) / 5), -96.3f);
		//else
		//	in_L_dBFS = -96.3f;

		//if (!inputs[SC_R_INPUT].active && inputs[IN_R_INPUT].active)
		//	in_R_dBFS = max(20 * log10((abs(inputs[IN_R_INPUT].value) + 1e-6f) / 5), -96.3f);
		//else if (inputs[SC_R_INPUT].active)
		//	in_R_dBFS = max(20 * log10((abs(inputs[SC_R_INPUT].value) + 1e-6f) / 5), -96.3f);
		//else
		//	in_R_dBFS = -96.3f;

		float data_L = in_L_dBFS * in_L_dBFS;

		if (!vu_L_Buffer.full()) {
			vu_L_Buffer.push(data_L);
		}
		if (!rms_L_Buffer.full()) {
			rms_L_Buffer.push(data_L);
		}

		float data_R = in_R_dBFS * in_R_dBFS;
		if (!vu_R_Buffer.full()) {
			vu_R_Buffer.push(data_R);
		}
		if (!rms_R_Buffer.full()) {
			rms_R_Buffer.push(data_R);
		}

		runningVU_L_Sum += data_L;
		runningRMS_L_Sum += data_L;
		runningVU_R_Sum += data_R;
		runningRMS_R_Sum += data_R;
		rms_L = clampf(-1 * sqrtf(runningRMS_L_Sum / 512), -96.3f, 0.0f);
		vu_L = clampf(-1 * sqrtf(runningVU_L_Sum / 16384), -96.3f, 0.0f);
		rms_R = clampf(-1 * sqrtf(runningRMS_R_Sum / 512), -96.3f, 0.0f);
		vu_R = clampf(-1 * sqrtf(runningVU_R_Sum / 16384), -96.3f, 0.0f);
		makeup = params[MAKEUP_PARAM].value;

		if (in_L_dBFS > peakL)
			peakL = in_L_dBFS;
		else
			peakL -= 50 / engineGetSampleRate();

		if (in_R_dBFS > peakR)
			peakR = in_R_dBFS;
		else
			peakR -= 50 / engineGetSampleRate();

		//float slope = 1 / ratio - 1;
		//float maxIn = max(in_L_dBFS, in_R_dBFS);
		//float dist = maxIn - threshold;
		//float gcurve = 0.0f;

		//if (dist<-1 * knee / 2)
		//	gcurve = maxIn;
		//else if ((dist > -1 * knee / 2) && (dist < knee / 2)) {
		//	gcurve = maxIn + slope * pow(dist + knee / 2, 2) / (2 * knee);
		//}
		//else {
		//	gcurve = maxIn + slope * dist;
		//}

		//float preGain = gcurve - maxIn;
		//float postGain = 0.0f;
		//float cAtt = exp(-1 / (attackTime*engineGetSampleRate() / 1000));
		//float cRel = exp(-1 / (releaseTime*engineGetSampleRate() / 1000));

		//if (preGain>previousPostGain) {
		//	postGain = cAtt * previousPostGain + (1 - cAtt) * preGain;
		//}
		//else {
		//	postGain = cRel * previousPostGain + (1 - cRel) * preGain;
		//}

		//previousPostGain = postGain;
		//gaindB = makeup + postGain;
		//gain = pow(10, gaindB / 20);

		//mix = params[MIX_PARAM].value;
		//lookAhead = params[LOOKAHEAD_PARAM].value;

		int nbSamples = clampi(floor(engineGetSampleRate() / 100000), 0, 19999);
		int readIndex;
		if (lookAheadWriteIndex - nbSamples >= 0)
			readIndex = (lookAheadWriteIndex - nbSamples) % 20000;
		else {
			readIndex = 20000 - abs(lookAheadWriteIndex - nbSamples);
		}

		outputs[OUT_L_OUTPUT].value = buffL[readIndex];
		outputs[OUT_R_OUTPUT].value = buffR[readIndex];

		lookAheadWriteIndex = (lookAheadWriteIndex + 1) % 20000;


		//Gain
		float out1 = inputs[IN_L_INPUT].value * params[MAKEUP_PARAM].value;
		float out2 = inputs[IN_R_INPUT].value * params[MAKEUP_PARAM].value;
		out1 = clampf(out1, -10.0, 10.0);
		out2 = clampf(out2, -10.0, 10.0);

		in_L_dBFS = inputs[IN_L_INPUT].value;
		in_R_dBFS = inputs[IN_R_INPUT].value;

		outputs[OUT_L_OUTPUT].value = in_L_dBFS;
		outputs[OUT_R_OUTPUT].value = in_R_dBFS;

		in_L_dBFS = in_L_dBFS * params[MAKEUP_PARAM].value / 2.5;
		in_R_dBFS = in_R_dBFS * params[MAKEUP_PARAM].value / 2.5;

		outputs[OUT_L_OUTPUT].value = in_L_dBFS;
		outputs[OUT_R_OUTPUT].value = in_R_dBFS;
	}
}

struct GainDisplay : TransparentWidget
{
	Gain *module;
	std::shared_ptr<Font> font;
	GainDisplay()
	{
		font = Font::load(assetPlugin(plugin, "")); //res/DejaVuSansMono.ttf
	}

	void draw(NVGcontext *vg) override
	{
		float height = 176;
		float width = 10.5;
		float spacer = 1;
		float vuL = rescalef(module->vu_L, -97, 0, 0, height);
		float rmsL = rescalef(module->rms_L, -97, 0, 0, height);		//103.5, -97, 0
		float vuR = rescalef(module->vu_R, -97, 0, 0, height);
		float rmsR = rescalef(module->rms_R, -97, 0, 0, height);
		//float threshold = rescalef(module->threshold, 0, -97, 0, height);
		//float gain = rescalef(1 - (module->gaindB - module->makeup), -97, 0, 97, 0);
		//float makeup = rescalef(module->makeup, 0, 60, 0, 60);
		float peakL = rescalef(module->peakL, -97, 97, 0, height);
		float peakR = rescalef(module->peakR, 97, -97, 0, height);
		float inL = rescalef(module->in_L_dBFS, 0, -97, 0, height);
		float inR = rescalef(module->in_R_dBFS, 0, -97, 0, height);
		nvgStrokeWidth(vg, 0);
		nvgBeginPath(vg);
		nvgFillColor(vg, BARK_GREEN4);
		nvgRoundedRect(vg, 0, height - vuL, width, vuL, 0);
		nvgRoundedRect(vg, 3 * (width + spacer), height - vuR, width, vuR, 0);
		nvgFill(vg);
		nvgClosePath(vg);

		nvgFillColor(vg, BARK_RED2);
		nvgBeginPath(vg);
		nvgRoundedRect(vg, 0, height - peakL, width, -13, 0);
		nvgRoundedRect(vg, 3 * (width + spacer), peakR, width, -13, 0);
		nvgFill(vg);
		nvgClosePath(vg);

		nvgBeginPath(vg);
		if (inL>rmsL - 2)
			nvgRoundedRect(vg, width + spacer, height - inL - 12, width, inL - rmsL - 15, 0);
		if (inR>rmsR - 2)
			nvgRoundedRect(vg, 3 * (width + spacer), height - inR - 12, width, inR - rmsR - 15, 0);
		nvgFill(vg);
		nvgClosePath(vg);

		nvgStroke(vg);
		nvgFill(vg);


	}

};

GainWidget::GainWidget()
{
	Gain *module = new Gain();
	setModule(module);
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/BarkGain.svg")));
		panel->box.size = box.size;
		addChild(panel);
	}

	GainDisplay *display = new GainDisplay();
	display->module = module;
	display->box.pos = Vec(23.4, 380 - 315);
	display->box.size = Vec(110, 70);
	addChild(display);

	///////////////////////////////////////////////////////////////
	// Screw Positions
	//		addChild(createScrew<NAMESCREW>(Vec(15, 0)));					//top left		pos1
	//		addChild(createScrew<NAMESCREW>(Vec(box.size.x - 30, 0)));		//top right		pos2
	//		addChild(createScrew<NAMESCREW>(Vec(15, 365)));					//bottom left	pos3
	//		addChild(createScrew<NAMESCREW>(Vec(box.size.x - 30, 365)));		//bottom right	pos4
	/////////////////////////////////////////////////////////////

	////////////
	//components 
	//positioning=>(Vec(x-x, y-y), = (x=Panel size in pixels -minus xInkscape co-ordinates, y=Panel size in pixels -minus, yInkscape co-ordinates)
	////////////
	//Knob---
	//addParam(createParam<BarkKnob57>(Vec(45.03, 91.91), module, Gain::GIAN_PARAM, 0, 60, 0));			//Bidoo - Bar
	addParam(createParam<BarkKnob57>(Vec(16.6, 380 - 121.56), module, Gain::MAKEUP_PARAM, 0.0, 10.0, 2.5));		//cf - Master
	//Port---
	addInput(createInput<BarkInPort>(Vec(13.65 + 3, 380 - 57.26), module, Gain::IN_L_INPUT));			//Botoom
	addInput(createInput<BarkInPort>(Vec(48.88 + 3, 380 - 57.26), module, Gain::IN_R_INPUT));			//Bottom
	addOutput(createOutput<BarkOutPort>(Vec(13.65 + 3, 380 - 360.12), module, Gain::OUT_L_OUTPUT));		//Top
	addOutput(createOutput<BarkOutPort>(Vec(48.88 + 3, 380 - 360.59), module, Gain::OUT_R_OUTPUT));		//Top
	//screw---
	addChild(createScrew<BarkScrew2>(Vec(2, 3)));							//pos1
	addChild(createScrew<BarkScrew1>(Vec(box.size.x - 13, 367.2)));			//pos4
}
