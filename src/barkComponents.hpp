#pragma once
# include "componentlibrary.hpp"
#include <vector>
#include <jansson.h>
#include "widgets.hpp"
#include <iostream>

using namespace std;

////Colour	
////	const NVGcolor BARK_ = nvgRGBA(0, 0, 0, 255);		//Red, Green, Blue, Alpha = Transparency
//const NVGcolor BARK_GREEN1 = nvgRGBA(34, 124, 34, 255);
//const NVGcolor BARK_GREEN2 = nvgRGBA(66, 66, 36, 255);
//const NVGcolor BARK_GREEN3 = nvgRGBA(75, 83, 32, 255);
//const NVGcolor BARK_GREEN4 = nvgRGBA(73, 191, 0, 255);		//#1
//const NVGcolor BARK_YELLOW = nvgRGBA(255, 220, 0, 255);
//const NVGcolor BARK_ORANGE1 = nvgRGBA(255, 150, 0, 255);		//#1
//const NVGcolor BARK_ORANGE2 = nvgRGBA(255, 170, 0, 255);
//const NVGcolor BARK_ORANGE3 = nvgRGBA(243, 123, 0, 255);
//const NVGcolor BARK_RED1 = nvgRGBA(179, 15, 0, 255);
//const NVGcolor BARK_RED2 = nvgRGBA(186, 15, 0, 255);			//#1	
//const NVGcolor BARK_RED3 = nvgRGBA(204, 15, 0, 255);


	////////////////////////
	//Module custom resources #ref
	/////////////////////////
	//struct FILENAME : SVGScrew {
	//	FILENAME() {
	//		setSVG(SVG::load(assetPlugin(plugin, "FILELOCATION"));	//Location will be in res/  or res/FOLDER
	//		sw->wrap();
	//		box.size = sw->box.size;		//box.size = Vec(r1, r2);
	//	}
	//};
	//			"res/components/		NAME.svg"

namespace rack {
	//Screw----
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

	struct BarkScrew01 : SVGKnob {
		BarkScrew01() {
			minAngle = -6.99 * M_PI;
			maxAngle = 6.99 * M_PI;
			setSVG(SVG::load(assetPlugin(plugin, "res/components/BarkScrew01.svg")));
			sw->wrap();
			box.size = sw->box.size;
			speed = 0.2;
		}
		void randomize() override {}
	};

	////ref Toggle Switch------------------------------------
	//	struct FILENAME : SVGSwitch, ToggleSwitch {
	//		FILENAME() {
	//	addFrame(SVG::load(assetPlugin(plugin, "res/components/FILENAME.svg")));	//	State=0
	//	addFrame(SVG::load(assetPlugin(plugin, "res/components/FILENAME.svg")));	//	State=1
	//}
	//};
	//------------------------------------------------------

	//Toggle----
	struct BarkSwitch : SVGSwitch, ToggleSwitch {
		BarkSwitch() {
			addFrame(SVG::load(assetPlugin(plugin, "res/components/BarkSwitch_0.svg")));			//	State=0
			addFrame(SVG::load(assetPlugin(plugin, "res/components/BarkSwitch_1.svg")));			//	State=1
																									//sw->wrap();
																									//box.size = sw->box.size;
		}
	};

	////Slider----
	struct BarkSlide1 : SVGFader {
		BarkSlide1() {
			///TODO: switch for snap or fade || momentary button to snap to nearest
			snap = false;
			maxHandlePos = Vec(95.f, 0.0f);
			minHandlePos = Vec(-5.0f, 0.0f);
			background->svg = SVG::load(assetPlugin(plugin, "res/components/Barkslider1.svg"));
			background->wrap();
			background->box.pos = Vec(0.0f, 0.0f);
			box.size = background->box.size;
			handle->svg = SVG::load(assetPlugin(plugin, "res/components/BarksliderHandle1.svg"));
			handle->wrap();
			handle->box.pos = Vec(0.0f, 0.0f);
			speed = 0.5f;
		}
		//flip up/down axis to left/right
		void onDragMove(EventDragMove &e) override {
			EventDragMove e2 = e;
			e2.mouseRel = Vec(e.mouseRel.y, -e.mouseRel.x);
			SVGFader::onDragMove(e2);
		}
		//turn off randomising
		void randomize() override {}	
	};

	////Ports----
	///Port In--
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

	struct BarkPatchPortIn : SVGPort {
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
	///Port Out--
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
	//TODO: Fix big knob low/high PI

	/*
	v0.6.0 build 0.0.4
	void SVGKnob::setSVG(std::shared_ptr<SVG> svg) {
	sw->setSVG(svg);
	tw->box.size = sw->box.size;
	box.size = sw->box.size;
	shadow->box.size = sw->box.size;
	shadow->box.pos = Vec(0, sw->box.size.y * 0.1);
	// shadow->box = shadow->box.grow(Vec(2, 2));
	}	*/

	struct BarkKnob9 : SVGKnob {
		BarkKnob9() {
			minAngle = -0.835 * M_PI;
			maxAngle = 0.831 * M_PI;
			setSVG(SVG::load(assetPlugin(plugin, "res/components/BarkKnob9.svg")));
			sw->wrap();
			box.size = sw->box.size;
		}
	};

	struct BarkKnob24 : SVGKnob {
		BarkKnob24() {
			minAngle = -0.835 * M_PI;
			maxAngle = 0.831 * M_PI;
			setSVG(SVG::load(assetPlugin(plugin, "res/components/BarkKnob24.svg")));
			sw->wrap();
			box.size = sw->box.size;
		}
	};

	struct BarkKnob26 : SVGKnob {
		BarkKnob26() {
			minAngle = -0.829 * M_PI;
			maxAngle = 0.831 * M_PI;
			setSVG(SVG::load(assetPlugin(plugin, "res/components/BarkKnob26.svg")));
			sw->wrap();
			box.size = sw->box.size;
		}
	};

	struct BarkKnob30 : SVGKnob {
		BarkKnob30() {
			minAngle = -0.835 * M_PI;
			maxAngle = 0.831 * M_PI;
			setSVG(SVG::load(assetPlugin(plugin, "res/components/BarkKnob30.svg")));
			sw->wrap();
			box.size = sw->box.size;
		}
	};

	struct BarkKnob40 : SVGKnob {
		BarkKnob40() {
			minAngle = -0.827 * M_PI;
			maxAngle = 0.825 * M_PI;
			setSVG(SVG::load(assetPlugin(plugin, "res/components/BarkKnob40.svg")));
			sw->wrap();
			box.size = sw->box.size;
		}
	};

	struct BarkKnob57 : SVGKnob {
		BarkKnob57() {
			minAngle = -0.835 * M_PI;
			maxAngle = 0.831 * M_PI;
			setSVG(SVG::load(assetPlugin(plugin, "res/components/BarkKnob57.svg")));
			sw->wrap();
			box.size = sw->box.size;
		}
	};

	struct BarkKnob70 : SVGKnob {
		BarkKnob70() {
			minAngle = -0.83 * M_PI;
			maxAngle = 0.828 * M_PI;
			setSVG(SVG::load(assetPlugin(plugin, "res/components/BarkKnob70.svg")));
			sw->wrap();
			box.size = sw->box.size;
			speed = 0.50;
		}
	};

	struct BarkKnob84 : SVGKnob {
		BarkKnob84() {
			minAngle = -0.835 * M_PI;
			maxAngle = 0.831 * M_PI;
			setSVG(SVG::load(assetPlugin(plugin, "res/components/BarkKnob84.svg")));
			sw->wrap();
			box.size = sw->box.size;
		}
	};

	struct BarkKnob92 : SVGKnob {
		BarkKnob92() {
			minAngle = -0.83 * M_PI;
			maxAngle = 0.83 * M_PI;
			setSVG(SVG::load(assetPlugin(plugin, "res/components/BarkKnob92.svg"))); //90.25px
			sw->wrap();
			box.size = sw->box.size;
		}
	};

	struct KnobTest1 : SVGKnob {
		KnobTest1() {
			setSVG(SVG::load(assetPlugin(plugin, "res/components/KnobTest1.svg"))); //90.25px
			
		}
	};

	struct BarkSinActive : SVGSwitch, ToggleSwitch {
		BarkSinActive() {
			addFrame(SVG::load(assetPlugin(plugin, "res/components/BarkBtnSin!Active.svg")));			//	State=0
			addFrame(SVG::load(assetPlugin(plugin, "res/components/BarkBtnSinActive.svg")));			//	State=1
		}

	};
}
