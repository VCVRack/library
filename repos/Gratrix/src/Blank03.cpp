//============================================================================================================
//!
//! \file Blank03.cpp
//!
//! \brief Blank 3 is a simple do nothing 3-hole high quality blank.
//!
//============================================================================================================


#include "Gratrix.hpp"


namespace GTX {
namespace Blank_03 {


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
		box.size = Vec(3*15, 380);

		#if GTX__SAVE_SVG
		{
			PanelGen pg(assetPlugin(plugin, "build/res/Blank03.svg"), box.size);
		}
		#endif

		setPanel(SVG::load(assetPlugin(plugin, "res/Blank03.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));
	}
};


Model *model = Model::create<GtxModule, GtxWidget>("Gratrix", "Blank3", "Blank 3", BLANK_TAG);


} // Blank_3
} // GTX
