#include "Z8K.hpp"
#include <sstream>

void Z8K::on_loaded()
{
	#ifdef DIGITAL_EXT
	connected = 0;
	#endif
	load();
}

void Z8K::load()
{
	// sequencer 1-4
	for(int k = 0; k < 4; k++)
	{
		int base = VOLTAGE_1 + 4 * k;
		std::vector<int> steps = {base, base + 1, base + 2, base + 3};
		seq[SEQ_1 + k].Init(&inputs[RESET_1 + k], &inputs[DIR_1 + k], &inputs[CLOCK_1 + k], &outputs[CV_1 + k], &lights[LED_ROW], params, steps);
	}
	// sequencer A-D
	for(int k = 0; k < 4; k++)
	{
		std::vector<int> steps = {k, k + 4, k + 8, k + 12};
		seq[SEQ_A + k].Init(&inputs[RESET_A + k], &inputs[DIR_A + k], &inputs[CLOCK_A + k], &outputs[CV_A + k], &lights[LED_COL], params, steps);
	}
	// horiz
	std::vector<int> steps_h = {0,1,2,3,7,6,5,4,8,9,10,11,15,14,13,12};
	seq[SEQ_HORIZ].Init(&inputs[RESET_HORIZ], &inputs[DIR_HORIZ], &inputs[CLOCK_HORIZ], &outputs[CV_HORIZ], &lights[LED_HORIZ], params, steps_h);
	//vert
	std::vector<int> steps_v = {0,4,8,12,13,9,5,1,2,6,10,14,15,11,7,3};
	seq[SEQ_VERT].Init(&inputs[RESET_VERT], &inputs[DIR_VERT], &inputs[CLOCK_VERT], &outputs[CV_VERT], &lights[LED_VERT], params, steps_v);
}

void Z8K::step()
{
	for(int k = 0; k < NUM_SEQUENCERS; k++)
		seq[k].Step();

	#ifdef DIGITAL_EXT
	bool dig_connected = false;

	/*#ifdef LAUNCHPAD
	if(drv->Connected())
		dig_connected = true;
	drv->ProcessLaunchpad();
	#endif*/

	#if defined(OSCTEST_MODULE)
	if(oscDrv->Connected())
		dig_connected = true;
	oscDrv->ProcessOSC();
	#endif	
	connected = dig_connected ? 1.0 : 0.0;
	#endif
}

Z8KWidget::Z8KWidget(Z8K *module) : ModuleWidget(module)
{
	#ifdef OSCTEST_MODULE
	char name[60];
	#endif

	box.size = Vec(28 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	SVGPanel *panel = new SVGPanel();
	panel->box.size = box.size;
	panel->setBackground(SVG::load(assetPlugin(plugin, "res/Z8KModule.svg")));
	addChild(panel);
	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, box.size.y - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, box.size.y - RACK_GRID_WIDTH)));

	int x = 10;
	int y = 30;
	int dist_v = 38;
	int dist_h = 32;
	for(int k = 0; k < 4; k++)
	{
		addInput(Port::create<PJ301YPort>(Vec(x, y + k * dist_v), Port::INPUT, module, Z8K::RESET_1 + k));
		addInput(Port::create<PJ301WPort>(Vec(x + dist_h, y + k * dist_v), Port::INPUT, module, Z8K::DIR_1 + k));
		addInput(Port::create<PJ301RPort>(Vec(x + 2 * dist_h, y + k * dist_v), Port::INPUT, module, Z8K::CLOCK_1 + k));
	}

	y += 5 * dist_v;
	for(int k = 0; k < 4; k++)
	{
		addInput(Port::create<PJ301YPort>(Vec(x, y + k * dist_v), Port::INPUT, module, Z8K::RESET_A + k));
		addInput(Port::create<PJ301WPort>(Vec(x + dist_h, y + k * dist_v), Port::INPUT, module, Z8K::DIR_A + k));
		addInput(Port::create<PJ301RPort>(Vec(x + 2 * dist_h, y + k * dist_v), Port::INPUT, module, Z8K::CLOCK_A + k));
	}

	y = 35;
	x += 2 * dist_h + 40;
	dist_h = 64;
	dist_v = 65;
	for(int r = 0; r < 4; r++)
	{
		for(int c = 0; c < 4; c++)
		{
			int n = c + r * 4;
			ParamWidget *pctrl = ParamWidget::create<Davies1900hBlackKnob>(Vec(x + dist_h * c, y + dist_v * r), module, Z8K::VOLTAGE_1 + n, 0.0, 1.0, 0.5);
			#ifdef OSCTEST_MODULE
			sprintf(name, "/Knob%i", n + 1);
			oscControl *oc = new oscControl(name);
			module->oscDrv->Add(oc, pctrl);
			#endif
			addParam(pctrl);
			const int displ = 4;

			int led_x = x-7 + 2 * dist_h / 3 + c * dist_h;
			int led_y = y-8 + 2 * dist_v / 3 + dist_v * r;
			ModuleLightWidget *plight = ModuleLightWidget::create<SmallLight<RedLight>>(Vec(led_x - displ,led_y - displ), module, Z8K::LED_ROW + n);
			#ifdef OSCTEST_MODULE
			sprintf(name, "/LedR%i", n + 1);
			oc = new oscControl(name);
			module->oscDrv->Add(oc, plight);
			#endif
			addChild(plight);

			plight = ModuleLightWidget::create<SmallLight<GreenLight>>(Vec(led_x+ displ, led_y - displ), module, Z8K::LED_COL + n);
			#ifdef OSCTEST_MODULE
			sprintf(name, "/LedC%i", n + 1);
			oc = new oscControl(name);
			module->oscDrv->Add(oc, plight);
			#endif
			addChild(plight);

			plight = ModuleLightWidget::create<SmallLight<YellowLight>>(Vec(led_x - displ, led_y + displ), module, Z8K::LED_VERT + n);
			#ifdef OSCTEST_MODULE
			sprintf(name, "/LedV%i", n + 1);
			oc = new oscControl(name);
			module->oscDrv->Add(oc, plight);
			#endif
			addChild(plight);

			plight = ModuleLightWidget::create<SmallLight<BlueLight>>(Vec(led_x + displ, led_y + displ), module, Z8K::LED_HORIZ + n);
			#ifdef OSCTEST_MODULE
			sprintf(name, "/LedH%i", n + 1);
			oc = new oscControl(name);
			module->oscDrv->Add(oc, plight);
			#endif
			addChild(plight);

			if(r == 3)
				addOutput(Port::create<PJ301GPort>(Vec(x + dist_h * c + 5, y+7 + dist_v * 4 - dist_v / 3), Port::OUTPUT, module, Z8K::CV_A + c));
		}
		addOutput(Port::create<PJ301GPort>(Vec(box.size.x - 40, y + 5 + dist_v * r), Port::OUTPUT, module, Z8K::CV_1 + r));
	}

	y += dist_v * 4 + 40;
	dist_h /= 2;
	dist_v = 20;
	for(int k = 0; k < 2; k++)
	{
		int px = 7 + x + k * 4 * (dist_h+5);
		addInput(Port::create<PJ301YPort>(Vec(px, y), Port::INPUT, module, Z8K::RESET_VERT + k));
		addInput(Port::create<PJ301WPort>(Vec(px + dist_h, y - dist_v), Port::INPUT, module, Z8K::DIR_VERT + k));
		addInput(Port::create<PJ301RPort>(Vec(px + 2 * dist_h, y), Port::INPUT, module, Z8K::CLOCK_VERT + k));
		addOutput(Port::create<PJ301GPort>(Vec(px + 3 * dist_h, y - dist_v), Port::OUTPUT, module, Z8K::CV_VERT + k));
	}

	#ifdef DIGITAL_EXT
	addChild(new DigitalLed(box.size.x - 40, box.size.y-70, &module->connected));
	#endif
}
