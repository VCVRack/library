#include "Renato.hpp"
#include <sstream>


bool Access(Renato *pr, bool is_x, int p) { return is_x ? pr->_accessX(p) : pr->_accessY(p); }

void Renato::on_loaded()
{
	#ifdef DIGITAL_EXT
	connected = 0;
	#endif
	load();
}

void Renato::load()
{
	seqX.Reset();
	seqY.Reset();
}

void Renato::step()
{
	if(resetTrigger.process(inputs[RESET].value))
	{
		seqX.Reset();
		seqY.Reset();
	} else
	{
		bool seek_mode = params[SEEKSLEEP].value > 0;
		int clkX = seqX.Step(inputs[XCLK].value, params[COUNTMODE_X].value, seek_mode, this, true);
		int clkY = seqY.Step(inputs[YCLK].value, params[COUNTMODE_Y].value, seek_mode, this, false);
		int n = xy(seqX.Position(), seqY.Position());
		if(_access(n))
		{
			if(_gateX(n))
				seqX.Gate(clkX, &outputs[XGATE], &lights[LED_GATEX]);

			if(_gateY(n))
				seqY.Gate(clkY, &outputs[YGATE], &lights[LED_GATEY]);

			outputs[CV].value = params[VOLTAGE_1 + n].value;
			led(n);
		}
	}
	#ifdef DIGITAL_EXT
	bool dig_connected = false;

	#ifdef LAUNCHPAD
	if(drv->Connected())
		dig_connected = true;
	drv->ProcessLaunchpad();
	#endif

	#if defined(OSCTEST_MODULE)
	if(oscDrv->Connected())
		dig_connected = true;
	oscDrv->ProcessOSC();
	#endif	
	connected = dig_connected ? 1.0 : 0.0;
	#endif
}

Menu *RenatoWidget::addContextMenu(Menu *menu)
{
	menu->addChild(new SeqMenuItem<RenatoWidget>("Randomize Pitch", this, RANDOMIZE_PITCH));
	menu->addChild(new SeqMenuItem<RenatoWidget>("Randomize Gate Xs", this, RANDOMIZE_GATEX));
	menu->addChild(new SeqMenuItem<RenatoWidget>("Randomize Gate Ys", this, RANDOMIZE_GATEY));
	menu->addChild(new SeqMenuItem<RenatoWidget>("Randomize Access", this, RANDOMIZE_ACCESS));
	return menu;
}

void RenatoWidget::onMenu(int action)
{
	switch(action)
	{
	case RANDOMIZE_PITCH: std_randomize(Renato::VOLTAGE_1, Renato::VOLTAGE_1 + 16); break;
	case RANDOMIZE_GATEX: std_randomize(Renato::GATEX_1, Renato::GATEX_1 + 16); break;
	case RANDOMIZE_GATEY: std_randomize(Renato::GATEY_1, Renato::GATEY_1 + 16); break;
	case RANDOMIZE_ACCESS: std_randomize(Renato::ACCESS_1, Renato::ACCESS_1 + 16); break;
	}
}

RenatoWidget::RenatoWidget(Renato *module ) : SequencerWidget(module)
{
	#ifdef OSCTEST_MODULE
	char name[60];
	#endif

	box.size = Vec(27 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	SVGPanel *panel = new SVGPanel();
	panel->box.size = box.size;
	panel->setBackground(SVG::load(assetPlugin(plugin, "res/RenatoModule.svg")));
	addChild(panel);
	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, box.size.y - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, box.size.y - RACK_GRID_WIDTH)));

	int x = 20;
	int y = 30;
	int dist_h = 32;
	addInput(Port::create<PJ301RPort>(Vec(x, y), Port::INPUT, module, Renato::XCLK));
	x += dist_h;
	addInput(Port::create<PJ301RPort>(Vec(x, y), Port::INPUT, module, Renato::YCLK));
	x += dist_h;
	addInput(Port::create<PJ301YPort>(Vec(x, y), Port::INPUT, module, Renato::RESET));
	x += dist_h+10;
	
	// page 0 (SESSION)
	ParamWidget *pwdg = ParamWidget::create<NKK2>(Vec(x, y - 10), module, Renato::COUNTMODE_X, 0.0, 2.0, 0.0);
	addParam(pwdg);
	#ifdef LAUNCHPAD
	LaunchpadRadio *radio = new LaunchpadRadio(0, ILaunchpadPro::RC2Key(2, 1), 3, LaunchpadLed::Color(47), LaunchpadLed::Color(32));
	module->drv->Add(radio, pwdg);
	#endif
	#ifdef OSCTEST_MODULE
	sprintf(name, "/ModeX");
	oscControl *oc = new oscControl(name);
	module->oscDrv->Add(oc, pwdg);
	#endif

	x += 5 * dist_h / 3;
	pwdg = ParamWidget::create<NKK2>(Vec(x, y - 10), module, Renato::COUNTMODE_Y, 0.0, 2.0, 0.0);
	addParam(pwdg);
	#ifdef LAUNCHPAD
	radio = new LaunchpadRadio(0, ILaunchpadPro::RC2Key(2, 3), 3, LaunchpadLed::Color(19), LaunchpadLed::Color(21));
	module->drv->Add(radio, pwdg);
	#endif
	#ifdef OSCTEST_MODULE
	sprintf(name, "/ModeY");
	oc = new oscControl(name);
	module->oscDrv->Add(oc, pwdg);
	#endif

	x += 5 * dist_h / 3;
	pwdg = ParamWidget::create<NKK2>(Vec(x, y - 10), module, Renato::SEEKSLEEP, 0.0, 1.0, 0.0);
	addParam(pwdg);
	#ifdef LAUNCHPAD
	radio = new LaunchpadRadio(0, ILaunchpadPro::RC2Key(2, 5), 2, LaunchpadLed::Color(51), LaunchpadLed::Color(52));
	module->drv->Add(radio, pwdg);
	#endif
	#ifdef OSCTEST_MODULE
	sprintf(name, "/Seek");
	oc = new oscControl(name);
	module->oscDrv->Add(oc, pwdg);
	#endif

	x = box.size.x - 3 * dist_h - 20;
	addOutput(Port::create<PJ301MPort>(Vec(x, y), Port::OUTPUT, module, Renato::CV));
	x += dist_h;
	addOutput(Port::create<PJ301GPort>(Vec(x, y), Port::OUTPUT, module, Renato::XGATE));
	ModuleLightWidget *plight = ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(x + 18, y + 27), module, Renato::LED_GATEX);
	#ifdef OSCTEST_MODULE
	sprintf(name, "/LedGX");
	oc = new oscControl(name);
	module->oscDrv->Add(oc, plight);
	#endif
	addChild(plight);

	x += dist_h;
	addOutput(Port::create<PJ301GPort>(Vec(x, y), Port::OUTPUT, module, Renato::YGATE));
	plight = ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(x + 18, y + 27), module, Renato::LED_GATEY);
	#ifdef OSCTEST_MODULE
	sprintf(name, "/LedGY");
	oc = new oscControl(name);
	module->oscDrv->Add(oc, plight);
	#endif
	addChild(plight);

	// page 1 (NOTES)
	x = 40;
	y = 90;
	dist_h = 95;
	int dist_v = 75;
	for(int r = 0; r < 4; r++)
	{
		for(int c = 0; c < 4; c++)
		{
			int n = c + r * 4;
			pwdg = ParamWidget::create<Davies1900hBlackKnob>(Vec(x + dist_h * c, y + dist_v * r), module, Renato::VOLTAGE_1 + n, 0.005, 6.0, 1.0);
			#ifdef OSCTEST_MODULE
			sprintf(name, "/Knob%i", n+1);
			oc = new oscControl(name);
			module->oscDrv->Add(oc, pwdg);
			#endif
			addParam(pwdg);

			pwdg = ParamWidget::create<CKSS>(Vec(x + dist_h * c - 18, y + dist_v * r + 8), module, Renato::ACCESS_1 + n, 0.0, 1.0, 1.0);
			addParam(pwdg);
			#ifdef LAUNCHPAD
			LaunchpadSwitch *pswitch = new LaunchpadSwitch(1, ILaunchpadPro::RC2Key(r + 4, c), LaunchpadLed::Off(), LaunchpadLed::Color(17));
			module->drv->Add(pswitch, pwdg);
			#endif
			#ifdef OSCTEST_MODULE
			sprintf(name, "/Access%i", n + 1);
			oc = new oscControl(name);
			module->oscDrv->Add(oc, pwdg);
			#endif

			pwdg = ParamWidget::create<CKSS>(Vec(x + dist_h * c + 40, y + dist_v * r - 12), module, Renato::GATEY_1 + n, 0.0, 1.0, 1.0);
			addParam(pwdg);
			#ifdef LAUNCHPAD
			pswitch = new LaunchpadSwitch(1, ILaunchpadPro::RC2Key(r, c + 4), LaunchpadLed::Off(), LaunchpadLed::Color(62));
			module->drv->Add(pswitch, pwdg);
			#endif
			#ifdef OSCTEST_MODULE
			sprintf(name, "/GateY%i", n + 1);
			oc = new oscControl(name);
			module->oscDrv->Add(oc, pwdg);
			#endif

			pwdg = ParamWidget::create<CKSS>(Vec(x + dist_h * c + 40, y + dist_v * r + 28), module, Renato::GATEX_1 + n, 0.0, 1.0, 1.0);
			addParam(pwdg);
			#ifdef LAUNCHPAD
			pswitch = new LaunchpadSwitch(1, ILaunchpadPro::RC2Key(r + 4, c + 4), LaunchpadLed::Off(), LaunchpadLed::Color(52));
			module->drv->Add(pswitch, pwdg);
			#endif
			#ifdef OSCTEST_MODULE
			sprintf(name, "/GateX%i", n + 1);
			oc = new oscControl(name);
			module->oscDrv->Add(oc, pwdg);
			#endif

			ModuleLightWidget *plight = ModuleLightWidget::create<LargeLight<RedLight>>(Vec(x + dist_h * c - 4, y + dist_v * r + 35), module, Renato::LED_1 + n);
			addChild(plight);
			#ifdef LAUNCHPAD
			LaunchpadLight *ld1 = new LaunchpadLight(1, ILaunchpadPro::RC2Key(r, c), LaunchpadLed::Off(), LaunchpadLed::Color(4));
			module->drv->Add(ld1, plight);
			#endif
			#ifdef OSCTEST_MODULE
			sprintf(name, "/Led%i", n + 1);
			oc = new oscControl(name);
			module->oscDrv->Add(oc, plight);
			#endif
		}
	}
	
	#ifdef DIGITAL_EXT
	addChild(new DigitalLed(20, 70, &module->connected));
	#endif
}
