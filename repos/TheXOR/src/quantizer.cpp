#include "common.hpp"
#include "quantizer.hpp"

void Quantizer::step()
{
	for(int k = 0; k < NUM_QUANTIZERS; k++)
	{
		if(outputs[OUT_1+k].active) 
			outputs[OUT_1 + k].value = quantize_out(inputs[IN_1+k]);
	}
}

float Quantizer::quantize_out(Input &in)
{
	float v = in.normalize(0.0);
	float octave = round(v);
	float rest = v - octave;
	float semi = round(rest*12.0);
	return octave + semi / 12.0;
}

QuantizerWidget::QuantizerWidget(Quantizer *module) : ModuleWidget(module)
{
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;

		panel->setBackground(SVG::load(assetPlugin(plugin, "res/quantizer.svg")));
		
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	int dist_v = RACK_GRID_HEIGHT / (1 + NUM_QUANTIZERS);
	int dist_h = RACK_GRID_WIDTH*3;
	int y = 40;
	int x = RACK_GRID_WIDTH/2;
	for(int k = 0; k < NUM_QUANTIZERS; k++)
	{
		addInput(Port::create<PJ301MPort>(Vec(x, y), Port::INPUT, module, Quantizer::IN_1 + k));
		addOutput(Port::create<PJ301MPort>(Vec(x+dist_h, y), Port::OUTPUT, module, Quantizer::OUT_1+k));
		y += dist_v;
	}
}

