#include "plugin.hpp"
#include "barkComponents.hpp"

using namespace barkComponents;

const int chSelect = 0;

struct tpChannelSelect : ParamQuantity {
	std::string getDisplayValueString() override {
		if (getValue() == 0.f)
			return "1-4";
		else if (getValue() == 1.f) {
			return "5-8";
		} else if (getValue() == 2.f) {
			return "9-12";
		} else
			return "13-16";
	}
};

struct tpOnOff : ParamQuantity {
	std::string getDisplayValueString() override {
		if (getValue() < 1.f)
			return "On";
		else
			return "Off";
	}
};

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
		//6 inputs
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
			configParam<tpOnOff>(INVERT_PARAM + i, 0.f, 1.f, 1.f, "Invert Ch " + std::to_string(i + 1));
			configParam<tpOnOff>(MUTE_PARAM + i, 0.f, 1.f, 1.f, "Mute Ch " + std::to_string(i + 1));
			configParam(SOLO_PARAM + i, 0.f, 1.f, 1.f, "Solo Ch " + std::to_string(i + 1));	//<tpOnOff>
			//TODO channels numbers on tp for channels 5-16
			//***pseudo constant int ??function for when channel changed??, i * chSel + 1
		}

		//pseudo
		//float constPowPan(float position) {
		//	PANPOS pos;
		//	/* pi/2: 1/4 cycle of a sinusoid */
		//	const double  piovr2 = 4.0 * atan(1.0) * 0.5;		//	M_PI_2 == 1.57079632679489661923
		//	const double  root2ovr2 = sqrt(2.0) * 0.5;			//	M_SQRT1_2 == 0.707106781186547524401
		//	params[PAN_PARAM].getValue();//±0.707106781186547524401
		//	/* scale position to fit the pi/2 range */
		//	double thispos = position * piovr2;						// paramvalue min == -M_SQRT1_2, max == M_SQRT1_2, thispos == ±1.1107207345395915617532801458281
		//	/* each channel uses a 1/4 of a cycle */
		//	double angle = thispos * 0.5;					    	// 1.1107207345395915617532801458281 / 2 == ±0.55536036726979578087664007291405

		//	pos.left = root2ovr2 * (cos(angle) - sin(angle));
		//	pos.right = root2ovr2 * (cos(angle) + sin(angle));
		//	return pos;
		//}

	}

	void process(const ProcessArgs &args) override {
		//initilize variables 
		//bool soloSafe[16] = {}, solo[16] = {}, mute[16] = {};
		//float auxSend1Lvl[16] = {}, auxSend2Lvl[16] = {}, pan[16] = {}; 
		float polyChAudioL[16], polyChAudioR[16], polyChLevel[16] = {}, polyChPan[16] = {}, chFadLevel[16];
		int chSel = ( int )params[CHSELECT_PARAM].getValue(), nChAudio = 0, nChLevel = 0, nChPan = 0, maxCh = 16, soloCase; //could use for index pos
		//holds 4 (L/R) values for summing to output
		float sumL[4], sumR[4];//master gain
		float cvMasterLevel = 1.f, ch1Send1, ch2Send1, ch3Send1, ch4Send1, ch1Send2, ch2Send2, ch3Send2, ch4Send2;

		//get audio channels
		//get #channels connected
		nChAudio = inputs[POLYAUDIO_INPUT].getChannels();//int
		if (inputs[POLYAUDIO_INPUT].isConnected()) {
			for (int i = 0; i < nChAudio; i++) {//change to channels?
				//assign channels to arrays
				polyChAudioL[i] = polyChAudioR[i] = inputs[POLYAUDIO_INPUT].getVoltage(i);
				maxCh = std::max(maxCh, nChAudio);//		??needed??
				//invert here?
				//outputs[INVERT_OUTPUT].setVoltage(-inputs[SIGN_INPUT].getVoltage());
			}
		}

		//get cv level channels
		//get #channels connected
		nChLevel = inputs[POLYLEVEL_INPUT].getChannels();	//int
		if (inputs[POLYLEVEL_INPUT].isConnected()) {
			for (int i = 0; i < nChLevel; i++) {//		!16 as disconnected channels are set to 0.f
				//assign channels to arrays
				polyChLevel[i] = inputs[POLYLEVEL_INPUT].getVoltage(i);	// recall
				float cvLevel = clamp(inputs[POLYLEVEL_INPUT].getPolyVoltage(i) / 10, 0.f, 1.f);
				polyChAudioL[i] = polyChAudioR[i] *= cvLevel;
				//**** when number of channels connected is not the same as audio connected level is set to 0.f

			}
		}
		//pan channels *** REDO!
		if (inputs[POLYPAN_INPUT].isConnected()) {
			for (int i = 0; i < 16; i++) {
				//get #channels connected
				nChPan = inputs[POLYPAN_INPUT].getChannels();//int
				//assign channels to array
				polyChLevel[i] = inputs[POLYPAN_INPUT].getVoltage(i);// recall
				float panLevel = inputs[POLYPAN_INPUT].getPolyVoltage(i) < 0 ?
					polyChAudioL[i] *= chFadLevel[i] * (1 - clamp(params[PAN_PARAM + i].getValue(), 0.f, 1.f)) :
					polyChAudioR[i] *= chFadLevel[i] * (1 - clamp(-params[PAN_PARAM + i].getValue(), 0.f, 1.f));
				polyChAudioL[i] *= panLevel;
				//polyChAudioR[i] *= panLevel;
			}
		}

		// set channel selection to 4 fader ch
		if (chSel == 0) {//ch1-4, index 0-3
			for (int ch1_4 = 0; ch1_4 < 4; ch1_4++) {
				//apply gain curve
				chFadLevel[ch1_4] = std::pow(params[LEVEL_PARAM + ch1_4].getValue(), 2.f);
				//gain and pan | mute --- if mute equals 1 gain assigned fader else gain assigned zero
				//GainParam * (1 - clamp(params[PAN_PARAM].value, 0.0f, 1.0f));
				params[MUTE_PARAM + ch1_4].getValue() == 1.f ? polyChAudioL[ch1_4] *= chFadLevel[ch1_4] *
					(1 - clamp(params[PAN_PARAM + ch1_4].getValue(), 0.f, 1.f)) : polyChAudioL[ch1_4] = 0.f;
				params[MUTE_PARAM + ch1_4].getValue() == 1.f ? polyChAudioR[ch1_4] *= chFadLevel[ch1_4] *
					(1 - clamp(-params[PAN_PARAM + ch1_4].getValue(), 0.f, 1.f)) : polyChAudioR[ch1_4] = 0.f;
				//get working---
				//params[INVERT_PARAM + ch1_4].getValue() == 0.f ? polyChAudioL[ch1_4] : -polyChAudioL[ch1_4];	//inputs[POLYAUDIO_INPUT + ch1_4].getVoltage(ch1_4)
			}
			//setAux
			ch1Send1 = polyChAudioL[0] + polyChAudioR[0]; ch1Send2 = polyChAudioL[0] + polyChAudioR[0];
			ch2Send1 = polyChAudioL[1] + polyChAudioR[1]; ch2Send2 = polyChAudioL[1] + polyChAudioR[1];
			ch3Send1 = polyChAudioL[2] + polyChAudioR[2]; ch3Send2 = polyChAudioL[2] + polyChAudioR[2];
			ch4Send1 = polyChAudioL[3] + polyChAudioR[3]; ch4Send2 = polyChAudioL[3] + polyChAudioR[3];

			//Assign soloCase
			/*for (int soloP = 0; soloP < 4; soloP++) {
			if (params[SOLO_PARAM + soloP].getValue() < 1.f) {
					soloCase = soloP;
				}
			}*/

			if (params[SOLO_PARAM + 0].getValue() < 1.f) { soloCase = 1; } 
			else if (params[SOLO_PARAM + 1].getValue() < 1.f) { soloCase = 2; } 
			else if (params[SOLO_PARAM + 2].getValue() < 1.f) { soloCase = 3; } 
			else if (params[SOLO_PARAM + 3].getValue() < 1.f) { soloCase = 4; } 
			else { soloCase = 0; } // default case - if solo's are not engaged
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
			}
			sumL[chSel] = (polyChAudioL[0] + polyChAudioL[1] + polyChAudioL[2] + polyChAudioL[3]);
			sumR[chSel] = (polyChAudioR[0] + polyChAudioR[1] + polyChAudioR[2] + polyChAudioR[3]);

		} else if (chSel == 1) {//ch5-8 == index 4-7
			for (int ch5_8 = 4, p = 0; ch5_8 < 8 && p < 4; ch5_8++, p++) {// p == param
				//apply gain curve
				chFadLevel[ch5_8] = std::pow(params[LEVEL_PARAM + p].getValue(), 2.f);
				//gain and pan | mute --- if mute equals 1 gain assigned fader else gain assigned zero
				params[MUTE_PARAM + p].getValue() == 1.f ? polyChAudioL[ch5_8] *= chFadLevel[ch5_8] *
					(1 - clamp(params[PAN_PARAM + p].getValue(), 0.f, 1.f)) : polyChAudioL[ch5_8] = 0.f;
				params[MUTE_PARAM + p].getValue() == 1.f ? polyChAudioR[ch5_8] *= chFadLevel[ch5_8] *
					(1 - clamp(-params[PAN_PARAM + p].getValue(), 0.f, 1.f)) : polyChAudioR[ch5_8] = 0.f;
				//get working---
				//params[INVERT_PARAM + ch1_4].getValue() == 0.f ? polyChAudioL[ch5_8] : -polyChAudioL[ch5_8];
			}
			//setAux
			ch1Send1 = polyChAudioL[4] + polyChAudioR[4]; ch1Send2 = polyChAudioL[4] + polyChAudioR[4];
			ch2Send1 = polyChAudioL[5] + polyChAudioR[5]; ch2Send2 = polyChAudioL[5] + polyChAudioR[5];
			ch3Send1 = polyChAudioL[6] + polyChAudioR[6]; ch3Send2 = polyChAudioL[6] + polyChAudioR[6];
			ch4Send1 = polyChAudioL[7] + polyChAudioR[7]; ch4Send2 = polyChAudioL[7] + polyChAudioR[7];
			//Assign soloCase
			if (params[SOLO_PARAM + 0].getValue() < 1.f) { soloCase = 1; } 
			else if (params[SOLO_PARAM + 1].getValue() < 1.f) { soloCase = 2; } 
			else if (params[SOLO_PARAM + 2].getValue() < 1.f) { soloCase = 3; } 
			else if (params[SOLO_PARAM + 3].getValue() < 1.f) { soloCase = 4; } 
			else { soloCase = 0; } // default case - if solo's are not engaged
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
			}

			sumL[chSel] = (polyChAudioL[4] + polyChAudioL[5] + polyChAudioL[6] + polyChAudioL[7]);
			sumR[chSel] = (polyChAudioR[4] + polyChAudioR[5] + polyChAudioR[6] + polyChAudioR[7]);
		} else if (chSel == 2) {//ch9-12 == index 8-11
			for (int ch9_12 = 8, p = 0; ch9_12 < 11 && p < 4; ch9_12++, p++) {// p == param
				//apply gain curve
				chFadLevel[ch9_12] = std::pow(params[LEVEL_PARAM + p].getValue(), 2.f);
				//gain and pan | mute --- if mute equals 1 gain assigned fader else gain assigned zero
				params[MUTE_PARAM + p].getValue() == 1.f ? polyChAudioL[ch9_12] *= chFadLevel[ch9_12] *
					(1 - clamp(params[PAN_PARAM + p].getValue(), 0.f, 1.f)) : polyChAudioL[ch9_12] = 0.f;
				params[MUTE_PARAM + p].getValue() == 1.f ? polyChAudioR[ch9_12] *= chFadLevel[ch9_12] *
					(1 - clamp(-params[PAN_PARAM + p].getValue(), 0.f, 1.f)) : polyChAudioR[ch9_12] = 0.f;
				//get working---
				//params[INVERT_PARAM + ch1_4].getValue() == 0.f ? polyChAudioL[ch5_8] : -polyChAudioL[ch5_8];
			}
			//setAux
			ch1Send1 = polyChAudioL[8] + polyChAudioR[8]; ch1Send2 = polyChAudioL[8] + polyChAudioR[8];
			ch2Send1 = polyChAudioL[9] + polyChAudioR[9]; ch2Send2 = polyChAudioL[9] + polyChAudioR[9];
			ch3Send1 = polyChAudioL[10] + polyChAudioR[10]; ch3Send2 = polyChAudioL[10] + polyChAudioR[10];
			ch4Send1 = polyChAudioL[11] + polyChAudioR[11]; ch4Send2 = polyChAudioL[11] + polyChAudioR[11];
			//Assign soloCase
			if (params[SOLO_PARAM + 0].getValue() < 1.f) { soloCase = 1; } else if (params[SOLO_PARAM + 1].getValue() < 1.f) { soloCase = 2; } else if (params[SOLO_PARAM + 2].getValue() < 1.f) { soloCase = 3; } else if (params[SOLO_PARAM + 3].getValue() < 1.f) { soloCase = 4; } else { soloCase = 0; } // default case - if solo's are not engaged
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
			}
			//sum mixer channels to array
			sumL[chSel] = (polyChAudioL[8] + polyChAudioL[9] + polyChAudioL[10] + polyChAudioL[11]);
			sumR[chSel] = (polyChAudioR[8] + polyChAudioR[9] + polyChAudioR[10] + polyChAudioR[11]);
		} else if (chSel == 3) {//ch13-16 == index 12-15
			for (int ch13_16 = 12, p = 0; ch13_16 < 15 && p < 4; ch13_16++, p++) {// p == param
				//apply gain curve
				chFadLevel[ch13_16] = std::pow(params[LEVEL_PARAM + p].getValue(), 2.f);
				//gain and pan | mute --- if mute equals 1 gain assigned fader else gain assigned zero
				params[MUTE_PARAM + p].getValue() == 1.f ? polyChAudioL[ch13_16] *= chFadLevel[ch13_16] *
					(1 - clamp(params[PAN_PARAM + p].getValue(), 0.f, 1.f)) : polyChAudioL[ch13_16] = 0.f;
				params[MUTE_PARAM + p].getValue() == 1.f ? polyChAudioR[ch13_16] *= chFadLevel[ch13_16] *
					(1 - clamp(-params[PAN_PARAM + p].getValue(), 0.f, 1.f)) : polyChAudioR[ch13_16] = 0.f;
				//get working---
				//params[INVERT_PARAM + ch1_4].getValue() == 0.f ? polyChAudioL[ch5_8] : -polyChAudioL[ch5_8];
			}
			//setAux
			ch1Send1 = polyChAudioL[12] + polyChAudioR[12]; ch1Send2 = polyChAudioL[12] + polyChAudioR[12];
			ch2Send1 = polyChAudioL[13] + polyChAudioR[13]; ch2Send2 = polyChAudioL[13] + polyChAudioR[13];
			ch3Send1 = polyChAudioL[14] + polyChAudioR[14]; ch3Send2 = polyChAudioL[14] + polyChAudioR[14];
			ch4Send1 = polyChAudioL[15] + polyChAudioR[15]; ch4Send2 = polyChAudioL[15] + polyChAudioR[15];
			//Assign soloCase
			if (params[SOLO_PARAM + 0].getValue() == 1.f) { soloCase = 1; } 
			else if (params[SOLO_PARAM + 1].getValue() == 1.f) { soloCase = 2; } 
			else if (params[SOLO_PARAM + 2].getValue() == 1.f) { soloCase = 3; } 
			else if (params[SOLO_PARAM + 3].getValue() == 1.f) { soloCase = 4; } 
			else { soloCase = 0; } // default case - if solo's are not engaged
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
			//sum mixer channels to array[4]
			sumL[chSel] = (polyChAudioL[12] + polyChAudioL[13] + polyChAudioL[14] + polyChAudioL[15]);
			sumR[chSel] = (polyChAudioR[12] + polyChAudioR[13] + polyChAudioR[14] + polyChAudioR[15]);
		}
		//*** TODO: storing values
		//float lastValue = params[LEVEL_PARAM + 0].getValue();

		//apply 2^send---
		float aux1Ch1 = std::pow(params[AUX1_PARAM + 0].getValue(), 2.f), aux2Ch1 = std::pow(params[AUX2_PARAM + 0].getValue(), 2.f),
			aux1Ch2 = std::pow(params[AUX1_PARAM + 1].getValue(), 2.f), aux2Ch2 = std::pow(params[AUX2_PARAM + 1].getValue(), 2.f),
			aux1Ch3 = std::pow(params[AUX1_PARAM + 2].getValue(), 2.f), aux2Ch3 = std::pow(params[AUX2_PARAM + 2].getValue(), 2.f),
			aux1Ch4 = std::pow(params[AUX1_PARAM + 3].getValue(), 2.f), aux2Ch4 = std::pow(params[AUX2_PARAM + 3].getValue(), 2.f);
		
		float return1 = inputs[AUX1RETURN_INPUT].getVoltage(), return2 = inputs[AUX2RETURN_INPUT].getVoltage();
		//sumAux

		outputs[AUX1SEND_OUTPUT].setVoltage( (ch1Send1 * aux1Ch1) + (ch2Send1 * aux1Ch2) + (ch3Send1 * aux1Ch3) + (ch4Send1 * aux1Ch4) );
		outputs[AUX2SEND_OUTPUT].setVoltage( (ch1Send2 * aux2Ch1) + (ch2Send2 * aux2Ch2) + (ch3Send2 * aux2Ch3) + (ch4Send2 * aux2Ch4) );

		inputs[GAINLEVEL_INPUT].isConnected() == true ? cvMasterLevel = clamp(inputs[GAINLEVEL_INPUT].getVoltage() / 10, 0.f, 1.f) : cvMasterLevel;
		float outGain = params[MASTERGAIN_PARAM].getValue() * cvMasterLevel;
		float Left = sumL[chSel] * outGain + ((return1 + return2) / 2), Right = sumR[chSel] * outGain + ((return1 + return2) / 2);
		float testL = sumL[chSel] * outGain, testR = sumR[chSel] * outGain;

		outputs[OUTL_OUTPUT].setVoltage(Left);
		outputs[OUTR_OUTPUT].setVoltage(Right);




		//noise on output when inputs not connected
		if (!inputs[POLYAUDIO_INPUT].isConnected()) {
			outputs[OUTL_OUTPUT].setVoltage(0.f);
			outputs[OUTR_OUTPUT].setVoltage(0.f);
		}
	}//process

};

struct PolyMixWidget : ModuleWidget {
	PolyMixWidget(PolyMix *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BarkPolyMix.svg")));

		int rackY = 380;
		float chParamX[4] = { 14.44f, 47.42f, 82.68f, 115.54f }, chInvX[4] = { 11.53f, 44.37f, 79.74f, 112.6f },
			chFaderX[4] = { 29.13f, 62.f, 97.38f, 130.24f }, chBtnX[4] = { 13.99f, 46.63f, 82.f, 114.86f };	//mY 128.82f sY 111.32f

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
		addParam(createParam<BarkKnob40>(Vec(69.49f, rackY - 292.32f), module, PolyMix::MASTERGAIN_PARAM));
		addParam(createParam<BarkKnobSwitchSmall>(Vec(79.5f, rackY - 361.89f), module, PolyMix::CHSELECT_PARAM));
		//Repeated---
		for (int i = 0; i < 4; i++) {
			addParam(createParam<BarkKnob20>(Vec(chParamX[i], rackY - 226.23f), module, PolyMix::AUX1_PARAM + i));
			addParam(createParam<BarkKnob20>(Vec(chParamX[i], rackY - 197.21f), module, PolyMix::AUX2_PARAM + i));
			addParam(createParam<BarkKnob20>(Vec(chParamX[i], rackY - 160.41f), module, PolyMix::PAN_PARAM + i));
			addParam(createParam<BarkFaderLong>(Vec(chFaderX[i], rackY - 131.06f), module, PolyMix::LEVEL_PARAM + i));
			//Switch/Button---
			addParam(createParam<BarkSwitchSmall>(Vec(chInvX[i], rackY - 92.27f), module, PolyMix::INVERT_PARAM + i));
			addParam(createParam<BarkBtnMute>(Vec(chBtnX[i], rackY - 128.82f), module, PolyMix::MUTE_PARAM + i));
			addParam(createParam<BarkBtnSolo>(Vec(chBtnX[i], rackY - 111.32f), module, PolyMix::SOLO_PARAM + i));

		}
		//Screws---
		addChild(createWidget<BarkScrew4>(Vec(3, 3)));							//pos1
		//addChild(createWidget<BarkScrew3>(Vec(box.size.x - 13, 3)));			//pos2
		//addChild(createWidget<BarkScrew1>(Vec(3, 367.2f)));					//pos3
		addChild(createWidget<BarkScrew1>(Vec(box.size.x - 13, 367.2)));			//pos4
		//Light---

	}

};

Model *modelPolyMix = createModel<PolyMix, PolyMixWidget>("PolyMix");