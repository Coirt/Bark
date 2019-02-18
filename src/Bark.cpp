#include "Bark.hpp"

Plugin *plugin;

void init(rack::Plugin *p) {

	plugin = p;
	p->slug = TOSTRING(SLUG);
	p->version = TOSTRING(VERSION);
// 	p->website = "https://github.com/Coirt/Bark/issues";
// 	p->manual = "https://github.com/Coirt/Bark/wiki";

	// p->addModel(model---);
	p->addModel(modelTrimLFO);
	p->addModel(modelTrimLFObpm);
	p->addModel(modelQuadLogic);
	p->addModel(modelPanel6);
	p->addModel(modelOneBand);
	
}
