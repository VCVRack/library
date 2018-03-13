///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////// KlokSpid is a 8 HP module, mainly designed to divide/multiply an external clock frequency (clock      //////
////// modulator), also it can work as standalone (BPM-based) clock generator.                               //////
////// - 2 input ports:                                                                                      //////
//////   - external clock (CLK), to work as divider/multiplier (standalone clock generator if not patched).  //////
//////   - multipurpose CV-RATIO/TRIG. port:                                                                 //////
//////     - when running as clock multiplier/divider: CV-controllable ratio (full range /64 to x64).        //////
//////     - when running as BPM-clock generator: trigger input (BPM start/stop, or BPM reset).              //////
////// - Outputs: 4 identical triggers/gates (default Gate 1/4, +5V high, 0V low).                           //////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Ohmer.hpp"
#include <dsp/digital.hpp>
#include <cstring>

struct KlokSpidModule : Module {
	enum ParamIds {
		PARAM_KNOB,
    PARAM_BUTTON,
		NUM_PARAMS
	};
	enum InputIds {
		INPUT_CLOCK,
		INPUT_CV_TRIG,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_1,
		OUTPUT_2,
		OUTPUT_3,
		OUTPUT_4,
		NUM_OUTPUTS
	};
	enum LightIds {
		LED_CLK,
		LED_OUTPUT,
		LED_CV_TRIG,
		LED_CVMODE,
		LED_TRIGMODE,
		LED_SYNC_GREEN,
		LED_SYNC_RED,
		NUM_LIGHTS
	};

	//// GENERAL PURPOSE VARIABLES/FLAGS/TABLES.

	// runAtFirstStep: "true" on early first step, then must stay "false".
	bool runAtFirstStep = true;
	// Real clock ratios list (array). Based on preset ratios while KlokSpid module runs as clock multiplier/divider, by knob position only.
	float list_fRatio[25] = {64.0f, 32.0f, 16.0f, 10.0f, 9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.5f, 1.0f/3.0f, 0.25f, 0.2f, 1.0f/6.0f, 1.0f/7.0f, 0.125f, 1.0f/9.0f, 0.1f, 0.0625f, 0.03125f, 0.015625f};

	//// MODEL (GUI THEME).

	// Current selected KlokSpid model (aka "theme"), from 0 to 3.
	unsigned int moduleTheme = 0; // Thanks to Arthur & Bernd for excellent suggestion, thanks to Dale and his Topograph using this feature (but for 0.5.x). For 0.6.0 conversion, mostly based from Audible Instruments' Tides.
	// DMD-font color (default is "Classic" beige model).
	NVGcolor DMDtextColor = nvgRGB(0x08, 0x08, 0x08);

	//// STEP-RELATED (REALTIME) COUNTERS/GAPS.

	// Step related variables: used to determine the frequency of source signal, and when KlokSpid must sends relevant pulses to output(s).
	unsigned long currentStep = 0;
	unsigned long previousStep = 0;
	unsigned long nextPulseStep = 0;
	unsigned long nextExpectedStep = 0;
	unsigned long stepGap = 0;
	unsigned long stepGapPrevious = 0;

	// Current ports states, voltages on input ports, knob position and button state.
	bool activeCLK = false;
	bool activeCLKPrevious = false;
	bool activeCV = false;
	float voltageOnCV = 0.0f;
	float knobPosition = 12.0f; // Assuming knob is, by default, centered (12.0f).
	unsigned int rateRatioKnob = 12; // Assuming knob is, by default, centered (12.0f), also for ratio.
	bool buttonPressed = false;

	// Clock modulator modes.
	enum ClkModModeIds {
		X1,	// work at x1.
		DIV,	// divider mode.
		MULT	// muliplier mode.
	};

	// Clock modulator mode, assuming default is X1.
	unsigned int clkModulMode = X1;

	//// SCHMITT TRIGGERS.

	// Schmitt trigger to check thresholds on CLK input connector.
	SchmittTrigger CLKInputPort;
	// Schmitt trigger to handle BPM start/stop state (only when KlokSpid is acting as clock generator) via button.
	SchmittTrigger runButton;
	// Schmitt trigger to handle the start/stop toggle button (also used for SETUP to confirm menu/parameter) - via CV/TRIG input port (if configured as "Start/Stop").
	SchmittTrigger runTriggerPort;

	//// RATIO-BY-CV VARIABLES/FLAGS.

	// Incoming CV may be bipolar (true) or unipolar (false).
	bool bipolarCV = true;
	// Is CV used to modulate ratio?
	bool isRatioCVmod = false;
	// Real ratio, given by current CV voltage.
	float rateRatioCV = 0.0f;
	// Real ratio, given by current CV voltage, integer is required only for display into DMD (to avoid "decimals" cosmetic issues, at the right side of DMD!).
	int rateRatioCVi = 0;

	//// BPM-RELATED VARIABLES (STANDALONE CLOCK GENERATOR).

	// Default BPM (when KlokSpid is acting as clock generator). Default is 120 BPM (centered knob).
	unsigned int BPM = 120;
	// Previous registed BPM (when KlokSpid is acting as clock generator), from previous step.
	unsigned int previousBPM = 120;
	// Indicates if "CV-RATIO/TRIG." input port (used as trigger, standalone BPM-clock mode only) is a transport trigger (true = toggle start/stop) or reset (false, default) useful for "re-sync" between modules.
	bool transportTrig = false;
	// Standalone clock generator mode only: indicates if BPM is running or stopped.
	bool isBPMRunning = true;
	bool runBPMOnInit = true;

	//// SETUP-RELATED VARIABLES/TABLES.

	// Enumeration of SETUP menu entries.
	enum setupMenuEntries {
		SETUP_MENU_WELCOME,	// SETUP menu entry for #0 is always dedicated to welcome message ("- SETUP -") displayed on DMD.
		SETUP_MENU_CVPOLARITY,	// SETUP menu entry for CV polarity (bipolar, or unipolar).
		SETUP_MENU_DURATION,	// SETUP menu entry for pulse duration (fixed and gate-based parameters).
		SETUP_MENU_OUTVOLTAGE,	// SETUP menu entry for output voltage.
		SETUP_MENU_CVTRIG,	// SETUP menu entry describing how CV/TRIG input port is working (as start/stop toggle, or as "RST" input).
		SETUP_MENU_EXIT,	// Lastest menu entry is always used to exit SETUP (options are "Save/Exit", "Canc/Exit", "Review" or "Factory").
		NUM_SETUP_ENTRIES // This position indicates how many entries the KlokSpid's SETUP menu have.
	};

	// This flag indicates if KlokSpid module is currently running SETUP, or not.
	bool isSetupRunning = false;
	// This flag indicates if KlokSpid module is entering SETUP (2 seconds delay), or not.
	bool isEnteringSetup = false;
	// This flag indicates if KlokSpid module is exiting SETUP (2 seconds delay), or not.
	bool isExitingSetup = false;
	// This flag is designed to avoid continuous SETUP entries/exits while button is continously held.
	bool allowedButtonHeld = false;
	// Item index (edited parameter number).
	unsigned int setup_ParamIdx = 0;
	// Current edited value for selected parameter.
	unsigned int setup_CurrentValue = 0;
	// Table containing number of possible values for each parameter.
	unsigned int setup_NumValue[NUM_SETUP_ENTRIES] = {0, 2, 9, 4, 2, 4};
	// Default factory values for each parameter.
	unsigned int setup_Factory[NUM_SETUP_ENTRIES] = {0, 0, 3, 0, 1, 1};
	// Table containing current values for all parameters.
	unsigned int setup_Current[NUM_SETUP_ENTRIES] = {0, 0, 3, 0, 1, 1};
	// Table containing edited parameters during SETUP (will be filled when entering SETUP).
	unsigned int setup_Edited[NUM_SETUP_ENTRIES] = {0, 0, 0, 0, 0, 1};
	// Table containing backup edited parameters during SETUP (will be filled when entering SETUP).
	unsigned int setup_Backup[NUM_SETUP_ENTRIES] = {0, 0, 0, 0, 0, 1};
	// Saved previous knob (used during SETUP to detect increment/decrement).
	// Unfortuntately, in VCV Rack v0.5.1, knob can't be defined as continuous encoder (too nad!) - see https://github.com/VCVRack/Rack/issues/687
	int paramPreviousKnob = 0;
	// Counter (as "delay") used to enter and (optionally) to saved/exit SETUP quickly on long press.
	unsigned long setupCounter = 0;
	// When running SETUP, this flag is used to alternate display on DMD the parameter name (true) and its value (false), every 2.5 s.
	bool setupShowParameterName = false;
	// Counter (as "delay") used swap (alternate) parameter name and parameter value during SETUP.
	unsigned long setupSwapMsgDelay = 0;

	//// PULSE TO OUTPUT RELATED VARIABLES AND PULSE GENERATORS.

	// Enumeration of possible pulse durations: fixed 1 ms, fixed 2 ms, fixed 5 ms, Gate 1/4, Gate 1/3, Square, Gate 2/3, Gate 3/4, Gate 95%.
	enum PulseDurations {
		FIXED1MS,	// Fixed 1 ms.
		FIXED2MS,	// Fixed 2 ms.
		FIXED5MS,	// Fixed 5 ms.
		GATE25,	// Gate 1/4 (25%).
		GATE33,	// Gate 1/3 (33%).
		SQUARE,	// Square waveform.
		GATE66,	// Gate 2/3 (66%).
		GATE75,	// Gate 3/4 (75%).
		GATE95,	// Gate 95%.
	};

	// Pulse counter for divider mode (set at max divider value, minus 1).
	unsigned int pulseDivCounter = 63;
	// Pulse counter for multiplier mode, to avoid continuous pulse when no more receiving (set at max divider value, minus 1). Kind of "timeout".
	unsigned int pulseMultCounter = 0;
	// Main pulse generator, to send "pulses" to output ports.
	PulseGenerator sendPulse;
	// These flags are related to "sendPulse" pulse generator (current pulse state).
	bool sendingOutput = false;
	// This flag indicates if sending pulse (to all outputs ports) is allowed (true) or not (false).
	bool canPulse = false;
	// Current pulse duration (time in second). Default is fixed 1 ms at start. Operational can be changed via SETUP.
	float pulseDuration = 0.001f;
	// Extension of "pulseDuration" value (for square and gate modes), set as Gate 1/4 (25 %) by default, can be changed via SETUP.
	unsigned int pulseDurationExt = GATE25;
	// Voltage for outputs (pulses/gates), default is +5V, can be changed to +10V, +12V (+11.7V) or +2V instead, via SETUP.
	float outVoltage = 5.0f;

	//// LEDS AFTERGLOW.

	// Counter used for red CLK LED afterglow (used together with "ledClkAfterglow" boolean flag).
	unsigned long ledClkDelay = 0; // long is required for highest engine samplerates!
	// Counter used for green OUTPUT LED afterglow (used together with "ledOutAfterglow" boolean flag).
	unsigned long ledOutDelay = 0; // long is required for highest engine samplerates!
	// This flag controls CLK (red) LED afterglow (active or not).
	bool ledClkAfterglow = false;
	// This flag controls OUTPUTS (green) LED afterglow (active or not).
	bool ledOutAfterglow = false;

	//// SYNC STATUS.

	// Assuming clock generator isn't synchronized (sync'd) with source clock on initialization.
	bool isSync = false;

	// Module interface definitions, such parameters (knob, button), input ports, output ports and LEDs.
	KlokSpidModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

	// v0.6 ONLY (must be disabled/commented for v0.5) - inhibit "Initialize" from context-menu / Ctrl+I / Command+I keyboard shortcut over module.
	void onReset() override {
		return;
	}

	// v0.6 ONLY (must be disabled/commented for v0.5) - inhibit "Randomize" from context-menu / Ctrl+R / Command+R keyboard shortcut over module.
	void onRandomize() override {
		return;
	}

	// Module's DSP method.
	void step() override;

	// This custom function applies current settings (useful after SETUP operation, also "on-the-fly" altered parameter during Setup - useful to experiment).
	void UpdateKlokSpidSettings(bool allowJsonUpdate) {
		// SETUP parameter SETUP_MENU_CVPOLARITY: CV polarity (bipolar or unipolar CV-Ratio).
		bipolarCV = (setup_Current[SETUP_MENU_CVPOLARITY] == 0); // json persistence (only if SETUP isn't running).
		if (allowJsonUpdate)
			this->bipolarCV = (setup_Current[SETUP_MENU_CVPOLARITY] == 0); // json persistence (only if SETUP isn't running).
		// SETUP parameter SETUP_MENU_DURATION: possible pulse durations (1 ms, 2 ms, 5 ms, Gate 1/4, Gate 1/3, Square, Gate 2/3, Gate 3/4, Gate 95%). Keept for compatibility with v0.5.2 .vcv patches!
		switch (setup_Current[SETUP_MENU_DURATION]) {
			case FIXED1MS:
				pulseDuration = 0.001f;
				break;
			case FIXED2MS:
				pulseDuration = 0.002f;
				break;
			case FIXED5MS:
				pulseDuration = 0.005f;
		}
		// Extension for pulse duration parameter (it's a kind of "descriptor" for non-fixed durations).
		pulseDurationExt  = setup_Current[SETUP_MENU_DURATION];
		if (allowJsonUpdate)
			this->pulseDurationExt  = setup_Current[SETUP_MENU_DURATION]; // json persistence (only if SETUP isn't running).
		// SETUP parameter SETUP_MENU_OUTVOLTAGE: output voltage: +2V, +5V, +10V or +12V (+11.7V).
		switch (setup_Current[SETUP_MENU_OUTVOLTAGE]) {
			case 0:
				outVoltage = 5.0f;
				if (allowJsonUpdate)
					this->outVoltage = 5.0f; // First setting is +5V, also factory (default) setting. json persistence (only if SETUP isn't running).
				break;
			case 1:
				outVoltage = 10.0f;
				if (allowJsonUpdate)
					this->outVoltage = 10.0f; // Second setting is +10V. json persistence (only if SETUP isn't running).
				break;
			case 2:
				outVoltage = 11.7f;
				if (allowJsonUpdate)
					this->outVoltage = 11.7f; // Third setting is +12V (real +11.7 V). json persistence (only if SETUP isn't running).
				break;
			case 3:
				outVoltage = 2.0f;
				if (allowJsonUpdate)
					this->outVoltage = 2.0f; // Last setting (introduced into v0.5.5/v0.6.0.4-beta): +2V. json persistence (only if SETUP isn't running).
		}
		// SETUP parameter SETUP_MENU_CVTRIG: CV-RATIO/TRIG. input port behavior (standalone clock generator only, this port is TRIG.).
		// - "true" is meaning the TRIG. input port acts as "start/stop toggle".
		// - "false" is meaning the TRIG. input port acts as "BPM reset" (useful to "re-sync" BPM from an external/reference source clock, for example).
		transportTrig = (setup_Current[SETUP_MENU_CVTRIG] == 0);
		if (allowJsonUpdate)
			this->transportTrig = (setup_Current[SETUP_MENU_CVTRIG] == 0); // json persistence (only if SETUP isn't running).
	}

	// This custom function returns pulse duration (ms), regardling number of samples (unsigned long int) and pulsation duration parameter (SETUP).
	float GetPulsingTime(unsigned long int stepGap, float rate) {
		float pTime = 0.001; // As default "degraded-mode/replied" pulse duration is set to 1ms (also can be forced to "fixed 1ms" via SETUP).
		if (stepGap == 0) {
			// No reference duration (number of samples is zero).
			switch (setup_Current[SETUP_MENU_DURATION]) {
				case FIXED2MS:
					pTime = 0.002f;	// Fixed 2 ms pulse.
					break;
				case FIXED5MS:
					pTime = 0.005f;	// Fixed 5 ms pulse.
			}
		}
		else {
			// Reference duration in number of samples (when known stepGap). Variable-length pulse duration can be defined.
			switch (setup_Current[SETUP_MENU_DURATION]) {
				case FIXED2MS:
					pTime = 0.002f;	// Fixed 2 ms pulse.
					break;
				case FIXED5MS:
					pTime = 0.005f;	// Fixed 5 ms pulse.
					break;
				case GATE25:
					pTime = rate * 0.25f * (stepGap / engineGetSampleRate());	// Gate 1/4 (25%)
					break;
				case GATE33:
					pTime = rate * (1.0f / 3.0f) * (stepGap / engineGetSampleRate());	// Gate 1/3 (33%)
					break;
				case SQUARE:
					pTime = rate * 0.5f * (stepGap / engineGetSampleRate());	// Square wave (50%)
					break;
				case GATE66:
					pTime = rate * (2.0f / 3.0f) * (stepGap / engineGetSampleRate());	// Gate 2/3 (66%)
					break;
				case GATE75:
					pTime = rate * 0.75f * (stepGap / engineGetSampleRate());	// Gate 3/4 (75%)
					break;
				case GATE95:
					pTime = rate * 0.95f * (stepGap / engineGetSampleRate());	// Gate 95%
			}
		}
		return pTime;
	}

	// Persistence for extra datas via json functions (in particular parameters defined via KlokSpid's SETUP, also BPM state).
	// These extra datas are saved to .vcv file (including "autosave.vcv") also are "transfered" when you duplicate the module.

	json_t *toJson() override	{
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "moduleTheme", json_integer(moduleTheme));
		json_object_set_new(rootJ, "bipolarCV", json_boolean(bipolarCV));
		json_object_set_new(rootJ, "pulseDurationExt", json_integer(pulseDurationExt));
		json_object_set_new(rootJ, "outVoltage", json_real(outVoltage));
		json_object_set_new(rootJ, "transportTrig", json_boolean(transportTrig)); // CV-RATIO/TRIG. port may be used as BPM "start/stop" toggle or as BPM-reset. BPM-reset is default factory (false).
		json_object_set_new(rootJ, "runBPMOnInit", json_boolean(runBPMOnInit));
		return rootJ;
	}

	void fromJson(json_t *rootJ) override	{
		// Retrieving module theme/variation (when loading .vcv and cloning module).
		json_t *moduleThemeJ = json_object_get(rootJ, "moduleTheme");
		if (moduleThemeJ)
			moduleTheme = json_integer_value(moduleThemeJ);
		// Retrieving bipolar or unipolar mode (for CV when running as clock multiplier/divider).
		json_t *bipolarCVJ = json_object_get(rootJ, "bipolarCV");
		if (bipolarCVJ)
			bipolarCV = json_is_true(bipolarCVJ);
		// Retrieving pulse duration "mode" data. Introducted since v0.5.3. Thanks Yoann for this idea!
		json_t *pulseDurationExtJ = json_object_get(rootJ, "pulseDurationExt");
		if (pulseDurationExtJ)
			pulseDurationExt = json_integer_value(pulseDurationExtJ);
		// Retrieving output voltage data (real/float value).
		json_t *outVoltageJ = json_object_get(rootJ, "outVoltage");
		if (outVoltageJ)
			outVoltage = json_real_value(outVoltageJ);
		// Retrieving usage of TRIG. input port: start/stop toggle (true) or BPM-reset (false).
		json_t *transportTrigJ = json_object_get(rootJ, "transportTrig");
		if (transportTrigJ)
			transportTrig = json_is_true(transportTrigJ);
		// Retrieving last saved BPM-clocking state (it was running or stopped).
		json_t *runBPMOnInitJ = json_object_get(rootJ, "runBPMOnInit");
		if (runBPMOnInitJ)
			runBPMOnInit = json_is_true(runBPMOnInitJ);
	}
};

void KlokSpidModule::step() {
	// step() function is the right place for DSP processing!

	// Doing some initializations once, at early first step.
	if (runAtFirstStep) {
		// Depending current KlokSpid model (theme), set the relevant DMD-text color.
		switch (moduleTheme) {
			case 0:
			case 1:
			case 2:
				DMDtextColor = nvgRGB(0x08, 0x08, 0x08); // LCD-like for Classic, Stage Repro or Absolute Night.
				break;
			case 3:
				DMDtextColor = nvgRGB(0xe0, 0xe0, 0xff); // Blue plasma-like for Dark "Signature".
				break;
			case 4:
				DMDtextColor = nvgRGB(0xff, 0x8a, 0x00); // Orange plasma-like for Deepblue "Signature".
				break;
			case 5:
				DMDtextColor = nvgRGB(0xb0, 0xff, 0xff); // Light cyan plasma-like for Carbon "Signature".
		}
		// Filling table containing current SETUP parameters.
		// SETUP parameter SETUP_MENU_CVPOLARITY: bipolar or unipolar CV.
		setup_Current[SETUP_MENU_CVPOLARITY] = bipolarCV ? 0 : 1;
		// SETUP parameter SETUP_MENU_DURATION: Pulse duration (extended, to keep compatibility with previous v0.5.2).
		// Parameter #2: possible pulse durations (fixed 1 ms, 2 ms or 5 ms durations, Gate 1/4, Gate 1/3, Square, Gate 2/3, Gate 3/4, Gate 95%).
		setup_Current[SETUP_MENU_DURATION] = pulseDurationExt;
		switch (pulseDurationExt) {
			case FIXED1MS:
				pulseDuration = 0.001f;
				break;
			case FIXED2MS:
				pulseDuration = 0.002f;
				break;
			case FIXED5MS:
				pulseDuration = 0.005f;
				break;
			default:
				pulseDuration = 0.001f; // It's a default value, but gates are defined in realtime (later).
		}
		// If output voltage is above +11V, assuming +11.7V.
		if (round(outVoltage * 10) > 110)
			outVoltage = 11.7f;
		// Assuming +5V is default output voltage.
		setup_Current[SETUP_MENU_OUTVOLTAGE] = 0; // +5V.
		// SETUP parameter SETUP_MENU_OUTVOLTAGE: Output voltage.
		if (round(outVoltage * 10) == 20)
			setup_Current[SETUP_MENU_OUTVOLTAGE] = 3; // +2V. Lastest value (instead of "inserted" at first, to preserve compatibility!).
			else if (round(outVoltage * 10) == 100)
				setup_Current[SETUP_MENU_OUTVOLTAGE] = 1; // +10V.
				else if (round(outVoltage * 10) == 117)
					setup_Current[SETUP_MENU_OUTVOLTAGE] = 2; // +11.7V (indicated +12V in module's SETUP).
		// SETUP parameter SETUP_MENU_CVTRIG: CV/TRIG port, as trigger input when running as standalone clock generator (only).
		setup_Current[SETUP_MENU_CVTRIG] = transportTrig ? 0 : 1;
		// Parameter's value is, by default 1 for default "Save/Exit".
		setup_Current[SETUP_MENU_EXIT] = 1;
		// Is standalone clock is running at init, or not (previous state).
		isBPMRunning = this->runBPMOnInit;
		////// CAUTION -- FOUR LINES BELOW ARE APPLICABLE ONLY FOR V0.5.X, SCHMITT TRIGGER RELATED LINES BELOW MUST BE DISABLED/SET AS COMMENTS FOR V0.6 (DEPRECATED, but why?)
		// v0.5.x only: high thresholds are set to +1.7V, low thresholds are set at +0.2 V (following https://vcvrack.com/manual/VoltageStandards.html).
		//CLKInputPort.reset();
		//CLKInputPort.setThresholds(0.2, 1.7);
		//runTriggerPort.reset();
		//runTriggerPort.setThresholds(0.2, 1.7);
		// This section is executed once, at early step().
		runAtFirstStep = false;
		// Exit this step.
		return;
	}

	// Current knob position.
	knobPosition = params[PARAM_KNOB].value;

	// Current state of CLK port.
	activeCLK = inputs[INPUT_CLOCK].active;

	if (activeCLK != activeCLKPrevious) {
		// Is state was changed (added or removed a patch cable to/away CLK port)?
		// New state will become "previous" state.
		activeCLKPrevious = activeCLK;
		// Reset all steps counter and "gaps", not synchronized.
		currentStep = 0;
		previousStep = 0;
		nextPulseStep= 0;
		nextExpectedStep = 0;
		stepGap = 0;
		stepGapPrevious = 0;
		isSync = false;
		canPulse = false;
	}

	// Current state and voltage (CV/TRIG port).
	activeCV = inputs[INPUT_CV_TRIG].active;

	// Considering CV (if applicable e.g. wired!).
	voltageOnCV = 0.0f;
	isRatioCVmod = false;
	rateRatioCV = 0.0f;

	if (activeCV) {
		voltageOnCV = inputs[INPUT_CV_TRIG].value;
		if (activeCLK) {
			// Considering CV-RATIO signal to modulate ratio (doesn't matter if SETUP is running, or not).
			isRatioCVmod = true;
			if (bipolarCV)
				rateRatioCV = round(clamp(static_cast<float>(voltageOnCV), -5.0f, 5.0f) * 12.6f); // By bipolar voltage (-5V/+5V).
				else rateRatioCV = round((clamp(static_cast<float>(voltageOnCV), 0.0f, 10.0f) - 5.0f) * 12.6f); // By unipolar voltage (0V/+10V).
			// Required to display ratio without artifacts!
			rateRatioCVi = static_cast<int>(rateRatioCV);
			if (round(rateRatioCV) == 0.0f) {
				clkModulMode = X1;
				rateRatioCV = 1.0f; // Real ratio becomes... 1.0f because it's multiplied by 1.
			}
			else if (round(rateRatioCV) > 0.0f) {
				clkModulMode = MULT;
				rateRatioCV = round(rateRatioCV + 1.0f);
			}
			else {
				clkModulMode = DIV;
				rateRatioCV = 1.0f / round(1.0f - rateRatioCV);
			}
		}
		else {
			// Backup cde was added here too, because when TRIG. port is connected, previously you'll can't change BPM via knob! (lastest-minute discovered issue).
			// BPM is set by knob (except while SETUP is running).
			if (!isSetupRunning) {
				BPM = round(knobPosition * 10);
				if (BPM < 10)
					BPM = 10; // Minimum BPM is 10.
			}
		}
	}
	else {
		if (!isSetupRunning) {
			if (activeCLK) {
				// Ratio is controled by knob.
				rateRatioKnob = round(knobPosition);
				// Related multiplier/divider mode.
				clkModulMode = DIV;
				if (rateRatioKnob == 12)
					clkModulMode = X1;
					else if (rateRatioKnob > 12)
						clkModulMode = MULT;
			}
			else {
				// BPM is set by knob.
				BPM = round(knobPosition * 10);
				if (BPM < 10)
					BPM = 10; // Minimum BPM is 10.
			}
		}
	}

	// Button state.
	buttonPressed = runButton.process(params[PARAM_BUTTON].value);

	// KlokSpid is working as multiplier/divider module (when CLK input port is connected - aka "active").
	if (activeCLK) {
		// Increment step number.
		currentStep++;
		// Folling block is designed to avoid possible variable "overflows" on 32-bit unsigned integer variables, simply by "shifting" all of them!
		// Long unsigned integers overfows at max. 32-bit value (4,294,967,296) may occur after... 6h 12min 49sec @ 192000 Hz, or 27h 3min 11sec @ 44100 Hz lol.
		if ((currentStep > 4000000000) && (previousStep > 4000000000)) {
			if (nextExpectedStep > currentStep) {
				nextExpectedStep -= 3999500000;
				if (nextPulseStep > currentStep)
					nextPulseStep -= 3999500000;
			}
			currentStep -= 3999500000;
			previousStep -= 3999500000;
		}

		// Using Schmitt trigger (SchmittTrigger is provided by dsp/digital.hpp) to detect thresholds from CLK input connector. Calibration: +1.7V (rising edge), low +0.2V (falling edge).
		if (CLKInputPort.process(rescale(inputs[INPUT_CLOCK].value, 0.2f, 1.7f, 0.0f, 1.0f))) {
			// CLK input is receiving a compliant trigger voltage (rising edge): lit and "afterglow" CLK (red) LED.
			ledClkDelay = 0;
			ledClkAfterglow = true;

			if (previousStep == 0) {
				// No "history", it's the first pulse received on CLK input after a frequency change. Not synchronized.
				nextExpectedStep = 0;
				stepGap = 0;
				stepGapPrevious = 0;
				// stepGap at 0: the pulse duration will be 1 ms (default), or 2 ms or 5 ms (depending SETUP). Variable pulses can't be used as long as frequency remains unknown.
				if (isRatioCVmod)
					pulseDuration = GetPulsingTime(0, 1.0f / rateRatioCV);  // Ratio is CV-controlled.
					else pulseDuration = GetPulsingTime(0, list_fRatio[rateRatioKnob]);  // Ratio is controlled by knob.
				// Not synchronized.
				isSync = false;
				pulseDivCounter = 0; // Used for DIV mode exclusively!
				pulseMultCounter = 0; // Used for MULT mode exclusively!
				canPulse = (clkModulMode != MULT); // MULT needs second pulse to establish source frequency.
				previousStep = currentStep;
			}
			else {
				// It's the second pulse received on CLK input after a frequency change.
				stepGapPrevious = stepGap;
				stepGap = currentStep - previousStep;
				nextExpectedStep = currentStep + stepGap;
				// The frequency is known, we can determine the pulse duration (defined by SETUP).
				// The pulse duration also depends of clocking ratio, such "X1", multiplied or divided, and its ratio.
				if (isRatioCVmod)
					pulseDuration = GetPulsingTime(stepGap, 1.0f / rateRatioCV); // Ratio is CV-controlled.
					else pulseDuration = GetPulsingTime(stepGap, list_fRatio[rateRatioKnob]); // Ratio is controlled by knob.
				isSync = true;
				if (stepGap > stepGapPrevious)
					isSync = ((stepGap - stepGapPrevious) < 2);
					else if (stepGap < stepGapPrevious)
						isSync = ((stepGapPrevious - stepGap) < 2);
				if (isSync)
					canPulse = (clkModulMode != DIV);
					else canPulse = (clkModulMode == X1);
				previousStep = currentStep;
			}

			switch (clkModulMode) {
				case X1:
					// Ratio is x1, following source clock, the easiest scenario! (always sync'd).
					canPulse = true;
					break;
				case DIV:
					// Divider mode scenario.
					if (pulseDivCounter == 0) {
						if (isRatioCVmod)
							pulseDivCounter = int(1.0f / rateRatioCV) - 1; // Ratio is CV-controlled.
							else pulseDivCounter = int(list_fRatio[rateRatioKnob] - 1); // Ratio is controlled by knob.
						canPulse = true;
					}
					else {
						pulseDivCounter--;
						canPulse = false;
					}
					break;
				case MULT:
					// Multiplier mode scenario: pulsing only when source frequency is established.
					if (isSync) {
						// Next step for pulsing in multiplier mode.
						if (isRatioCVmod) {
							// Ratio is CV-controlled.
							nextPulseStep = currentStep + round(stepGap / rateRatioCV);
							pulseMultCounter = int(rateRatioCV) - 1;
						}
						else {
						// Ratio is controlled by knob.
							nextPulseStep = currentStep + round(stepGap * list_fRatio[rateRatioKnob]);
							pulseMultCounter = round(1.0f / list_fRatio[rateRatioKnob]) - 1;
						}
						canPulse = true;
					}
			}
		}
		else {
			// At this point, it's not a rising edge!

			// When running as multiplier, may pulse here too during low voltages on CLK input!
			if (isSync && (nextPulseStep == currentStep) && (clkModulMode == MULT)) {
				if (isRatioCVmod)
					nextPulseStep = currentStep + round(stepGap / rateRatioCV); // Ratio is CV-controlled.
					else nextPulseStep = currentStep + round(stepGap * list_fRatio[rateRatioKnob]); // Ratio is controlled by knob.
				// This block is to avoid continuous pulsing if no more receiving incoming signal.
				if (pulseMultCounter > 0) {
					pulseMultCounter--;
					canPulse = true;
				}
				else {
					canPulse = false;
					isSync = false;
				}
			}
		}
	}
	else {
		// CLK input port isn't connected - aka "not active".
		// From this point, KlokSpid is working as standalone BPM-based clock generator.
		ledClkAfterglow = false;
		if (previousBPM == BPM) {
			// CV-RATIO/TRIG. input port is used as TRIG. to reset clock generator or to toggle BPM-clocking, while voltage is +1.7 V (or above) - rising edge.
			if (activeCV) {
				if (runTriggerPort.process(rescale(voltageOnCV, 0.2f, 1.7f, 0.0f, 1.0f))) {
					// On +1.7 V trigger (rising edge), the clock generator state if toggled (started or stopped).
					if (transportTrig) {
						// CV-RATIO/TRIG. input port (TRIG.) is configured as "play/stop toggle".
						isBPMRunning = !isBPMRunning;
						// BPM state persistence (json).
						this->runBPMOnInit = isBPMRunning;
						if (isBPMRunning) {
							currentStep = 0;
							nextPulseStep = 0;
						}
						else ledOutAfterglow = false;
					}
					else {
						// CV-RATIO/TRIG. input port (TRIG.) is configured as RESET input (default factory): it's a BPM-reset.
						currentStep = 0;
						nextPulseStep = 0;
					}
				}
			}
			// Incrementing step counter...
			currentStep++;
			if (currentStep >= nextPulseStep) {
				// Current step is greater than... next step: senting immediate pulse (if unchanged BPM by knob).
				nextPulseStep = currentStep;
				canPulse = true;
			}

			if (isBPMRunning) {
				if (canPulse) {
					// Setting pulse...
					// Define the step for next pulse. Time reference is given by (current) engine samplerate setting.
					nextPulseStep = round(60.0f * engineGetSampleRate() / BPM);
					// Define the pulse duration (fixed or variable-length).
					pulseDuration = GetPulsingTime(engineGetSampleRate(), 60.0f / BPM);
					currentStep = 0;
				}
			}
			else {
				// BPM clock is stopped.
				canPulse = false;
				currentStep = 0;
				nextPulseStep = 0;
				ledOutAfterglow = false;
			}
		}
		previousBPM = BPM;
	}

	// Using pulse generator to output to all ports.
	if (canPulse) {
		canPulse = false;
		// Sending pulse, using pulse generator.
		sendPulse.pulseTime = pulseDuration;
  	sendPulse.trigger(pulseDuration);
		// OUTPUTS LED.
		ledOutDelay = 0;
		ledOutAfterglow = true;
	}
	sendingOutput = sendPulse.process(1.0 / engineGetSampleRate());
  outputs[OUTPUT_1].value = sendingOutput ? outVoltage : 0.0f;
  outputs[OUTPUT_2].value = sendingOutput ? outVoltage : 0.0f;
  outputs[OUTPUT_3].value = sendingOutput ? outVoltage : 0.0f;
  outputs[OUTPUT_4].value = sendingOutput ? outVoltage : 0.0f;

	// Afterglow for CLK (red) LED.
	if (ledClkAfterglow && (ledClkDelay < round(engineGetSampleRate() / 16)))
		ledClkDelay++;
		else {
			ledClkAfterglow = false;
			ledClkDelay = 0;
		}

	// Afterglow for OUTPUT (green) LED ;
	if (ledOutAfterglow && (ledOutDelay < round(engineGetSampleRate() / 16)))
		ledOutDelay++;
		else {
			ledOutAfterglow = false;
			ledOutDelay = 0;
		}

	// Handling the button (it's a momentary button, handled by a dedicated Schmitt trigger).
	// - Short presses toggles BPM clock start/stops (when released).
	// - Long press to enter SETUP.
	// - When SETUP is running, press to advance to next parameter.
	if (buttonPressed) {
		if (!isSetupRunning && !isEnteringSetup) {
			// Try to enter SETUP... starting delay counter for 2 seconds.
			isEnteringSetup = true;
			setupCounter = 0;
			allowedButtonHeld = true; // Allow to keep button held.
		}
		else if (isSetupRunning && !isExitingSetup) {
			// Try to quick save/exit SETUP... starting delay counter for 2 seconds.
			isExitingSetup = true;
			setupCounter = 0;
			// Necessary to avoid continuous entry/exit SETUP while button is held.
			allowedButtonHeld = true; // Allow to keep button held.
		}
		else allowedButtonHeld = false; // Button must be released.
	} // Don't add "else" clause from here, otherwise be sure it doesn't work!

	if (buttonPressed && isSetupRunning) {
		// SETUP is running: when (shortly) pressed, advance to next parameter.
		// Storing previous edited parameter into "edited" table prior to advance to next SETUP parameter.
		setup_Edited[setup_ParamIdx] = setup_CurrentValue;
		// Advance to next SETUP parameter.
		setup_ParamIdx++;
		// These variables are used to cycle display (parameter name, then its value).
		setupCounter = 0;
		setupShowParameterName = true;
		if (setup_ParamIdx > SETUP_MENU_EXIT) {
			// Exiting SETUP. From here, all required actions on exit SETUP (such save, cancel changes, reset to default factory etc), except "Review" option!
			switch (setup_Edited[SETUP_MENU_EXIT]) {
				case 0:
					// Cancel/Exit: all changes from SETUP are ignored (changes are cancelled).
					// all previous (backuped) will be restored (any change is ignored).
					for (int i=1; i<SETUP_MENU_EXIT; i++)
						setup_Current[i] = setup_Backup[i];
					// Restored pre-SETUP settings, so it's useless to save them as "json" persistent.
					UpdateKlokSpidSettings(false);
					break;
				case 1:
					// Save/Exit: all parameters from SETUP will be saved.
					for (int i=1; i<SETUP_MENU_EXIT; i++)
						setup_Current[i] = setup_Edited[i];
					// Using new settings (because edited are saved), so it's mandatory to save them as "json" persistent datas.
					UpdateKlokSpidSettings(true);
					break;
				case 2:
					// Review: return to first parameter (don't exit "SETUP" in this choice is selected).
					setup_ParamIdx = 1;
					setup_CurrentValue = setup_Edited[1]; // Bypass the welcome message and edit first parameter.
					setupShowParameterName = true;
					break;
				case 3:
					// Factory: restore all factory default parameters.
					for (int i=1; i<SETUP_MENU_EXIT; i++)
						setup_Current[i] = setup_Factory[i];
					// Using new settings (because restored as default factory), like "Save/Exit", it's mandatory to save them as "json" persistent datas.
					UpdateKlokSpidSettings(true);
					break;
			}
			// Exit SETUP, except if "Review" was selected.
			if (setup_Edited[SETUP_MENU_EXIT] != 2) {
				// Exit SETUP (except if "Review" was selected).
				setupCounter = 0;
				isSetupRunning = false;
			}
		}
		else if (setup_ParamIdx == SETUP_MENU_EXIT) {
			// Last default proposed parameter will be "Save and Exit" (SETUP).
			setupShowParameterName = false;
			setup_CurrentValue = 1;
		}
		else {
			// Set currently displayed (on DMD) value as current (edited) parameter.
			setup_CurrentValue = setup_Edited[setup_ParamIdx];
		}
	}

	if (runButton.isHigh()) {
		// Button is held, don't matter if SETUP is running or not.
		// Always increment the counter.
		setupCounter++;
		if (isSetupRunning && isExitingSetup) {
			if (setupCounter >= 2 * engineGetSampleRate()) {
				// Button was held during 2 seconds (while SETUP is running): now KlokSpid module exit SETUP (doing auto "Save/Exit").
				//
				isExitingSetup = false;
				setupCounter = 0;
				allowedButtonHeld = false; // Button must be released (retrigger is required).
				for (int i=0; i<SETUP_MENU_EXIT; i++)
					setup_Current[i] = setup_Edited[i];
				// Quick SETUP-exit, doing automatic "Save/exit", by this way using new settings (because edited are saved), so it's mandatory to save them as "json" persistent datas.
				UpdateKlokSpidSettings(true);
				// Clearing flag because now exit SETUP.
				isSetupRunning = false;
			}
		}
		else {
			if (isEnteringSetup && (setupCounter >= 2 * engineGetSampleRate())) {
				// Button was finally held during 2 seconds: now KlokSpid module runs its SETUP. Initializing some variables/arrays/flags first.
				//
				isEnteringSetup = false;
				setupCounter = 0;
				// Button must be released (retrigger is required).
				allowedButtonHeld = false;
				// 
				setupShowParameterName = true;
				// Menu entry #0 is used to display "- SETUP -" as welcome message (don't have parameters, so be sure "parameter" is set to 0).
				setup_Current[SETUP_MENU_WELCOME] = 0;
				// Copy current parameters (since initialization or previous SETUP) to "edited" parameters before entering SETUP.
				// Also use a "backup" table in case of "Cancel/Exit" choice.
				for (int i=0; i<SETUP_MENU_EXIT; i++) {
					setup_Backup[i] = setup_Current[i];
					setup_Edited[i] = setup_Current[i];
				}
				// Lastest menu entry is used to exit SETUP menu, by default with save.
				setup_Edited[SETUP_MENU_EXIT] = 1;
				// Select first parameter will ne displayed. In fact, the welcome message "- SETUP -" on DMD when entered SETUP.
				setup_ParamIdx = 0;
				// This flag indicates SETUP is running.
				isSetupRunning = true;
			}
		}
	}
	else {
		if (isEnteringSetup) {
			// Abort entering SETUP.
			isEnteringSetup = false;
			// Button works as BPM start/stop toggle: inverting state.
			isBPMRunning = !isBPMRunning;
			// Persistence for current BPM-state (toJson).
			this->runBPMOnInit = isBPMRunning;
		}
		else if (isSetupRunning && isExitingSetup) {
			// Abort quick exit SETUP.
			isExitingSetup = false;
		}
		setupCounter = 0;
		allowedButtonHeld = false; // button must be "retriggered", to avoid continuous entry/exit SETUP while held.
	}

	// KlokSpid module's SETUP.
	if (isSetupRunning) {
		// SETUP is running.
		if (round(knobPosition) > paramPreviousKnob) {
			// Incremented knob (turned clockwise).
			setup_CurrentValue++;
			if (setup_CurrentValue >= setup_NumValue[setup_ParamIdx])
				setup_CurrentValue = 0; // End of values list: return to first value.
			setupShowParameterName = false;
			setupSwapMsgDelay = 0;
			// Update current parameter "in realtime".
			setup_Current[setup_ParamIdx] = setup_CurrentValue;
			// Update parameters, but without "jSon" persistence while SETUP is running!
			UpdateKlokSpidSettings(false);
			// Saving new position, for next knob move.
			paramPreviousKnob = round(knobPosition);
		}
		else if (round(knobPosition) < paramPreviousKnob) {
			// Decremented knob (turned counter-clockwise).
			if (setup_NumValue[setup_ParamIdx] != 0) {
				if (setup_CurrentValue == 0)
					setup_CurrentValue = setup_NumValue[setup_ParamIdx] - 1; // Return to last possible value.
					else setup_CurrentValue--; //Previous value.
			}
			setupShowParameterName = false;
			setupSwapMsgDelay = 0;
			// Update current parameter "in realtime".
			setup_Current[setup_ParamIdx] = setup_CurrentValue;
			// Update parameters, but without "jSon" persistence while SETUP is running!
			UpdateKlokSpidSettings(false);
			// Saving new position, for next knob move.
			paramPreviousKnob = round(knobPosition);
		}
		// During SETUP, the button is used to advance to next parameter.
	  if (runButton.process(params[PARAM_BUTTON].value))
			setupSwapMsgDelay = 0;
			else {
				// Increment counter for swap message delay.
				setupSwapMsgDelay++;
				if (setupSwapMsgDelay >= (2 * engineGetSampleRate()))
				{
					// Alternate (swap) the message displayed on DMD (changed every 2 seconds): either the parameter name, or it's current setting.
					setupShowParameterName = !setupShowParameterName;
					// ...and reset its counter for next message swap.
					setupSwapMsgDelay = 0;
				}
				if (setup_ParamIdx == SETUP_MENU_EXIT)
				{
					// Last option are only parameters (choices) prior to exit SETUP (don't have parameter name). Force to display choices only.
					setupShowParameterName = false;
				}
			}
	}

	// Handling LEDs on KlokSpid module (at the end of step).
	bool isSyncFake = isSync || (clkModulMode == X1); // Assuming X1 is always "in sync", without affecting real sync status!
  lights[LED_SYNC_GREEN].value = ((activeCLK && isSyncFake) || (!activeCLK && isBPMRunning)) ? 1.0 : 0.0; // Unique "SYNC" LED: will be lit green color when sync'd / BPM is running.
  lights[LED_SYNC_RED].value = ((activeCLK && isSyncFake) || (!activeCLK && isBPMRunning)) ? 0.0 : 1.0; // Unique "SYNC" LED: will be lit red color (opposite cases).
  lights[LED_CLK].value = (isSetupRunning || ledClkAfterglow) ? 1.0 : 0.0;
  lights[LED_OUTPUT].value = (isSetupRunning || ledOutAfterglow) ? 1.0 : 0.0;
	lights[LED_CV_TRIG].value = (isSetupRunning || activeCV) ? 1.0 : 0.0; // TODO -- MUST BE ENHANCED!
	lights[LED_CVMODE].value = (isSetupRunning || activeCLK) ? 1.0 : 0.0;
	lights[LED_TRIGMODE].value = (isSetupRunning || !activeCLK) ? 1.0 : 0.0;

}	// End of KlokSpid module's step() function.


// Dot-matrix display (DMD) handler. Mainly hardcoded for best performances.
struct KlokSpidDMD : TransparentWidget {
	KlokSpidModule *module;
	std::shared_ptr<Font> font;
	KlokSpidDMD() {
		font = Font::load(assetPlugin(plugin, "res/fonts/DOTMBold.ttf"));
	}

	void updateDMD(NVGcontext *vg, Vec pos, NVGcolor DMDtextColor, bool isSetupRunning, int rateRatioKnob, bool isRatioCVmod, int rateRatioCVi, bool activeCLK, unsigned int BPM, unsigned int setup_ParamIdx, bool setupShowParameterName, unsigned int setup_CurrentValue) {
		char text[16];
		nvgFontSize(vg, 18);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -2);
		nvgFillColor(vg, nvgTransRGBA(DMDtextColor, 0xff));
		unsigned int uDMDshiftx = 24;
		if (!isSetupRunning) {
			if (activeCLK) {
				if (isRatioCVmod) {
					// Ratio is modulated by CV.
					if (rateRatioCVi >= 0) {
						rateRatioCVi = rateRatioCVi + 1;
						snprintf(text, sizeof(text), "CV-R. x%2i", rateRatioCVi);
					}
					else {
						rateRatioCVi = 1 - rateRatioCVi;
						snprintf(text, sizeof(text), "CV-R. /%2i", rateRatioCVi);
					}
					uDMDshiftx = 16; // Custom horizontal position for CV-RATIO display.
				}
				else {
					// Ratio is selected from knob.
					static const unsigned int list_iRatio[25] = {64, 32, 16, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 16, 32, 64};
					if (rateRatioKnob >= 12)
						snprintf(text, sizeof(text), "CLK x%2u", list_iRatio[rateRatioKnob]);
					else snprintf(text, sizeof(text), "CLK /%2u", list_iRatio[rateRatioKnob]);
				}
			}
			else snprintf(text, sizeof(text), "BPM %3u", BPM);
			nvgText(vg, pos.x + uDMDshiftx, pos.y - 8, text, NULL);
		}
		else {
			switch (setup_ParamIdx) {
				case 0:
					// First item displayed when is introduction message. No parameter.
					std::strcpy(text, "- SETUP -");
					break;
				case 1:
					if (setupShowParameterName) {
						// Parameter name.
						std::strcpy(text, "CV Polrty.");
					}
					else {
						// Possible values for this parameter.
						switch (setup_CurrentValue) {
							case 0:
								std::strcpy(text, "Bipolar");
								break;
							case 1:
								std::strcpy(text, "Unipolar");
						}
					}
					break;
				case 2:
					if (setupShowParameterName) {
						// Parameter name.
						std::strcpy(text, "Pulse Dur.");
					}
					else {
						// Possible values for this parameter.
						switch (setup_CurrentValue) {
							case 0:
								std::strcpy(text, "Fixed 1ms"); // FIXED1MS
								break;
							case 1:
								std::strcpy(text, "Fixed 2ms");	// FIXED2MS
								break;
							case 2:
								std::strcpy(text, "Fixed 5ms");	// FIXED5MS
								break;
							case 3:
								std::strcpy(text, "Gate 1/4");	// GATE25
								break;
							case 4:
								std::strcpy(text, "Gate 1/3");	// GATE33
								break;
							case 5:
								std::strcpy(text, "Square W.");	// SQUARE
								break;
							case 6:
								std::strcpy(text, "Gate 2/3");	// GATE66
								break;
							case 7:
								std::strcpy(text, "Gate 3/4");	// GATE75
								break;
							case 8:
								std::strcpy(text, "Gate 95%");	// GATE95
						}
					}
					break;
				case 3:
					if (setupShowParameterName) {
						// Parameter name.
						std::strcpy(text, "Out Voltg.");
					}
					else {
						// Possible values for this parameter.
						switch (setup_CurrentValue) {
							case 0:
								std::strcpy(text, "Std. +5V");
								break;
							case 1:
								std::strcpy(text, "Std. +10V");
								break;
							case 2:
								std::strcpy(text, "Max. +12V");
								break;
							case 3:
								std::strcpy(text, "Std. +2V"); // Added as index 3 (instead of "inserted" as index 0) to preserve compatibility.
						}
					}
					break;
				case 4:
					if (setupShowParameterName) {
						// Parameter name.
						std::strcpy(text, "Trig. Port");
					}
					else {
						// Possible values for this parameter.
						switch (setup_CurrentValue) {
							case 0:
								std::strcpy(text, "Play/Stop");
								break;
							case 1:
								std::strcpy(text, "Reset In.");
						}
					}
					break;
				case 5:
					// Special for exit with save, abort or restore factory parameters.
					switch (setup_CurrentValue) {
						case 0:
							std::strcpy(text, "Canc/Exit");
							break;
						case 1:
							std::strcpy(text, "Save/Exit");
							break;
						case 2:
							std::strcpy(text, "Review");
							break;
						case 3:
							std::strcpy(text, "Factory");
					}
					break;
			}
			nvgText(vg, pos.x + 14, pos.y - 8, text, NULL);
		}
	}

	void draw(NVGcontext *vg) override {
		updateDMD(vg, Vec(0, box.size.y - 150), module->DMDtextColor, module->isSetupRunning, module->rateRatioKnob, module->isRatioCVmod, module->rateRatioCVi, module->activeCLK, module->BPM, module->setup_ParamIdx, module->setupShowParameterName, module->setup_CurrentValue);
	}

};

struct KlokSpidWidget : ModuleWidget {
	// Themed plates.
	SVGPanel *panelClassic;
	SVGPanel *panelStageRepro;
	SVGPanel *panelAbsoluteNight;
	SVGPanel *panelDarkSignature;
	SVGPanel *panelDeepBlueSignature;
	SVGPanel *panelCarbonSignature;
	// Silver Torx screws.
	SVGScrew *topLeftScrewSilver;
	SVGScrew *topRightScrewSilver;
	SVGScrew *bottomLeftScrewSilver;
	SVGScrew *bottomRightScrewSilver;
	// Gold Torx screws.
	SVGScrew *topLeftScrewGold;
	SVGScrew *topRightScrewGold;
	SVGScrew *bottomLeftScrewGold;
	SVGScrew *bottomRightScrewGold;
	// Gold button.
	SVGSwitch *buttonSilver;
	SVGSwitch *buttonGold;
	//
	KlokSpidWidget(KlokSpidModule *module);
	void step() override;

	// Action for "Initialize", from context-menu, is (for now) bypassed.
	void reset() override {
	};

	// Action for "Randomize", from context-menu, is (for now) bypassed.
	void randomize() override {
	};

	Menu* createContextMenu() override;
};

KlokSpidWidget::KlokSpidWidget(KlokSpidModule *module) : ModuleWidget(module) {
	box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	// Model: Classic (beige plate, always default GUI when added from modules menu).
	panelClassic = new SVGPanel();
	panelClassic->box.size = box.size;
	panelClassic->setBackground(SVG::load(assetPlugin(plugin, "res/KlokSpid_Classic.svg")));
	addChild(panelClassic);
	// Model: Stage Repro.
	panelStageRepro = new SVGPanel();
	panelStageRepro->box.size = box.size;
	panelStageRepro->setBackground(SVG::load(assetPlugin(plugin, "res/KlokSpid_Stage_Repro.svg")));
	addChild(panelStageRepro);
	// Model: Absolute Night.
	panelAbsoluteNight = new SVGPanel();
	panelAbsoluteNight->box.size = box.size;
	panelAbsoluteNight->setBackground(SVG::load(assetPlugin(plugin, "res/KlokSpid_Absolute_Night.svg")));
	addChild(panelAbsoluteNight);
	// Model: Dark "Signature".
	panelDarkSignature = new SVGPanel();
	panelDarkSignature->box.size = box.size;
	panelDarkSignature->setBackground(SVG::load(assetPlugin(plugin, "res/KlokSpid_Dark_Signature.svg")));
	addChild(panelDarkSignature);
	// Model: Deepblue "Signature".
	panelDeepBlueSignature = new SVGPanel();
	panelDeepBlueSignature->box.size = box.size;
	panelDeepBlueSignature->setBackground(SVG::load(assetPlugin(plugin, "res/KlokSpid_Deepblue_Signature.svg")));
	addChild(panelDeepBlueSignature);
	// Model: Carbon "Signature".
	panelCarbonSignature = new SVGPanel();
	panelCarbonSignature->box.size = box.size;
	panelCarbonSignature->setBackground(SVG::load(assetPlugin(plugin, "res/KlokSpid_Carbon_Signature.svg")));
	addChild(panelCarbonSignature);
	// Always four screws for 8 HP module, may are silver or gold, depending model.
	// Top-left silver Torx screw.
	topLeftScrewSilver = Widget::create<Torx_Silver>(Vec(RACK_GRID_WIDTH, 0));
	addChild(topLeftScrewSilver);
	// Top-right silver Torx screw.
	topRightScrewSilver = Widget::create<Torx_Silver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0));
	addChild(topRightScrewSilver);
	// Bottom-left silver Torx screw.
	bottomLeftScrewSilver = Widget::create<Torx_Silver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
	addChild(bottomLeftScrewSilver);
	// Bottom-right silver Torx screw.
	bottomRightScrewSilver = Widget::create<Torx_Silver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
	addChild(bottomRightScrewSilver);
	// Top-left gold Torx screw.
	topLeftScrewGold = Widget::create<Torx_Gold>(Vec(RACK_GRID_WIDTH, 0));
	addChild(topLeftScrewGold);
	// Top-right gold Torx screw.
	topRightScrewGold = Widget::create<Torx_Gold>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0));
	addChild(topRightScrewGold);
	// Bottom-left gold Torx screw.
	bottomLeftScrewGold = Widget::create<Torx_Gold>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
	addChild(bottomLeftScrewGold);
	// Bottom-right gold Torx screw.
	bottomRightScrewGold = Widget::create<Torx_Gold>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
	addChild(bottomRightScrewGold);
	// DMD display.
	{
		KlokSpidDMD *display = new KlokSpidDMD();
		display->module = module;
		display->box.pos = Vec(0, 0);
		display->box.size = Vec(box.size.x, 234);
		addChild(display);
	}
	// Ratio/BPM knob: 12.0 is the default (x1) centered knob position/setting, 25 possible settings for ratio or BPM.
	addParam(ParamWidget::create<KlokSpid_Knob>(Vec(20, 106), module, KlokSpidModule::PARAM_KNOB, 0.0, 24.0, 12.0));
	// Push button (silver), used to toggle START/STOP, also used to enter SETUP, and to advance to next parameter in SETUP.
	buttonSilver = ParamWidget::create<KS_ButtonSilver>(Vec(94, 178), module, KlokSpidModule::PARAM_BUTTON , 0.0, 1.0, 0.0);
	addParam(buttonSilver);
	// Push button (gold), used to toggle START/STOP, also used to enter SETUP, and to advance to next parameter in SETUP.
	buttonGold = ParamWidget::create<KS_ButtonGold>(Vec(94, 178), module, KlokSpidModule::PARAM_BUTTON , 0.0, 1.0, 0.0);
	addParam(buttonGold);
	// Input ports (golden jacks).
	addInput(Port::create<PJ301M_In>(Vec(24, 215), Port::INPUT, module, KlokSpidModule::INPUT_CLOCK));
	addInput(Port::create<PJ301M_In>(Vec(72, 215), Port::INPUT, module, KlokSpidModule::INPUT_CV_TRIG));
	// Output ports (golden jacks).
	addOutput(Port::create<PJ301M_Out>(Vec(24, 262), Port::OUTPUT, module, KlokSpidModule::OUTPUT_1));
	addOutput(Port::create<PJ301M_Out>(Vec(72, 262), Port::OUTPUT, module, KlokSpidModule::OUTPUT_2));
	addOutput(Port::create<PJ301M_Out>(Vec(24, 304), Port::OUTPUT, module, KlokSpidModule::OUTPUT_3));
	addOutput(Port::create<PJ301M_Out>(Vec(72, 304), Port::OUTPUT, module, KlokSpidModule::OUTPUT_4));
	// LEDs (red for CLK input, yellow for CV-RATIO/TRIG input, green for outputs).
	addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(32, 242), module, KlokSpidModule::LED_CLK));
	addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(56, 242), module, KlokSpidModule::LED_OUTPUT));
	addChild(ModuleLightWidget::create<MediumLight<KlokSpidOrangeLight>>(Vec(80, 242), module, KlokSpidModule::LED_CV_TRIG));
	addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(7, 96), module, KlokSpidModule::LED_SYNC_GREEN)); // Unified SYNC LED (green/red).
	// Small-sized orange LEDs near CV-RATIO/TRIG input port (when lit, each LED indicates the port role).
	addChild(ModuleLightWidget::create<SmallLight<KlokSpidOrangeLight>>(Vec(67.5, 206), module, KlokSpidModule::LED_CVMODE));
	addChild(ModuleLightWidget::create<SmallLight<KlokSpidOrangeLight>>(Vec(95, 206), module, KlokSpidModule::LED_TRIGMODE));
}

//// Make one visible theme at once!

void KlokSpidWidget::step() {
	KlokSpidModule *klokspidmodule = dynamic_cast<KlokSpidModule*>(module);
	assert(klokspidmodule);
	// "Signature"-line modules are using gold parts (instead of silver).
	bool isSignatureLine = (klokspidmodule->moduleTheme > 2);
	// Themed module plate, selected via context-menu, is visible (all others are obviously, hidden).
	panelClassic->visible = (klokspidmodule->moduleTheme == 0);
	panelStageRepro->visible = (klokspidmodule->moduleTheme == 1);
	panelAbsoluteNight->visible = (klokspidmodule->moduleTheme == 2);
	panelDarkSignature->visible = (klokspidmodule->moduleTheme == 3);
	panelDeepBlueSignature->visible = (klokspidmodule->moduleTheme == 4);
	panelCarbonSignature->visible = (klokspidmodule->moduleTheme == 5);
	// Silver Torx screws are visible only for non-"Signature" modules.
	topLeftScrewSilver->visible = !isSignatureLine;
	topRightScrewSilver->visible = !isSignatureLine;
	bottomLeftScrewSilver->visible = !isSignatureLine;
	bottomRightScrewSilver->visible = !isSignatureLine;
	// Gold Torx screws are visible only for "Signature" modules.
	topLeftScrewGold->visible = isSignatureLine;
	topRightScrewGold->visible = isSignatureLine;
	bottomLeftScrewGold->visible = isSignatureLine;
	bottomRightScrewGold->visible = isSignatureLine;
	// Silver or gold button is visible at once (opposite is, obvisouly, hidden).
	buttonSilver->visible = !isSignatureLine;
	buttonGold->visible = isSignatureLine;
	// Resume original step() method.
	ModuleWidget::step();
}

//// CONTEXT-MENU (RIGHT-CLICK ON MODULE).

// Classic (default beige) module.
struct classicThemeMenuItem : MenuItem {
	KlokSpidModule *klokspidmodule;
	void onAction(EventAction &e) override {
		klokspidmodule->moduleTheme = 0;
		klokspidmodule->DMDtextColor = nvgRGB(0x08, 0x08, 0x08); // LCD-like.
	}
	void step() override {
		rightText = (klokspidmodule->moduleTheme == 0) ? "✔" : "";
		MenuItem::step();
	}
};

// Stage Repro module.
struct stageReproThemeMenuItem : MenuItem {
	KlokSpidModule *klokspidmodule;
	void onAction(EventAction &e) override {
		klokspidmodule->moduleTheme = 1;
		klokspidmodule->DMDtextColor = nvgRGB(0x08, 0x08, 0x08); // LCD-like.
	}
	void step() override {
		rightText = (klokspidmodule->moduleTheme == 1) ? "✔" : "";
		MenuItem::step();
	}
};

// Absolute Night module.
struct absolutenightThemeMenuItem : MenuItem {
	KlokSpidModule *klokspidmodule;
	void onAction(EventAction &e) override {
		klokspidmodule->moduleTheme = 2;
		klokspidmodule->DMDtextColor = nvgRGB(0x08, 0x08, 0x08); // LCD-like.
	}
	void step() override {
		rightText = (klokspidmodule->moduleTheme == 2) ? "✔" : "";
		MenuItem::step();
	}
};

// Dark "Signature" module.
struct darkSignatureThemeMenuItem : MenuItem {
	KlokSpidModule *klokspidmodule;
	void onAction(EventAction &e) override {
		klokspidmodule->moduleTheme = 3;
		klokspidmodule->DMDtextColor = nvgRGB(0xe0, 0xe0, 0xff); // Blue plasma-like.
	}
	void step() override {
		rightText = (klokspidmodule->moduleTheme == 3) ? "✔" : "";
		MenuItem::step();
	}
};

// Deepblue "Signature" module.
struct deepblueSignatureThemeMenuItem : MenuItem {
	KlokSpidModule *klokspidmodule;
	void onAction(EventAction &e) override {
		klokspidmodule->moduleTheme = 4;
		klokspidmodule->DMDtextColor = nvgRGB(0xff, 0x8a, 0x00); // Orange plasma-like.
	}
	void step() override {
		rightText = (klokspidmodule->moduleTheme == 4) ? "✔" : "";
		MenuItem::step();
	}
};

// Carbon "Signature" module.
struct carbonSignatureThemeMenuItem : MenuItem {
	KlokSpidModule *klokspidmodule;
	void onAction(EventAction &e) override {
		klokspidmodule->moduleTheme = 5;
		klokspidmodule->DMDtextColor = nvgRGB(0xb0, 0xff, 0xff); // Light cyan plasma-like.
	}
	void step() override {
		rightText = (klokspidmodule->moduleTheme == 5) ? "✔" : "";
		MenuItem::step();
	}
};

// CONTEXT-MENU CONSTRUCTION.

Menu* KlokSpidWidget::createContextMenu() {
	Menu* menu = ModuleWidget::createContextMenu();
	KlokSpidModule *klokspidmodule = dynamic_cast<KlokSpidModule*>(module);
	assert(klokspidmodule);
	menu->addChild(construct<MenuEntry>());
	menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Model:"));
	menu->addChild(construct<classicThemeMenuItem>(&classicThemeMenuItem::text, "Classic (default)", &classicThemeMenuItem::klokspidmodule, klokspidmodule));
	menu->addChild(construct<stageReproThemeMenuItem>(&stageReproThemeMenuItem::text, "Stage Repro", &stageReproThemeMenuItem::klokspidmodule, klokspidmodule));
	menu->addChild(construct<absolutenightThemeMenuItem>(&absolutenightThemeMenuItem::text, "Absolute Night", &absolutenightThemeMenuItem::klokspidmodule, klokspidmodule));
	menu->addChild(construct<darkSignatureThemeMenuItem>(&darkSignatureThemeMenuItem::text, "Dark \"Signature\"", &darkSignatureThemeMenuItem::klokspidmodule, klokspidmodule));
	menu->addChild(construct<deepblueSignatureThemeMenuItem>(&deepblueSignatureThemeMenuItem::text, "Deepblue \"Signature\"", &deepblueSignatureThemeMenuItem::klokspidmodule, klokspidmodule));
	menu->addChild(construct<carbonSignatureThemeMenuItem>(&carbonSignatureThemeMenuItem::text, "Carbon \"Signature\"", &carbonSignatureThemeMenuItem::klokspidmodule, klokspidmodule));
	return menu;
}

Model *modelKlokSpid = Model::create<KlokSpidModule, KlokSpidWidget>("Ohmer Modules", "KlokSpid", "KlokSpid", CLOCK_TAG, CLOCK_MODULATOR_TAG); // CLOCK_MODULATOR_TAG introduced in 0.6 API.
