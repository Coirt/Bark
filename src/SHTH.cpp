#include "plugin.hpp"
#include "barkComponents.hpp"

using namespace barkComponents;

struct SHTH : Module {
	enum ParamIds {
		ST1_PARAM,
		ST2_PARAM,
		RANGE_PARAM,
		POLE_PARAM,
		INVERT_PARAM,
		OFFSET_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		GATE1_INPUT,
		GATE2_INPUT,
		IN1_INPUT,
		IN2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	dsp::SchmittTrigger T1, T2;
	float sample1 = 0.f, sample2 = 0.f;
	bool isTH1; 
	bool isTH2;
	float range, pole, offset;//TODO: context menu

	SHTH() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam<tpMode_sh>(ST1_PARAM, 0.f, 1.f, 0.f, "Mode");
		configParam<tpMode_th>(ST2_PARAM, 0.f, 1.f, 1.f, "Mode");
		configParam<tpPlusMinus>(POLE_PARAM, 0.f, 1.f, 1.f, "Mode");
		configParam<tpOnOff>(INVERT_PARAM, 0.f, 1.f, 1.f, "Invert");
		configParam(RANGE_PARAM, 1.f, 10.f, 10.f, "V/Oct Range", "v");
		configParam(OFFSET_PARAM, -5.f, 5.f, 0.f, "Offset", "v");
	}
	
	void process(const ProcessArgs &args) override {
		range = params[RANGE_PARAM].getValue();
		offset = params[OFFSET_PARAM].getValue();
		pole = range / 2;
		bool gate1Con = inputs[GATE1_INPUT].isConnected();
		bool gate2Con = inputs[GATE2_INPUT].isConnected();
		bool in1Con = inputs[IN1_INPUT].isConnected();
		bool in2Con = inputs[IN2_INPUT].isConnected();
		bool out1Con = outputs[OUT1_OUTPUT].isConnected();
		bool out2Con = outputs[OUT2_OUTPUT].isConnected();
		bool noise1Only = (out1Con && (!in1Con && !gate1Con));
		bool gate1Only = (out1Con && (!in1Con && gate1Con));
		bool noise2Only = (out2Con && (!in2Con && !gate2Con));
		bool gate2Only = (out2Con && (!in2Con && gate2Con));
		//bool gate1On2 = (out2Con && (!in2Con && !gate2Con));
		bool invert = params[INVERT_PARAM].getValue() < 1;
		bool uniPolar = params[POLE_PARAM].getValue() > 0;
		//text conditions
		isTH1 = params[ST1_PARAM].getValue();
		isTH2 = params[ST2_PARAM].getValue();
		//TODO: polyphony
		uniPolar ? pole = pole : pole += 5;
		float noise1 = random::uniform() * range - pole;
		float noise2 = random::uniform() * range - pole;

		//process gate
		bool SrT1 = T1.process(inputs[GATE1_INPUT].getVoltage());
		bool SrT2 = T2.process(inputs[GATE2_INPUT].getVoltage());

		//S&H else T&H
		if (!isTH1 ? SrT1 : T1.isHigh()) {//default s&h
			//sample internal or external
			gate1Only ? sample1 = noise1 : sample1 = (((-inputs[IN1_INPUT].getVoltage() + 1.5f) / range - -pole) - 2)/* - offset*/;//divided attenuates
		}

		//T&H else S&H
		if (isTH2 ? SrT2 : T2.isHigh()) {//default t&h - with button position 0 (not green)
			//sample internal or external
			gate2Only ? sample2 = noise2 : sample2 = (((-inputs[IN2_INPUT].getVoltage() + 1.5f) / range - -pole) - 2)/* - offset*/;
		}

		//output ? sampleInternal : sampleExternal
		noise1Only ? outputs[OUT1_OUTPUT].setVoltage(invert ? noise1 - offset : -noise1 - offset) : outputs[OUT1_OUTPUT].setVoltage(invert ? sample1 - offset : -sample1 - offset);
		noise2Only ? outputs[OUT2_OUTPUT].setVoltage(invert ? noise2 - offset : -noise2 - offset) : outputs[OUT2_OUTPUT].setVoltage(invert ? sample2 - offset : -sample2 - offset);
		

		/*
		///debug
		if (isTH1) {
			//DEBUG("\nGate 1 Connected: %s\nInput 1 Connected: %s\n", gate1Con ? "true" : "false", in1Con ? "true" : "false");
			DEBUG("\nButton enabled: %s", isTH1 ? "true" : "false");
			params[ST1_PARAM].setValue(!isTH1);
		}
		if (isTH2) {
			//DEBUG("\nGate 2 Connected: %s\nInput 2 Connected: %s\n", gate2Con ? "true" : "false", in2Con ? "true" : "false");
			DEBUG("\nButton enabled: %s", isTH2 ? "true" : "false");
			params[ST2_PARAM].setValue(!isTH2);
		}
		*/

	}//process
};

struct label1Widget : TransparentWidget {
	SHTH *module;
	bool *isTrack1;

	std::shared_ptr<Font> font;

	label1Widget() {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/segoescb.ttf"));
	}

	void draw(const DrawArgs &labelDisp) override {

		NVGcolor textColor1 = nvgRGB(0xFF, 0xFF, 0xFF);		//White
		NVGcolor textColor2 = nvgRGB(0x00, 0xA8, 0x00);		//Green

		Vec textPos = Vec(*isTrack1 ? 22.457f : 22.838f, 34.906f + 5.302f);

		nvgTextAlign(labelDisp.vg, 1 << 0);
		nvgFontSize(labelDisp.vg, 10);
		nvgFontFaceId(labelDisp.vg, font->handle);
		nvgTextLetterSpacing(labelDisp.vg, .1f);
		if (*isTrack1 < 1) {
			nvgFillColor(labelDisp.vg, textColor1);
		} else if (*isTrack1 > 0) {
			nvgFillColor(labelDisp.vg, textColor2);
		}
		nvgText(labelDisp.vg, textPos.x, textPos.y, *isTrack1 ? "T&H" : "S&H", NULL);
	}
};

struct label2Widget : TransparentWidget {
	SHTH *module;
	bool *isTrack2;

	std::shared_ptr<Font> font;

	label2Widget() {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/segoescb.ttf"));
	}

	void draw(const DrawArgs &labelDisp) override {

		NVGcolor textColor1 = nvgRGB(0xFF, 0xFF, 0xFF);		//White
		NVGcolor textColor2 = nvgRGB(0x00, 0xA8, 0x00);		//Green

		Vec textPos = Vec(*isTrack2 ? 22.838f : 22.457f, 344.945f + 5.302f);

		nvgTextAlign(labelDisp.vg, 1 << 0);
		nvgFontSize(labelDisp.vg, 11);
		nvgFontFaceId(labelDisp.vg, font->handle);
		nvgTextLetterSpacing(labelDisp.vg, .1f);
		if (*isTrack2 < 1) {
			nvgFillColor(labelDisp.vg, textColor1);
		} else if (*isTrack2 > 0) {
			nvgFillColor(labelDisp.vg, textColor2);
		}
		nvgText(labelDisp.vg, textPos.x, textPos.y, *isTrack2 ? "S&H" : "T&H", NULL);
	}
};

struct SHTHWidget : ModuleWidget {
	SHTHWidget(SHTH *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BarkSHTH_2.svg")));


		box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		constexpr float portX = 10.387f, btnX = 7.708f, switchY = 200.025f, knbY = 165.136f;
		constexpr float portY[6] = { 49.443f, 86.352f, 121.455f, 242.352f, 277.455f, 314.364f };

		///Ports---
		//Out---
		addOutput(createOutput<BarkPatchPortOut>(Vec(portX, portY[2]), module, SHTH::OUT1_OUTPUT));
		addOutput(createOutput<BarkPatchPortOut>(Vec(portX, portY[5]), module, SHTH::OUT2_OUTPUT));
		//In---
		addInput(createInput<BarkPatchPortIn>(Vec(portX, portY[0]), module, SHTH::GATE1_INPUT));
		addInput(createInput<BarkPatchPortIn>(Vec(portX, portY[3]), module, SHTH::GATE2_INPUT));
		addInput(createInput<BarkPatchPortIn>(Vec(portX, portY[1]), module, SHTH::IN1_INPUT));
		addInput(createInput<BarkPatchPortIn>(Vec(portX, portY[4]), module, SHTH::IN2_INPUT));
		//Knob---
		addParam(createParam<BarkKnob_26>(Vec(9.536f, knbY), module, SHTH::RANGE_PARAM));
		//Button---
		addParam(createParam<BarkPushButton4>(Vec(btnX, 32.268f), module, SHTH::ST1_PARAM));
		addParam(createParam<BarkPushButton4>(Vec(btnX, 342.306f), module, SHTH::ST2_PARAM));
		//Switch---
		addParam(createParam<BarkSwitchSmall>(Vec(5.264f, switchY), module, SHTH::INVERT_PARAM));
		addParam(createParam<BarkSwitchSmall>(Vec(24.827f, switchY), module, SHTH::POLE_PARAM));
		//screw---
		addParam(createParam<BarkScrew01>(Vec(2, 3), module, SHTH::OFFSET_PARAM));	//pos1
		addChild(createWidget<BarkScrew2>(Vec(box.size.x - 13, 367.2)));				//pos4

		if (module != NULL) {
			label1Widget *label1 = new label1Widget;
			label1->isTrack1 = &module->isTH1;
			addChild(label1);
			label2Widget *label2 = new label2Widget;
			label2->isTrack2 = &module->isTH2;
			addChild(label2);			
		}
	}
};

Model *modelSHTH = createModel<SHTH, SHTHWidget>("SHTH");

struct SHTH2 : Module {
	enum ParamIds {
		ST1_PARAM,
		ST2_PARAM,
		RANGE_PARAM,
		POLE_PARAM,
		INVERT_PARAM,
		OFFSET_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		GATE1_INPUT,
		GATE2_INPUT,
		IN1_INPUT,
		IN2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		SETGATEPOLY_LIGHT,
		SETINPOLY_LIGHT,
		NUM_LIGHTS
	};

	dsp::SchmittTrigger T1[16], T2[16];
	float sample1[16] = { 0.f }, sample2[16] = { 0.f };
	bool isTH1;
	bool isTH2;
	float range, pole, offset;//TODO: context menu
	int nCh1, nCh2;

	SHTH2() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam<tpMode_sh>(ST1_PARAM, 0.f, 1.f, 0.f, "Mode");
		configParam<tpMode_th>(ST2_PARAM, 0.f, 1.f, 1.f, "Mode");
		configParam<tpPlusMinus>(POLE_PARAM, 0.f, 1.f, 1.f, "Mode");
		configParam<tpOnOff>(INVERT_PARAM, 0.f, 1.f, 1.f, "Invert");
		configParam(RANGE_PARAM, 1.f, 10.f, 10.f, "V/Oct Range", "v");
		configParam(OFFSET_PARAM, -5.f, 5.f, 0.f, "Offset", "v");
	}

	void process(const ProcessArgs &args) override {
		range = params[RANGE_PARAM].getValue();
		offset = params[OFFSET_PARAM].getValue();
		pole = range / 2;
		bool gate1Con = inputs[GATE1_INPUT].isConnected();
		bool gate2Con = inputs[GATE2_INPUT].isConnected();
		bool in1Con = inputs[IN1_INPUT].isConnected();
		bool in2Con = inputs[IN2_INPUT].isConnected();
		bool out1Con = outputs[OUT1_OUTPUT].isConnected();
		bool out2Con = outputs[OUT2_OUTPUT].isConnected();
		bool noise1Only = (out1Con && (!in1Con && !gate1Con));
		bool gate1Only = (out1Con && (!in1Con && gate1Con));
		bool noise2Only = (out2Con && (!in2Con && !gate2Con));
		bool gate2Only = (out2Con && (!in2Con && gate2Con));
		//bool gate1On2 = (out2Con && (!in2Con && !gate2Con));
		bool invert = params[INVERT_PARAM].getValue() < 1;
		bool uniPolar = params[POLE_PARAM].getValue() > 0;
		//text conditions
		isTH1 = params[ST1_PARAM].getValue();
		isTH2 = params[ST2_PARAM].getValue();
		//TODO: polyphony
		uniPolar ? pole = pole : pole += 5;

		
		

		//TODO: get channels from gate or in
		if (true) {

		}
		nCh1 = inputs[GATE1_INPUT].getChannels();
		outputs[OUT1_OUTPUT].setChannels(nCh1);
		float noise1[nCh1];
		for (int i = 0; i < nCh1; i++) {
			//process gate
			bool SrT1 = T1[i].process(inputs[GATE1_INPUT].getPolyVoltage(i));
			noise1[i] = random::uniform() * range - pole;
			//S&H else T&H
			if (!isTH1 ? SrT1 : T1[i].isHigh()) {//default s&h
				//sample internal or external
				gate1Only ? sample1[i] = noise1[i] : sample1[i] = (((-inputs[IN1_INPUT].getVoltage(i) + 1.5f) / range - -pole) - 2)/* - offset*/;//divided attenuates
			}
			//output ? sampleInternal : sampleExternal
			noise1Only ? outputs[OUT1_OUTPUT].setVoltage(invert ? noise1[i] - offset : -noise1[i] - offset, i) : outputs[OUT1_OUTPUT].setVoltage(invert ? sample1[i] - offset : -sample1[i] - offset, i);
		}
		
		nCh2 = inputs[GATE2_INPUT].getChannels() || inputs[IN2_INPUT].getChannels();
		outputs[OUT2_OUTPUT].setChannels(nCh2);
		float noise2[nCh2] = { random::uniform() * range - pole };
		for (int i = 0; i < nCh2; i++) {
			bool SrT2 = T2[i].process(inputs[GATE2_INPUT].getPolyVoltage(i));
			//T&H else S&H
			if (isTH2 ? SrT2 : T2[i].isHigh()) {//default t&h - with button position 0 (not green)
				//sample internal or external
				gate2Only ? sample2[i] = noise2[i] : sample2[i] = (((-inputs[IN2_INPUT].getVoltage(i) + 1.5f) / range - -pole) - 2)/* - offset*/;
			}
			float n2 = invert ? noise2[i] - offset : -noise2[i] - offset;
			float s2 = invert ? sample2[i] - offset : -sample2[i] - offset;
			noise2Only ? outputs[OUT2_OUTPUT].setVoltage(n2, i) : outputs[OUT2_OUTPUT].setVoltage(s2, i);

		}
		

		


		/*
		///debug
		if (isTH1) {
			//DEBUG("\nGate 1 Connected: %s\nInput 1 Connected: %s\n", gate1Con ? "true" : "false", in1Con ? "true" : "false");
			DEBUG("\nButton enabled: %s", isTH1 ? "true" : "false");
			params[ST1_PARAM].setValue(!isTH1);
		}
		if (isTH2) {
			//DEBUG("\nGate 2 Connected: %s\nInput 2 Connected: %s\n", gate2Con ? "true" : "false", in2Con ? "true" : "false");
			DEBUG("\nButton enabled: %s", isTH2 ? "true" : "false");
			params[ST2_PARAM].setValue(!isTH2);
		}
		*/

	}//process
};

struct label1Widget2 : TransparentWidget {
	SHTH2 *module;
	bool *isTrack1;

	std::shared_ptr<Font> font;

	label1Widget2() {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/segoescb.ttf"));
	}

	void draw(const DrawArgs &labelDisp) override {

		NVGcolor textColor1 = nvgRGB(0xFF, 0xFF, 0xFF);		//White
		NVGcolor textColor2 = nvgRGB(0x00, 0xA8, 0x00);		//Green

		Vec textPos = Vec(*isTrack1 ? 22.457f : 22.838f, 34.906f + 5.302f);

		nvgTextAlign(labelDisp.vg, 1 << 0);
		nvgFontSize(labelDisp.vg, 10);
		nvgFontFaceId(labelDisp.vg, font->handle);
		nvgTextLetterSpacing(labelDisp.vg, .1f);
		if (*isTrack1 < 1) {
			nvgFillColor(labelDisp.vg, textColor1);
		} else if (*isTrack1 > 0) {
			nvgFillColor(labelDisp.vg, textColor2);
		}
		nvgText(labelDisp.vg, textPos.x, textPos.y, *isTrack1 ? "T&H" : "S&H", NULL);
	}
};

struct label2Widget2 : TransparentWidget {
	SHTH2 *module;
	bool *isTrack2;

	std::shared_ptr<Font> font;

	label2Widget2() {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/segoescb.ttf"));
	}

	void draw(const DrawArgs &labelDisp) override {

		NVGcolor textColor1 = nvgRGB(0xFF, 0xFF, 0xFF);		//White
		NVGcolor textColor2 = nvgRGB(0x00, 0xA8, 0x00);		//Green

		Vec textPos = Vec(*isTrack2 ? 22.838f : 22.457f, 344.945f + 5.302f);

		nvgTextAlign(labelDisp.vg, 1 << 0);
		nvgFontSize(labelDisp.vg, 11);
		nvgFontFaceId(labelDisp.vg, font->handle);
		nvgTextLetterSpacing(labelDisp.vg, .1f);
		if (*isTrack2 < 1) {
			nvgFillColor(labelDisp.vg, textColor1);
		} else if (*isTrack2 > 0) {
			nvgFillColor(labelDisp.vg, textColor2);
		}
		nvgText(labelDisp.vg, textPos.x, textPos.y, *isTrack2 ? "S&H" : "T&H", NULL);
	}
};

struct SHTH2Widget : ModuleWidget {
	SHTH2Widget(SHTH2 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BarkSHTH_2.svg")));


		box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		constexpr float portX = 10.387f, btnX = 7.708f, switchY = 200.025f, knbY = 165.136f;
		constexpr float portY[6] = { 49.443f, 86.352f, 121.455f, 242.352f, 277.455f, 314.364f };

		///Ports---
		//Out---
		addOutput(createOutput<BarkPatchPortOut>(Vec(portX, portY[2]), module, SHTH2::OUT1_OUTPUT));
		addOutput(createOutput<BarkPatchPortOut>(Vec(portX, portY[5]), module, SHTH2::OUT2_OUTPUT));
		//In---
		addInput(createInput<BarkPatchPortIn>(Vec(portX, portY[0]), module, SHTH2::GATE1_INPUT));
		addInput(createInput<BarkPatchPortIn>(Vec(portX, portY[3]), module, SHTH2::GATE2_INPUT));
		addInput(createInput<BarkPatchPortIn>(Vec(portX, portY[1]), module, SHTH2::IN1_INPUT));
		addInput(createInput<BarkPatchPortIn>(Vec(portX, portY[4]), module, SHTH2::IN2_INPUT));
		//Knob---
		addParam(createParam<BarkKnob_26>(Vec(9.536f, knbY), module, SHTH2::RANGE_PARAM));
		//Button---
		addParam(createParam<BarkPushButton4>(Vec(btnX, 32.268f), module, SHTH2::ST1_PARAM));
		addParam(createParam<BarkPushButton4>(Vec(btnX, 342.306f), module, SHTH2::ST2_PARAM));
		//Switch---
		addParam(createParam<BarkSwitchSmall>(Vec(5.264f, switchY), module, SHTH2::INVERT_PARAM));
		addParam(createParam<BarkSwitchSmall>(Vec(24.827f, switchY), module, SHTH2::POLE_PARAM));
		//screw---
		addParam(createParam<BarkScrew01>(Vec(2, 3), module, SHTH2::OFFSET_PARAM));	//pos1
		addChild(createWidget<BarkScrew2>(Vec(box.size.x - 13, 367.2)));				//pos4

		if (module != NULL) {
			label1Widget2 *label1 = new label1Widget2;
			label1->isTrack1 = &module->isTH1;
			addChild(label1);
			label2Widget2 *label2 = new label2Widget2;
			label2->isTrack2 = &module->isTH2;
			addChild(label2);
		}
	}
};

Model *modelSHTH2 = createModel<SHTH2, SHTH2Widget>("SHTH2");