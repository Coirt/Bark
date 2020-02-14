#include "plugin.hpp"
#include "barkComponents.hpp"

using namespace barkComponents;

struct EOsum : Module {
	enum ParamIds {
		ODDLVL_PARAM,
		EVENLVL_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		POLY_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		MONOODD_OUTPUT,
		MONOEVEN_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(chPolySTATE_LIGHT, 16),
		ENUMS(ODDVU_LIGHTS, 8),
		ENUMS(EVENVU_LIGHTS, 8),
		NUM_LIGHTS
	};

	dsp::VuMeter2 vuMeterOdd, vuMeterEven;
	dsp::ClockDivider vuDividerOdd, vuDividerEven;
	dsp::ClockDivider polyLights;//Poly Lights

	EOsum() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(EVENLVL_PARAM, 0.f, 1.f, 1.f, "Even Level", "%", 0.f, 100.f);
		configParam(ODDLVL_PARAM, 0.f, 1.f, 1.f, "Odd Level", "%", 0.f, 100.f);

		vuMeterOdd.lambda = 1 / 0.1f;
		vuDividerOdd.setDivision(16);
		vuMeterEven.lambda = 1 / 0.1f;
		vuDividerEven.setDivision(16);
		polyLights.setDivision(256);
	}

	void process(const ProcessArgs &args) override {
		
		float sumOdd = (inputs[POLY_INPUT].getVoltage(0) +
				inputs[POLY_INPUT].getVoltage(2) +
				inputs[POLY_INPUT].getVoltage(4) +
				inputs[POLY_INPUT].getVoltage(6) +
				inputs[POLY_INPUT].getVoltage(8) +
				inputs[POLY_INPUT].getVoltage(10) +
				inputs[POLY_INPUT].getVoltage(12) +
				inputs[POLY_INPUT].getVoltage(14));
		float sumEven = (inputs[POLY_INPUT].getVoltage(1) +
				 inputs[POLY_INPUT].getVoltage(3) +
				 inputs[POLY_INPUT].getVoltage(5) +
				 inputs[POLY_INPUT].getVoltage(7) +
				 inputs[POLY_INPUT].getVoltage(9) +
				 inputs[POLY_INPUT].getVoltage(11) +
				 inputs[POLY_INPUT].getVoltage(13) +
				 inputs[POLY_INPUT].getVoltage(15));
		
		sumOdd *= params[ODDLVL_PARAM].getValue();
		sumEven *= params[EVENLVL_PARAM].getValue();
		
		if (vuDividerOdd.process()) {
			vuMeterOdd.process(args.sampleTime * vuDividerOdd.getDivision(), sumOdd / 10.f);
		}
		if (vuDividerEven.process()) {
			vuMeterEven.process(args.sampleTime * vuDividerEven.getDivision(), sumEven / 10.f);
		}
		
		// Set channel lights infrequently
		if (polyLights.process()) {
			for (int c = 0; c < 16; c++) {
				bool active = (c < inputs[POLY_INPUT].getChannels());
				lights[chPolySTATE_LIGHT + c].setBrightness(active);
			}
			
			lights[ODDVU_LIGHTS + 0].setBrightness(vuMeterOdd.getBrightness(0.f, 0.f));
			lights[EVENVU_LIGHTS + 0].setBrightness(vuMeterEven.getBrightness(0.f, 0.f));
			for (int i = 1; i < 8; i++) {
				lights[ODDVU_LIGHTS + i].setBrightness(vuMeterOdd.getBrightness(-3.f * i, -3.f * (i - 1)));
				lights[EVENVU_LIGHTS + i].setBrightness(vuMeterEven.getBrightness(-3.f * i, -3.f * (i - 1)));
			}
			
		}
		
		outputs[MONOODD_OUTPUT].setVoltage(sumOdd);
		outputs[MONOEVEN_OUTPUT].setVoltage(sumEven);
	
	}//process
};



struct EOsumWidget : ModuleWidget {
	EOsumWidget(EOsum *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BarkEOsum.svg")));

		box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
		///Ports---
		//Out---
		addOutput(createOutput<BarkOutPort350>(Vec(5.399f, 321.039f), module, EOsum::MONOODD_OUTPUT));
		addOutput(createOutput<BarkOutPort350>(Vec(32.381f, 321.039f), module, EOsum::MONOEVEN_OUTPUT));
		//In---
		addInput(createInput<BarkPatchPortIn>(Vec(17.887f, 70.970f), module, EOsum::POLY_INPUT));
		//Knobs---
		addParam(createParam<BarkKnob_40>(Vec(10.f, 112.624f), module, EOsum::ODDLVL_PARAM));
		addParam(createParam<BarkKnob_40>(Vec(10.f, 173.672f), module, EOsum::EVENLVL_PARAM));
		//screw---
		addChild(createWidget<BarkScrew1>(Vec(2.7f, 2.7f)));						//pos1
		addChild(createWidget<BarkScrew3>(Vec(box.size.x - 12.3f, 367.7f)));			//pos4
		///Light---
		constexpr float lightCol1 = 19.5f, lightCol2 = lightCol1 + 6.f, lightCol3 = lightCol2 + 6.f, lightCol4 = lightCol3 + 6.f,
				lightRow1 = 26.815f, lightRow2 = lightRow1 + 6.109f, lightRow3 = lightRow2 + 6.109f, lightRow4 = lightRow3 + 6.109f;
		//Poly Light--
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol1, lightRow1), module, EOsum::chPolySTATE_LIGHT + 0));
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol2, lightRow1), module, EOsum::chPolySTATE_LIGHT + 1));
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol3, lightRow1), module, EOsum::chPolySTATE_LIGHT + 2));
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol4, lightRow1), module, EOsum::chPolySTATE_LIGHT + 3));
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol1, lightRow2), module, EOsum::chPolySTATE_LIGHT + 4));
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol2, lightRow2), module, EOsum::chPolySTATE_LIGHT + 5));
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol3, lightRow2), module, EOsum::chPolySTATE_LIGHT + 6));
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol4, lightRow2), module, EOsum::chPolySTATE_LIGHT + 7));
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol1, lightRow3), module, EOsum::chPolySTATE_LIGHT + 8));
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol2, lightRow3), module, EOsum::chPolySTATE_LIGHT + 9));
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol3, lightRow3), module, EOsum::chPolySTATE_LIGHT + 10));
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol4, lightRow3), module, EOsum::chPolySTATE_LIGHT + 11));
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol1, lightRow4), module, EOsum::chPolySTATE_LIGHT + 12));
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol2, lightRow4), module, EOsum::chPolySTATE_LIGHT + 13));
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol3, lightRow4), module, EOsum::chPolySTATE_LIGHT + 14));
		addChild(createLight<SmallestLightInverse<PolyLight>>(Vec(lightCol4, lightRow4), module, EOsum::chPolySTATE_LIGHT + 15));
		///VU Light--
		//Odd-
		constexpr float lightXpos1 = 12.512f, lightXpos2 = 40.494f;
		addChild(createLight<BiggerLight<clipLight>>(Vec(lightXpos1 - 1, lightY[0]), module, EOsum::ODDVU_LIGHTS + 0));
		addChild(createLight<BigLight<redLight>>(Vec(lightXpos1, lightY[1]), module, EOsum::ODDVU_LIGHTS + 1));
		addChild(createLight<BigLight<orangeLight>>(Vec(lightXpos1, lightY[2]), module, EOsum::ODDVU_LIGHTS + 2));
		addChild(createLight<BigLight<yellowLight2>>(Vec(lightXpos1, lightY[3]), module, EOsum::ODDVU_LIGHTS + 3));
		addChild(createLight<BigLight<yellowLight1>>(Vec(lightXpos1, lightY[4]), module, EOsum::ODDVU_LIGHTS + 4));
		addChild(createLight<BigLight<greenLight>>(Vec(lightXpos1, lightY[5]), module, EOsum::ODDVU_LIGHTS + 5));
		addChild(createLight<BigLight<greenLight>>(Vec(lightXpos1, lightY[6]), module, EOsum::ODDVU_LIGHTS + 6));
		addChild(createLight<BigLight<greenLight>>(Vec(lightXpos1, lightY[7]), module, EOsum::ODDVU_LIGHTS + 7));
		//Even-
		addChild(createLight<BiggerLight<clipLight>>(Vec(lightXpos2 - 1, lightY[0]), module, EOsum::EVENVU_LIGHTS + 0));
		addChild(createLight<BigLight<redLight>>(Vec(lightXpos2, lightY[1]), module, EOsum::EVENVU_LIGHTS + 1));
		addChild(createLight<BigLight<orangeLight>>(Vec(lightXpos2, lightY[2]), module, EOsum::EVENVU_LIGHTS + 2));
		addChild(createLight<BigLight<yellowLight2>>(Vec(lightXpos2, lightY[3]), module, EOsum::EVENVU_LIGHTS + 3));
		addChild(createLight<BigLight<yellowLight1>>(Vec(lightXpos2, lightY[4]), module, EOsum::EVENVU_LIGHTS + 4));
		addChild(createLight<BigLight<greenLight>>(Vec(lightXpos2, lightY[5]), module, EOsum::EVENVU_LIGHTS + 5));
		addChild(createLight<BigLight<greenLight>>(Vec(lightXpos2, lightY[6]), module, EOsum::EVENVU_LIGHTS + 6));
		addChild(createLight<BigLight<greenLight>>(Vec(lightXpos2, lightY[7]), module, EOsum::EVENVU_LIGHTS + 7));
	}
};

Model *modelEOsum = createModel<EOsum, EOsumWidget>("EOsum");
