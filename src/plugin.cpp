#include "plugin.hpp"

Plugin *pluginInstance;

void init(rack::Plugin *p) {

	pluginInstance = p;

	// p->addModel(model---);
	p->addModel(modelTrimLFO);
	p->addModel(modelQuadLogic);
	p->addModel(modelPanel6);
	p->addModel(modelOneBand);
	p->addModel(modelbpmTrimLFO);
	p->addModel(modelTestPanel);
	//p->addModel(modelNavPanel);
	//p->addModel(modelClockSeq16);
	p->addModel(modelPolyMix);
	p->addModel(modelClamp);
	p->addModel(modelPolyX);
}
