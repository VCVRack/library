#ifndef TSTEXTFIELD_HPP
#define TSTEXTFIELD_HPP

#include "rack.hpp"
#include "widgets.hpp"

using namespace rack;

#include <iostream>
#include <regex>
#include <sstream>
#include <string>

// Integer/Digits: Entire string validation
#define TROWA_REGEX_NUMERIC_STR_ONLY		"^[0-9]*$"
// Integer/Digits: Single char validation
#define TROWA_REGEX_NUMERIC_CHAR_ONLY		"^[0-9]$"
// Integer/Digits: Not an digit
#define TROWA_REGEX_NUMERIC_CHAR_NOT		"[^0-9]"

// IP Address: Entire string validation
#define TROWA_REGEX_IP_ADDRESS			 "^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$" 
// IP Address: Single char validation
#define TROWA_REGEX_IP_CHAR				"^([0-9]|\\.)$"
// IP Address: Not a valid character 
#define TROWA_REGEX_IP_CHAR_NOT		"[^0-9\\.]"

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// isPrintableKey()
// @keyCode : (IN) The key that is pressed.
// @returns: True if the key represents a printable character, false if not.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
bool isPrintableKey(int keyCode);

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSTextField - overload rack TextField
// trowaSoft text field with some basic validation and character limiting enforced.
// 1. Consume invalid characters and throw away.
// 2. Validate (regex) input (can be checked with IsValid()).
// 3. Limit input length (with maxLength).
/// TODO: Implement scrolling and cropping so that characters can't break out of the field box.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
/// TODO: Redo this struct completely.
struct TSTextField : TextField {
	//int begin = 0;
	//int end = 0; // Removed from new version TextField.
	// Maximum length allowed.
	uint16_t maxLength = 50;
	// The id of this widget. For use like tab order or something.
	int id = 0;
	// The call back if any (parameter is id). i.e. For tabbing to another field.
	// So parent widget should do this since it is aware of any other field siblings, 
	// or alternatively parent should point to the next guy's requestFocus()....
	// EDIT: Realized you can't do function pointer to member procedure in C++, so this may be of limited use.
	void (*onTabCallback)(int id) = NULL;

	// The call back if any (parameter is id). i.e. For tabbing to the previous field if any.
	// So parent widget should do this since it is aware of any other field siblings, 
	// or alternatively parent should point to the next guy's requestFocus()....
	// EDIT: Realized you can't do function pointer to member procedure in C++, so this may be of limited use.
	void(*onShiftTabCallback)(int id) = NULL;

	// Previous field to focus if Shift-Tab (in lieu of callback).
	TSTextField* nextField = NULL;
	// Next field to focus if Tab (in lieu of callback).
	TSTextField* prevField = NULL;

	// Text type for validation of input.
	enum TextType {
		// Any text allowed (i.e. no regex check).
		Any,
		// Digits only (no decimals even).
		DigitsOnly,
		// IP address (digits and periods), IPv4.
		IpAddress
	};
	// Text type for validation of input.
	TextType allowedTextType = TextType::Any;
	// Regex for a single char.
	std::regex regexChar;
	// Regex for the entire string.
	std::regex regexStr;
	// Regex for invalid characters (to match on invalid and remove).
	std::regex regexInvalidChar;

	TSTextField(TextType textType) : TextField() {
		setTextType(textType);
		return;
	}
	TSTextField(TextType textType, int maxLength) : TSTextField(textType) {
		this->maxLength = maxLength;
		return;
	}
	//-----------------------------------------------------------------------------------------------
	// TSTextField()
	// @id : (IN) Hopefully unique id for the form. Should indicate tab order.
	// @textType: (IN) Text type for this field (for validation).
	// @maxLength: (IN) Max length for this field.
	// @onTabHandler: (IN) Callback/Event handler for tab (i.e. focus on the next field).
	// @onShiftTabHandler: (IN) Callback/Event handler for shift-tab (i.e. focus on the previous field).
	//-----------------------------------------------------------------------------------------------
	TSTextField(int id, TextType textType, int maxLength, void (*onTabHandler)(int),  void (*onShiftTabHandler)(int)) : TSTextField(textType, maxLength) {
		this->onTabCallback = onTabHandler;
		this->onShiftTabCallback = onShiftTabHandler;
		return;
	}

	// If the text is valid.
	bool isValid() {
		return (allowedTextType == TextType::Any
			|| std::regex_match(text, regexStr));
	}
	// Set the text type/validation
	void setTextType(TextType validationType)
	{
		this->allowedTextType = validationType;
		switch (allowedTextType)
		{
		case TextType::DigitsOnly:
			regexChar = std::regex(TROWA_REGEX_NUMERIC_CHAR_ONLY);
			regexStr = std::regex(TROWA_REGEX_NUMERIC_STR_ONLY);
			regexInvalidChar = std::regex(TROWA_REGEX_NUMERIC_CHAR_NOT);
			break;
		case TextType::IpAddress:
			regexChar = std::regex(TROWA_REGEX_IP_CHAR);
			regexStr = std::regex(TROWA_REGEX_IP_ADDRESS);
			regexInvalidChar = std::regex(TROWA_REGEX_IP_CHAR_NOT);
			break;
		case TextType::Any:
		default:
			break;
		}
		return;
	}
	// Remove invalid chars.
	std::string cleanseString(std::string newText);
	void draw(NVGcontext *vg) override;
	void onKey(EventKey &e) override;
	void insertText(std::string newText);
	void onTextChange() override;
	// Request focus on this field from the Rack engine.
	void requestFocus();
}; // end struct TSTextField




#endif // end if not defined
