#include "plugin.hpp"
#include "barkComponents.hpp"

struct NavPanel : Module {
	enum ParamIds {NUM_PARAMS};
	enum InputIds {NUM_INPUTS};
	enum OutputIds {	NUM_OUTPUTS};
	enum LightIds {NUM_LIGHTS};

	
	NavPanel() {config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);	}
	void process(const ProcessArgs &args) override {}
};

struct NavPanelWidget : ModuleWidget {
	NavPanelWidget(NavPanel *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/NavigationPanel.svg")));
	}

};

Model *modelNavPanel = createModel<NavPanel, NavPanelWidget>("NavPanel");