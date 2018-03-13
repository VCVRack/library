
#include "widgets.hpp"

using namespace bogaudio;

StatefulButton::StatefulButton(const char* offSVGPath, const char* onSVGPath) {
	_svgWidget = new SVGWidget();
	addChild(_svgWidget);

	auto svg = SVG::load(assetPlugin(plugin, offSVGPath));
	_frames.push_back(svg);
	_frames.push_back(SVG::load(assetPlugin(plugin, onSVGPath)));

	_svgWidget->setSVG(svg);
	box.size = _svgWidget->box.size;
}

void StatefulButton::step() {
	FramebufferWidget::step();
}

void StatefulButton::onDragStart(EventDragStart& e) {
	_svgWidget->setSVG(_frames[1]);
	dirty = true;

	if (value >= maxValue) {
		setValue(minValue);
	}
	else {
		setValue(value + 1.0);
	}
}

void StatefulButton::onDragEnd(EventDragEnd& e) {
	_svgWidget->setSVG(_frames[0]);
	dirty = true;
}
