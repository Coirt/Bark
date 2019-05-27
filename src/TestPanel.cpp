#include "plugin.hpp"
#include "barkComponents.hpp"
#include "dependancies/filt/lp24.cpp"
using namespace barkComponents;
struct TestPanel : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		INPUT,
		INPUTTOCAST,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT,
		CASTOUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		BLINK_LIGHT,
		NUM_LIGHTS
	};
	LadderFilter lplf24;
	TestPanel() { 
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	
	}
	void process(const ProcessArgs &args) override {
	
		//-----Low pass ladder
		float input = inputs[INPUT].getVoltage() / 5.f, cutoff = 116.54f;
		lplf24.setCutoff(cutoff);
		lplf24.process(input, args.sampleTime);
		outputs[OUTPUT].setVoltage((lplf24.lowpass * 5.f) * 2);

		//-----cast to int, module idea take an input step with a cast adjust cast multiplication with params[].getValue()
		float inToCast = (int)inputs[INPUTTOCAST].getVoltage() * .3335f;	//10 = 3.335
		outputs[CASTOUT].setVoltage((inToCast * .667f) * (4.4953922f));	////==10//10 = 2.22445 * 4.6791891 = 10.408622
	
	}
};

//----------------draw half hex shadow test
struct hexShadow : TransparentWidget {

	void draw(const DrawArgs &hex) override {
		//TestPanel *module;
		float  height = 26.257f, width = 15.175f, posX = 0.f, posY = posX, deg1 = NVG_PI / 180, deg60 = deg1 * 60;
		NVGcolor blackTransparent = nvgRGBA(0, 0, 0, 35);
		//NVGcolor tealTransparent = nvgRGBA(0, 128, 128, 35);
		//NVGcolor yellowTransparent = nvgRGBA(255, 255, 0, 35);
		//90°----------------
		nvgBeginPath(hex.vg);
		nvgRect(hex.vg, posX, posY, width, height);
		//120°----------------
		nvgRotate(hex.vg, deg1 * 60);
		nvgRect(hex.vg, posX + (width / (deg60 *2)), posY - (height / width * (deg60 * 7)), width, height);
		//60°---------------
		nvgRotate(hex.vg, deg1 * 150);
		nvgRect(hex.vg, posX - height, posY - width, height, width);		//flip h = w, w = h
		nvgFillColor(hex.vg, blackTransparent);
		nvgFill(hex.vg);
	}
};
//----------------draw half hex shadow test

struct TestPanelWidget : ModuleWidget {
	TestPanelWidget(TestPanel *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BarkPanel6.svg")));

		if (module != NULL) {//draw hex
			hexShadow *hex = createWidget<hexShadow>(Vec(40.f, 200.f));
			addChild(hex);
		}

		int rackY = 380;

		box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		
		//screw all of them
		addChild(createWidget<BarkScrew1>(Vec(2, 3)));							//pos1
		addChild(createWidget<BarkScrew2>(Vec(box.size.x - 13, 3)));				//pos2
		addChild(createWidget<BarkScrew3>(Vec(2, 367.2f)));						//pos3
		addChild(createWidget<BarkScrew4>(Vec(box.size.x - 13, 367.2)));			//pos4

		//In------
		addInput(createInput<BarkInPort350>(Vec(4.05f, rackY - 46.16f), module, TestPanel::INPUT));
		addInput(createInput<BarkInPort350>(Vec(4.05f, rackY - 126.16f), module, TestPanel::INPUTTOCAST));
		//Out-----
		addOutput(createOutput<BarkOutPort350>(Vec(64.05f, rackY - 46.16f), module, TestPanel::OUTPUT));
		addOutput(createOutput<BarkOutPort350>(Vec(64.05f, rackY - 126.16f), module, TestPanel::CASTOUT));

	}
};

Model *modelTestPanel = createModel<TestPanel, TestPanelWidget>("TestPanel");