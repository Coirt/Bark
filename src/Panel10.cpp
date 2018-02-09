#include "Bark.hpp"

struct Panel10 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		BLINK_LIGHT,
		NUM_LIGHTS
	};

	Panel10() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

void Panel10::step()
{
}

Panel10Widget::Panel10Widget()
{
	Panel10 *module = new Panel10();
	setModule(module);
	box.size = Vec(10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/BarkPanel10.svg")));
		panel->box.size = box.size;
		addChild(panel);
	}

	////////////
	//components
	////////////
	addChild(createScrew<BarkScrew4>(Vec(2, 3)));							//pos1
	addChild(createScrew<BarkScrew3>(Vec(box.size.x - 13, 367.2)));			//pos4
}