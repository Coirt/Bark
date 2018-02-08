#include "rack.hpp"


using namespace rack;

//Colour	
//	const NVGcolor BARK_ = nvgRGBA(0, 0, 0, 255);		//Red, Green, Blue, Alpha = Transparency
const NVGcolor BARK_GREEN1 = nvgRGBA(34, 124, 34, 255);		
const NVGcolor BARK_GREEN2 = nvgRGBA(66, 66, 36, 255);		
const NVGcolor BARK_GREEN3 = nvgRGBA(75, 83, 32, 255);		
const NVGcolor BARK_GREEN4 = nvgRGBA(73, 191, 0, 255);		//#1
const NVGcolor BARK_YELLOW = nvgRGBA(255, 220, 0, 255);		
const NVGcolor BARK_ORANGE1 = nvgRGBA(255, 150, 0, 255);		//#1
const NVGcolor BARK_ORANGE2 = nvgRGBA(255, 170, 0, 255);		
const NVGcolor BARK_ORANGE3 = nvgRGBA(243, 123, 0, 255);		
const NVGcolor BARK_RED1 = nvgRGBA(179, 15, 0, 255);			
const NVGcolor BARK_RED2 = nvgRGBA(186, 15, 0, 255);			//#1	
const NVGcolor BARK_RED3 = nvgRGBA(204, 15, 0, 255);			



extern Plugin *plugin;

/////////////////
// module widgets	//The name of your module
/////////////////
//	struct NAMEHERE : ModuleWidget {
//	NAMEHEREWidget();
//	};
////////////////////

//struct TrimLFOWidget : ModuleWidget {
//	TrimLFOWidget();
//};

struct GainWidget : ModuleWidget {
	GainWidget();
};

struct ClipperWidget : ModuleWidget {
	ClipperWidget();
};

struct Panel13Widget : ModuleWidget {
	Panel13Widget();
};

struct Panel10Widget : ModuleWidget {
	Panel10Widget();
};

struct Panel6Widget : ModuleWidget {
	Panel6Widget();
};

//struct FloatsPanelWidget : ModuleWidget {
//	FloatsPanelWidget();
//};


/////////////////////////
//Module custom resources
/////////////////////////
//struct FILENAME : SVGScrew {
//	FILENAME() {
//		sw->svg = SVG::load(assetPlugin(plugin, "FILELOCATION"));	//Location will be in res/  or res/FOLDER
//		sw->wrap();
//		box.size = sw->box.size;
//	}
//};
//			"res/components/		NAME.svg"

////Screw
struct BarkScrew1 : SVGScrew {
	BarkScrew1() {
		sw->svg = SVG::load(assetPlugin(plugin, "res/components/BarkScrew1.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct BarkScrew2 : SVGScrew {
	BarkScrew2() {
		sw->svg = SVG::load(assetPlugin(plugin, "res/components/BarkScrew2.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct BarkScrew3 : SVGScrew {
	BarkScrew3() {
		sw->svg = SVG::load(assetPlugin(plugin, "res/components/BarkScrew3.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct BarkScrew4 : SVGScrew {
	BarkScrew4() {
		sw->svg = SVG::load(assetPlugin(plugin, "res/components/BarkScrew4.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};
////Port----
	//Port In
struct BarkInPort : SVGPort {
	BarkInPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/components/BarkInPort.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct BarkInPort1 : SVGPort {
	BarkInPort1() {
		background->svg = SVG::load(assetPlugin(plugin, "res/components/BarkInPort1.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct BarkInPort2 : SVGPort {
	BarkInPort2() {
		background->svg = SVG::load(assetPlugin(plugin, "res/components/BarkInPort2.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct BarkPatchPortIn : SVGPort{
	BarkPatchPortIn() {
	background->svg = SVG::load(assetPlugin(plugin, "res/components/BarkPatchPortIn.svg"));
	background->wrap();
	box.size = background->box.size;
}
};

struct BarkInPort350 : SVGPort {
	BarkInPort350() {
		background->svg = SVG::load(assetPlugin(plugin, "res/components/BarkInPort350.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};


	//Port Out
struct BarkOutPort : SVGPort {
	BarkOutPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/components/BarkOutPort.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct BarkOutPort1 : SVGPort {
	BarkOutPort1() {
		background->svg = SVG::load(assetPlugin(plugin, "res/components/BarkOutPort1.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct BarkOutPort2 : SVGPort {
	BarkOutPort2() {
		background->svg = SVG::load(assetPlugin(plugin, "res/components/BarkOutPort2.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct BarkPatchPortOut : SVGPort {
	BarkPatchPortOut() {
		background->svg = SVG::load(assetPlugin(plugin, "res/components/BarkPatchPortOut.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct BarkOutPort350 : SVGPort {
	BarkOutPort350() {
		background->svg = SVG::load(assetPlugin(plugin, "res/components/BarkOutPort350.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

////Knobs

struct BarkKnob24 : SVGKnob {
	BarkKnob24() {
		minAngle = -0.835 * M_PI;
		maxAngle = 0.831 * M_PI;
		sw->svg = SVG::load(assetPlugin(plugin, "res/components/BarkKnob24.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct BarkKnob30 : SVGKnob {
	BarkKnob30() {
		minAngle = -0.835 * M_PI;
		maxAngle = 0.831 * M_PI;
		sw->svg = SVG::load(assetPlugin(plugin, "res/components/BarkKnob30.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct BarkKnob57 : SVGKnob {
	BarkKnob57() {
		minAngle = -0.835 * M_PI;
		maxAngle = 0.831 * M_PI;
		sw->svg = SVG::load(assetPlugin(plugin, "res/components/BarkKnob57.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct BarkKnob70 : SVGKnob {
	BarkKnob70() {
		minAngle = -0.835 * M_PI;
		maxAngle = 0.831 * M_PI;
		sw->svg = SVG::load(assetPlugin(plugin, "res/components/BarkKnob70.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct BarkKnob84 : SVGKnob {
	BarkKnob84() {
		minAngle = -0.835 * M_PI;
		maxAngle = 0.831 * M_PI;
		sw->svg = SVG::load(assetPlugin(plugin, "res/components/BarkKnob84.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct BarkKnob92 : SVGKnob {
	BarkKnob92() {
		minAngle = -0.835 * M_PI;
		maxAngle = 0.831 * M_PI;
		sw->svg = SVG::load(assetPlugin(plugin, "res/components/BarkKnob92.svg")); //90.25px
		sw->wrap();
		box.size = sw->box.size;
	}
};