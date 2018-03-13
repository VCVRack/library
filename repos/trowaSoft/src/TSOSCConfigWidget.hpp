#ifndef  TSOSCCONFIGWIDGET_HPP
#define TSOSCCONFIGWIDGET_HPP

#include "rack.hpp"
using namespace rack;

extern Plugin *plugin;
#include "componentlibrary.hpp"
#include "widgets.hpp"
#include "dsp/digital.hpp"
#include <string.h>
#include <stdlib.h>
#include "TSTextField.hpp"
#include "trowaSoftComponents.hpp"
#include "TSOSCCommon.hpp"
//#include "TSSequencerModuleBase.hpp"

#define TSOSC_NUM_TXTFIELDS		3
#define TSOSC_STATUS_COLOR		nvgRGB(0x00, 0xff, 0xff)

// Button to chose OSC client from drop down.
struct TSOSCClientSelectBtn : ChoiceButton {
	// The selected value.
	OSCClient selectedOSCClient;
	bool visible = false;
	void step() override;
	void onAction(EventAction &e) override;
	// Draw if visible
	void draw(NVGcontext *vg) override;
};
// An OSC client option in dropdown.
struct TSOSCClientItem : MenuItem {
	OSCClient oscClient;
	TSOSCClientSelectBtn* parentButton;
	TSOSCClientItem(TSOSCClientSelectBtn* parent)
	{
		parentButton = parent;
		return;
	}
	void onAction(EventAction &e) override;
};


struct TSOSCConfigWidget : OpaqueWidget
{
	// For looping, array of txt boxes
	TSTextField* textBoxes[TSOSC_NUM_TXTFIELDS];
	TSTextField* tbIpAddress;
	TSTextField* tbTxPort;
	TSTextField* tbRxPort;
	// Client select/drop down.
	TSOSCClientSelectBtn* btnClientSelect;
	// Reference to module.
	Module* module;
	// Save (Enable/Disable) button
	TS_PadBtn* btnSave;
	// If the btn should be Enable (true) or Disable (false).
	bool btnActionEnable = true;
	// Disable OSC
	//TS_PadBtn* btnDisable;
	std::string errorMsg;
	std::string successMsg;
	// The current status
	std::string statusMsg;
	// The current status (on the bottom).
	std::string statusMsg2;
	bool visible;
	std::shared_ptr<Font> font;
	float fontSize = 12;
	NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
	NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
	NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);
	NVGcolor errorColor = nvgRGB(0xee, 0x00, 0x00);
	NVGcolor successColor = nvgRGB(0x00, 0xee, 0x00);
	NVGcolor statusColor = TSOSC_STATUS_COLOR;
	// Callback/event for when a VALID form is submitted.
	void(*formSubmitted)();

	TSOSCConfigWidget(Module* mod, int btnSaveId, int btnDisableId, OSCClient selectedClient);
	TSOSCConfigWidget(Module* mod, int btnSaveId, int btnDisableId, OSCClient selectedClient, std::string ipAddress, uint16_t txPort, uint16_t rxPort);

	// If visible, check for btn submit
	void step() override;

	// Draw if visible
	void draw(NVGcontext *vg) override;
	// Callback for tabbing between our text boxes.
	void onTabField(int id);
	// Callback for shift-tabbing between our text boxes.
	void onShiftTabField(int id);

	// Sets the values
	void setValues(std::string ipAddress, uint16_t txPort, uint16_t rxPort)
	{
		tbIpAddress->text = ipAddress;
		tbTxPort->text = std::to_string(txPort);
		tbRxPort->text = std::to_string(rxPort);
		return;
	}
	// Sets the visibility
	void setVisible(bool isVisible) {
		visible = isVisible;
		tbIpAddress->visible = isVisible;
		tbTxPort->visible = isVisible;
		tbRxPort->visible = isVisible;
		btnClientSelect->visible = isVisible;
		return;
	}
	// Get the selected OSC Client.
	OSCClient getSelectedClient() {
		return btnClientSelect->selectedOSCClient;
	}
	// Set the selected OSC Client.
	void setSelectedClient(OSCClient client) {
		btnClientSelect->selectedOSCClient = client;
	}

	// Get the input ip address.
	std::string getIpAddress() { return tbIpAddress->text; }

	bool isValidIpAddress()
	{
		return tbIpAddress->isValid();
	}

	bool isValidPort(std::string port) {
		bool isValid = false;
		try
		{
			if (port.length() > 0)
			{
				int portNumber = std::atoi(port.c_str());
				isValid = portNumber > -1 && portNumber <= 0xFFFF;
			}
		}
		catch (const std::exception& ex)
		{
			isValid = false;
		} 
		return isValid;
	}
	// If the Tx port is valid.
	bool isValidTxPort()
	{
		return tbTxPort->isValid() && isValidPort(tbTxPort->text);
	}
	// If the Rx port is valid.
	bool isValidRxPort()
	{
		return tbRxPort->isValid() && isValidPort(tbRxPort->text);
	}

	// Get the output port.
	uint16_t getTxPort() {
		if (tbTxPort->text.length() > 0)
			return std::atoi(tbTxPort->text.c_str());
		else
			return 0;
	}
	// Get the input port.
	uint16_t getRxPort() {
		if (tbRxPort->text.length() > 0)
			return std::atoi(tbRxPort->text.c_str());
		else
			return 0;
	}
};

#endif // ! TSOSCCONFIGWIDGET_HPP
