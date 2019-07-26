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

		//screw... *whispers* 'all of them'
		addChild(createWidget<BarkScrew1>(Vec(2, 3)));							//pos1
		addChild(createWidget<BarkScrew2>(Vec(box.size.x - 13, 3)));				//pos2
		addChild(createWidget<BarkScrew3>(Vec(2, 367.2f)));						//pos3
		addChild(createWidget<BarkScrew4>(Vec(box.size.x - 13, 367.2)));			//pos4
	}
};

Model *modelPanel6 = createModel<Panel6, Panel6Widget>("Panel6");