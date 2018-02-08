#include "Bark.hpp"

struct Panel13 : Module {
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

	Panel13() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

void Panel13::step()
{
}

Panel13Widget::Panel13Widget()
{
	Panel13 *module = new Panel13();
	setModule(module);
	box.size = Vec(13 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/BarkClipGainDistort.svg")));
		panel->box.size = box.size;
		addChild(panel);
	}

	///////////////////////////////////////////////////////////////
	// Screw Positions
	//		addChild(createScrew<NAMESCREW>(Vec(15, 0)));						//top left		pos1
	//		addChild(createScrew<NAMESCREW>(Vec(box.size.x - 30, 0)));			//top right		pos2
	//		addChild(createScrew<NAMESCREW>(Vec(15, 365)));					//bottom left	pos3
	//		addChild(createScrew<NAMESCREW>(Vec(box.size.x - 30, 365)));		//bottom right	pos4
	/////////////////////////////////////////////////////////////

	////////////
	//components
	////////////

	//screw
	addChild(createScrew<BarkScrew1>(Vec(2, 3)));							//pos1
	addChild(createScrew<BarkScrew3>(Vec(box.size.x - 13, 367.2)));			//pos4
}