#pragma once
#include "rack.hpp"


namespace barkComponents {		//		componentlibrary
	///Colour--------------------------------------------------
	static const NVGcolor BARK_GREEN = nvgRGBA(73, 191, 0, 255);
	static const NVGcolor BARK_YELLOW1 = nvgRGBA(255, 212, 42, 255);
	static const NVGcolor BARK_YELLOW2 = nvgRGBA(255, 192, 42, 255);
	static const NVGcolor BARK_ORANGE = nvgRGBA(250, 123, 0, 255);
	static const NVGcolor BARK_RED = nvgRGBA(186, 15, 0, 255);
	static const NVGcolor BARK_CLIPPING = nvgRGBA(240, 255, 255, 255);//white
	///Colour--------------------------------------------------

	////Screw----
	struct BarkScrew1 : app::SvgScrew {
		BarkScrew1() {
			sw->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkScrew1.svg")));
			//sw->wrap();
			box.size = sw->box.size;
		}
	};

	struct BarkScrew2 : app::SvgScrew {
		BarkScrew2() {
			sw->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkScrew2.svg")));
			//sw->wrap();
			box.size = sw->box.size;
		}
	};

	struct BarkScrew3 : app::SvgScrew {
		BarkScrew3() {
			sw->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkScrew3.svg")));
			//sw->wrap();
			box.size = sw->box.size;
		}
	};

	struct BarkScrew4 : app::SvgScrew {
		BarkScrew4() {
			sw->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkScrew4.svg")));
			//sw->wrap();
			box.size = sw->box.size;
		}
	};

	struct BarkScrew01 : app::SvgKnob {
		BarkScrew01() {
			minAngle = -6.99 * M_PI;
			maxAngle = 6.99 * M_PI;
			//sw->
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkScrew01.svg")));
			sw->wrap();
			box.size = sw->box.size;
			speed = 0.18f;
		}
		void randomize() override {}
	};

	struct BarkScrew02 : app::SvgKnob {
		BarkScrew02() {
			minAngle = -2.0 * M_PI;
			maxAngle = 2.0 * M_PI;
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkScrew01.svg")));
			sw->wrap();
			box.size = sw->box.size;
			speed = 0.5f;
		}
		void randomize() override {}
	};

	////Switch----
	struct BarkSwitch : app::SvgSwitch {
		BarkSwitch() {
			addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkSwitch_0.svg")));
			addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkSwitch_1.svg")));
		}
	};

	struct BarkSwitchSmall : app::SvgSwitch {
		BarkSwitchSmall() {
			addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkSwitchSmall_0.svg")));
			addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkSwitchSmall_1.svg")));
		}
	};

	struct BarkSwitchSmallSide : app::SvgSwitch {
		BarkSwitchSmallSide() {
			addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkSwitchSmallSide_0.svg")));
			addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkSwitchSmallSide_1.svg")));
		}
	};

	struct BarkButton1 : app::SvgSwitch {
		BarkButton1() {
			momentary = true;
			addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkButtonReset_0.svg")));
			//addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkButtonReset_0.svg")));
		}
	};
	
	struct BarkPushButton1 : app::SvgSwitch {
		BarkPushButton1() {
			momentary = true;
			addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkButtonRound1_0.svg")));
		}
	};

	//struct BarkButtonMinus : app::SvgSwitch {
	//	BarkButtonMinus() {
	//		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkButtonMinus.svg")));
	//		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkButtonMinus_0.svg")));
	//	}
	//};

	//struct BarkButtonPlus : app::SvgSwitch {	//MomentarySwitch
	//	BarkButtonPlus() {
	//		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkButtonPlus.svg")));
	//		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkButtonPlus_0.svg")));
	//	}
	//};

	//struct BarkBtnLockSnap : app::SvgSwitch {
	//	BarkBtnLockSnap() {
	//		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkButtonLock.svg")));
	//		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkButtonUnlock.svg")));
	//	}
	//};

	////Slider----
	struct BarkSlide1 : app::SvgSlider {
		BarkSlide1() {
			///TODO: toggle for snap or fade or momentary button to snap to nearest
			math::Vec position = math::Vec(0.f, 0.f);
			maxHandlePos = math::Vec(95.f, 0.0f).plus(position);
			minHandlePos = math::Vec(-5.0f, 0.0f).plus(position);
			setBackgroundSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Barkslider1.svg")));
			background->wrap();
			background->box.pos = position;
			box.size = background->box.size.plus(position.mult(2));
			//box.size = background->box.size;
			setHandleSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarksliderHandle1.svg")));
			handle->wrap();
			handle->box.pos = position;
			speed = 0.5f;
			horizontal = true;
		}
		///flips up/down axis to left/right
		/*void onDragMove(EventDragMove &e) override {
			EventDragMove e2 = e;
			e2.mouseRel = Vec(e.mouseRel.y, -e.mouseRel.x);
			SVGFader::onDragMove(e2);
		}*/
		///turns off randomising
		/*void randomize() override {}*/
	};

	////Ports----
	///Port In--
	struct BarkInPort : app::SvgPort {
		BarkInPort() {
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkInPort.svg")));
		}
	};

	struct BarkInPort1 : app::SvgPort {
		BarkInPort1() {
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkInPort1.svg")));
		}
	};

	struct BarkInPort2 : app::SvgPort {
		BarkInPort2() {
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkInPort2.svg")));
		}
	};

	struct BarkPatchPortIn : app::SvgPort {
		BarkPatchPortIn() {
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkPatchPortIn.svg")));
		}
	};

	struct BarkInPort350 : app::SvgPort {
		BarkInPort350() {
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkInPort350.svg")));
			//blurRadius = 1.2f;
			//void draw(const DrawArgs &args) override; {
			//}
		}
	};
	///Port Out--
	struct BarkOutPort : app::SvgPort {
		BarkOutPort() {
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkOutPort.svg")));
		}
	};

	struct BarkOutPort1 : app::SvgPort {
		BarkOutPort1() {
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkOutPort1.svg")));
		}
	};

	struct BarkOutPort2 : app::SvgPort {
		BarkOutPort2() {
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkOutPort2.svg")));
		}
	};

	struct BarkPatchPortOut : app::SvgPort {
		BarkPatchPortOut() {
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkPatchPortOut.svg")));
		}
	};

	struct BarkOutPort350 : app::SvgPort {
		BarkOutPort350() {
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkOutPort350.svg")));
		}
	};

	////Knobs----
	struct BarkKnobSwitch : app::SvgKnob {
		BarkKnobSwitch() {
			snap = true;
			shadow->opacity = 0.f;
			minAngle = -0.f * M_PI;
			maxAngle = 0.58f * M_PI;
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkKnobSwitch.svg")));
		}
	};

	struct BarkKnob9 : app::SvgKnob {
		BarkKnob9() {
			minAngle = -0.835 * M_PI;
			maxAngle = 0.831 * M_PI;
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkKnob9.svg")));
			sw->wrap();
			box.size = sw->box.size;
		}
	};

	struct BarkKnob24 : app::SvgKnob {
		BarkKnob24() {
			minAngle = -0.835 * M_PI;
			maxAngle = 0.831 * M_PI;
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkKnob24.svg")));
			sw->wrap();
			box.size = sw->box.size;
		}
	};

	struct BarkKnob26 : app::SvgKnob {
		BarkKnob26() {
			minAngle = -0.829 * M_PI;
			maxAngle = 0.831 * M_PI;
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkKnob26.svg")));
			sw->wrap();
			box.size = sw->box.size;
			speed = 0.65f;
		}
	};

	struct BarkKnob30a : app::SvgKnob {
		BarkKnob30a() {
			minAngle = -0.558 * M_PI;
			maxAngle = 0.831 * M_PI;
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkKnob30.svg")));
			sw->wrap();
			box.size = sw->box.size;
			speed = 0.7f;
		}
	};
	
	struct BarkKnob30b : app::SvgKnob {
		BarkKnob30b() {
			minAngle = -0.835 * M_PI;
			maxAngle = 0.831 * M_PI;
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkKnob30.svg")));
			sw->wrap();
			box.size = sw->box.size;
			speed = 0.7f;
		}
	};

	struct BarkKnob40 : app::SvgKnob {
		BarkKnob40() {
			minAngle = -0.827 * M_PI;
			maxAngle = 0.825 * M_PI;
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkKnob40.svg")));
			sw->wrap();
			box.size = sw->box.size;
			speed = 0.8f;
			shadow->box.pos = Vec(0, sw->box.size.y * 0.07f);
		}
	};

	struct BarkKnob57 : app::SvgKnob {
		BarkKnob57() {
			minAngle = -0.835 * M_PI;
			maxAngle = 0.831 * M_PI;
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkKnob57.svg")));
			sw->wrap();
			box.size = sw->box.size;
		}
	};

	struct BarkKnob70 : app::SvgKnob {
		BarkKnob70() {
			minAngle = -0.83 * M_PI;
			maxAngle = 0.828 * M_PI;
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkKnob70.svg")));
			sw->wrap();
			box.size = sw->box.size;
			speed = 0.5f;
			shadow->box.pos = Vec(0, sw->box.size.y * 0.05f);
		}
	};

	struct BarkKnob70Snap : app::SvgKnob {
		BarkKnob70Snap() {	//bpmTrimLFO, 
			snap = true;
			minAngle = -0.83 * M_PI;
			maxAngle = 0.828 * M_PI;
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkKnob70.svg")));
			sw->wrap();
			box.size = sw->box.size;
			shadow->box.pos = Vec(0, sw->box.size.y * 0.05);
		}
	};

	struct BarkKnob84 : app::SvgKnob {
		BarkKnob84() {
			minAngle = -0.835 * M_PI;
			maxAngle = 0.831 * M_PI;
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkKnob84.svg")));
			sw->wrap();
			box.size = sw->box.size;
		}
	};

	struct BarkKnob92 : app::SvgKnob {
		BarkKnob92() {
			minAngle = -0.83 * M_PI;
			maxAngle = 0.83 * M_PI;
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkKnob92.svg")));
			sw->wrap();
			box.size = sw->box.size;
		}
	};

	struct KnobTest1 : app::SvgKnob {
		KnobTest1() {
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/KnobTest1.svg")));

		}
	};

	///Light----
	struct greenLight : GrayModuleLightWidget {
		greenLight() {
			addBaseColor(BARK_GREEN);
		}
	};

	struct yellowLight1 : GrayModuleLightWidget {
		yellowLight1() {
			addBaseColor(BARK_YELLOW1);
		}
	};

	struct yellowLight2 : GrayModuleLightWidget {
		yellowLight2() {
			addBaseColor(BARK_YELLOW2);
		}
	};

	struct orangeLight : GrayModuleLightWidget {
		orangeLight() {
			addBaseColor(BARK_ORANGE);
		}
	};

	struct redLight : GrayModuleLightWidget {
		redLight() {
			addBaseColor(BARK_RED);
		}
	};

	struct clipLight : GrayModuleLightWidget {
		clipLight() {
			addBaseColor(BARK_CLIPPING);
		}
	};

	struct ParamInLight : GrayModuleLightWidget {
		ParamInLight() {
			addBaseColor(BARK_CLIPPING);
		}
	};

	template <typename BASE>
	struct BiggerLight : BASE {
		BiggerLight() {
			this->box.size = Vec(10, 10);//px
			this->bgColor = nvgRGBA(192, 192, 192, 32);//silver
		}
	};

	template <typename BASE>
	struct BigLight : BASE {
		BigLight() {
			this->box.size = Vec(8, 8);//px
			this->bgColor = nvgRGBA(192, 192, 192, 32);//silver
		}
	};

	template <typename BASE>
	struct SmallerLight : BASE {
		SmallerLight() {
			this->box.size = Vec(4, 4);//px
			this->bgColor = nvgRGBA(192, 192, 192, 45);//silver
		}
	};

	template <typename BASE>
	struct SmallerLightFA : BASE {
		SmallerLightFA() {
			this->box.size = Vec(4, 4);//px
			this->bgColor = nvgRGBA(56, 56, 56, 255);//panel
			this->borderColor = nvgRGBA(56, 56, 56, 255);//panel
		}
	};

	template <typename BASE>
	struct SmallestLight : BASE {
		SmallestLight() {
			this->box.size = Vec(3, 3);//px
			this->bgColor = nvgRGBA(192, 192, 192, 45);//silver
		}
	};
}//namespace barkComponents

