#include "plugin.hpp"
#include "barkComponents.hpp"
#include "dependancies/dsp/cpPan.h"

using namespace barkComponents;

struct PolyMix : Module {
	enum ParamIds {
		CHSELECT_PARAM,
		ENUMS(LEVEL_PARAM, 4),
		ENUMS(PAN_PARAM, 4),
		ENUMS(AUX1_PARAM, 4),
		ENUMS(AUX2_PARAM, 4),
		ENUMS(INVERT_PARAM, 4),
		ENUMS(MUTE_PARAM, 4),
		ENUMS(SOLO_PARAM, 4),
		MASTERGAIN_PARAM,
		NUM_PARAMS
	};

	enum InputIds {
		POLYAUDIO_INPUT,
		POLYLEVEL_INPUT,
		POLYPAN_INPUT,
		GAINLEVEL_INPUT,
		AUX1RETURN_INPUT,
		AUX2RETURN_INPUT,
		NUM_INPUTS
	};

	enum OutputIds {
		OUTL_OUTPUT,
		OUTR_OUTPUT,
		AUX1SEND_OUTPUT,
		AUX2SEND_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightIds {
		NUM_LIGHTS
	};

	dsp::RCFilter dcblockL, dcblockR;

	PolyMix() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam<tpChannelSelect>(CHSELECT_PARAM, 0.f, 3.f, 0.f, "Channel");
		configParam(MASTERGAIN_PARAM, 0.f, 2.f, 1.f, "Output Gain", "dB", -10, 40);
		for (int i = 0; i < 4; i++) {
			configParam(AUX1_PARAM + i, 0., M_2_SQRTPI, 0., "Aux 1 Ch" + std::to_string(i + 1), "dB", -10, 40, 1.71209928474);
			configParam(AUX2_PARAM + i, 0., M_2_SQRTPI, 0., "Aux 2 Ch" + std::to_string(i + 1), "dB", -10, 40, 1.71209928474);
			configParam(PAN_PARAM + i, -1.f, 1.f, 0.f, "Pan Ch " + std::to_string(i + 1));
			//M_LOG2E == 1.44269504088896340736
			configParam(LEVEL_PARAM + i, 0., M_LOG2E, 1., "Level Ch " + std::to_string(i + 1), "dB", -10, 40);	//M_SQRT2
			//switch---
			configSwitch(INVERT_PARAM + i, 0.f, 1.f, 1.f, "Invert Ch " + std::to_string(i + 1), { "On", "Off" });
			configSwitch(MUTE_PARAM + i, 0.f, 1.f, 1.f, "Mute Ch " + std::to_string(i + 1), { "On", "Off" });
			configSwitch(SOLO_PARAM + i, 0.f, 1.f, 1.f, "Solo Ch " + std::to_string(i + 1), { "On", "Off" });
		}
		//input---
		configInput(POLYAUDIO_INPUT, "Polyphonic");
		configInput(POLYLEVEL_INPUT, "Level modulation");
		configInput(POLYPAN_INPUT, "Pan modulation");
		configInput(GAINLEVEL_INPUT, "Master level modulation");
		configInput(AUX1RETURN_INPUT, "Return 1");
		configInput(AUX2RETURN_INPUT, "Return 2");
		//ouput---
		configOutput(OUTL_OUTPUT, "Master L");
		configOutput(OUTR_OUTPUT, "Master R");
		configOutput(AUX1SEND_OUTPUT, "Send 1");
		configOutput(AUX2SEND_OUTPUT, "Send 2");
	}

	void process(const ProcessArgs &args) override {
		//initilize variables 
		//bool soloSafe[16] = {}, solo[16] = {}, mute[16] = {};
		//float auxSend1Lvl[16] = {}, auxSend2Lvl[16] = {}, pan[16] = {}; 
		float polyChAudioL[16] = { 0.f }, polyChAudioR[16] = { 0.f }, polyChLevel[16] = { 0.f },
			polyChPan[16] = { 0.f }, chFadLevel[16] = { 0.f };
		int chSel = (int)params[CHSELECT_PARAM].getValue(),
			nChAudio = 0, nChLevel = 0, nChPan = 0, soloCase = 0;
		//hold 4 (L/R) values for sum to output L/R
		float sumL[4], sumR[4];//master gain
		float ch1Send1 = 0.f, ch2Send1 = 0.f, ch3Send1 = 0.f, ch4Send1 = 0.f,
			ch1Send2 = 0.f, ch2Send2 = 0.f, ch3Send2 = 0.f, ch4Send2 = 0.f,
			cvMasterLevel = 1.f;

		//get audio channels
		//get #channels connected
		nChAudio = inputs[POLYAUDIO_INPUT].getChannels();//int
		if (inputs[POLYAUDIO_INPUT].isConnected()) {
				if (chSel == 0) {
					params[INVERT_PARAM + 0].getValue() == 1.f ?
						polyChAudioL[0] = polyChAudioR[0] = inputs[POLYAUDIO_INPUT].getVoltage(0) :
						polyChAudioL[0] = polyChAudioR[0] = -inputs[POLYAUDIO_INPUT].getVoltage(0);
					params[INVERT_PARAM + 1].getValue() == 1.f ?
						polyChAudioL[1] = polyChAudioR[1] = inputs[POLYAUDIO_INPUT].getVoltage(1) :
						polyChAudioL[1] = polyChAudioR[1] = -inputs[POLYAUDIO_INPUT].getVoltage(1);
					params[INVERT_PARAM + 2].getValue() == 1.f ?
						polyChAudioL[2] = polyChAudioR[2] = inputs[POLYAUDIO_INPUT].getVoltage(2) :
						polyChAudioL[2] = polyChAudioR[2] = -inputs[POLYAUDIO_INPUT].getVoltage(2);
					params[INVERT_PARAM + 3].getValue() == 1.f ?
						polyChAudioL[3] = polyChAudioR[3] = inputs[POLYAUDIO_INPUT].getVoltage(3) :
						polyChAudioL[3] = polyChAudioR[3] = -inputs[POLYAUDIO_INPUT].getVoltage(3);
						
				} if (chSel == 1) {
					params[INVERT_PARAM + 0].getValue() == 1.f ?
						polyChAudioL[4] = polyChAudioR[4] = inputs[POLYAUDIO_INPUT].getVoltage(4) :
						polyChAudioL[4] = polyChAudioR[4] = -inputs[POLYAUDIO_INPUT].getVoltage(4);
					params[INVERT_PARAM + 1].getValue() == 1.f ?
						polyChAudioL[5] = polyChAudioR[5] = inputs[POLYAUDIO_INPUT].getVoltage(5) :
						polyChAudioL[5] = polyChAudioR[5] = -inputs[POLYAUDIO_INPUT].getVoltage(5);
					params[INVERT_PARAM + 2].getValue() == 1.f ?
						polyChAudioL[6] = polyChAudioR[6] = inputs[POLYAUDIO_INPUT].getVoltage(6) :
						polyChAudioL[6] = polyChAudioR[6] = -inputs[POLYAUDIO_INPUT].getVoltage(6);
					params[INVERT_PARAM + 3].getValue() == 1.f ?
						polyChAudioL[7] = polyChAudioR[7] = inputs[POLYAUDIO_INPUT].getVoltage(7) :
						polyChAudioL[7] = polyChAudioR[7] = -inputs[POLYAUDIO_INPUT].getVoltage(7);
				}

				if (chSel == 2) {
					params[INVERT_PARAM + 0].getValue() == 1.f ?
						polyChAudioL[8] = polyChAudioR[8] = inputs[POLYAUDIO_INPUT].getVoltage(8) :
						polyChAudioL[8] = polyChAudioR[8] = -inputs[POLYAUDIO_INPUT].getVoltage(8);
					params[INVERT_PARAM + 1].getValue() == 1.f ?
						polyChAudioL[9] = polyChAudioR[9] = inputs[POLYAUDIO_INPUT].getVoltage(9) :
						polyChAudioL[9] = polyChAudioR[9] = -inputs[POLYAUDIO_INPUT].getVoltage(9);
					params[INVERT_PARAM + 2].getValue() == 1.f ?
						polyChAudioL[10] = polyChAudioR[10] = inputs[POLYAUDIO_INPUT].getVoltage(10) :
						polyChAudioL[10] = polyChAudioR[10] = -inputs[POLYAUDIO_INPUT].getVoltage(10);
					params[INVERT_PARAM + 3].getValue() == 1.f ?
						polyChAudioL[11] = polyChAudioR[11] = inputs[POLYAUDIO_INPUT].getVoltage(11) :
						polyChAudioL[11] = polyChAudioR[11] = -inputs[POLYAUDIO_INPUT].getVoltage(11);
				}

				if (chSel == 3) {
					params[INVERT_PARAM + 0].getValue() == 1.f ?
						polyChAudioL[12] = polyChAudioR[12] = inputs[POLYAUDIO_INPUT].getVoltage(12) :
						polyChAudioL[12] = polyChAudioR[12] = -inputs[POLYAUDIO_INPUT].getVoltage(12);
					params[INVERT_PARAM + 1].getValue() == 1.f ?
						polyChAudioL[13] = polyChAudioR[13] = inputs[POLYAUDIO_INPUT].getVoltage(13) :
						polyChAudioL[13] = polyChAudioR[13] = -inputs[POLYAUDIO_INPUT].getVoltage(13);
					params[INVERT_PARAM + 2].getValue() == 1.f ?
						polyChAudioL[14] = polyChAudioR[14] = inputs[POLYAUDIO_INPUT].getVoltage(14) :
						polyChAudioL[14] = polyChAudioR[14] = -inputs[POLYAUDIO_INPUT].getVoltage(14);
					params[INVERT_PARAM + 3].getValue() == 1.f ?
						polyChAudioL[15] = polyChAudioR[15] = inputs[POLYAUDIO_INPUT].getVoltage(15) :
						polyChAudioL[15] = polyChAudioR[15] = -inputs[POLYAUDIO_INPUT].getVoltage(15);
				}
		}//set audio or invert

		//get #channels connected
		nChLevel = inputs[POLYLEVEL_INPUT].getChannels();	//int
		if (inputs[POLYLEVEL_INPUT].isConnected()) {
			for (int i = 0; i < nChLevel; i++) {
				//assign channels to arrays
				polyChLevel[i] = inputs[POLYLEVEL_INPUT].getVoltage(i);	// recall
				float cvLevel = simd::clamp(inputs[POLYLEVEL_INPUT].getPolyVoltage(i) / 10.f, 0.f, 1.f);
				polyChAudioL[i] = polyChAudioR[i] *= cvLevel;
			}
		}

		// Channels 1 to 4
		if (chSel == 0) {//ch1-4, index 0-3
			for (int ch1_4 = 0; ch1_4 < 4; ch1_4++) {
				//apply gain curve
				chFadLevel[ch1_4] = simd::pow(params[LEVEL_PARAM + ch1_4].getValue(), 2.f);
				//gain and pan | mute --- if mute equals 1 gain assigned fader else gain assigned zero
				params[MUTE_PARAM + ch1_4].getValue() == 1.f ? polyChAudioL[ch1_4] *= chFadLevel[ch1_4] *
					cpPanL(params[PAN_PARAM + ch1_4].getValue(), inputs[POLYPAN_INPUT].getVoltage(ch1_4)) :
					polyChAudioL[ch1_4] = 0.f;
				params[MUTE_PARAM + ch1_4].getValue() == 1.f ? polyChAudioR[ch1_4] *= chFadLevel[ch1_4] *
					cpPanR(params[PAN_PARAM + ch1_4].getValue(), inputs[POLYPAN_INPUT].getVoltage(ch1_4)) :
					polyChAudioR[ch1_4] = 0.f;
			}
			//setAux
			///ch2
			ch1Send1 = polyChAudioL[0] *
				cpPanL(params[PAN_PARAM + 0].getValue(), inputs[POLYPAN_INPUT].getVoltage(0)) +
				polyChAudioR[0] *
				cpPanR(params[PAN_PARAM + 0].getValue(), inputs[POLYPAN_INPUT].getVoltage(0));
			ch1Send2 = polyChAudioL[0] *
				cpPanL(params[PAN_PARAM + 0].getValue(), inputs[POLYPAN_INPUT].getVoltage(0)) +
				polyChAudioR[0] *
				cpPanR(params[PAN_PARAM + 0].getValue(), inputs[POLYPAN_INPUT].getVoltage(0));
			///ch2
			ch2Send1 = polyChAudioL[1] *
				cpPanL(params[PAN_PARAM + 1].getValue(), inputs[POLYPAN_INPUT].getVoltage(1)) +
				polyChAudioR[1] *
				cpPanR(params[PAN_PARAM + 1].getValue(), inputs[POLYPAN_INPUT].getVoltage(1));
			ch2Send2 = polyChAudioL[1] *
				cpPanL(params[PAN_PARAM + 1].getValue(), inputs[POLYPAN_INPUT].getVoltage(1)) +
				polyChAudioR[1] *
				cpPanR(params[PAN_PARAM + 1].getValue(), inputs[POLYPAN_INPUT].getVoltage(1));
			///ch3
			ch3Send1 = polyChAudioL[2] *
				cpPanL(params[PAN_PARAM + 2].getValue(), inputs[POLYPAN_INPUT].getVoltage(2)) +
				polyChAudioR[2] *
				cpPanR(params[PAN_PARAM + 2].getValue(), inputs[POLYPAN_INPUT].getVoltage(2));
			ch3Send2 = polyChAudioL[2] *
				cpPanL(params[PAN_PARAM + 2].getValue(), inputs[POLYPAN_INPUT].getVoltage(2)) +
				polyChAudioR[2] *
				cpPanR(params[PAN_PARAM + 2].getValue(), inputs[POLYPAN_INPUT].getVoltage(2));
			///ch4
			ch4Send1 = polyChAudioL[3] *
				cpPanL(params[PAN_PARAM + 3].getValue(), inputs[POLYPAN_INPUT].getVoltage(3)) +
				polyChAudioR[3] *
				cpPanR(params[PAN_PARAM + 3].getValue(), inputs[POLYPAN_INPUT].getVoltage(3));
			ch4Send2 = polyChAudioL[3] *
				cpPanL(params[PAN_PARAM + 3].getValue(), inputs[POLYPAN_INPUT].getVoltage(3)) +
				polyChAudioR[3] *
				cpPanR(params[PAN_PARAM + 3].getValue(), inputs[POLYPAN_INPUT].getVoltage(3));

			//assign solo case
			if (params[SOLO_PARAM + 0].getValue() < 1.f) { soloCase = 1; }
			else if (params[SOLO_PARAM + 1].getValue() < 1.f) { soloCase = 2; }
			else if (params[SOLO_PARAM + 2].getValue() < 1.f) { soloCase = 3; }
			else if (params[SOLO_PARAM + 3].getValue() < 1.f) { soloCase = 4; }
			else { soloCase = 0; } // default case - when solo's are not enabled
			//Solo channels
			switch (soloCase) {
				case 1:
					params[SOLO_PARAM + 0].getValue() == 0.f ? polyChAudioL[0] *= chFadLevel[0] : polyChAudioL[0] = 0.f;
					params[SOLO_PARAM + 1].getValue() == 1.f ? polyChAudioL[1] = 0.f : polyChAudioL[1] *= chFadLevel[1];
					params[SOLO_PARAM + 2].getValue() == 1.f ? polyChAudioL[2] = 0.f : polyChAudioL[2] *= chFadLevel[2];
					params[SOLO_PARAM + 3].getValue() == 1.f ? polyChAudioL[3] = 0.f : polyChAudioL[3] *= chFadLevel[3];
					params[SOLO_PARAM + 0].getValue() == 0.f ? polyChAudioR[0] *= chFadLevel[0] : polyChAudioR[0] = 0.f;
					params[SOLO_PARAM + 1].getValue() == 1.f ? polyChAudioR[1] = 0.f : polyChAudioR[1] *= chFadLevel[1];
					params[SOLO_PARAM + 2].getValue() == 1.f ? polyChAudioR[2] = 0.f : polyChAudioR[2] *= chFadLevel[2];
					params[SOLO_PARAM + 3].getValue() == 1.f ? polyChAudioR[3] = 0.f : polyChAudioR[3] *= chFadLevel[3];
					break;
				case 2:
					params[SOLO_PARAM + 0].getValue() == 1.f ? polyChAudioL[0] = 0.f : polyChAudioL[0] *= chFadLevel[0];
					params[SOLO_PARAM + 1].getValue() == 0.f ? polyChAudioL[1] *= chFadLevel[1] : polyChAudioL[1] = 0.f;
					params[SOLO_PARAM + 2].getValue() == 1.f ? polyChAudioL[2] = 0.f : polyChAudioL[2] *= chFadLevel[2];
					params[SOLO_PARAM + 3].getValue() == 1.f ? polyChAudioL[3] = 0.f : polyChAudioL[3] *= chFadLevel[3];
					params[SOLO_PARAM + 0].getValue() == 1.f ? polyChAudioR[0] = 0.f : polyChAudioR[0] *= chFadLevel[0];
					params[SOLO_PARAM + 1].getValue() == 0.f ? polyChAudioR[1] *= chFadLevel[1] : polyChAudioR[1] = 0.f;
					params[SOLO_PARAM + 2].getValue() == 1.f ? polyChAudioR[2] = 0.f : polyChAudioR[2] *= chFadLevel[2];
					params[SOLO_PARAM + 3].getValue() == 1.f ? polyChAudioR[3] = 0.f : polyChAudioR[3] *= chFadLevel[3];
					break;
				case 3:
					params[SOLO_PARAM + 0].getValue() == 1.f ? polyChAudioL[0] = 0.f : polyChAudioL[0] *= chFadLevel[0];
					params[SOLO_PARAM + 1].getValue() == 1.f ? polyChAudioL[1] = 0.f : polyChAudioL[1] *= chFadLevel[1];
					params[SOLO_PARAM + 2].getValue() == 0.f ? polyChAudioL[2] *= chFadLevel[2] : polyChAudioL[2] = 0.f;
					params[SOLO_PARAM + 3].getValue() == 1.f ? polyChAudioL[3] = 0.f : polyChAudioL[3] *= chFadLevel[3];
					params[SOLO_PARAM + 0].getValue() == 1.f ? polyChAudioR[0] = 0.f : polyChAudioR[0] *= chFadLevel[0];
					params[SOLO_PARAM + 1].getValue() == 1.f ? polyChAudioR[1] = 0.f : polyChAudioR[1] *= chFadLevel[1];
					params[SOLO_PARAM + 2].getValue() == 0.f ? polyChAudioR[2] *= chFadLevel[2] : polyChAudioR[2] = 0.f;
					params[SOLO_PARAM + 3].getValue() == 1.f ? polyChAudioR[3] = 0.f : polyChAudioR[3] *= chFadLevel[3];
					break;
				case 4:
					params[SOLO_PARAM + 0].getValue() == 1.f ? polyChAudioL[0] = 0.f : polyChAudioL[0] *= chFadLevel[0];
					params[SOLO_PARAM + 1].getValue() == 1.f ? polyChAudioL[1] = 0.f : polyChAudioL[1] *= chFadLevel[1];
					params[SOLO_PARAM + 2].getValue() == 1.f ? polyChAudioL[2] = 0.f : polyChAudioL[2] *= chFadLevel[2];
					params[SOLO_PARAM + 3].getValue() == 0.f ? polyChAudioL[3] *= chFadLevel[3] : polyChAudioL[3] = 0.f;
					params[SOLO_PARAM + 0].getValue() == 1.f ? polyChAudioR[0] = 0.f : polyChAudioR[0] *= chFadLevel[0];
					params[SOLO_PARAM + 1].getValue() == 1.f ? polyChAudioR[1] = 0.f : polyChAudioR[1] *= chFadLevel[1];
					params[SOLO_PARAM + 2].getValue() == 1.f ? polyChAudioR[2] = 0.f : polyChAudioR[2] *= chFadLevel[2];
					params[SOLO_PARAM + 3].getValue() == 0.f ? polyChAudioR[3] *= chFadLevel[3] : polyChAudioR[3] = 0.f;
					break;
				default:
					polyChAudioL[0] *= chFadLevel[0];
					polyChAudioL[1] *= chFadLevel[1];
					polyChAudioL[2] *= chFadLevel[2];
					polyChAudioL[3] *= chFadLevel[3];
					polyChAudioR[0] *= chFadLevel[0];
					polyChAudioR[1] *= chFadLevel[1];
					polyChAudioR[2] *= chFadLevel[2];
					polyChAudioR[3] *= chFadLevel[3];
					break;
			}//chSel 0--------------------------------------------------------------------------------------------------------------------------

			sumL[chSel] = (polyChAudioL[0] + polyChAudioL[1] + polyChAudioL[2] + polyChAudioL[3]);
			sumR[chSel] = (polyChAudioR[0] + polyChAudioR[1] + polyChAudioR[2] + polyChAudioR[3]);
		
//---------------------------------------------------------------------------------------------------
		// Channels 5 to 8
		} else if (chSel == 1) {//ch5-8 == index 4-7
			for (int ch5_8 = 4, p = 0; ch5_8 < 8 && p < 4; ch5_8++, p++) {// p == param
				//apply gain curve
				chFadLevel[ch5_8] = simd::pow(params[LEVEL_PARAM + p].getValue(), 2.f);
				//gain and pan | mute --- if mute equals 1 gain assigned fader else gain assigned zero
				params[MUTE_PARAM + p].getValue() == 1.f ? polyChAudioL[ch5_8] *= chFadLevel[ch5_8] *
					cpPanL(params[PAN_PARAM + p].getValue(), inputs[POLYPAN_INPUT].getVoltage(ch5_8)) : 
					polyChAudioL[ch5_8] = 0.f;
				params[MUTE_PARAM + p].getValue() == 1.f ? polyChAudioR[ch5_8] *= chFadLevel[ch5_8] *
					cpPanR(params[PAN_PARAM + p].getValue(), inputs[POLYPAN_INPUT].getVoltage(ch5_8)) :
					polyChAudioR[ch5_8] = 0.f;
			}

			//setAux
			///ch2
			ch1Send1 = polyChAudioL[4] *
				cpPanL(params[PAN_PARAM + 4].getValue(), inputs[POLYPAN_INPUT].getVoltage(4)) +
				polyChAudioR[4] *
				cpPanR(params[PAN_PARAM + 4].getValue(), inputs[POLYPAN_INPUT].getVoltage(4));
			ch1Send2 = polyChAudioL[4] *
				cpPanL(params[PAN_PARAM + 4].getValue(), inputs[POLYPAN_INPUT].getVoltage(4)) +
				polyChAudioR[4] *
				cpPanR(params[PAN_PARAM + 4].getValue(), inputs[POLYPAN_INPUT].getVoltage(4));
			///ch2
			ch2Send1 = polyChAudioL[5] *
				cpPanL(params[PAN_PARAM + 5].getValue(), inputs[POLYPAN_INPUT].getVoltage(5)) +
				polyChAudioR[5] *
				cpPanR(params[PAN_PARAM + 5].getValue(), inputs[POLYPAN_INPUT].getVoltage(5));
			ch2Send2 = polyChAudioL[5] *
				cpPanL(params[PAN_PARAM + 5].getValue(), inputs[POLYPAN_INPUT].getVoltage(5)) +
				polyChAudioR[5] *
				cpPanR(params[PAN_PARAM + 5].getValue(), inputs[POLYPAN_INPUT].getVoltage(5));
			///ch3
			ch3Send1 = polyChAudioL[6] *
				cpPanL(params[PAN_PARAM + 6].getValue(), inputs[POLYPAN_INPUT].getVoltage(6)) +
				polyChAudioR[6] *
				cpPanR(params[PAN_PARAM + 6].getValue(), inputs[POLYPAN_INPUT].getVoltage(6));
			ch3Send2 = polyChAudioL[6] *
				cpPanL(params[PAN_PARAM + 6].getValue(), inputs[POLYPAN_INPUT].getVoltage(6)) +
				polyChAudioR[6] *
				cpPanR(params[PAN_PARAM + 6].getValue(), inputs[POLYPAN_INPUT].getVoltage(6));
			///ch4
			ch4Send1 = polyChAudioL[7] *
				cpPanL(params[PAN_PARAM + 7].getValue(), inputs[POLYPAN_INPUT].getVoltage(7)) +
				polyChAudioR[7] *
				cpPanR(params[PAN_PARAM + 7].getValue(), inputs[POLYPAN_INPUT].getVoltage(7));
			ch4Send2 = polyChAudioL[7] *
				cpPanL(params[PAN_PARAM + 7].getValue(), inputs[POLYPAN_INPUT].getVoltage(7)) +
				polyChAudioR[7] *
				cpPanR(params[PAN_PARAM + 7].getValue(), inputs[POLYPAN_INPUT].getVoltage(7));
			
			//Assign solo case
			if (params[SOLO_PARAM + 0].getValue() < 1.f) { soloCase = 1; } 
			else if (params[SOLO_PARAM + 1].getValue() < 1.f) { soloCase = 2; } 
			else if (params[SOLO_PARAM + 2].getValue() < 1.f) { soloCase = 3; } 
			else if (params[SOLO_PARAM + 3].getValue() < 1.f) { soloCase = 4; } 
			else { soloCase = 0; } // default case - when solo's are not engaged

			//Solo channels
			switch (soloCase) {
				case 1:
					params[SOLO_PARAM + 0].getValue() == 0.f ? polyChAudioL[4] *= chFadLevel[4] : polyChAudioL[4] = 0.f;
					params[SOLO_PARAM + 1].getValue() == 1.f ? polyChAudioL[5] = 0.f : polyChAudioL[5] *= chFadLevel[5];
					params[SOLO_PARAM + 2].getValue() == 1.f ? polyChAudioL[6] = 0.f : polyChAudioL[6] *= chFadLevel[6];
					params[SOLO_PARAM + 3].getValue() == 1.f ? polyChAudioL[7] = 0.f : polyChAudioL[7] *= chFadLevel[7];
					params[SOLO_PARAM + 0].getValue() == 0.f ? polyChAudioR[4] *= chFadLevel[4] : polyChAudioR[4] = 0.f;
					params[SOLO_PARAM + 1].getValue() == 1.f ? polyChAudioR[5] = 0.f : polyChAudioR[5] *= chFadLevel[5];
					params[SOLO_PARAM + 2].getValue() == 1.f ? polyChAudioR[6] = 0.f : polyChAudioR[6] *= chFadLevel[6];
					params[SOLO_PARAM + 3].getValue() == 1.f ? polyChAudioR[7] = 0.f : polyChAudioR[7] *= chFadLevel[7];
					break;
				case 2:
					params[SOLO_PARAM + 0].getValue() == 1.f ? polyChAudioL[4] = 0.f : polyChAudioL[4] *= chFadLevel[4];
					params[SOLO_PARAM + 1].getValue() == 0.f ? polyChAudioL[5] *= chFadLevel[5] : polyChAudioL[5] = 0.f;
					params[SOLO_PARAM + 2].getValue() == 1.f ? polyChAudioL[6] = 0.f : polyChAudioL[6] *= chFadLevel[6];
					params[SOLO_PARAM + 3].getValue() == 1.f ? polyChAudioL[7] = 0.f : polyChAudioL[7] *= chFadLevel[7];
					params[SOLO_PARAM + 0].getValue() == 1.f ? polyChAudioR[4] = 0.f : polyChAudioR[4] *= chFadLevel[4];
					params[SOLO_PARAM + 1].getValue() == 0.f ? polyChAudioR[5] *= chFadLevel[5] : polyChAudioR[5] = 0.f;
					params[SOLO_PARAM + 2].getValue() == 1.f ? polyChAudioR[6] = 0.f : polyChAudioR[6] *= chFadLevel[6];
					params[SOLO_PARAM + 3].getValue() == 1.f ? polyChAudioR[7] = 0.f : polyChAudioR[7] *= chFadLevel[7];
					break;
				case 3:
					params[SOLO_PARAM + 0].getValue() == 1.f ? polyChAudioL[4] = 0.f : polyChAudioL[4] *= chFadLevel[4];
					params[SOLO_PARAM + 1].getValue() == 1.f ? polyChAudioL[5] = 0.f : polyChAudioL[5] *= chFadLevel[5];
					params[SOLO_PARAM + 2].getValue() == 0.f ? polyChAudioL[6] *= chFadLevel[6] : polyChAudioL[6] = 0.f;
					params[SOLO_PARAM + 3].getValue() == 1.f ? polyChAudioL[7] = 0.f : polyChAudioL[7] *= chFadLevel[7];
					params[SOLO_PARAM + 0].getValue() == 1.f ? polyChAudioR[4] = 0.f : polyChAudioR[4] *= chFadLevel[4];
					params[SOLO_PARAM + 1].getValue() == 1.f ? polyChAudioR[5] = 0.f : polyChAudioR[5] *= chFadLevel[5];
					params[SOLO_PARAM + 2].getValue() == 0.f ? polyChAudioR[6] *= chFadLevel[6] : polyChAudioR[6] = 0.f;
					params[SOLO_PARAM + 3].getValue() == 1.f ? polyChAudioR[7] = 0.f : polyChAudioR[7] *= chFadLevel[7];
					break;
				case 4:
					params[SOLO_PARAM + 0].getValue() == 1.f ? polyChAudioL[4] = 0.f : polyChAudioL[4] *= chFadLevel[4];
					params[SOLO_PARAM + 1].getValue() == 1.f ? polyChAudioL[5] = 0.f : polyChAudioL[5] *= chFadLevel[5];
					params[SOLO_PARAM + 2].getValue() == 1.f ? polyChAudioL[6] = 0.f : polyChAudioL[6] *= chFadLevel[6];
					params[SOLO_PARAM + 3].getValue() == 0.f ? polyChAudioL[7] *= chFadLevel[7] : polyChAudioL[7] = 0.f;
					params[SOLO_PARAM + 0].getValue() == 1.f ? polyChAudioR[4] = 0.f : polyChAudioR[4] *= chFadLevel[4];
					params[SOLO_PARAM + 1].getValue() == 1.f ? polyChAudioR[5] = 0.f : polyChAudioR[5] *= chFadLevel[5];
					params[SOLO_PARAM + 2].getValue() == 1.f ? polyChAudioR[6] = 0.f : polyChAudioR[6] *= chFadLevel[6];
					params[SOLO_PARAM + 3].getValue() == 0.f ? polyChAudioR[7] *= chFadLevel[7] : polyChAudioR[7] = 0.f;
					break;
				default:
					polyChAudioL[4] *= chFadLevel[4];
					polyChAudioL[5] *= chFadLevel[5];
					polyChAudioL[6] *= chFadLevel[6];
					polyChAudioL[7] *= chFadLevel[7];
					polyChAudioR[4] *= chFadLevel[4];
					polyChAudioR[5] *= chFadLevel[5];
					polyChAudioR[6] *= chFadLevel[6];
					polyChAudioR[7] *= chFadLevel[7];
					break;
			}//chSel 1---------------------------------------------------------------------------------------------------------------------------

			sumL[chSel] = (polyChAudioL[4] + polyChAudioL[5] + polyChAudioL[6] + polyChAudioL[7]);
			sumR[chSel] = (polyChAudioR[4] + polyChAudioR[5] + polyChAudioR[6] + polyChAudioR[7]);

//------------------------------------------------------------------------------------------------------

		//channels 9 to 12
		} else if (chSel == 2) {//ch9-12 == index 8-11
			for (int ch9_12 = 8, p = 0; ch9_12 < 12 && p < 4; ch9_12++, p++) {// p == param
				//apply gain curve
				chFadLevel[ch9_12] = simd::pow(params[LEVEL_PARAM + p].getValue(), 2.f);
				//gain and pan | mute --- if mute equals 1 gain assigned fader else gain assigned zero
				params[MUTE_PARAM + p].getValue() == 1.f ? polyChAudioL[ch9_12] *= chFadLevel[ch9_12] *
					cpPanL(params[PAN_PARAM + p].getValue(), inputs[POLYPAN_INPUT].getVoltage(ch9_12)) :
					polyChAudioL[ch9_12] = 0.f;
				params[MUTE_PARAM + p].getValue() == 1.f ? polyChAudioR[ch9_12] *= chFadLevel[ch9_12] *
					cpPanR(params[PAN_PARAM + p].getValue(), inputs[POLYPAN_INPUT].getVoltage(ch9_12)) :
					polyChAudioR[ch9_12] = 0.f;
			}

			//setAux
			///ch2
			ch1Send1 = polyChAudioL[8] *
				cpPanL(params[PAN_PARAM + 8].getValue(), inputs[POLYPAN_INPUT].getVoltage(8)) +
				polyChAudioR[8] *
				cpPanR(params[PAN_PARAM + 8].getValue(), inputs[POLYPAN_INPUT].getVoltage(8));
			ch1Send2 = polyChAudioL[8] *
				cpPanL(params[PAN_PARAM + 8].getValue(), inputs[POLYPAN_INPUT].getVoltage(8)) +
				polyChAudioR[8] *
				cpPanR(params[PAN_PARAM + 8].getValue(), inputs[POLYPAN_INPUT].getVoltage(8));
			///ch2
			ch2Send1 = polyChAudioL[9] *
				cpPanL(params[PAN_PARAM + 9].getValue(), inputs[POLYPAN_INPUT].getVoltage(9)) +
				polyChAudioR[9] *
				cpPanR(params[PAN_PARAM + 9].getValue(), inputs[POLYPAN_INPUT].getVoltage(9));
			ch2Send2 = polyChAudioL[9] *
				cpPanL(params[PAN_PARAM + 9].getValue(), inputs[POLYPAN_INPUT].getVoltage(9)) +
				polyChAudioR[9] *
				cpPanR(params[PAN_PARAM + 9].getValue(), inputs[POLYPAN_INPUT].getVoltage(9));
			///ch3
			ch3Send1 = polyChAudioL[10] *
				cpPanL(params[PAN_PARAM + 10].getValue(), inputs[POLYPAN_INPUT].getVoltage(10)) +
				polyChAudioR[10] *
				cpPanR(params[PAN_PARAM + 10].getValue(), inputs[POLYPAN_INPUT].getVoltage(10));
			ch3Send2 = polyChAudioL[10] *
				cpPanL(params[PAN_PARAM + 10].getValue(), inputs[POLYPAN_INPUT].getVoltage(10)) +
				polyChAudioR[10] *
				cpPanR(params[PAN_PARAM + 10].getValue(), inputs[POLYPAN_INPUT].getVoltage(10));
			///ch4
			ch4Send1 = polyChAudioL[11] *
				cpPanL(params[PAN_PARAM + 11].getValue(), inputs[POLYPAN_INPUT].getVoltage(11)) +
				polyChAudioR[11] *
				cpPanR(params[PAN_PARAM + 11].getValue(), inputs[POLYPAN_INPUT].getVoltage(11));
			ch4Send2 = polyChAudioL[11] *
				cpPanL(params[PAN_PARAM + 11].getValue(), inputs[POLYPAN_INPUT].getVoltage(11)) +
				polyChAudioR[11] *
				cpPanR(params[PAN_PARAM + 11].getValue(), inputs[POLYPAN_INPUT].getVoltage(11));
			
			//Assign solo case
			if (params[SOLO_PARAM + 0].getValue() < 1.f) { soloCase = 1; } 
			else if (params[SOLO_PARAM + 1].getValue() < 1.f) { soloCase = 2; } 
			else if (params[SOLO_PARAM + 2].getValue() < 1.f) { soloCase = 3; } 
			else if (params[SOLO_PARAM + 3].getValue() < 1.f) { soloCase = 4; } 
			else { soloCase = 0; } // default case - when solo's are not engaged

			//Solo channels
			switch (soloCase) {
				case 1:
					params[SOLO_PARAM + 0].getValue() == 0.f ? polyChAudioL[8] *= chFadLevel[8] : polyChAudioL[8] = 0.f;
					params[SOLO_PARAM + 1].getValue() == 1.f ? polyChAudioL[9] = 0.f : polyChAudioL[9] *= chFadLevel[9];
					params[SOLO_PARAM + 2].getValue() == 1.f ? polyChAudioL[10] = 0.f : polyChAudioL[10] *= chFadLevel[10];
					params[SOLO_PARAM + 3].getValue() == 1.f ? polyChAudioL[11] = 0.f : polyChAudioL[11] *= chFadLevel[11];
					params[SOLO_PARAM + 0].getValue() == 0.f ? polyChAudioR[8] *= chFadLevel[8] : polyChAudioR[8] = 0.f;
					params[SOLO_PARAM + 1].getValue() == 1.f ? polyChAudioR[9] = 0.f : polyChAudioR[9] *= chFadLevel[9];
					params[SOLO_PARAM + 2].getValue() == 1.f ? polyChAudioR[10] = 0.f : polyChAudioR[10] *= chFadLevel[10];
					params[SOLO_PARAM + 3].getValue() == 1.f ? polyChAudioR[11] = 0.f : polyChAudioR[11] *= chFadLevel[11];
					break;
				case 2:
					params[SOLO_PARAM + 0].getValue() == 1.f ? polyChAudioL[8] = 0.f : polyChAudioL[8] *= chFadLevel[8];
					params[SOLO_PARAM + 1].getValue() == 0.f ? polyChAudioL[9] *= chFadLevel[9] : polyChAudioL[9] = 0.f;
					params[SOLO_PARAM + 2].getValue() == 1.f ? polyChAudioL[10] = 0.f : polyChAudioL[10] *= chFadLevel[10];
					params[SOLO_PARAM + 3].getValue() == 1.f ? polyChAudioL[11] = 0.f : polyChAudioL[11] *= chFadLevel[11];
					params[SOLO_PARAM + 0].getValue() == 1.f ? polyChAudioR[8] = 0.f : polyChAudioR[8] *= chFadLevel[8];
					params[SOLO_PARAM + 1].getValue() == 0.f ? polyChAudioR[9] *= chFadLevel[9] : polyChAudioR[9] = 0.f;
					params[SOLO_PARAM + 2].getValue() == 1.f ? polyChAudioR[10] = 0.f : polyChAudioR[10] *= chFadLevel[10];
					params[SOLO_PARAM + 3].getValue() == 1.f ? polyChAudioR[11] = 0.f : polyChAudioR[11] *= chFadLevel[11];
					break;
				case 3:
					params[SOLO_PARAM + 0].getValue() == 1.f ? polyChAudioL[8] = 0.f : polyChAudioL[8] *= chFadLevel[8];
					params[SOLO_PARAM + 1].getValue() == 1.f ? polyChAudioL[9] = 0.f : polyChAudioL[9] *= chFadLevel[9];
					params[SOLO_PARAM + 2].getValue() == 0.f ? polyChAudioL[10] *= chFadLevel[10] : polyChAudioL[10] = 0.f;
					params[SOLO_PARAM + 3].getValue() == 1.f ? polyChAudioL[11] = 0.f : polyChAudioL[11] *= chFadLevel[11];
					params[SOLO_PARAM + 0].getValue() == 1.f ? polyChAudioR[8] = 0.f : polyChAudioR[8] *= chFadLevel[8];
					params[SOLO_PARAM + 1].getValue() == 1.f ? polyChAudioR[9] = 0.f : polyChAudioR[9] *= chFadLevel[9];
					params[SOLO_PARAM + 2].getValue() == 0.f ? polyChAudioR[10] *= chFadLevel[10] : polyChAudioR[10] = 0.f;
					params[SOLO_PARAM + 3].getValue() == 1.f ? polyChAudioR[11] = 0.f : polyChAudioR[11] *= chFadLevel[11];
					break;
				case 4:
					params[SOLO_PARAM + 0].getValue() == 1.f ? polyChAudioL[8] = 0.f : polyChAudioL[8] *= chFadLevel[8];
					params[SOLO_PARAM + 1].getValue() == 1.f ? polyChAudioL[9] = 0.f : polyChAudioL[9] *= chFadLevel[9];
					params[SOLO_PARAM + 2].getValue() == 1.f ? polyChAudioL[10] = 0.f : polyChAudioL[10] *= chFadLevel[10];
					params[SOLO_PARAM + 3].getValue() == 0.f ? polyChAudioL[11] *= chFadLevel[11] : polyChAudioL[11] = 0.f;
					params[SOLO_PARAM + 0].getValue() == 1.f ? polyChAudioR[8] = 0.f : polyChAudioR[8] *= chFadLevel[8];
					params[SOLO_PARAM + 1].getValue() == 1.f ? polyChAudioR[9] = 0.f : polyChAudioR[9] *= chFadLevel[9];
					params[SOLO_PARAM + 2].getValue() == 1.f ? polyChAudioR[10] = 0.f : polyChAudioR[10] *= chFadLevel[10];
					params[SOLO_PARAM + 3].getValue() == 0.f ? polyChAudioR[11] *= chFadLevel[11] : polyChAudioR[11] = 0.f;
					break;
				default:
					polyChAudioL[8] *= chFadLevel[8];
					polyChAudioL[9] *= chFadLevel[9];
					polyChAudioL[10] *= chFadLevel[10];
					polyChAudioL[11] *= chFadLevel[11];
					polyChAudioR[8] *= chFadLevel[8];
					polyChAudioR[9] *= chFadLevel[9];
					polyChAudioR[10] *= chFadLevel[10];
					polyChAudioR[11] *= chFadLevel[11];
					break;
			}//chSel 2---------------------------------------------------------------------------------------------------------------------------

			//sum mixer channels to array
			sumL[chSel] = (polyChAudioL[8] + polyChAudioL[9] + polyChAudioL[10] + polyChAudioL[11]);
			sumR[chSel] = (polyChAudioR[8] + polyChAudioR[9] + polyChAudioR[10] + polyChAudioR[11]);

//--------------------------------------------------------------------------------------------------------

		//channels 12 to 16
		} else if (chSel == 3) {//ch12-16 == index 11-15
			for (int ch12_15 = 12, p = 0; ch12_15 < 16 && p < 4; ch12_15++, p++) {// p == param
				//apply gain curve
				chFadLevel[ch12_15] = simd::pow(params[LEVEL_PARAM + p].getValue(), 2.f);
				//gain and pan | mute --- if mute equals 1 gain assigned fader else gain assigned zero
				params[MUTE_PARAM + p].getValue() == 1.f ? polyChAudioL[ch12_15] *= chFadLevel[ch12_15] *
					cpPanL(params[PAN_PARAM + p].getValue(), inputs[POLYPAN_INPUT].getVoltage(ch12_15)) :
					polyChAudioL[ch12_15] = 0.f;
				params[MUTE_PARAM + p].getValue() == 1.f ? polyChAudioR[ch12_15] *= chFadLevel[ch12_15] *
					cpPanR(params[PAN_PARAM + p].getValue(), inputs[POLYPAN_INPUT].getVoltage(ch12_15)) :
					polyChAudioR[ch12_15] = 0.f;
			}

			//setAux
			///ch2
			ch1Send1 = polyChAudioL[12] *
				cpPanL(params[PAN_PARAM + 12].getValue(), inputs[POLYPAN_INPUT].getVoltage(12)) +
				polyChAudioR[12] *
				cpPanR(params[PAN_PARAM + 12].getValue(), inputs[POLYPAN_INPUT].getVoltage(12));
			ch1Send2 = polyChAudioL[12] *
				cpPanL(params[PAN_PARAM + 12].getValue(), inputs[POLYPAN_INPUT].getVoltage(12)) +
				polyChAudioR[12] *
				cpPanR(params[PAN_PARAM + 12].getValue(), inputs[POLYPAN_INPUT].getVoltage(12));
			///ch2
			ch2Send1 = polyChAudioL[13] *
				cpPanL(params[PAN_PARAM + 13].getValue(), inputs[POLYPAN_INPUT].getVoltage(13)) +
				polyChAudioR[13] *
				cpPanR(params[PAN_PARAM + 13].getValue(), inputs[POLYPAN_INPUT].getVoltage(13));
			ch2Send2 = polyChAudioL[13] *
				cpPanL(params[PAN_PARAM + 13].getValue(), inputs[POLYPAN_INPUT].getVoltage(13)) +
				polyChAudioR[13] *
				cpPanR(params[PAN_PARAM + 13].getValue(), inputs[POLYPAN_INPUT].getVoltage(13));
			///ch3
			ch3Send1 = polyChAudioL[14] *
				cpPanL(params[PAN_PARAM + 14].getValue(), inputs[POLYPAN_INPUT].getVoltage(14)) +
				polyChAudioR[14] *
				cpPanR(params[PAN_PARAM + 14].getValue(), inputs[POLYPAN_INPUT].getVoltage(14));
			ch3Send2 = polyChAudioL[14] *
				cpPanL(params[PAN_PARAM + 14].getValue(), inputs[POLYPAN_INPUT].getVoltage(14)) +
				polyChAudioR[14] *
				cpPanR(params[PAN_PARAM + 14].getValue(), inputs[POLYPAN_INPUT].getVoltage(14));
			///ch4
			ch4Send1 = polyChAudioL[15] *
				cpPanL(params[PAN_PARAM + 15].getValue(), inputs[POLYPAN_INPUT].getVoltage(15)) +
				polyChAudioR[15] *
				cpPanR(params[PAN_PARAM + 15].getValue(), inputs[POLYPAN_INPUT].getVoltage(15));
			ch4Send2 = polyChAudioL[15] *
				cpPanL(params[PAN_PARAM + 15].getValue(), inputs[POLYPAN_INPUT].getVoltage(15)) +
				polyChAudioR[15] *
				cpPanR(params[PAN_PARAM + 15].getValue(), inputs[POLYPAN_INPUT].getVoltage(15));

			//Assign solo case
			if (params[SOLO_PARAM + 0].getValue() < 1.f) { soloCase = 1; } 
			else if (params[SOLO_PARAM + 1].getValue() < 1.f) { soloCase = 2; } 
			else if (params[SOLO_PARAM + 2].getValue() < 1.f) { soloCase = 3; } 
			else if (params[SOLO_PARAM + 3].getValue() < 1.f) { soloCase = 4; } 
			else { soloCase = 0; } // default case - when solo's are not engaged

			//Solo channels
			switch (soloCase) {
				case 1:
					params[SOLO_PARAM + 0].getValue() == 0.f ? polyChAudioL[12] *= chFadLevel[12] : polyChAudioL[12] = 0.f;
					params[SOLO_PARAM + 1].getValue() == 1.f ? polyChAudioL[13] = 0.f : polyChAudioL[13] *= chFadLevel[13];
					params[SOLO_PARAM + 2].getValue() == 1.f ? polyChAudioL[14] = 0.f : polyChAudioL[14] *= chFadLevel[14];
					params[SOLO_PARAM + 3].getValue() == 1.f ? polyChAudioL[15] = 0.f : polyChAudioL[15] *= chFadLevel[15];
					params[SOLO_PARAM + 0].getValue() == 0.f ? polyChAudioR[12] *= chFadLevel[12] : polyChAudioR[12] = 0.f;
					params[SOLO_PARAM + 1].getValue() == 1.f ? polyChAudioR[13] = 0.f : polyChAudioR[13] *= chFadLevel[13];
					params[SOLO_PARAM + 2].getValue() == 1.f ? polyChAudioR[14] = 0.f : polyChAudioR[14] *= chFadLevel[14];
					params[SOLO_PARAM + 3].getValue() == 1.f ? polyChAudioR[15] = 0.f : polyChAudioR[15] *= chFadLevel[15];
					break;
				case 2:
					params[SOLO_PARAM + 0].getValue() == 1.f ? polyChAudioL[12] = 0.f : polyChAudioL[12] *= chFadLevel[12];
					params[SOLO_PARAM + 1].getValue() == 0.f ? polyChAudioL[13] *= chFadLevel[13] : polyChAudioL[13] = 0.f;
					params[SOLO_PARAM + 2].getValue() == 1.f ? polyChAudioL[14] = 0.f : polyChAudioL[14] *= chFadLevel[14];
					params[SOLO_PARAM + 3].getValue() == 1.f ? polyChAudioL[15] = 0.f : polyChAudioL[15] *= chFadLevel[15];
					params[SOLO_PARAM + 0].getValue() == 1.f ? polyChAudioR[12] = 0.f : polyChAudioR[12] *= chFadLevel[12];
					params[SOLO_PARAM + 1].getValue() == 0.f ? polyChAudioR[13] *= chFadLevel[13] : polyChAudioR[13] = 0.f;
					params[SOLO_PARAM + 2].getValue() == 1.f ? polyChAudioR[14] = 0.f : polyChAudioR[14] *= chFadLevel[14];
					params[SOLO_PARAM + 3].getValue() == 1.f ? polyChAudioR[15] = 0.f : polyChAudioR[15] *= chFadLevel[15];
					break;
				case 3:
					params[SOLO_PARAM + 0].getValue() == 1.f ? polyChAudioL[12] = 0.f : polyChAudioL[12] *= chFadLevel[12];
					params[SOLO_PARAM + 1].getValue() == 1.f ? polyChAudioL[13] = 0.f : polyChAudioL[13] *= chFadLevel[13];
					params[SOLO_PARAM + 2].getValue() == 0.f ? polyChAudioL[14] *= chFadLevel[14] : polyChAudioL[14] = 0.f;
					params[SOLO_PARAM + 3].getValue() == 1.f ? polyChAudioL[15] = 0.f : polyChAudioL[15] *= chFadLevel[15];
					params[SOLO_PARAM + 0].getValue() == 1.f ? polyChAudioR[12] = 0.f : polyChAudioR[12] *= chFadLevel[12];
					params[SOLO_PARAM + 1].getValue() == 1.f ? polyChAudioR[13] = 0.f : polyChAudioR[13] *= chFadLevel[13];
					params[SOLO_PARAM + 2].getValue() == 0.f ? polyChAudioR[14] *= chFadLevel[14] : polyChAudioR[14] = 0.f;
					params[SOLO_PARAM + 3].getValue() == 1.f ? polyChAudioR[15] = 0.f : polyChAudioR[15] *= chFadLevel[15];
					break;
				case 4:
					params[SOLO_PARAM + 0].getValue() == 1.f ? polyChAudioL[12] = 0.f : polyChAudioL[12] *= chFadLevel[12];
					params[SOLO_PARAM + 1].getValue() == 1.f ? polyChAudioL[13] = 0.f : polyChAudioL[13] *= chFadLevel[13];
					params[SOLO_PARAM + 2].getValue() == 1.f ? polyChAudioL[14] = 0.f : polyChAudioL[14] *= chFadLevel[14];
					params[SOLO_PARAM + 3].getValue() == 0.f ? polyChAudioL[15] *= chFadLevel[15] : polyChAudioL[15] = 0.f;
					params[SOLO_PARAM + 0].getValue() == 1.f ? polyChAudioR[12] = 0.f : polyChAudioR[12] *= chFadLevel[12];
					params[SOLO_PARAM + 1].getValue() == 1.f ? polyChAudioR[13] = 0.f : polyChAudioR[13] *= chFadLevel[13];
					params[SOLO_PARAM + 2].getValue() == 1.f ? polyChAudioR[14] = 0.f : polyChAudioR[14] *= chFadLevel[14];
					params[SOLO_PARAM + 3].getValue() == 0.f ? polyChAudioR[15] *= chFadLevel[15] : polyChAudioR[15] = 0.f;
					break;
				default:
					polyChAudioL[12] *= chFadLevel[12];
					polyChAudioL[13] *= chFadLevel[13];
					polyChAudioL[14] *= chFadLevel[14];
					polyChAudioL[15] *= chFadLevel[15];
					polyChAudioR[12] *= chFadLevel[12];
					polyChAudioR[13] *= chFadLevel[13];
					polyChAudioR[14] *= chFadLevel[14];
					polyChAudioR[15] *= chFadLevel[15];
					break;
			}

			//sum mixer channels to array
			sumL[chSel] = (polyChAudioL[12] + polyChAudioL[13] + polyChAudioL[14] + polyChAudioL[15]);
			sumR[chSel] = (polyChAudioR[12] + polyChAudioR[13] + polyChAudioR[14] + polyChAudioR[15]);
		}//chSel 3---------------------------------------------------------------------------------------------------------------------------

		//*** TODO: storing values
		//float lastValue = params[LEVEL_PARAM + 0].getValue();

		//apply 2^send---
		float aux1Ch1 = simd::pow(params[AUX1_PARAM + 0].getValue(), 2.f), aux2Ch1 = simd::pow(params[AUX2_PARAM + 0].getValue(), 2.f),
			aux1Ch2 = simd::pow(params[AUX1_PARAM + 1].getValue(), 2.f), aux2Ch2 = simd::pow(params[AUX2_PARAM + 1].getValue(), 2.f),
			aux1Ch3 = simd::pow(params[AUX1_PARAM + 2].getValue(), 2.f), aux2Ch3 = simd::pow(params[AUX2_PARAM + 2].getValue(), 2.f),
			aux1Ch4 = simd::pow(params[AUX1_PARAM + 3].getValue(), 2.f), aux2Ch4 = simd::pow(params[AUX2_PARAM + 3].getValue(), 2.f);

		float return1 = inputs[AUX1RETURN_INPUT].getVoltage(), return2 = inputs[AUX2RETURN_INPUT].getVoltage();
		
		//sumAux
		outputs[AUX1SEND_OUTPUT].setVoltage((ch1Send1 * aux1Ch1) + (ch2Send1 * aux1Ch2) + (ch3Send1 * aux1Ch3) + (ch4Send1 * aux1Ch4));
		outputs[AUX2SEND_OUTPUT].setVoltage((ch1Send2 * aux2Ch1) + (ch2Send2 * aux2Ch2) + (ch3Send2 * aux2Ch3) + (ch4Send2 * aux2Ch4));

		inputs[GAINLEVEL_INPUT].isConnected() == true ? cvMasterLevel = simd::clamp(inputs[GAINLEVEL_INPUT].getVoltage(), 0.f, 1.f) : cvMasterLevel;
		float outGain = params[MASTERGAIN_PARAM].getValue() * cvMasterLevel;
		float Left = sumL[chSel] * outGain + ((return1 + return2) / 2), Right = sumR[chSel] * outGain + ((return1 + return2) / 2);

		//simple dcblocker
		float hpFreq = 16.35f;
		dcblockL.setCutoff(hpFreq / args.sampleRate);
		dcblockR.setCutoff(hpFreq / args.sampleRate);
		dcblockL.process(Left);
		dcblockR.process(Right);

		Left -= dcblockL.lowpass();
		Right -= dcblockL.lowpass();
		
		outputs[OUTL_OUTPUT].setVoltage(Left);
		outputs[OUTR_OUTPUT].setVoltage(Right);

	}//process

};

struct PolyMixWidget : ModuleWidget {
	PolyMixWidget(PolyMix *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BarkPolyMix.svg")));

		//constexpr int rackY = 380;
		constexpr float chParamX[4] = { 14.453f, 47.311f, 82.689f, 115.546f }, chInvX[4] = { 11.53f, 44.37f, 79.74f, 112.6f },
			chFaderX[4] = { 29.13f, 62.f, 97.38f, 130.24f }, chBtnX[4] = { 13.99f, 46.63f, 82.f, 114.86f };

		///Ports---
		//Out---
		addOutput(createOutput<BarkOutPort350>(Vec(5.68f, rackY - 353.13f), module, PolyMix::OUTL_OUTPUT));
		addOutput(createOutput<BarkOutPort350>(Vec(32.12f, rackY - 353.13f), module, PolyMix::OUTR_OUTPUT));
		addOutput(createOutput<BarkOutPort350>(Vec(5.68f, rackY - 285.05f), module, PolyMix::AUX1SEND_OUTPUT));
		addOutput(createOutput<BarkOutPort350>(Vec(32.12f, rackY - 285.05f), module, PolyMix::AUX2SEND_OUTPUT));
		//In---
		addInput(createInput<BarkPatchPortIn>(Vec(118.74f, rackY - 353.13f), module, PolyMix::POLYAUDIO_INPUT));
		addInput(createInput<BarkPatchPortIn>(Vec(118.74f, rackY - 318.13f), module, PolyMix::POLYLEVEL_INPUT));
		addInput(createInput<BarkPatchPortIn>(Vec(118.74f, rackY - 283.15f), module, PolyMix::POLYPAN_INPUT));
		addInput(createInput<BarkPatchPortIn>(Vec(77.37f, rackY - 318.27f), module, PolyMix::GAINLEVEL_INPUT));
		addInput(createInput<BarkInPort350>(Vec(5.67f, rackY - 315.31f), module, PolyMix::AUX1RETURN_INPUT));
		addInput(createInput<BarkInPort350>(Vec(32.90f, rackY - 315.31f), module, PolyMix::AUX2RETURN_INPUT));
		//Knobs---
		addParam(createParam<BarkKnob_40>(Vec(69.49f, rackY - 292.32f), module, PolyMix::MASTERGAIN_PARAM));
		addParam(createParam<BarkKnobSwitchSmall>(Vec(79.5f, rackY - 361.89f), module, PolyMix::CHSELECT_PARAM));
		//Repeated---
		for (int i = 0; i < 4; i++) {
			addParam(createParam<BarkKnob_20>(Vec(chParamX[i], rackY - 226.21f), module, PolyMix::AUX1_PARAM + i));
			addParam(createParam<BarkKnob_20>(Vec(chParamX[i], rackY - 195.53f), module, PolyMix::AUX2_PARAM + i));
			addParam(createParam<BarkKnob_20>(Vec(chParamX[i], rackY - 160.4f), module, PolyMix::PAN_PARAM + i));
			addParam(createParam<BarkFaderLong>(Vec(chFaderX[i], rackY - 131.06f), module, PolyMix::LEVEL_PARAM + i));
			//Switch/Button---
			addParam(createParam<BarkSwitchSmall>(Vec(chInvX[i], rackY - 92.27f), module, PolyMix::INVERT_PARAM + i));
			addParam(createParam<BarkBtnMute>(Vec(chBtnX[i], rackY - 128.82f), module, PolyMix::MUTE_PARAM + i));
			addParam(createParam<BarkBtnSolo>(Vec(chBtnX[i], rackY - 111.32f), module, PolyMix::SOLO_PARAM + i));

		}
		//Screws---
		addChild(createWidget<RandomRotateScrew>(Vec(2.7f, 2.7f)));				//pos1
		addChild(createWidget<RandomRotateScrew>(Vec(box.size.x - 12.3f, 367.7f)));		//pos4
		//Light---

	}

};

Model *modelPolyMix = createModel<PolyMix, PolyMixWidget>("PolyMix");
