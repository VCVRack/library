#include "Southpole.hpp"

Blank1HPWidget::Blank1HPWidget() 
{
	box.size = Vec(1 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	{
		auto *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/sp-Blank2HP.svg")));
		addChild(panel);
	}
}

Blank2HPWidget::Blank2HPWidget() 
{
	box.size = Vec(2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	{
		auto *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/sp-Blank2HP.svg")));
		addChild(panel);
	}
}

Blank4HPWidget::Blank4HPWidget() 
{
	box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	{
		auto *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/sp-Blank4HP.svg")));
		addChild(panel);
	}
}

Blank8HPWidget::Blank8HPWidget() 
{
	box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	{
		auto *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/sp-Blank8HP.svg")));
		addChild(panel);
	}
	/*
	textField = new TextField();
	textField->box.pos = Vec(15, 15);
	textField->box.size = box.size.minus(Vec(30, 40));
	textField->multiline = true;
	addChild(textField);
	*/
}
/*
json_t *Blank8HPWidget::toJson() {
	json_t *rootJ = ModuleWidget::toJson();
	json_object_set_new(rootJ, "text", json_string(textField->text.c_str()));
	return rootJ;
}

void Blank8HPWidget::fromJson(json_t *rootJ) {
	ModuleWidget::fromJson(rootJ);
	json_t *textJ = json_object_get(rootJ, "text");
	if (textJ)
		textField->text = json_string_value(textJ);
}
*/

Blank16HPWidget::Blank16HPWidget() 
{
	box.size = Vec(16 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	{
		auto *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/sp-Blank16HP.svg")));
		addChild(panel);
	}
}

Blank42HPWidget::Blank42HPWidget() 
{
	box.size = Vec(42 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	{
		auto *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/sp-Blank42HP.svg")));
		addChild(panel);
	}
}





