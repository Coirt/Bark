#include "plugin.hpp"
#include "barkComponents.hpp"

using namespace barkComponents;

struct Panel6 : Module {
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
		NUM_LIGHTS
	};

	Panel6() { 
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	
	}
	void process(const ProcessArgs &args) override {
	
	
	}
};



struct Panel6Widget : ModuleWidget {
	Panel6Widget(Panel6 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BarkPanel6.svg")));

		//constexpr int rackY = 380;

		box.size = Vec(7 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		//screw...
		addChild(createWidget<BarkScrew1>(Vec(2.7f, 2.7f)));					//pos1
		addChild(createWidget<BarkScrew2>(Vec(box.size.x - 12.3f, 2.7f)));			//pos2
		addChild(createWidget<BarkScrew3>(Vec(2.7f, 367.7f)));					//pos3
		addChild(createWidget<BarkScrew4>(Vec(box.size.x - 12.3f, 367.7f)));			//pos4
	}
};

Model *modelPanel6 = createModel<Panel6, Panel6Widget>("Panel6");
