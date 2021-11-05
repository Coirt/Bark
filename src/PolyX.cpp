#include "plugin.hpp"
#include "barkComponents.hpp"

using namespace barkComponents;

struct PolyX : Module {
	enum ParamIds {
		MUTEALL_PARAM,
		ENUMS(MUTEFAKE_PARAM, 16),
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(MONO_INPUT, 16),
		NUM_INPUTS
	};
	enum OutputIds {
		POLY_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(chPolySTATE_LIGHT, 16),//when input connected
		ENUMS(chMuteSTATE_LIGHT, 16),//when input connected and mute && when not conneted and mute (0v) level mute / pan centre
		ENUMS(chOpenSTATE_LIGHT, 16),//when not connected and voltage set to 10v, open
		NUM_LIGHTS
	};

	dsp::ClockDivider lightDivider, setSafe;
	dsp::BooleanTrigger setP1; dsp::BooleanTrigger setP2;
	int channels;
	bool sendOpenVolt;
	bool whatIsTheMsg[16];
	//TODO: make description more succinct
	std::string connection = "\nIn order to make level modulation viable,\nwhere no port is connected.We need to send\na voltage.Check the menu option to send\nthat voltage on all disconnected ports\n";

	PolyX() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configSwitch(MUTEALL_PARAM, 0.f, 2.f, 1.f, "Set Channels", { "Open", "Mute" });
		for (int i = 0; i < 16; i++) {
			configSwitch(MUTEFAKE_PARAM + i, 0.f, 1.f, 1.f, "Voltage ch. " + std::to_string(i + 1), { "Open", "Mute" });
			configInput(MONO_INPUT + i, "Channel " + std::to_string(i + 1));
			//inputInfos[MONO_INPUT + i]->description = "";
		}
		configOutput(POLY_OUTPUT, "Polyphonic");
		lightDivider.setDivision(512);//8
		setSafe.setDivision(4);
		onReset();
	}

	void onReset() override {
		channels = 4;	//Auto == -1
		sendOpenVolt = false;
	}

	void process(const ProcessArgs &args) override {
		float openVolts = sendOpenVolt ? 10.f : 0.f;	//if send open volt checked, assign openVolts 10.f
		int lastCh = -1;
		bool setChannelState1 = setP1.process(params[MUTEALL_PARAM].getValue() == 1.f);
		bool setChannelState2 = setP2.process(params[MUTEALL_PARAM].getValue() == 2.f);

		for (int ch = 0; ch < 16; ch++) {
			float volt = 0.f;
			if (inputs[MONO_INPUT + ch].isConnected()) {
				lastCh = ch;
				params[MUTEFAKE_PARAM + ch].getValue() == 0.f ? volt = inputs[MONO_INPUT + ch].getVoltage() : volt = 0.f;
			} else if (!inputs[MONO_INPUT + ch].isConnected()) {
				lastCh = ch;
				/**	
				*	moved option for opening voltage to menu item, check this for sending 10v to level modulation 
				*	such as Poly Mix where there is no poly connection to that input channel				
				*/
				params[MUTEFAKE_PARAM + ch].getValue() == 0.f ? volt = openVolts : volt = 0.f;	
			}
			outputs[POLY_OUTPUT].setVoltage(clamp(volt, -10.f, 10.f), ch);
			
			whatIsTheMsg[ch] = inputs[MONO_INPUT + ch].isConnected();
			
		}
		// In order to allow 0 channels, modify channels directly instead of using `setChannels()`
		outputs[POLY_OUTPUT].channels = (channels >= 0) ? channels : (lastCh + 1);
		int nCh = channels == -1 ? MAX_CH : channels;
		
		
		if (setChannelState1) {
			for (int i = 0; i < nCh; i++) {
				params[MUTEFAKE_PARAM + i].setValue(1.f);
			}
		}

		if (setChannelState2) {
			for (int i = 0; i < nCh; i++) {
				params[MUTEFAKE_PARAM + i].setValue(0.f);
			}
			params[MUTEALL_PARAM].setValue(0.f);
		}
		
		
		// Set channel lights infrequently
		if (lightDivider.process()) {
			for (int c = 0; c < 16; c++) {

				if (!inputs[MONO_INPUT + c].isConnected()) {
					bool isRed = (c < outputs[POLY_OUTPUT].getChannels() && params[MUTEFAKE_PARAM + c].getValue() == 1.f);
					bool isGreen = (c < outputs[POLY_OUTPUT].getChannels() && params[MUTEFAKE_PARAM + c].getValue() == 0.f);
					lights[chMuteSTATE_LIGHT + c].setBrightness(isRed);
					lights[chOpenSTATE_LIGHT + c].setBrightness(isGreen);
				}
				if (inputs[MONO_INPUT + c].isConnected()) {
					lights[chOpenSTATE_LIGHT + c].setBrightness(0);
					bool isBlue = (c < outputs[POLY_OUTPUT].getChannels() && params[MUTEFAKE_PARAM + c].getValue() == 0.f);//
					bool isRed = (c < outputs[POLY_OUTPUT].getChannels() && params[MUTEFAKE_PARAM + c].getValue() == 1.f);
					lights[chPolySTATE_LIGHT + c].setBrightness(isBlue);
					lights[chMuteSTATE_LIGHT + c].setBrightness(isRed);
				}
			}
		}

	}//process

	///store channels to json and retrieve when loading patch
	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "Channels", json_integer(channels));
		json_object_set_new(rootJ, "Open Voltage", json_boolean(sendOpenVolt));
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* channelsJ = json_object_get(rootJ, "Channels");
		if (channelsJ)
			channels = json_integer_value(channelsJ);
		json_t* openJ = json_object_get(rootJ, "Open Voltage");
		if (openJ)
			sendOpenVolt = json_boolean_value(openJ);
	}
};

struct PolyXChannelItem : MenuItem {
	PolyX* module;
	int channels;
	void onAction(const event::Action& e) override {
		module->channels = channels;
	}
};


struct PolyXChannelsItem : MenuItem {
	PolyX* module;
	Menu* createChildMenu() override {
		Menu* menu = new Menu;
		for (int channels = -1; channels <= 16; channels++) {
			PolyXChannelItem* item = new PolyXChannelItem;
			if (channels < 0)
				item->text = "All";
			else
				item->text = string::f("%d", channels);
			item->rightText = CHECKMARK(module->channels == channels);
			item->module = module;
			item->channels = channels;
			menu->addChild(item);
		}
		return menu;
	}
};

struct PolyXWidget : ModuleWidget {
	PolyXWidget(PolyX *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BarkPolyX.svg")));

		box.size = Vec(5 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);


		constexpr float portY[8] = { 303.53f, 268.66f, 233.81f, 198.96f, 164.1f, 129.24f, 94.38f, 59.52f },
				btnY[8] = { 314.03f, 279.16f, 244.31f, 209.46f, 142.83f, 107.98f, 73.11f, 38.26f };
		constexpr float lightCol1 = 44.12f, lightCol2 = lightCol1 + 6.f, lightCol3 = lightCol2 + 6.f, lightCol4 = lightCol3 + 6.f,
				lightRow1 = 353.17f, lightRow2 = lightRow1 - 6.1f, lightRow3 = lightRow2 - 6.1f, lightRow4 = lightRow3 - 6.1f,
				portC1X = 11.78f, portC2X = 39.f, btnC1 = 5.92f, btnC2 = 58.59f;

		///Ports---
		//Out---
		addOutput(createOutput<BarkPatchPortOut>(Vec(7.27f, rackY - 353.13f), module, PolyX::POLY_OUTPUT));
		//In---
		addInput(createInput<BarkPatchPortIn>(Vec(portC1X, rackY - portY[0]), module, PolyX::MONO_INPUT + 0));
		addInput(createInput<BarkPatchPortIn>(Vec(portC2X, rackY - portY[0]), module, PolyX::MONO_INPUT + 1));
		addInput(createInput<BarkPatchPortIn>(Vec(portC1X, rackY - portY[1]), module, PolyX::MONO_INPUT + 2));
		addInput(createInput<BarkPatchPortIn>(Vec(portC2X, rackY - portY[1]), module, PolyX::MONO_INPUT + 3));
		addInput(createInput<BarkPatchPortIn>(Vec(portC1X, rackY - portY[2]), module, PolyX::MONO_INPUT + 4));
		addInput(createInput<BarkPatchPortIn>(Vec(portC2X, rackY - portY[2]), module, PolyX::MONO_INPUT + 5));
		addInput(createInput<BarkPatchPortIn>(Vec(portC1X, rackY - portY[3]), module, PolyX::MONO_INPUT + 6));
		addInput(createInput<BarkPatchPortIn>(Vec(portC2X, rackY - portY[3]), module, PolyX::MONO_INPUT + 7));
		addInput(createInput<BarkPatchPortIn>(Vec(portC1X, rackY - portY[4]), module, PolyX::MONO_INPUT + 8));
		addInput(createInput<BarkPatchPortIn>(Vec(portC2X, rackY - portY[4]), module, PolyX::MONO_INPUT + 9));
		addInput(createInput<BarkPatchPortIn>(Vec(portC1X, rackY - portY[5]), module, PolyX::MONO_INPUT + 10));
		addInput(createInput<BarkPatchPortIn>(Vec(portC2X, rackY - portY[5]), module, PolyX::MONO_INPUT + 11));
		addInput(createInput<BarkPatchPortIn>(Vec(portC1X, rackY - portY[6]), module, PolyX::MONO_INPUT + 12));
		addInput(createInput<BarkPatchPortIn>(Vec(portC2X, rackY - portY[6]), module, PolyX::MONO_INPUT + 13));
		addInput(createInput<BarkPatchPortIn>(Vec(portC1X, rackY - portY[7]), module, PolyX::MONO_INPUT + 14));
		addInput(createInput<BarkPatchPortIn>(Vec(portC2X, rackY - portY[7]), module, PolyX::MONO_INPUT + 15));
		//Button---
		addParam(createParam<BarkChBtnMute>(Vec(41.618f, 24.123f), module, PolyX::MUTEALL_PARAM));
		addParam(createParam<BarkPushButton3>(Vec(btnC1, rackY - btnY[0]), module, PolyX::MUTEFAKE_PARAM + 0));
		addParam(createParam<BarkPushButton3>(Vec(btnC2, rackY - btnY[0]), module, PolyX::MUTEFAKE_PARAM + 1));
		addParam(createParam<BarkPushButton3>(Vec(btnC1, rackY - btnY[1]), module, PolyX::MUTEFAKE_PARAM + 2));
		addParam(createParam<BarkPushButton3>(Vec(btnC2, rackY - btnY[1]), module, PolyX::MUTEFAKE_PARAM + 3));
		addParam(createParam<BarkPushButton3>(Vec(btnC1, rackY - btnY[2]), module, PolyX::MUTEFAKE_PARAM + 4));
		addParam(createParam<BarkPushButton3>(Vec(btnC2, rackY - btnY[2]), module, PolyX::MUTEFAKE_PARAM + 5));
		addParam(createParam<BarkPushButton3>(Vec(btnC1, rackY - btnY[3]), module, PolyX::MUTEFAKE_PARAM + 6));
		addParam(createParam<BarkPushButton3>(Vec(btnC2, rackY - btnY[3]), module, PolyX::MUTEFAKE_PARAM + 7));
		addParam(createParam<BarkPushButton3>(Vec(btnC1, rackY - btnY[4]), module, PolyX::MUTEFAKE_PARAM + 8));
		addParam(createParam<BarkPushButton3>(Vec(btnC2, rackY - btnY[4]), module, PolyX::MUTEFAKE_PARAM + 9));
		addParam(createParam<BarkPushButton3>(Vec(btnC1, rackY - btnY[5]), module, PolyX::MUTEFAKE_PARAM + 10));
		addParam(createParam<BarkPushButton3>(Vec(btnC2, rackY - btnY[5]), module, PolyX::MUTEFAKE_PARAM + 11));
		addParam(createParam<BarkPushButton3>(Vec(btnC1, rackY - btnY[6]), module, PolyX::MUTEFAKE_PARAM + 12));
		addParam(createParam<BarkPushButton3>(Vec(btnC2, rackY - btnY[6]), module, PolyX::MUTEFAKE_PARAM + 13));
		addParam(createParam<BarkPushButton3>(Vec(btnC1, rackY - btnY[7]), module, PolyX::MUTEFAKE_PARAM + 14));
		addParam(createParam<BarkPushButton3>(Vec(btnC2, rackY - btnY[7]), module, PolyX::MUTEFAKE_PARAM + 15));
		//screw---
		addChild(createWidget<RandomRotateScrew>(Vec(2.7f, 2.7f)));				//pos1
		addChild(createWidget<RandomRotateScrew>(Vec(box.size.x - 12.3f, 367.7f)));		//pos4
		//Light---
		///lightRow1
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol1, rackY - lightRow1), module, PolyX::chPolySTATE_LIGHT + 0));//on mono in
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol1, rackY - lightRow1), module, PolyX::chMuteSTATE_LIGHT + 0));//on mono in / no connection 
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol1, rackY - lightRow1), module, PolyX::chOpenSTATE_LIGHT + 0));//no connection
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol2, rackY - lightRow1), module, PolyX::chPolySTATE_LIGHT + 1));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol2, rackY - lightRow1), module, PolyX::chMuteSTATE_LIGHT + 1));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol2, rackY - lightRow1), module, PolyX::chOpenSTATE_LIGHT + 1));
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol3, rackY - lightRow1), module, PolyX::chPolySTATE_LIGHT + 2));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol3, rackY - lightRow1), module, PolyX::chMuteSTATE_LIGHT + 2));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol3, rackY - lightRow1), module, PolyX::chOpenSTATE_LIGHT + 2));
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol4, rackY - lightRow1), module, PolyX::chPolySTATE_LIGHT + 3));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol4, rackY - lightRow1), module, PolyX::chMuteSTATE_LIGHT + 3));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol4, rackY - lightRow1), module, PolyX::chOpenSTATE_LIGHT + 3));
		///lightRow2
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol1, rackY - lightRow2), module, PolyX::chPolySTATE_LIGHT + 4));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol1, rackY - lightRow2), module, PolyX::chMuteSTATE_LIGHT + 4));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol1, rackY - lightRow2), module, PolyX::chOpenSTATE_LIGHT + 4));
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol2, rackY - lightRow2), module, PolyX::chPolySTATE_LIGHT + 5));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol2, rackY - lightRow2), module, PolyX::chMuteSTATE_LIGHT + 5));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol2, rackY - lightRow2), module, PolyX::chOpenSTATE_LIGHT + 5));
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol3, rackY - lightRow2), module, PolyX::chPolySTATE_LIGHT + 6));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol3, rackY - lightRow2), module, PolyX::chMuteSTATE_LIGHT + 6));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol3, rackY - lightRow2), module, PolyX::chOpenSTATE_LIGHT + 6));
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol4, rackY - lightRow2), module, PolyX::chPolySTATE_LIGHT + 7));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol4, rackY - lightRow2), module, PolyX::chMuteSTATE_LIGHT + 7));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol4, rackY - lightRow2), module, PolyX::chOpenSTATE_LIGHT + 7));
		///lightRow3
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol1, rackY - lightRow3), module, PolyX::chPolySTATE_LIGHT + 8));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol1, rackY - lightRow3), module, PolyX::chMuteSTATE_LIGHT + 8));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol1, rackY - lightRow3), module, PolyX::chOpenSTATE_LIGHT + 8));
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol2, rackY - lightRow3), module, PolyX::chPolySTATE_LIGHT + 9));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol2, rackY - lightRow3), module, PolyX::chMuteSTATE_LIGHT + 9));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol2, rackY - lightRow3), module, PolyX::chOpenSTATE_LIGHT + 9));
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol3, rackY - lightRow3), module, PolyX::chPolySTATE_LIGHT + 10));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol3, rackY - lightRow3), module, PolyX::chMuteSTATE_LIGHT + 10));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol3, rackY - lightRow3), module, PolyX::chOpenSTATE_LIGHT + 10));
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol4, rackY - lightRow3), module, PolyX::chPolySTATE_LIGHT + 11));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol4, rackY - lightRow3), module, PolyX::chMuteSTATE_LIGHT + 11));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol4, rackY - lightRow3), module, PolyX::chOpenSTATE_LIGHT + 11));
		///lightRow4
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol1, rackY - lightRow4), module, PolyX::chPolySTATE_LIGHT + 12));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol1, rackY - lightRow4), module, PolyX::chMuteSTATE_LIGHT + 12));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol1, rackY - lightRow4), module, PolyX::chOpenSTATE_LIGHT + 12));
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol2, rackY - lightRow4), module, PolyX::chPolySTATE_LIGHT + 13));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol2, rackY - lightRow4), module, PolyX::chMuteSTATE_LIGHT + 13));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol2, rackY - lightRow4), module, PolyX::chOpenSTATE_LIGHT + 13));
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol3, rackY - lightRow4), module, PolyX::chPolySTATE_LIGHT + 14));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol3, rackY - lightRow4), module, PolyX::chMuteSTATE_LIGHT + 14));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol3, rackY - lightRow4), module, PolyX::chOpenSTATE_LIGHT + 14));
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol4, rackY - lightRow4), module, PolyX::chPolySTATE_LIGHT + 15));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol4, rackY - lightRow4), module, PolyX::chMuteSTATE_LIGHT + 15));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol4, rackY - lightRow4), module, PolyX::chOpenSTATE_LIGHT + 15));
	}
	
	void appendContextMenu(Menu* menu) override {
		PolyX* module = dynamic_cast<PolyX*>(this->module);

		menu->addChild(new MenuSeparator);
		menu->addChild(createBoolPtrMenuItem("Open state, send 10v", "", &module->sendOpenVolt));
		//menu->addChild(new MenuSeparator);
		//menu->addChild(new MenuEntry);

		PolyXChannelsItem* channelsItem = new PolyXChannelsItem;
		channelsItem->text = "Channels";
		channelsItem->rightText = RIGHT_ARROW;
		channelsItem->module = module;
		menu->addChild(channelsItem);
	}

};

Model *modelPolyX = createModel<PolyX, PolyXWidget>("PolyX");
