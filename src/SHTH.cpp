#include "plugin.hpp"
#include "barkComponents.hpp"

using namespace barkComponents;

struct SHTH : Module {

	enum ParamIds {
		SMP_OR_TRK_PARAM,
		RANGE_PARAM,
		POLE_PARAM,
		INVERT_PARAM,
		OFFSET_PARAM,
		SAMPLE_PARAM,
		INCREMENT_PARAM,
		DECREMENT_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		GATE_INPUT,
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

	dsp::SchmittTrigger sampleTrg[MAX_CH];
	dsp::SchmittTrigger nextChTrig, prevChTrig, paramsChange;
	//dsp::SchmittTrigger paramsChanged1, paramsChanged2, paramsChanged3;
	dsp::ClockDivider setSafely;

	bool init = true;
	bool gateCon = false;
	bool btnPressed = false;
	bool normalMode = true;
	int nCh = 1;
	int index;
	int prevIndex;
	int* displayNumber;
	int overrideCh = 1;

	float sample1[MAX_CH] = { 0.f };

	//array to store params values
	bool sampleParam[MAX_CH];
	bool invertParam[MAX_CH];
	bool polarpatParam[MAX_CH];
	bool sampleNoise[MAX_CH];
	float scaleParam[MAX_CH];
	float offsetParam[MAX_CH];
	

	SHTH() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam<tpMode_sh>(SMP_OR_TRK_PARAM, 0.f, 1.f, 0.f, "Mode");
		configParam<tpIntExt>(SAMPLE_PARAM, 0.f, 1.f, 1.f, "Sample Mode");
		configParam(INCREMENT_PARAM, 0.f, 1.f, 0.f, "Next");
		configParam(DECREMENT_PARAM, 0.f, 1.f, 0.f, "Prev");
		configParam<tpPlusMinus>(POLE_PARAM, 0.f, 1.f, 0.f, "Mode");
		configParam<tpOnOffBtn>(INVERT_PARAM, 0.f, 1.f, 0.f, "Invert");
		configParam(RANGE_PARAM, (1 / 12.0)* 6.f, 10.f, 1.f, "V/Oct Range", "v");
		configParam(OFFSET_PARAM, -5.f, 5.f, 0.f, "Offset", "v");

		//set display to address of index
		displayNumber = &index;
		onInit();
		setSafely.setDivision(256);
	}

	void onInit() {
		for (int i = 0; i < MAX_CH; i++) {
			sampleParam[i] = false;
			invertParam[i] = false;
			polarpatParam[i] = false;
			sampleNoise[i] = true;
			scaleParam[i] = 1.f;
			offsetParam[i] = 0.f;
		}
		index = 0;
		prevIndex = 0;
		nCh = 1;
		overrideCh = 1;
		init = true;
		normalMode = true;
		//debugReset();
	}

	void onReset() override {
		onInit();
	}

	void process(const ProcessArgs &args) override {
		bool storeValues = false;
		bool nextChannel = false;
		bool prevChannel = false;
		if (setSafely.process()) {
			storeValues = paramsChange.process(inputs[GATE_INPUT].getPolyVoltage(index) > .5f);
		}
		nextChannel = nextChTrig.process(params[INCREMENT_PARAM].getValue());
		prevChannel = prevChTrig.process(params[DECREMENT_PARAM].getValue());

		//conditions
		bool gateCon = inputs[GATE_INPUT].isConnected(),
			inCon = inputs[IN_INPUT].isConnected(),
			outCon = outputs[OUT_OUTPUT].isConnected(),
			noiseOnly = (outCon && (!inCon && !gateCon)),
			gateOnly = (outCon && (!inCon && gateCon));

		//get number of channels
		//noiseOnly ? nCh = 1 : nCh = inputs[GATE_INPUT].getChannels();
		/*
		if (normalMode) {
			nCh = inputs[GATE_INPUT].isMonophonic() ? 1 : inputs[GATE_INPUT].getChannels();
		}
		else {
			nCh = overrideCh;
		}*/
		nCh = normalMode ? inputs[GATE_INPUT].isMonophonic() ? 1 : inputs[GATE_INPUT].getChannels() : overrideCh;
		outputs[OUT_OUTPUT].setChannels(nCh);

		//set noise nCh
		float noise1[nCh];

		//S&H or T&H
		for (int i = 0; i < nCh; i++) {
			//bool isTH[nCh] = { false };
			bool isTH[nCh];
			std::fill(isTH, isTH + nCh, false);
			isTH[i] = sampleParam[i];
			//bool inverted[nCh] = { false };
			bool inverted[nCh];
			std::fill(inverted, inverted + nCh, false);
			inverted[i] = invertParam[i];
			//bool uniPolar[nCh] = { false };
			bool uniPolar[nCh];
			std::fill(uniPolar, uniPolar + nCh, false);
			uniPolar[i] = polarpatParam[i];
			
			bool ext[nCh];
			ext[i] = sampleNoise[i];
			float pole[nCh];
			pole[i] = uniPolar[i] ? !ext[i] ? (scaleParam[i] / 2) - (scaleParam[i] * 5.5f) : (scaleParam[i] / 2) - scaleParam[i] : 0;

			bool outputNoise[nCh];
			outputNoise[i] = inputs[GATE_INPUT].getVoltage(i) == 0.f;

			bool SrT1;
			//process gate
			if (normalMode) {
				SrT1 = sampleTrg[i].process(inputs[GATE_INPUT].getPolyVoltage(i));
			} 
			else {
				SrT1 = sampleTrg[i].process(inputs[GATE_INPUT].getVoltage());
				//SrT1 = sampleTrg[i].process(inputs[GATE_INPUT].getPolyVoltage(overrideCh));
			}

			noise1[i] = random::uniform() * scaleParam[i] + pole[i];
			//if S&H else T&H
			if (isTH[i] ? sampleTrg[i].isHigh() : SrT1) {
				//sample internal else external
				sampleNoise[i] ? sample1[i] = noise1[i] :
								sample1[i] = (inputs[IN_INPUT].getVoltage(i) * scaleParam[i] + pole[i]) / 10;	
				
			}
			//set outputs sampleInternal else sampleExternal
			noiseOnly ? outputs[OUT_OUTPUT].setVoltage(inverted[i] ? -noise1[i] + offsetParam[i] :
														noise1[i] + offsetParam[i], i) :
							outputs[OUT_OUTPUT].setVoltage(inverted[i] ? -sample1[i] + offsetParam[i] :
															sample1[i] + offsetParam[i], i);
		}

		
		if (storeValues) {
			init = false;
			sampleParam[index] = params[SMP_OR_TRK_PARAM].getValue();

			invertParam[index] = params[INVERT_PARAM].getValue();
			polarpatParam[index] = params[POLE_PARAM].getValue();

			sampleNoise[index] = params[SAMPLE_PARAM].getValue();

			scaleParam[index] = params[RANGE_PARAM].getValue();
			offsetParam[index] = params[OFFSET_PARAM].getValue();
		}
		

		if (nextChannel) {
			init = false;
			btnPressed = true;

			if (btnPressed == true) {
				init = false;

				sampleParam[index] = params[SMP_OR_TRK_PARAM].getValue();

				invertParam[index] = params[INVERT_PARAM].getValue();
				polarpatParam[index] = params[POLE_PARAM].getValue();

				sampleNoise[index] = params[SAMPLE_PARAM].getValue();

				scaleParam[index] = params[RANGE_PARAM].getValue();
				offsetParam[index] = params[OFFSET_PARAM].getValue();
				
				btnPressed = !btnPressed;
			}
			
			if (index >= 0 && index < (nCh - 1)) {
				prevIndex = index;
				index++;
			} else if (index >= (nCh - 1)) {
				prevIndex = nCh - 1;
				index = 0;
			}
			params[SMP_OR_TRK_PARAM].setValue(sampleParam[index]);

			params[INVERT_PARAM].setValue(invertParam[index]);
			params[POLE_PARAM].setValue(polarpatParam[index]);

			params[SAMPLE_PARAM].setValue(sampleNoise[index]);

			params[RANGE_PARAM].setValue(scaleParam[index]);
			params[OFFSET_PARAM].setValue(offsetParam[index]);

			btnPressed = !btnPressed;
		}

		if (prevChannel) {
			init = false;
			btnPressed = true;
			
			if (btnPressed == true) {
				init = false;

				sampleParam[index] = params[SMP_OR_TRK_PARAM].getValue();

				invertParam[index] = params[INVERT_PARAM].getValue();
				polarpatParam[index] = params[POLE_PARAM].getValue();

				sampleNoise[index] = params[SAMPLE_PARAM].getValue();

				scaleParam[index] = params[RANGE_PARAM].getValue();
				offsetParam[index] = params[OFFSET_PARAM].getValue();

				btnPressed = !btnPressed;
			}
			
			if (index > 0 && index < nCh) {
				prevIndex = index;
				index--;
			} else if (index <= 0) {
				prevIndex = 0;
				index = nCh - 1;
			}

			params[SMP_OR_TRK_PARAM].setValue(sampleParam[index]);

			params[INVERT_PARAM].setValue(invertParam[index]);
			params[POLE_PARAM].setValue(polarpatParam[index]);

			params[SAMPLE_PARAM].setValue(sampleNoise[index]);

			params[RANGE_PARAM].setValue(scaleParam[index]);
			params[OFFSET_PARAM].setValue(offsetParam[index]);

			btnPressed = !btnPressed;
		}
	}//process

	json_t* dataToJson() override {

		json_t *rootJ = json_object();
		json_t *sampleParametersJ = json_array();
		json_t *invertParametersJ = json_array();
		json_t *polarityParametersJ = json_array();
		json_t *sampleNoiseParametersJ = json_array();
		json_t *scaleParametersJ = json_array();
		json_t *offsetParametersJ = json_array();

		for (int i = 0; i < MAX_CH; i++) {
			json_array_insert_new(sampleParametersJ, i, json_integer(sampleParam[i]));
			json_array_insert_new(invertParametersJ, i, json_integer((int)invertParam[i]));
			json_array_insert_new(polarityParametersJ, i, json_integer((int)polarpatParam[i]));
			json_array_insert_new(sampleNoiseParametersJ, i, json_integer((int)sampleNoise[i]));
			json_array_insert_new(scaleParametersJ, i, json_real(scaleParam[i]));
			json_array_insert_new(offsetParametersJ, i, json_real(offsetParam[i]));
		}
		
		json_object_set_new(rootJ, "Current Index", json_integer(index));
		json_object_set_new(rootJ, "Number of Channels", json_integer(nCh));
		json_object_set_new(rootJ, "Mode", json_integer((int)normalMode));
		json_object_set_new(rootJ, "Number of Channels Override", json_integer(overrideCh));
		json_object_set_new(rootJ, "S&H (int)bool", sampleParametersJ);
		json_object_set_new(rootJ, "Inverted (int)bool", invertParametersJ);
		json_object_set_new(rootJ, "Uni-Polar (int)bool", polarityParametersJ);
		json_object_set_new(rootJ, "Noise (int)bool", sampleNoiseParametersJ);
		json_object_set_new(rootJ, "Range", scaleParametersJ);
		json_object_set_new(rootJ, "Offset", offsetParametersJ);

		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {

		json_t *currentChJ = json_object_get(rootJ, "Current Index");
		if (currentChJ) {
			index = json_integer_value(currentChJ);
		}
		json_t *nChJ = json_object_get(rootJ, "Number of Channels");
		if (nChJ) {
			nCh = json_integer_value(nChJ);
		}
		json_t *modeJ = json_object_get(rootJ, "Mode");
		if (modeJ) {
			normalMode = json_integer_value(modeJ);
		}
		json_t *overrideChJ = json_object_get(rootJ, "Number of Channels Override");
		if (overrideChJ) {
			overrideCh = json_integer_value(overrideChJ);
		}
		json_t *sampleParametersJ = json_object_get(rootJ, "S&H (int)bool");
		if (sampleParametersJ) {
			for (int i = 0; i < MAX_CH; i++) {
				json_t *sampleParamArrJ = json_array_get(sampleParametersJ, i);
				if (sampleParamArrJ) {
					sampleParam[i] = json_integer_value(sampleParamArrJ);
				}
			}
		}

		json_t *invertParametersJ = json_object_get(rootJ, "Inverted (int)bool");
		if (invertParametersJ) {
			for (int i = 0; i < MAX_CH; i++) {
				json_t *invertParamArrJ = json_array_get(invertParametersJ, i);
				if (invertParamArrJ) {
					invertParam[i] = json_integer_value(invertParamArrJ);
				}
			}
		}

		json_t *polarityParametersJ = json_object_get(rootJ, "Uni-Polar (int)bool");
		if (polarityParametersJ) {
			for (int i = 0; i < MAX_CH; i++) {
				json_t *polarityParamArrJ = json_array_get(polarityParametersJ, i);
				if (polarityParamArrJ) {
					polarpatParam[i] = json_integer_value(polarityParamArrJ);
				}
			}
		}

		json_t *sampleNoiseParametersJ = json_object_get(rootJ, "Noise (int)bool");
		if (sampleNoiseParametersJ) {
			for (int i = 0; i < MAX_CH; i++) {
				json_t *sampleNoiseArrJ = json_array_get(sampleNoiseParametersJ, i);
				if (sampleNoiseArrJ) {
					sampleNoise[i] = json_integer_value(sampleNoiseArrJ);
				}
			}
		}

		json_t *scaleParametersJ = json_object_get(rootJ, "Range");
		if (scaleParametersJ) {
			for (int i = 0; i < MAX_CH; i++) {
				json_t *scaleParamArrJ = json_array_get(scaleParametersJ, i);
				if (scaleParamArrJ) {
					scaleParam[i] = json_real_value(scaleParamArrJ);
				}
			}
		}

		json_t *offsetParametersJ = json_object_get(rootJ, "Offset");
		if (offsetParametersJ) {
			for (int i = 0; i < MAX_CH; i++) {
				json_t *offsetParamArrJ = json_array_get(offsetParametersJ, i);
				if (offsetParamArrJ) {
					offsetParam[i] = json_real_value(offsetParamArrJ);
				}
			}
		}

	}
};

struct SHTHnChItem : MenuItem {
	SHTH* module;
	int channels;
	void onAction(const event::Action& e) override {
		module->overrideCh = channels;
	}
};

struct SHTHChannelsItem : MenuItem {
	SHTH* module;
	Menu* createChildMenu() override {
		Menu* nChans = new Menu;
		for (int ch = 1; ch <= MAX_CH; ch++) {
			if (module->normalMode) break;
			SHTHnChItem* nChan = new SHTHnChItem;
			if (ch == 1)
				nChan->text = "Monophonic";
			else
				nChan->text = string::f("%d", ch);
			nChan->rightText = CHECKMARK(module->overrideCh == ch);
			nChan->module = module;
			nChan->channels = ch;
			nChans->addChild(nChan);
		}
		return nChans;
	}
};

struct SHTHGateModeItem : MenuItem {
	SHTH* module;
	
	void onAction(const event::Action &e) override {
		if (module->normalMode) {
			module->normalMode = false;
		} else {
			module->normalMode = true;
		}
		
	}
};

struct ChannelNumberWidget : TransparentWidget {
	SHTH *module;
	int* chNum;

	std::shared_ptr<Font> font;

	ChannelNumberWidget() {
		font = FONT;
	}

	void draw(const DrawArgs &labelDisp) override {

		NVGcolor textColor = nvgRGB(0xFF, 0xFF, 0xFF);

		Vec textPos = Vec(22.5f, 281.089f);

		nvgTextAlign(labelDisp.vg, 1 << 1);
		nvgFontSize(labelDisp.vg, 9);
		nvgFontFaceId(labelDisp.vg, font->handle);
		nvgTextLetterSpacing(labelDisp.vg, .1f);
		char display_string_channel[5];
		snprintf(display_string_channel, sizeof(display_string_channel), "%1.d", (*chNum) + 1);
		nvgText(labelDisp.vg, textPos.x, textPos.y + 5.856f, display_string_channel, NULL);
		nvgFillColor(labelDisp.vg, textColor);		

	}
};

struct SHTHWidget : ModuleWidget {
	SHTHWidget(SHTH *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BarkSHTH_3.svg")));

		constexpr float portX = 10.387f, btnX = 7.708f, switchY = 180.15f, btnY = 275.411f;
		constexpr float portY[6] = { 49.443f, 87.454f, 126.411f }, knbY[2] = { 219.6f, 309.495f };

		///Ports---
		//Out---
		addOutput(createOutput<BarkPatchPortOut>(Vec(portX, portY[2]), module, SHTH::OUT_OUTPUT));
		//In---
		addInput(createInput<BarkPatchPortIn>(Vec(portX, portY[0]), module, SHTH::GATE_INPUT));
		addInput(createInput<BarkPatchPortIn>(Vec(portX, portY[1]), module, SHTH::IN_INPUT));
		//Knob---
		addParam(createParam<BarkKnob_26>(Vec(9.536f, knbY[0]), module, SHTH::RANGE_PARAM));
		addParam(createParam<BarkKnob_26>(Vec(9.536f, knbY[1]), module, SHTH::OFFSET_PARAM));
		//Button---
		addParam(createParam<BarkPushButtonSH>(Vec(btnX, 32.268f), module, SHTH::SMP_OR_TRK_PARAM));
		addParam(createParam<BarkPushButton2>(Vec(17.25f, 380 - 119.45f), module, SHTH::SAMPLE_PARAM));
		addParam(createParam<BarkPushButtonDec>(Vec(2.739f, btnY), module, SHTH::DECREMENT_PARAM));
		addParam(createParam<BarkPushButtonInc>(Vec(31.975f, btnY), module, SHTH::INCREMENT_PARAM)); 
		//Switch---
		addParam(createParam<BarkSwitchSmall>(Vec(5.264f, switchY), module, SHTH::INVERT_PARAM));
		addParam(createParam<BarkSwitchSmall>(Vec(24.827f, switchY), module, SHTH::POLE_PARAM));
		//screw---
		addChild(createWidget<BarkScrew1>(Vec(2.7f, 2.7f)));					//pos1
		addChild(createWidget<BarkScrew2>(Vec(box.size.x - 12.3f, 367.7f)));	//pos4

		if (module != NULL) {
			ChannelNumberWidget *label = new ChannelNumberWidget;
			label->chNum = module->displayNumber;
			addChild(label);
		}
	}

	void appendContextMenu(Menu* menu) override {
		SHTH* module = dynamic_cast<SHTH*>(this->module);
		assert(module);

		int numCh = module->normalMode ? module->nCh : module->overrideCh;

		//blank space
		//menu->addChild(new MenuEntry);	
		menu->addChild(new MenuSeparator());

		SHTHGateModeItem *modeItem = new SHTHGateModeItem;
		modeItem->text = "Mode: ";
		modeItem->rightText = module->normalMode ? "Normal✔  Override" : "Normal   Override✔";
		modeItem->module = module;
		menu->addChild(modeItem);

		SHTHChannelsItem* overrideChan = new SHTHChannelsItem;
		overrideChan->text = "Number of Channels:       " + string::f("%d", numCh);
		overrideChan->rightText = RIGHT_ARROW;
		overrideChan->module = module;
		//when normalMode == true disable menu
		overrideChan->disabled = module->normalMode;
		menu->addChild(overrideChan);
		
	}
	
};

Model *modelSHTH = createModel<SHTH, SHTHWidget>("SHTH");
