#include "plugin.hpp"
#include "barkComponents.hpp"

using namespace barkComponents;

bool light1by = true, light2by = false;


struct tpGateVal : ParamQuantity {
	std::string getDisplayValueString() override {
		if (getValue() < 1.f) return "On";
		else return "Off";
	}
};

struct tpStepMultVal : ParamQuantity {
	std::string getDisplayValueString() override {
		if (getValue() > 0.f) return "1";
		else return "2";
	}
};

struct ClockSeq16 : Module {
	enum ParamIds {
		CLK_PARAM,
		STEP_PARAM,
		STEPMODE_PARAM,
		EXTSTEP_PARAM,
		vnDISP_PARAM,
		RUN_PARAM,
		PAUSE_PARAM,
		TAP_PARAM,
		ENUMS(GATEr1_PARAM, 8),
		ENUMS(GATEr2_PARAM, 8),
		ENUMS(CVr1_PARAM, 8),
		ENUMS(CVr2_PARAM, 8),
		NUM_PARAMS
	};
	enum InputIds {
		EXT_INPUT,
		RUN_INPUT,
		PAUSE_INPUT,
		CLK_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		GATE_OUTPUT,
		CV_OUTPUT,
		GATEr1_OUTPUT,
		GATEr2_OUTPUT,
		CVr1_OUTPUT,
		CVr2_OUTPUT,
		CLK_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {//TODO
		ENUMS(RUN1by_LIGHTS, 2 * 8),
		ENUMS(RUN2by_LIGHTS, 16),
		NUM_LIGHTS
	};

	dsp::SchmittTrigger clockTrigger, runningTrigger, resetTrigger, gateTrig1byR1[8], gateTrig1byR2[8], gateTrig2by[16];
	bool gates1byR1[8] = {}, gates1byR2[8] = {}, gates2by[16] = {}, running = true;
	float phase = 0.f; int index = 0;

	ClockSeq16() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(STEP_PARAM, 1.f, 8.f, 8.f, "Step length");
		configParam<tpStepMultVal>(STEPMODE_PARAM, 0.f, 1.f, 1.f, "Step Multiplier", "x");
		configParam(CLK_PARAM, -2.f, 4.f, 1.f, "Tempo", " BPM", 2.f, std::pow(2.f, params[CLK_PARAM].getValue()) * 60);
		configParam(vnDISP_PARAM, 0.f, 1.f, 0.f, "Display Mode");
		configParam(RUN_PARAM, 0.f, 1.f, 1.f, "Run");
		configParam(PAUSE_PARAM, 0.f, 1.f, 0.f, "Pause");
		configParam(TAP_PARAM, 0.f, 1.f, 0.f, "Tap Tempo");
		for (int i = 0; i < 8; i++) {
			configParam<tpGateVal>(GATEr1_PARAM + i, 0.f, 1.f, 0.f, "Gate " + std::to_string(i + 1));
			configParam<tpGateVal>(GATEr2_PARAM + i, 0.f, 1.f, 0.f, "Gate " + std::to_string(i + 9));
			configParam(CVr1_PARAM + i, -4.f, 6.f, 0.f);
			configParam(CVr2_PARAM + i, -4.f, 6.f, 0.f);
		}

		onReset();
	}
	
	void onReset() override {
		for (int i = 0; i < 16; i++) {
			gates2by[i] = true;
		}
	}

	void onRandomize() override {
		for (int i = 0; i < 16; i++) {
			gates2by[i] = (random::uniform() > 0.5f);
		}
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();

		// running
		json_object_set_new(rootJ, "running", json_boolean(running));

		// gates
		json_t *gates2byJ = json_array();
		for (int i = 0; i < 16; i++) {
			json_array_insert_new(gates2byJ, i, json_integer((int)gates2by[i]));
		}
		json_object_set_new(rootJ, "gates", gates2byJ);

		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		// running
		json_t *runningJ = json_object_get(rootJ, "running");
		if (runningJ)
			running = json_is_true(runningJ);

		// gates
		json_t *gates2byJ = json_object_get(rootJ, "gates");
		if (gates2byJ) {
			for (int i = 0; i < 16; i++) {
				json_t *gateJ = json_array_get(gates2byJ, i);
				if (gateJ)
					gates2by[i] = !!json_integer_value(gateJ);
			}
		}
	}

	void setIndex(int index) {
		int numSteps;
		if (params[STEPMODE_PARAM].getValue() == 1.f) {//1x8
			numSteps = (int)clamp(std::round(params[STEP_PARAM].getValue() + inputs[EXT_INPUT].getVoltage()), 1.f, 8.f);
		} else {	//2x8
			numSteps = (int)clamp(std::round(params[STEP_PARAM].getValue() + inputs[EXT_INPUT].getVoltage()), 1.f, 16.f);
		}
		phase = 0.f;
		this->index = index;
		if (this->index >= numSteps)
			this->index = 0;
	}

	void process(const ProcessArgs &args) override {
		////pseudo---------------------------------------------------------------------
		//if (STEPMODE_PARAM==1x) {//process both rows as 16 steps for centre output, 
			////-----process row1/2 outputs as 16 steps with 8 steps of silence repective of position on the row 
			////-----(i.e. steps 1-8 row 1, 8 steps on then 8 steps silence == 16steps. row 2 opposite: 8 silence 8 steps on)
		//	for (int i = 0; i < 16; i++) {

		//	}
		//} else {
		//	for (int i = 0; i < 8; i++) {//process both rows as seperated 8 steps 
		//		for (int j = 0; j < 8; j++) {

		//		}
		//	}
		//}
		////pseudo---------------------------------------------------------------------

		if (params[STEPMODE_PARAM].getValue() == 1.f) { light1by = true;}
		else if (params[STEPMODE_PARAM].getValue() == 0.f) { light2by = true; light1by = false; }
		
		// Run
		if (runningTrigger.process(params[RUN_PARAM].getValue())) {
			running = !running;
		}

		bool gateIn = false;
		if (running) {
			if (inputs[EXT_INPUT].isConnected()) {
				// External clock
				if (clockTrigger.process(inputs[EXT_INPUT].getVoltage())) {
					setIndex(index + 1);
				}
				gateIn = clockTrigger.isHigh();
			} else {
				// Internal clock
				float clockTime = std::pow(2.f, params[CLK_PARAM].getValue() + inputs[CLK_INPUT].getVoltage());
				phase += clockTime * args.sampleTime;
				if (phase >= 1.f) {
					setIndex(index + 1);
				}
				gateIn = (phase < 0.5f);
			}
		}
		/*
		// Pause
		if (resetTrigger.process(params[PAUSE_PARAM].getValue() + inputs[PAUSE_INPUT].getVoltage())) {
			setIndex(puasedValue);
		}
		*/
		// Gate buttons
		for (int i = 0; i < 8; i++) {
			if (gateTrig2by[i].process(params[GATEr1_PARAM + i].getValue())) {
				gates2by[i] = !gates2by[i];
			}
			outputs[GATE_OUTPUT + i].setVoltage((running && gateIn && i == index && gates2by[i])?10.f:0.f);
			//lights[GATE_LIGHTS + i].setSmoothBrightness((gateIn && i == index)?(gates2by[i]?1.f:0.33):(gates2by[i]?0.66:0.0), args.sampleTime);
		}

		//Outputs

	}//process
};

struct ClockSeq16Widget : ModuleWidget {
	ClockSeq16Widget(ClockSeq16 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ClockSeq16.svg")));//TODO: final name res/BarkClockSeq16.svg
		int rackY = 380;
		float gatePosX[8] = { 11.57f, 47.69f, 83.82f, 119.94f, 156.06f, 192.19f, 228.32f, 264.44f },
			cvPosX[8] = { 8.57f, 44.69f, 80.82f, 116.94f, 153.07f, 189.19f, 225.32f, 261.44f },
			lightPosX[8] = { 22.07f, 58.19f, 94.32f, 130.44f, 166.57f, 202.69f, 238.82f, 274.94f },
			btnX = 283.19f, portX = 254.57f;

		///Ports---
		//Out---
		addOutput(createOutput<BarkOutPort350>(Vec(140.61f, rackY - 332.38f), module, ClockSeq16::GATE_OUTPUT));
		addOutput(createOutput<BarkOutPort350>(Vec(220.11f, rackY - 332.38f), module, ClockSeq16::CV_OUTPUT));
		addOutput(createOutput<BarkOutPort350>(Vec(166.6f, rackY - 351.73f), module, ClockSeq16::GATEr1_OUTPUT));
		addOutput(createOutput<BarkOutPort350>(Vec(195.51f, rackY - 351.73f), module, ClockSeq16::CVr1_OUTPUT));
		addOutput(createOutput<BarkOutPort350>(Vec(166.6f, rackY - 310.68f), module, ClockSeq16::GATEr2_OUTPUT));
		addOutput(createOutput<BarkOutPort350>(Vec(195.51f, rackY - 310.68f), module, ClockSeq16::CVr2_OUTPUT));
		addOutput(createOutput<BarkOutPort350>(Vec(portX, rackY - 280.25f), module, ClockSeq16::CLK_OUTPUT));
		//In---
		addInput(createInput<BarkInPort350>(Vec(106.27f, rackY - 344.01f), module, ClockSeq16::EXT_INPUT));
		addInput(createInput<BarkInPort350>(Vec(portX, rackY - 364.49f), module, ClockSeq16::RUN_INPUT));
		addInput(createInput<BarkInPort350>(Vec(portX, rackY - 337.26f), module, ClockSeq16::PAUSE_INPUT));
		addInput(createInput<BarkInPort350>(Vec(portX, rackY - 310.04f), module, ClockSeq16::CLK_INPUT));
		//Knobs---
		addParam(createParam<BarkKnobSwitch>(Vec(12.94f, rackY - 362.93f), module, ClockSeq16::STEP_PARAM));
		addParam(createParam<BarkKnob30b>(Vec(14.14f, rackY - 297.f), module, ClockSeq16::CLK_PARAM));
		for (int i = 0; i < 8; i++) {
			addParam(createParam<BarkKnob30a>(Vec(cvPosX[i], rackY - 119.66f), module, ClockSeq16::CVr1_PARAM + i));
			addParam(createParam<BarkKnob30a>(Vec(cvPosX[i], rackY - 80.73f), module, ClockSeq16::CVr2_PARAM + i));
		}

		//Switch---
		addParam(createParam<BarkSwitchSmall>(Vec(80.96f, rackY - 358.59f), module, ClockSeq16::STEPMODE_PARAM));
		addParam(createParam<BarkSwitchSmall>(Vec(116.45f, rackY - 297.55f), module, ClockSeq16::vnDISP_PARAM));
		for (int i = 0;  i < 8; i++) {
			addParam(createParam<BarkSwitch>(Vec(gatePosX[i], rackY - 237.82f), module, ClockSeq16::GATEr1_PARAM + i));
			addParam(createParam<BarkSwitch>(Vec(gatePosX[i], rackY - 178.32f), module, ClockSeq16::GATEr2_PARAM + i));
		}
		addParam(createParam<BarkPushButton1>(Vec(btnX, rackY - 364.49f), module, ClockSeq16::RUN_PARAM));
		addParam(createParam<BarkPushButton1>(Vec(btnX, rackY - 337.26f), module, ClockSeq16::PAUSE_PARAM));
		addParam(createParam<BarkPushButton1>(Vec(btnX, rackY - 310.04f), module, ClockSeq16::TAP_PARAM));

		//screw---
		addChild(createWidget<BarkScrew3>(Vec(3, 3)));							//pos1
		addChild(createWidget<BarkScrew4>(Vec(box.size.x - 13, 3)));				//pos2
		addChild(createWidget<BarkScrew2>(Vec(3, 367.2f)));						//pos3
		addChild(createWidget<BarkScrew1>(Vec(box.size.x - 13, 367.2)));			//pos4
		//Light---
		//addChild(createLight<SmallerLightFA<ParamInLight>>(Vec(floatyMcFloatFace, rackY - 280.05f), module, OneBand::FreqParamOn));
		//x = 22.07 y1 = 247.22 y2 = 187.72
		if (light1by == true) {
			for (int i = 0; i < 8; i++) {
				addChild(createLight<SmallestLight<ParamInLight>>(Vec(lightPosX[i], rackY - 247.22f), module, ClockSeq16::RUN1by_LIGHTS + i));
				addChild(createLight<SmallestLight<ParamInLight>>(Vec(lightPosX[i], rackY - 187.72f), module, ClockSeq16::RUN1by_LIGHTS + i));
			}
		} else if (light2by == true) {
			for (int i = 0; i < 16; i++) {
				addChild(createLight<SmallestLight<ParamInLight>>(Vec(lightPosX[i], rackY - 247.22f), module, ClockSeq16::RUN2by_LIGHTS + i));
				addChild(createLight<SmallestLight<ParamInLight>>(Vec(lightPosX[i], rackY - 187.72f), module, ClockSeq16::RUN2by_LIGHTS + i));
			}
		}
		
	}

};

Model *modelClockSeq16 = createModel<ClockSeq16, ClockSeq16Widget>("Clock Seq 16");