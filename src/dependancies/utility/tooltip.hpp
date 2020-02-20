#include "plugin.hpp"

#define BLANK "\n"

//General-----------------------
struct tpOnOff : ParamQuantity {
	std::string getDisplayValueString() override {
		if (getValue() < 1.f)
			return "On";
		else
			return "Off";
	}
};

struct tpOnOffBtn : ParamQuantity {
	std::string getDisplayValueString() override {
		if (getValue() > 0.f)
			return "On";
		else
			return "Off";
	}
};
struct tpCycled : ParamQuantity {
	std::string getDisplayValueString() override {
		if (getValue() < 1.f)
			return "Cycle";
		else
			return "Cycled";
	}
};
struct tpDone : ParamQuantity {
	std::string getDisplayValueString() override {
		if (getValue() < 1.f)
			return BLANK;
		else
			return "\nDone ";
	}
};
struct tpIntExt : ParamQuantity {
	std::string getDisplayValueString() override {
		if (getValue() > 0.f)
			return "\nInternal";
		else
			return "\nExternal";
	}
};
struct tpGainVal : ParamQuantity {
	std::string getDisplayValueString() override {
		if (getValue() < 1.f)
			return "Pre";
		else
			return "Post";
	}
};

struct tpPhaseVal : ParamQuantity {
	std::string getDisplayValueString() override {
		if (getValue() > 0.f)
			return "0";
		else
			return "180";
	}
};

//module specific
///OneBand--------------------------
struct tpEQprocess : ParamQuantity {
	std::string getDisplayValueString() override {
		if (getValue() > 0.f)
			return "Listen";
		else
			return "Process";
	}
};

struct tpSwapLR : ParamQuantity {
	std::string getDisplayValueString() override {
		if (getValue() < 1.f)
			return "L/R";
		else
			return "R/L";
	}
};

///Poly Mix-----------------------------
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

///Poly X-------------------------
struct tpMute10v : ParamQuantity {
	std::string getDisplayValueString() override {
		if (getValue() < 1.f)
			return "Open";
		else
			return "Mute";
	}
};

///TrimLFO
struct tpPolarVal : ParamQuantity {
	std::string getDisplayValueString() override {
		if (getValue() > 0.f)
			return "Uni-";
		else
			return "Bi-";
	}
};

struct tpWave : ParamQuantity {
	std::string getDisplayValueString() override {
		if (getValue() >= 0.f && getValue() <= 0.001f) { 
			return "Sine"; } 
		else if (getValue() >= 1.f && getValue() <= 1.001f) { 
			return "Saw"; } 
		else if (getValue() >= 2.f && getValue() <= 2.001f) { 
			return "Triangle"; } 
		else if (getValue() > 2.998f) { 
			return "Square"; } 
		else return  std::to_string(getValue());
	}
};

///Clamp--------------------------
struct tpCeiling : ParamQuantity {
	std::string getDisplayValueString() override {
		if (getValue() < 1.f)
			return "Off";
		else
			return "-0.1dB";
	}
};

///SHTH---------------------------
struct tpMode_sh : ParamQuantity {
	std::string getDisplayValueString() override {
		if (getValue() < 1.f)
			return "Sample & Hold";
		else
			return "Track & Hold";
	}
};

struct tpMode_th : ParamQuantity {
	std::string getDisplayValueString() override {
		if (getValue() < 1.f)
			return "Track & Hold";
		else
			return "Sample & Hold";
	}
};

struct tpPlusMinus : ParamQuantity {
	std::string getDisplayValueString() override {
		if (getValue() > 0.f)
			return "+/-";
		else
			return "+";
	}
};

/*-------------------------------------------------------------------------------------------*/
/*-----------------------------------       MACROS        -----------------------------------*/
/*-------------------------------------------------------------------------------------------*/

///DEBUG()---------------------------
inline const char * const BoolToString(bool b) {
	return b ? "True" : "False";
}
