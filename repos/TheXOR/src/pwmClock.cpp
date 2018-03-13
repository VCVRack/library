#include "pwmClock.hpp"

#define PWM_MINVALUE (0.05)
#define PWM_MAXVALUE (0.95)
void PwmClock::on_loaded()
{
	bpm = 0;
	swing = 0;
	_reset();
	load();
}

void PwmClock::_reset()
{
	for(int k = 0; k < OUT_SOCKETS; k++)
	{
		sa_timer[k].Reset();
		odd_beat[k] = false;
	}
}

void PwmClock::load()
{
	updateBpm();
}

void PwmClock::process_keys()
{
	if(btnup.process(params[BPM_INC].value))
	{
		if(bpm_integer < 220.0)
			bpm_integer += 1;
		pWidget->SetBpm(bpm_integer);
	}

	if(btndwn.process(params[BPM_DEC].value))
	{
		if(bpm_integer > 0)
			bpm_integer -= 1;
		pWidget->SetBpm(bpm_integer);
	}
}

void PwmClock::step()
{
	process_keys();
	bpm_integer = roundf(params[BPM].value);
	updateBpm();
	
	if(resetTrigger.process(inputs[RESET].value))
	{
		_reset();
	} else
	{
		for(int k = 0; k < OUT_SOCKETS; k++)
		{
			float gate_len = getDuration(k) * getPwm();
			sa_timer[k].Step();
			float elps = sa_timer[k].Elapsed();
			if(elps >= getDuration(k))
			{
				elps = sa_timer[k].Reset();
				odd_beat[k] = !odd_beat[k];
			}
			if(elps <= gate_len)
				outputs[OUT_1 + k].value = LVL_ON;
			else
				outputs[OUT_1 + k].value = LVL_OFF;
		}
	}
}

float PwmClock::getPwm()
{
	float offs = inputs[PWM_IN].active ? rescale(inputs[PWM_IN].value, 0.0, 5.0, PWM_MINVALUE, PWM_MAXVALUE) : 0.0;
	return clamp(offs + params[PWM].value, PWM_MINVALUE, PWM_MAXVALUE);
}

PwmClockWidget::PwmClockWidget(PwmClock *module) : SequencerWidget(module)
{
	module->setWidget(this);
	box.size = Vec(150, 380);
	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;

		panel->setBackground(SVG::load(assetPlugin(plugin, "res/PwmClock.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

	int pos_y = 35;
	SigDisplayWidget *display = new SigDisplayWidget(4, 1);
	display->box.pos = Vec(25, pos_y);
	display->box.size = Vec(78, 24);

	display->value = &module->bpm;
	addChild(display);
	
	addParam(ParamWidget::create<UPSWITCH>(Vec(8, pos_y ), module, PwmClock::BPM_INC, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<DNSWITCH>(Vec(8, pos_y + 12), module, PwmClock::BPM_DEC, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<Rogan1PSWhiteSnapped>(Vec(12, pos_y + 36), module, PwmClock::BPM, 20.0, 220.0, 120.0));
	addParam(ParamWidget::create<Rogan1PSWhiteSnappedSmall>(Vec(62, pos_y + 42), module, PwmClock::BPMDEC, 0.0, 9.0, 0.0));
	addParam(ParamWidget::create<Rogan1PSGreen>(Vec(97, pos_y + 36), module, PwmClock::PWM, PWM_MINVALUE, PWM_MAXVALUE, 0.5));
	addParam(ParamWidget::create<Rogan1PSRedSmall>(Vec(112, pos_y-2), module, PwmClock::SWING, 0.0, 0.5, 0.0));

	int row = 0;
	int col = 0;
	const char *divisor_len[] = {"1/1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/64"};
	Label *label = new Label();
	pos_y = 122;
	label->box.pos = Vec(72, pos_y + 2);
	label->text = "PWM";
	addChild(label);
	addInput(Port::create<PJ301GPort>(Vec(38+72, pos_y), Port::INPUT, module, PwmClock::PWM_IN));

	label = new Label();
	label->box.pos = Vec(0, pos_y + 2);
	label->text = "RST";
	addChild(label);
	addInput(Port::create<PJ301YPort>(Vec(38, pos_y), Port::INPUT, module, PwmClock::RESET));

	for(int k = 0; k < OUT_SOCKETS; k++)
	{
		int x = col * 36 + 38;
		int y = row * 29 + pos_y +29;
		addOutput(Port::create<PJ301MPort>(Vec(x, y), Port::OUTPUT, module, PwmClock::OUT_1 + k));

		if(col == 0)
		{
			label = new Label();
			label->box.pos = Vec(0, y + 2);
			label->text = divisor_len[row];
			addChild(label);
		}

		if(++col >= 3)
		{
			col = 0;
			row++;
		}
	}
}

void PwmClockWidget::SetBpm(float bpm_integer)
{
	int index = getParamIndex(PwmClock::BPM);
	if(index >= 0)
	{
		// VCV interface update is ahem.... migliorabile....
		bool smooth = params[index]->smooth;
		params[index]->smooth = false;
		params[index]->setValue((double)bpm_integer);
		params[index]->smooth = smooth;
	}
}