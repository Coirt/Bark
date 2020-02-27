#include "plugin.hpp"
#include "dsp/digital.hpp"
#include "barkComponents.hpp"

using namespace barkComponents;

struct QuadLogic : Module {

	enum ParamIds { NUM_PARAMS };

	enum InputIds {
		LOGIC_A1_INPUT,
		LOGIC_B1_INPUT,
		LOGIC_A2_INPUT,
		LOGIC_B2_INPUT,
		LOGIC_A3_INPUT,
		LOGIC_B3_INPUT,
		LOGIC_A4_INPUT,
		LOGIC_B4_INPUT,
		NUM_INPUTS
	};

	enum OutputIds {
		MAX1_OUTPUT,
		MIN1_OUTPUT,
		MAX2_OUTPUT,
		MIN2_OUTPUT,
		MAX3_OUTPUT,
		MIN3_OUTPUT,
		MAX4_OUTPUT,
		MIN4_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightIds {
		LOGIC_POS1_LIGHT, LOGIC_NEG1_LIGHT,
		LOGIC_POS2_LIGHT, LOGIC_NEG2_LIGHT,
		LOGIC_POS3_LIGHT, LOGIC_NEG3_LIGHT,
		LOGIC_POS4_LIGHT, LOGIC_NEG4_LIGHT,
		NUM_LIGHTS
	};

	QuadLogic() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs &args) override {
		
		//Outs
		outputs[MAX1_OUTPUT].setVoltage(fmaxf(inputs[LOGIC_A1_INPUT].getVoltage(), inputs[LOGIC_B1_INPUT].getVoltage()));
		outputs[MIN1_OUTPUT].setVoltage(fminf(inputs[LOGIC_A1_INPUT].getVoltage(), inputs[LOGIC_B1_INPUT].getVoltage()));
		outputs[MAX2_OUTPUT].setVoltage(fmaxf(inputs[LOGIC_A2_INPUT].getVoltage(), inputs[LOGIC_B2_INPUT].getVoltage()));
		outputs[MIN2_OUTPUT].setVoltage(fminf(inputs[LOGIC_A2_INPUT].getVoltage(), inputs[LOGIC_B2_INPUT].getVoltage()));
		outputs[MAX3_OUTPUT].setVoltage(fmaxf(inputs[LOGIC_A3_INPUT].getVoltage(), inputs[LOGIC_B3_INPUT].getVoltage()));
		outputs[MIN3_OUTPUT].setVoltage(fminf(inputs[LOGIC_A3_INPUT].getVoltage(), inputs[LOGIC_B3_INPUT].getVoltage()));
		outputs[MAX4_OUTPUT].setVoltage(fmaxf(inputs[LOGIC_A4_INPUT].getVoltage(), inputs[LOGIC_B4_INPUT].getVoltage()));
		outputs[MIN4_OUTPUT].setVoltage(fminf(inputs[LOGIC_A4_INPUT].getVoltage(), inputs[LOGIC_B4_INPUT].getVoltage()));
		//Lights
		float logicSum1 = inputs[LOGIC_A1_INPUT].getVoltage() + inputs[LOGIC_B1_INPUT].getVoltage();
		float logicSum2 = inputs[LOGIC_A2_INPUT].getVoltage() + inputs[LOGIC_B2_INPUT].getVoltage();
		float logicSum3 = inputs[LOGIC_A3_INPUT].getVoltage() + inputs[LOGIC_B3_INPUT].getVoltage();
		float logicSum4 = inputs[LOGIC_A4_INPUT].getVoltage() + inputs[LOGIC_B4_INPUT].getVoltage();
		lights[LOGIC_POS1_LIGHT].setSmoothBrightness(fmaxf(0.f, logicSum1 / 5.f), args.sampleTime);
		lights[LOGIC_NEG1_LIGHT].setSmoothBrightness(fmaxf(0.f, -logicSum1 / 5.f), args.sampleTime);
		lights[LOGIC_POS2_LIGHT].setSmoothBrightness(fmaxf(0.f, logicSum2 / 5.f), args.sampleTime);
		lights[LOGIC_NEG2_LIGHT].setSmoothBrightness(fmaxf(0.f, -logicSum2 / 5.f), args.sampleTime);
		lights[LOGIC_POS3_LIGHT].setSmoothBrightness(fmaxf(0.f, logicSum3 / 5.f), args.sampleTime);
		lights[LOGIC_NEG3_LIGHT].setSmoothBrightness(fmaxf(0.f, -logicSum3 / 5.f), args.sampleTime);
		lights[LOGIC_POS4_LIGHT].setSmoothBrightness(fmaxf(0.f, logicSum4 / 5.f), args.sampleTime);
		lights[LOGIC_NEG4_LIGHT].setSmoothBrightness(fmaxf(0.f, -logicSum4 / 5.f), args.sampleTime);

	}//process
};

struct QuadLogicWidget : ModuleWidget {
	QuadLogicWidget(QuadLogic *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BarkQuadLogic.svg")));

		//constexpr int rackY = 380;

		//PortIn---
		addInput(createInput<BarkPatchPortIn>(Vec(8.45f, rackY - 348.52f + 0.35f),  module, QuadLogic::LOGIC_A1_INPUT));
		addInput(createInput<BarkPatchPortIn>(Vec(42.71f, rackY - 348.52f + 0.35f),  module, QuadLogic::LOGIC_B1_INPUT));
		addInput(createInput<BarkPatchPortIn>(Vec(8.1f + 0.35f, rackY - 266.09f + 0.35f),  module, QuadLogic::LOGIC_B2_INPUT));
		addInput(createInput<BarkPatchPortIn>(Vec(42.71f, rackY - 266.09f + 0.35f),  module, QuadLogic::LOGIC_A2_INPUT));
		addInput(createInput<BarkPatchPortIn>(Vec(8.1f + 0.35f, rackY - 133.18f),  module, QuadLogic::LOGIC_A3_INPUT));
		addInput(createInput<BarkPatchPortIn>(Vec(42.71f, rackY - 133.18f),  module, QuadLogic::LOGIC_B3_INPUT));
		addInput(createInput<BarkPatchPortIn>(Vec(8.1f + 0.35f, rackY - 49.53f),  module, QuadLogic::LOGIC_B4_INPUT));
		addInput(createInput<BarkPatchPortIn>(Vec(42.71f, rackY - 49.53f),  module, QuadLogic::LOGIC_A4_INPUT));
		//PortOut---
		addOutput(createOutput<BarkPatchPortOut>(Vec(8.1f + 0.35f, rackY - 320.3f),  module, QuadLogic::MIN1_OUTPUT));
		addOutput(createOutput<BarkPatchPortOut>(Vec(42.71f, rackY - 320.3f),  module, QuadLogic::MAX1_OUTPUT));
		addOutput(createOutput<BarkPatchPortOut>(Vec(8.1f + 0.35f, rackY - 230.2f),  module, QuadLogic::MAX2_OUTPUT));
		addOutput(createOutput<BarkPatchPortOut>(Vec(42.71, rackY - 230.2f),  module, QuadLogic::MIN2_OUTPUT));
		addOutput(createOutput<BarkPatchPortOut>(Vec(8.1f + 0.35f, rackY - 169.08f + 0.35f),  module, QuadLogic::MIN3_OUTPUT));
		addOutput(createOutput<BarkPatchPortOut>(Vec(42.71, rackY - 169.08f + 0.35f),  module, QuadLogic::MAX3_OUTPUT));
		addOutput(createOutput<BarkPatchPortOut>(Vec(8.1f + 0.35f, rackY - 77.73f + 0.35f),  module, QuadLogic::MAX4_OUTPUT));
		addOutput(createOutput<BarkPatchPortOut>(Vec(42.71f, rackY - 77.73f + 0.35f),  module, QuadLogic::MIN4_OUTPUT));
		//screw---
		addChild(createWidget<BarkScrew1>(Vec(2.7f, 367.7f)));						//pos3
		addChild(createWidget<BarkScrew3>(Vec(box.size.x - 12.3f, 2.7f)));				//pos2
		//Lights---
		addChild(createLight<SmallLight<GreenRedLight>>(Vec(34.82f - 0.35f, rackY - 326.8f), module, QuadLogic::LOGIC_POS1_LIGHT));
		addChild(createLight<SmallLight<GreenRedLight>>(Vec(34.82f - 0.35f, rackY - 240.7f), module, QuadLogic::LOGIC_POS2_LIGHT));
		addChild(createLight<SmallLight<GreenRedLight>>(Vec(34.82f, rackY - 144.68f), module, QuadLogic::LOGIC_POS3_LIGHT));
		addChild(createLight<SmallLight<GreenRedLight>>(Vec(34.82f, rackY - 56.04f), module, QuadLogic::LOGIC_POS4_LIGHT));
	}
};

Model *modelQuadLogic = createModel<QuadLogic, QuadLogicWidget>("QuadLogic");
