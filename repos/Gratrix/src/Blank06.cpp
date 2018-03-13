//============================================================================================================
//!
//! \file Blank06.cpp
//!
//! \brief Blank 6 is a simple do nothing 6-hole high quality blank.
//!
//============================================================================================================


#include "Gratrix.hpp"


namespace GTX {
namespace Blank_06 {


//============================================================================================================
//! \brief The module.

struct GtxModule : Module
{
	GtxModule() : Module(0, 0, 0) {}
};


//============================================================================================================
//! \brief The widget.

struct GtxWidget : ModuleWidget
{
	GtxWidget(GtxModule *module) : ModuleWidget(module)
	{
		GTX__WIDGET();
		box.size = Vec(6*15, 380);

		#if GTX__SAVE_SVG
		{
			PanelGen pg(assetPlugin(plugin, "build/res/Blank06.svg"), box.size);
		}
		#endif

		setPanel(SVG::load(assetPlugin(plugin, "res/Blank06.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));
	}
};


Model *model = Model::create<GtxModule, GtxWidget>("Gratrix", "Blank6", "Blank 6", BLANK_TAG);


} // Blank_6
} // GTX
