#include "plugin.hpp"
#include "barkComponents.hpp"

using namespace barkComponents;

struct PolyX : Module {
	enum ParamIds {
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

	dsp::ClockDivider lightDivider;
	int channels;

	PolyX() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		for (int i = 0; i < 16; i++) {
			configParam<tpMute10v>(MUTEFAKE_PARAM + i, 0.f, 1.f, 1.f, "Voltage");
		}
		lightDivider.setDivision(8);//512
		onReset();
	}

	void onReset() override {
		channels = -1;
	}
	void process(const ProcessArgs &args) override {
		int lastCh = -1;
		for (int ch = 0; ch < 16; ch++) {
			float volt = 0.f;
			if (inputs[MONO_INPUT + ch].isConnected()) {
				lastCh = ch;
				params[MUTEFAKE_PARAM + ch].getValue() == 0.f ? volt = inputs[MONO_INPUT + ch].getVoltage() : volt = 0.f;
				/***PolyMix
				might need context menu options!
				When polyaudio, disconnected channels that are unmuted send 10v that DC offsets that channel
				When polylevel, no issue
				When polypan,
				*/
			} else if (!inputs[MONO_INPUT + ch].isConnected()) {
				lastCh = ch;
				params[MUTEFAKE_PARAM + ch].getValue() == 0.f ? volt = 10.f : volt = 0.f;
			}
			outputs[POLY_OUTPUT].setVoltage(clamp(volt, -10.f, 10.f), ch);
		}
		// In order to allow 0 channels, modify channels directly instead of using `setChannels()`
		outputs[POLY_OUTPUT].channels = (channels >= 0) ? channels : (lastCh + 1);

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
};

struct PolyXWidget : ModuleWidget {
	PolyXWidget(PolyX *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BarkPolyX.svg")));

		//constexpr int rackY = 380;
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
		addParam(createParam<BarkPushButton2>(Vec(btnC1, rackY - btnY[0]), module, PolyX::MUTEFAKE_PARAM + 0));
		addParam(createParam<BarkPushButton2>(Vec(btnC2, rackY - btnY[0]), module, PolyX::MUTEFAKE_PARAM + 1));
		addParam(createParam<BarkPushButton2>(Vec(btnC1, rackY - btnY[1]), module, PolyX::MUTEFAKE_PARAM + 2));
		addParam(createParam<BarkPushButton2>(Vec(btnC2, rackY - btnY[1]), module, PolyX::MUTEFAKE_PARAM + 3));
		addParam(createParam<BarkPushButton2>(Vec(btnC1, rackY - btnY[2]), module, PolyX::MUTEFAKE_PARAM + 4));
		addParam(createParam<BarkPushButton2>(Vec(btnC2, rackY - btnY[2]), module, PolyX::MUTEFAKE_PARAM + 5));
		addParam(createParam<BarkPushButton2>(Vec(btnC1, rackY - btnY[3]), module, PolyX::MUTEFAKE_PARAM + 6));
		addParam(createParam<BarkPushButton2>(Vec(btnC2, rackY - btnY[3]), module, PolyX::MUTEFAKE_PARAM + 7));
		addParam(createParam<BarkPushButton2>(Vec(btnC1, rackY - btnY[4]), module, PolyX::MUTEFAKE_PARAM + 8));
		addParam(createParam<BarkPushButton2>(Vec(btnC2, rackY - btnY[4]), module, PolyX::MUTEFAKE_PARAM + 9));
		addParam(createParam<BarkPushButton2>(Vec(btnC1, rackY - btnY[5]), module, PolyX::MUTEFAKE_PARAM + 10));
		addParam(createParam<BarkPushButton2>(Vec(btnC2, rackY - btnY[5]), module, PolyX::MUTEFAKE_PARAM + 11));
		addParam(createParam<BarkPushButton2>(Vec(btnC1, rackY - btnY[6]), module, PolyX::MUTEFAKE_PARAM + 12));
		addParam(createParam<BarkPushButton2>(Vec(btnC2, rackY - btnY[6]), module, PolyX::MUTEFAKE_PARAM + 13));
		addParam(createParam<BarkPushButton2>(Vec(btnC1, rackY - btnY[7]), module, PolyX::MUTEFAKE_PARAM + 14));
		addParam(createParam<BarkPushButton2>(Vec(btnC2, rackY - btnY[7]), module, PolyX::MUTEFAKE_PARAM + 15));
		//screw---
		addChild(createWidget<BarkScrew3>(Vec(2, 3)));							//pos1
		addChild(createWidget<BarkScrew4>(Vec(box.size.x - 13, 367.2)));			//pos4
		//Light---
		///lightRow1
		addChild(createLight<SmallestLight<PolyLight>>(Vec(lightCol1, rackY - lightRow1), module, PolyX::chPolySTATE_LIGHT + 0));//on mono in
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol1, rackY - lightRow1), module, PolyX::chMuteSTATE_LIGHT + 0));//on mono in / no connection 
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol1, rackY - lightRow1), module, PolyX::chOpenSTATE_LIGHT + 0));//no connection
		addChild(createLight<SmallestLight<PolyLight>>(Vec(lightCol2, rackY - lightRow1), module, PolyX::chPolySTATE_LIGHT + 1));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol2, rackY - lightRow1), module, PolyX::chMuteSTATE_LIGHT + 1));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol2, rackY - lightRow1), module, PolyX::chOpenSTATE_LIGHT + 1));
		addChild(createLight<SmallestLight<PolyLight>>(Vec(lightCol3, rackY - lightRow1), module, PolyX::chPolySTATE_LIGHT + 2));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol3, rackY - lightRow1), module, PolyX::chMuteSTATE_LIGHT + 2));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol3, rackY - lightRow1), module, PolyX::chOpenSTATE_LIGHT + 2));
		addChild(createLight<SmallestLight<PolyLight>>(Vec(lightCol4, rackY - lightRow1), module, PolyX::chPolySTATE_LIGHT + 3));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol4, rackY - lightRow1), module, PolyX::chMuteSTATE_LIGHT + 3));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol4, rackY - lightRow1), module, PolyX::chOpenSTATE_LIGHT + 3));
		///lightRow2
		addChild(createLight<SmallestLight<PolyLight>>(Vec(lightCol1, rackY - lightRow2), module, PolyX::chPolySTATE_LIGHT + 4));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol1, rackY - lightRow2), module, PolyX::chMuteSTATE_LIGHT + 4));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol1, rackY - lightRow2), module, PolyX::chOpenSTATE_LIGHT + 4));
		addChild(createLight<SmallestLight<PolyLight>>(Vec(lightCol2, rackY - lightRow2), module, PolyX::chPolySTATE_LIGHT + 5));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol2, rackY - lightRow2), module, PolyX::chMuteSTATE_LIGHT + 5));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol2, rackY - lightRow2), module, PolyX::chOpenSTATE_LIGHT + 5));
		addChild(createLight<SmallestLight<PolyLight>>(Vec(lightCol3, rackY - lightRow2), module, PolyX::chPolySTATE_LIGHT + 6));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol3, rackY - lightRow2), module, PolyX::chMuteSTATE_LIGHT + 6));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol3, rackY - lightRow2), module, PolyX::chOpenSTATE_LIGHT + 6));
		addChild(createLight<SmallestLight<PolyLight>>(Vec(lightCol4, rackY - lightRow2), module, PolyX::chPolySTATE_LIGHT + 7));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol4, rackY - lightRow2), module, PolyX::chMuteSTATE_LIGHT + 7));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol4, rackY - lightRow2), module, PolyX::chOpenSTATE_LIGHT + 7));
		///lightRow3
		addChild(createLight<SmallestLight<PolyLight>>(Vec(lightCol1, rackY - lightRow3), module, PolyX::chPolySTATE_LIGHT + 8));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol1, rackY - lightRow3), module, PolyX::chMuteSTATE_LIGHT + 8));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol1, rackY - lightRow3), module, PolyX::chOpenSTATE_LIGHT + 8));
		addChild(createLight<SmallestLight<PolyLight>>(Vec(lightCol2, rackY - lightRow3), module, PolyX::chPolySTATE_LIGHT + 9));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol2, rackY - lightRow3), module, PolyX::chMuteSTATE_LIGHT + 9));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol2, rackY - lightRow3), module, PolyX::chOpenSTATE_LIGHT + 9));
		addChild(createLight<SmallestLight<PolyLight>>(Vec(lightCol3, rackY - lightRow3), module, PolyX::chPolySTATE_LIGHT + 10));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol3, rackY - lightRow3), module, PolyX::chMuteSTATE_LIGHT + 10));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol3, rackY - lightRow3), module, PolyX::chOpenSTATE_LIGHT + 10));
		addChild(createLight<SmallestLight<PolyLight>>(Vec(lightCol4, rackY - lightRow3), module, PolyX::chPolySTATE_LIGHT + 11));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol4, rackY - lightRow3), module, PolyX::chMuteSTATE_LIGHT + 11));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol4, rackY - lightRow3), module, PolyX::chOpenSTATE_LIGHT + 11));
		///lightRow4
		addChild(createLight<SmallestLight<PolyLight>>(Vec(lightCol1, rackY - lightRow4), module, PolyX::chPolySTATE_LIGHT + 12));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol1, rackY - lightRow4), module, PolyX::chMuteSTATE_LIGHT + 12));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol1, rackY - lightRow4), module, PolyX::chOpenSTATE_LIGHT + 12));
		addChild(createLight<SmallestLight<PolyLight>>(Vec(lightCol2, rackY - lightRow4), module, PolyX::chPolySTATE_LIGHT + 13));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol2, rackY - lightRow4), module, PolyX::chMuteSTATE_LIGHT + 13));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol2, rackY - lightRow4), module, PolyX::chOpenSTATE_LIGHT + 13));
		addChild(createLight<SmallestLight<PolyLight>>(Vec(lightCol3, rackY - lightRow4), module, PolyX::chPolySTATE_LIGHT + 14));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol3, rackY - lightRow4), module, PolyX::chMuteSTATE_LIGHT + 14));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol3, rackY - lightRow4), module, PolyX::chOpenSTATE_LIGHT + 14));
		addChild(createLight<SmallestLight<PolyLight>>(Vec(lightCol4, rackY - lightRow4), module, PolyX::chPolySTATE_LIGHT + 15));
		addChild(createLight<SmallestLight<PolyMute>>(Vec(lightCol4, rackY - lightRow4), module, PolyX::chMuteSTATE_LIGHT + 15));
		addChild(createLight<SmallestLight<PolyFake>>(Vec(lightCol4, rackY - lightRow4), module, PolyX::chOpenSTATE_LIGHT + 15));

	}

};

Model *modelPolyX = createModel<PolyX, PolyXWidget>("PolyX");
