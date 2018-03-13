#include "rack.hpp"
using namespace rack;
#include "asset.hpp"
#include "componentlibrary.hpp"
#include "plugin.hpp"
#include "TSOSCConfigWidget.hpp"
#include "trowaSoftUtilities.hpp"
//#include "TSSequencerModuleBase.hpp"
#include "TSOSCCommon.hpp"

#define START_Y   15

/// TODO: DISABLE WHEN NOT VISIBLE


void TSOSCClientItem::onAction(EventAction &e) {
	parentButton->selectedOSCClient = this->oscClient;
	return;
}
// On button click, create drop down menu.
void TSOSCClientSelectBtn::onAction(EventAction &e) {
	if (visible)
	{
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y)).round();
		menu->box.size.x = box.size.x;
		for (unsigned int i = 0; i < OSCClient::NUM_OSC_CLIENTS; i++) {
			TSOSCClientItem *option = new TSOSCClientItem(this);
			option->oscClient = static_cast<OSCClient>(i);
			option->text = OSCClientStr[i];
			menu->addChild(option);
		}
	}
	return;
}

void TSOSCClientSelectBtn::step() {
	text = ellipsize(OSCClientStr[selectedOSCClient], 15);
}

void TSOSCClientSelectBtn::draw(NVGcontext *vg) {
	if (visible)
	{
		ChoiceButton::draw(vg);
	}
	return;
}



TSOSCConfigWidget::TSOSCConfigWidget(Module* mod, int btnSaveId, int btnDisableId, OSCClient selectedClient) : TSOSCConfigWidget(mod, btnSaveId, btnDisableId, selectedClient, "", 1000, 1001)
{
	return;
}

TSOSCConfigWidget::TSOSCConfigWidget(Module* mod, int btnSaveId, int btnDisableId, OSCClient selectedClient, std::string ipAddress, uint16_t txPort, uint16_t rxPort)
{
	this->module = mod;
	font = Font::load(assetPlugin(plugin, TROWA_LABEL_FONT));
	

	statusMsg2 = "";


	this->box.size = Vec(400, 50);
	int height = 20;
	visible = true;
	int x, y;
	int dx = 4;
	int i = 0;
	x = 6;
	y = START_Y;
	tbIpAddress = new TSTextField(TSTextField::TextType::IpAddress, 15);
	tbIpAddress->box.size = Vec(105, height); //115
	tbIpAddress->box.pos = Vec(x, y);
	tbIpAddress->visible = visible;
	tbIpAddress->text = ipAddress;
	tbIpAddress->placeholder = "127.0.0.1";
	tbIpAddress->id = i;
	addChild(tbIpAddress);
	textBoxes[i++] = tbIpAddress;

	x += tbIpAddress->box.size.x + dx;
	tbTxPort = new TSTextField(TSTextField::TextType::DigitsOnly, 5);
	tbTxPort->box.size = Vec(50, height);
	tbTxPort->box.pos = Vec(x, y);
	tbTxPort->visible = visible;
	tbTxPort->text = std::to_string(txPort);
	tbTxPort->id = i;
	addChild(tbTxPort);
	textBoxes[i++] = tbTxPort;

	x += tbTxPort->box.size.x + dx;
	tbRxPort = new TSTextField(TSTextField::TextType::DigitsOnly, 5);
	tbRxPort->box.size = Vec(50, height);
	tbRxPort->box.pos = Vec(x, y);
	tbRxPort->visible = visible;
	tbRxPort->text = std::to_string(rxPort);
	tbRxPort->id = i;
	addChild(tbRxPort);
	textBoxes[i++] = tbRxPort;

	// OSC Client Type:
	// (since touchOSC needs special handling, Lemur probably does too)
	x += tbRxPort->box.size.x + dx;
	btnClientSelect = new TSOSCClientSelectBtn();
	btnClientSelect->selectedOSCClient = selectedClient;
	btnClientSelect->box.size = Vec(78, height);
	btnClientSelect->box.pos = Vec(x, y);
	btnClientSelect->visible = visible;
	addChild(btnClientSelect);

	// Button Enable/Disable
	x += btnClientSelect->box.size.x + dx + 3;
	Vec btnSize = Vec(36, height);
	this->btnSave = new TS_PadBtn();
	this->btnSave->module = module;
	this->btnSave->paramId = btnSaveId;
	this->btnSave->box.size = btnSize;
	this->btnSave->box.pos = Vec(x, y);
	addChild(btnSave);

	//x += btnSave->box.size.x + dx;
	//this->btnDisable = new TS_PadBtn();
	//this->btnDisable->module = module;
	//this->btnDisable->paramId = btnDisableId;
	//this->btnDisable->box.size = btnSize;
	//this->btnDisable->box.pos = Vec(x, y);
	//addChild(btnDisable);

	for (i = 0; i < TSOSC_NUM_TXTFIELDS; i++)
	{
		int prevIx = (i > 0) ? i - 1 : TSOSC_NUM_TXTFIELDS - 1;
		int nextIx = (i < TSOSC_NUM_TXTFIELDS - 1) ? i + 1 : 0;
		textBoxes[i]->nextField = textBoxes[nextIx];
		textBoxes[i]->prevField = textBoxes[prevIx];

	}
	return;
}

void TSOSCConfigWidget::step() {
	Widget::step();
	return;
}

// Callback for tabbing between our text boxes.
void TSOSCConfigWidget::onTabField(int id)
{
	int focusIx = (id + 1) % 3;
	textBoxes[focusIx]->requestFocus();
	return;
}

// Callback for shift-tabbing between our text boxes.
// This doesn't work since in C++ I can't figure out how to get a damn pointer to a member function...
void TSOSCConfigWidget::onShiftTabField(int id)
{
	int focusIx = id - 1;
	if (focusIx < 0)
		focusIx = 2;
	textBoxes[focusIx]->requestFocus();
	return;
}

void TSOSCConfigWidget::draw(NVGcontext *vg) {
	if (!visible)
	{

		return;
	}
	nvgFontSize(vg, fontSize);
	nvgFontFaceId(vg, font->handle);
	
	// Screen:
	nvgBeginPath(vg);
	nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
	nvgFillColor(vg, backgroundColor);
	nvgFill(vg);
	nvgStrokeWidth(vg, 1.0);
	nvgStrokeColor(vg, borderColor);
	nvgStroke(vg);

	// Draw labels
	nvgFillColor(vg, textColor);
	nvgFontSize(vg, fontSize);
	nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM);
	float y = START_Y - 1;
	float x;
	const char* labels[] = { "OSC IP Address", "Out Port", "In Port" };
	for (int i = 0; i < TSOSC_NUM_TXTFIELDS; i++)
	{
		x = textBoxes[i]->box.pos.x + 2;
		nvgText(vg, x, y, labels[i], NULL);
	}

	// Current status:
	x = box.size.x - 8;
	nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_BOTTOM);
	nvgFillColor(vg, statusColor);
	nvgText(vg, x, y, statusMsg.c_str(), NULL);
	// Status 2
	y += textBoxes[0]->box.size.y + 2;
	if (!statusMsg2.empty())
	{
		nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
		nvgText(vg, x, y, statusMsg2.c_str(), NULL);
	}

	// Draw Messages:
	x = textBoxes[0]->box.pos.x + 2;
	nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
	if (!errorMsg.empty())
	{
		nvgFillColor(vg, errorColor);		
		nvgText(vg, x, y, errorMsg.c_str(), NULL);
	}
	else if (!successMsg.empty())
	{
		nvgFillColor(vg, successColor);
		nvgText(vg, x, y, successMsg.c_str(), NULL);
	}
	else
	{
		nvgFillColor(vg, textColor);
		nvgText(vg, x, y, "Open Sound Control Configuration", NULL);
	}

	OpaqueWidget::draw(vg);

	// Quick and dirty -- Draw labels on buttons:
	nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
	y = btnSave->box.pos.y + btnSave->box.size.y / 2.0 + 1;
	// Save:
	x = btnSave->box.pos.x + btnSave->box.size.x / 2.0 + 7;
	if (btnActionEnable)
	{
		nvgFillColor(vg, COLOR_TS_GREEN);
		nvgText(vg, x, y, "ENABLE", NULL);
	}
	else
	{
		nvgFillColor(vg, COLOR_TS_ORANGE);
		nvgText(vg, x, y, "DISABLE", NULL);
	}
	//// Disable:
	//x = btnDisable->box.pos.x + btnDisable->box.size.x / 2.0;
	//nvgFillColor(vg, COLOR_TS_ORANGE);
	//nvgText(vg, x, y, "DISABLE", NULL);



	return;
}
void onTabField(int id)
{
	return;
}
void onShiftTabField(int id)
{
	return;
}
