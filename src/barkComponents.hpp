#pragma once
#include "rack.hpp"
#include "dependancies/utility/tooltip.hpp"

///VS2019 test

namespace barkComponents {
	///Colour--------------------------------------------------
	static const NVGcolor BARK_GREEN = nvgRGBA(73, 191, 0, 255);
	static const NVGcolor BARK_YELLOW1 = nvgRGBA(255, 212, 42, 255);
	static const NVGcolor BARK_YELLOW2 = nvgRGBA(255, 192, 42, 255);
	static const NVGcolor BARK_ORANGE = nvgRGBA(250, 123, 0, 255);
	static const NVGcolor BARK_RED = nvgRGBA(186, 15, 0, 255);
	static const NVGcolor BARK_CLIPPING = nvgRGBA(240, 255, 255, 255);	//white
	static const NVGcolor VCVPOLY_BLUE = nvgRGBA(41, 178, 239, 255);
	static const NVGcolor BARK_FAKEPOLY = nvgRGBA(0, 128, 0, 255);
	static const NVGcolor BARK_MUTEPOLY = BARK_RED;
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

	struct BarkSwitchSmallSide2 : app::SvgSwitch {
		BarkSwitchSmallSide2() {
			addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkSwitchSmallSide_3.svg")));
			addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkSwitchSmallSide_4.svg")));
			shadow->opacity = 0.f;
		}
	};

	struct BarkButton1 : app::SvgSwitch {
		BarkButton1() {
			momentary = true;
			addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkButtonReset_0.svg")));
			//addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkButtonReset_0.svg")));
			shadow->opacity = 0.f;
		}
	};

	struct BarkBtnMute : app::SvgSwitch {
		BarkBtnMute() {
			addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkBTNMuteDown.svg")));
			addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkBTNMuteUp.svg")));
		}
	};
	
	struct BarkBtnSolo : app::SvgSwitch {
		BarkBtnSolo() {
			addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkBTNSoloDown.svg")));
			addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkBTNSoloUp.svg")));
		}
	};
	
	struct BarkPushButton1 : app::SvgSwitch {
		BarkPushButton1() {
			momentary = true;
			addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkButtonRound1_0.svg")));
			addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkButtonRound1_1.svg")));
		}
	};

	struct BarkPushButton2 : app::SvgSwitch {
		BarkPushButton2() {
			momentary = false;
			addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkButtonRound1_0.svg")));
			addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkButtonRound1_1.svg")));
		}
	};

	struct BarkPushButton3 : app::SvgSwitch {
		BarkPushButton3() {
			momentary = false;
			addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkButtonRound1_3.svg")));
			addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkButtonRound1_4.svg")));
		}
	};

	struct BarkPushButtonBig1 : app::SvgSwitch {
		BarkPushButtonBig1() {
			momentary = true;
			addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkButtonRoundBig_0.svg")));
			addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkButtonRoundBig_1.svg")));
		}
	};

	////Slider----
	struct BarkSlide1 : app::SvgSlider {
		BarkSlide1() {
			math::Vec position = math::Vec(0.f, 0.f);
			maxHandlePos = math::Vec(95.f, 0.f).plus(position);
			minHandlePos = math::Vec(-5.f, 0.f).plus(position);
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
	};

	struct BarkFaderLong : app::SvgSlider {
		BarkFaderLong() {
			minHandlePos = math::Vec(0, 95);
			maxHandlePos = math::Vec(0, 3);
			setBackgroundSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkSliderFaderLong.svg")));
			setHandleSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkSliderHandleFaderLong.svg")));
			speed = 0.8f;
		}
	};
	////Ports----
	///Port In--
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

	struct BarkKnobSwitchSmall : app::SvgKnob {
		BarkKnobSwitchSmall() {
			snap = true;
			shadow->opacity = 0.f;
			minAngle = -0.f * M_PI;
			maxAngle = 0.25f * M_PI;	//0.29
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkKnobSwitchSmall.svg")));
		}
	};

	struct BarkKnob_20 : app::SvgKnob {
		BarkKnob_20() {
			minAngle = -0.835 * M_PI;
			maxAngle = 0.831 * M_PI;
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkKnob_20.svg")));
			sw->wrap();
			box.size = sw->box.size;
			speed = 0.65f;
		}
	};

	struct BarkKnob_26 : app::SvgKnob {
		BarkKnob_26() {
			minAngle = -0.829 * M_PI;
			maxAngle = 0.831 * M_PI;
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkKnob_26.svg")));
			sw->wrap();
			box.size = sw->box.size;
			speed = 0.65f;
		}
	};

	struct BarkKnob_30 : app::SvgKnob {
		BarkKnob_30() {
			minAngle = -0.835 * M_PI;
			maxAngle = 0.831 * M_PI;
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkKnob_30.svg")));
			sw->wrap();
			box.size = sw->box.size;
			speed = 0.7f;
			shadow->box.pos = Vec(0, sw->box.size.y * 0.09f);
		}
	};

	struct BarkKnob_40 : app::SvgKnob {
		BarkKnob_40() {
			minAngle = -0.827 * M_PI;
			maxAngle = 0.825 * M_PI;
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkKnob_40.svg")));
			sw->wrap();
			box.size = sw->box.size;
			speed = 0.8f;
			shadow->box.pos = Vec(0, sw->box.size.y * 0.07f);
		}
	};

	struct BarkKnob_60 : app::SvgKnob {
		BarkKnob_60() {
			minAngle = -0.83 * M_PI;
			maxAngle = 0.828 * M_PI;
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/BarkKnob_60.svg")));
			sw->wrap();
			box.size = sw->box.size;
			speed = 0.5f;
			shadow->box.pos = Vec(0, sw->box.size.y * 0.05f);
		}
	};

	struct BarkKnob_60snap : BarkKnob_60 {
		BarkKnob_60snap() {
			snap = true;
		}
	};

	///Light----
	struct greenRedLight : GrayModuleLightWidget {
		greenRedLight() {
			addBaseColor(BARK_GREEN);
			addBaseColor(BARK_RED);
		}
	};

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
	
	struct PolyLight : GrayModuleLightWidget {
		PolyLight() {
			addBaseColor(VCVPOLY_BLUE);
			//addBaseColor(BARK_MUTEPOLY);
		}
	};
	
	struct PolyMute : GrayModuleLightWidget {
		PolyMute() {
			addBaseColor(BARK_MUTEPOLY);
		}
	};
	
	struct PolyFake : GrayModuleLightWidget {
		PolyFake() {
			addBaseColor(BARK_FAKEPOLY);
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
	struct LessBigLight : BASE {
		LessBigLight() {
			this->box.size = Vec(6.5f, 6.5f);//px
			this->bgColor = nvgRGBA(192, 192, 192, 16);//silver
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
			this->borderColor = nvgRGBA(56, 56, 56, 45);//panel
		}
	};
	
	template <typename BASE>
	struct SmallestLightInverse : BASE {
		SmallestLightInverse() {
			this->box.size = Vec(3, 3);//px
			this->bgColor = nvgRGBA(56, 56, 56, 128);//panel
			this->borderColor = nvgRGBA(30, 31, 0, 45);//black
		}
	};

	//Common co-ordinates
	///openGL / inkscape_v0.92.x co-ordinates negotiation
	static constexpr int rackY = 380;
	///VU Lights: OneBand, EOSum
	static constexpr float lightY[8] = {232.548f, 233.548f, 246.099f, 257.650f, 269.201f, 280.752f, 292.303f, 303.854f};
	
}//namespace barkComponents

