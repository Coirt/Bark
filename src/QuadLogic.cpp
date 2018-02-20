#include "Bark.hpp"
#include "dsp/digital.hpp"

struct QuadLogic : Module
{
	enum ParamIds {
		NUM_PARAMS
	};
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
	QuadLogic() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	
	}
	void step() override;
};

void QuadLogic::step()
{
	float logicSum1 = inputs[LOGIC_A1_INPUT].value + inputs[LOGIC_B1_INPUT].value;
	float logicSum2 = inputs[LOGIC_A2_INPUT].value + inputs[LOGIC_B2_INPUT].value;
	float logicSum3 = inputs[LOGIC_A3_INPUT].value + inputs[LOGIC_B3_INPUT].value;
	float logicSum4 = inputs[LOGIC_A4_INPUT].value + inputs[LOGIC_B4_INPUT].value;
	lights[LOGIC_POS1_LIGHT].setBrightnessSmooth(fmaxf(0.0, logicSum1 / 5.0));
	lights[LOGIC_NEG1_LIGHT].setBrightnessSmooth(fmaxf(0.0, -logicSum1 / 5.0));
	lights[LOGIC_POS2_LIGHT].setBrightnessSmooth(fmaxf(0.0, logicSum2 / 5.0));
	lights[LOGIC_NEG2_LIGHT].setBrightnessSmooth(fmaxf(0.0, -logicSum2 / 5.0));
	lights[LOGIC_POS3_LIGHT].setBrightnessSmooth(fmaxf(0.0, logicSum3 / 5.0));
	lights[LOGIC_NEG3_LIGHT].setBrightnessSmooth(fmaxf(0.0, -logicSum3 / 5.0));
	lights[LOGIC_POS4_LIGHT].setBrightnessSmooth(fmaxf(0.0, logicSum4 / 5.0));
	lights[LOGIC_NEG4_LIGHT].setBrightnessSmooth(fmaxf(0.0, -logicSum4 / 5.0));

	outputs[MAX1_OUTPUT].value = fmaxf(inputs[LOGIC_A1_INPUT].value, inputs[LOGIC_B1_INPUT].value);
	outputs[MIN1_OUTPUT].value = fminf(inputs[LOGIC_A1_INPUT].value, inputs[LOGIC_B1_INPUT].value);
	outputs[MAX2_OUTPUT].value = fmaxf(inputs[LOGIC_A2_INPUT].value, inputs[LOGIC_B2_INPUT].value);
	outputs[MIN2_OUTPUT].value = fminf(inputs[LOGIC_A2_INPUT].value, inputs[LOGIC_B2_INPUT].value);
	outputs[MAX3_OUTPUT].value = fmaxf(inputs[LOGIC_A3_INPUT].value, inputs[LOGIC_B3_INPUT].value);
	outputs[MIN3_OUTPUT].value = fminf(inputs[LOGIC_A3_INPUT].value, inputs[LOGIC_B3_INPUT].value);
	outputs[MAX4_OUTPUT].value = fmaxf(inputs[LOGIC_A4_INPUT].value, inputs[LOGIC_B4_INPUT].value);
	outputs[MIN4_OUTPUT].value = fminf(inputs[LOGIC_A4_INPUT].value, inputs[LOGIC_B4_INPUT].value);
}


QuadLogicWidget::QuadLogicWidget(){

	QuadLogic *module = new QuadLogic();
	setModule(module);
	box.size = Vec(75, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/QuadLogic.svg")));
		panel->box.size = box.size;
		addChild(panel);
	}

	////////////
	//components
	////////////

//PortIn---
	addInput(createInput<BarkPatchPortIn>(Vec(8.1, 380 - 348.52), module, QuadLogic::LOGIC_A1_INPUT));
	addInput(createInput<BarkPatchPortIn>(Vec(42.71, 380 - 348.52), module, QuadLogic::LOGIC_B1_INPUT));
	addInput(createInput<BarkPatchPortIn>(Vec(8.1, 380 - 266.09), module, QuadLogic::LOGIC_B2_INPUT));
	addInput(createInput<BarkPatchPortIn>(Vec(42.71, 380 - 266.09), module, QuadLogic::LOGIC_A2_INPUT));
	addInput(createInput<BarkPatchPortIn>(Vec(8.1, 380 - 133.18), module, QuadLogic::LOGIC_A3_INPUT));
	addInput(createInput<BarkPatchPortIn>(Vec(42.71, 380 - 133.18), module, QuadLogic::LOGIC_B3_INPUT));
	addInput(createInput<BarkPatchPortIn>(Vec(8.1, 380 - 49.53), module, QuadLogic::LOGIC_B4_INPUT));
	addInput(createInput<BarkPatchPortIn>(Vec(42.71, 380 - 49.53), module, QuadLogic::LOGIC_A4_INPUT));

//PortOut---
	addOutput(createOutput<BarkPatchPortOut>(Vec(8.1, 380 - 320.3), module, QuadLogic::MIN1_OUTPUT));
	addOutput(createOutput<BarkPatchPortOut>(Vec(42.71, 380 - 320.3), module, QuadLogic::MAX1_OUTPUT));
	addOutput(createOutput<BarkPatchPortOut>(Vec(8.1, 380 - 230.2), module, QuadLogic::MAX2_OUTPUT));
	addOutput(createOutput<BarkPatchPortOut>(Vec(42.71, 380 - 230.2), module, QuadLogic::MIN2_OUTPUT));
	addOutput(createOutput<BarkPatchPortOut>(Vec(8.1, 380 - 169.08), module, QuadLogic::MIN3_OUTPUT));
	addOutput(createOutput<BarkPatchPortOut>(Vec(42.71, 380 - 169.08), module, QuadLogic::MAX3_OUTPUT));
	addOutput(createOutput<BarkPatchPortOut>(Vec(8.1, 380 - 77.73), module, QuadLogic::MAX4_OUTPUT));
	addOutput(createOutput<BarkPatchPortOut>(Vec(42.71, 380 - 77.73), module, QuadLogic::MIN4_OUTPUT));

//screw---
	addChild(createScrew<BarkScrew1>(Vec(2, 367.2)));					//pos3
	addChild(createScrew<BarkScrew4>(Vec(box.size.x - 13, 3)));			//pos2

	//Lights---
	addChild(createLight<SmallLight<GreenRedLight>>(Vec(34.82, 380 - 326.8), module, QuadLogic::LOGIC_POS1_LIGHT));
	addChild(createLight<SmallLight<GreenRedLight>>(Vec(34.82, 380 - 240.7), module, QuadLogic::LOGIC_POS2_LIGHT));
	addChild(createLight<SmallLight<GreenRedLight>>(Vec(34.82, 380 - 144.68), module, QuadLogic::LOGIC_POS3_LIGHT));
	addChild(createLight<SmallLight<GreenRedLight>>(Vec(34.82, 380 - 56.04), module, QuadLogic::LOGIC_POS4_LIGHT));
}