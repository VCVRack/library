#include "luckyxxl.hpp"


Plugin *pluginLuckyxxl;

void init(rack::Plugin *p) {
	pluginLuckyxxl = p;
	p->slug = TOSTRING(SLUG);
	p->version = TOSTRING(VERSION);

	p->addModel(modelDistribute2Module);
	p->addModel(modelDistribute4Module);
	p->addModel(modelQuantizeModule);
	p->addModel(modelTickModule);
}
