#include "Bark.hpp"

Plugin *plugin;

void init(rack::Plugin *p) {

	plugin = p;
	p->slug = TOSTRING(SLUG);
	p->version = TOSTRING(VERSION);

	// p->addModel(model---);
	p->addModel(modelTrimLFO);
	p->addModel(modelQuadLogic);
	p->addModel(modelPanel6);
}
