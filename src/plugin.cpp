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
	p->addModel(modelPolyMix);
	p->addModel(modelClamp);
	p->addModel(modelPolyX);
	p->addModel(modelEOsum);
	p->addModel(modelSHTH);
	p->addModel(modelLMH);
}

