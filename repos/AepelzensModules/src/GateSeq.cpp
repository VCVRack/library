#include "aepelzen.hpp"
#include "dsp/digital.hpp"

const int NUM_STEPS = 16;
const int NUM_CHANNELS = 8;
const int NUM_GATES = NUM_STEPS * NUM_CHANNELS;

struct GateSeq : Module {

    enum ParamIds {
	LENGTH_PARAM,
	CLOCK_PARAM,
	RUN_PARAM,
	RESET_PARAM,
	INIT_PARAM,
	COPY_PARAM,
	MERGE_PARAM,
	MERGE_MODE_PARAM,
	PATTERN_SWITCH_MODE_PARAM,
	CHANNEL_PROB_PARAM,
	BANK_PARAM = CHANNEL_PROB_PARAM + NUM_CHANNELS,
	PATTERN_PARAM = BANK_PARAM + 8,
	GATE1_PARAM = PATTERN_PARAM + 8,
	NUM_PARAMS = GATE1_PARAM + NUM_GATES + NUM_CHANNELS
    };
    enum InputIds {
	CLOCK_INPUT,
	EXT_CLOCK_INPUT,
	RESET_INPUT,
	PATTERN_INPUT,
	CHANNEL_CLOCK_INPUT,
	CHANNEL_PROB_INPUT = CHANNEL_CLOCK_INPUT + NUM_CHANNELS,
	NUM_INPUTS = CHANNEL_PROB_INPUT + NUM_CHANNELS
    };
    enum OutputIds {
	CLOCK_OUTPUT,
	GATE1_OUTPUT,
	NUM_OUTPUTS = GATE1_OUTPUT + NUM_CHANNELS
    };
    enum LightIds {
	RUNNING_LIGHT,
	RESET_LIGHT,
	COPY_LIGHT,
	MERGE_LIGHT,
	LENGTH_LIGHT,
	BANK_LIGHTS,
	PATTERN_LIGHTS = BANK_LIGHTS + 8,
	GATE_LIGHTS = PATTERN_LIGHTS + 8,
	NUM_LIGHTS = GATE_LIGHTS + NUM_GATES + NUM_GATES
    };

    GateSeq() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;
    void initializePattern(int bank, int pattern);
    void copyPattern(int sourcePattern, int bank, int pattern);
    void processPatternSelection();
    bool mergePatterns(bool gate, int channel, int index, bool step);

    enum MergeModes {
	MERGE_OR,
	MERGE_AND,
	MERGE_XOR,
	MERGE_NOR,
	MERGE_RAND,
    };
    int mergeMode = 0;

    struct patternInfo {
	bool gates[NUM_GATES] = {false};
	int length[NUM_CHANNELS] = { 16, 16, 16, 16, 16, 16, 16, 16};
	//float prob[NUM_CHANNELS] = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    };

    patternInfo patterns [64] = {};
    patternInfo* currentPattern;

    int bank = 0;
    int pattern = 0;
    //source pattern for copying (this uses the actual pattern index 0..64)
    int sourcePattern = 0;
    //source pattern for merging
    int mergePattern = 0;

    SchmittTrigger clockTrigger; // for external clock
    SchmittTrigger channelClockTrigger[NUM_CHANNELS]; // for external clock
    SchmittTrigger runningTrigger;
    SchmittTrigger resetTrigger;
    SchmittTrigger initTrigger;
    SchmittTrigger copyTrigger;
    SchmittTrigger mergeTrigger;
    SchmittTrigger lengthTrigger;
    SchmittTrigger gateTriggers[NUM_GATES];
    SchmittTrigger bankTriggers[8];
    SchmittTrigger patternTriggers[8];

    PulseGenerator gatePulse[NUM_CHANNELS];
    PulseGenerator clockOutPulse;

    float stepLights[NUM_GATES] = {};
    int channel_index[NUM_CHANNELS] = {};
    bool running = true;
    bool copyMode = false;
    bool mergeParam = false;
    bool lengthMode = false;
    float phase = 0.0;
    float prob = 0;
    float rand = 0;

    void reset() override {
	for(int y=0;y<64;y++) {
	    for (int i = 0; i < NUM_GATES; i++) {
		patterns[y].gates[i] = false;
	    }
	    for (int i=0; i<NUM_CHANNELS; i++) {
		patterns[y].length[i] = 16;
	    }
	}
	bank = 0;
	pattern = 0;
    }

    void randomize() override {
	for (int i=0; i<NUM_CHANNELS; i++) {
	    for (int y=0; y<NUM_STEPS; y++) {
		currentPattern->gates[i*NUM_CHANNELS + y] = (randomf() > 0.5);
	    }
	    currentPattern->length[i] = (int)(randomf()*15) + 1;
	}
    }
};


void GateSeq::step() {
    float gSampleRate = engineGetSampleRate();
    //const float lightLambda = 0.075;
    const float lightLambda = 0.05;

    // Run
    if (runningTrigger.process(params[RUN_PARAM].value))
	running = !running;
    lights[RUNNING_LIGHT].value = running ? 1.0 : 0.0;

    processPatternSelection();
    bool nextStep = false;

    if(lengthTrigger.process(params[LENGTH_PARAM].value)) {
	lengthMode = !lengthMode;
    }
    lights[LENGTH_LIGHT].value = (lengthMode) ? 1.0 : 0.0;

    if (running) {
	if (inputs[EXT_CLOCK_INPUT].active) {
	    // External clock
	    if (clockTrigger.process(inputs[EXT_CLOCK_INPUT].value)) {
		nextStep = true;
	    }
	}
	else {
	    // Internal clock
	    float clockTime = powf(2.0, params[CLOCK_PARAM].value + inputs[CLOCK_INPUT].value);
	    phase += clockTime / gSampleRate;
	    if (phase >= 1.0) {
		phase -= 1.0;
		nextStep = true;
	    }
	}

	if(nextStep)
	    clockOutPulse.trigger(1e-3);

	bool pulse = false;
	bool channelStep = false;

	for (int y = 0; y < NUM_CHANNELS; y++) {
	    float channelProb = clampf(inputs[CHANNEL_PROB_INPUT + y].value /5.0 + params[CHANNEL_PROB_PARAM + y].value, 0.0, 1.0);
	    //channel clock overwrite
	    channelStep = false;
	    if(inputs[CHANNEL_CLOCK_INPUT + y].active) {
		if (channelClockTrigger[y].process(inputs[CHANNEL_CLOCK_INPUT + y].value)) {
		    channelStep = true;
		}
	    }
	    else {
		channelStep = nextStep;
	    }
	    // Advance step
	    if (channelStep) {
		//int numSteps = clampi(roundf(params[CHANNEL_STEPS_PARAM+y].value), 1, NUM_STEPS);
		int numSteps = currentPattern->length[y];
		//workaround to fix crashes on old saves without pattern support
		if(numSteps == 0)
		    numSteps = 16;
		channel_index[y] = (channel_index[y] + 1) % numSteps;
		stepLights[y*NUM_STEPS + channel_index[y]] = 1.0;
		gatePulse[y].trigger(1e-3);
		//only compute new random number for active steps
		if (currentPattern->gates[y*NUM_STEPS + channel_index[y]] && channelProb < 1) {
		    prob = randomf();
		}
	    }

	    pulse = gatePulse[y].process(1.0 / engineGetSampleRate());
	    bool gateOn = currentPattern->gates[y*NUM_STEPS + channel_index[y]];

	    if(mergeParam) {
		//gateOn = gateOn || patterns[mergePattern].gates[y*NUM_STEPS + channel_index[y]];
		gateOn =  mergePatterns(gateOn, y, channel_index[y], channelStep);
	    }
	    //probability
	    if(prob > channelProb) {
		gateOn = false;
	    }
	    gateOn = gateOn && !pulse;
	    outputs[GATE1_OUTPUT + y].value = (gateOn) ? 10.0 : 0.0;
	}
    }
    else {
	//clear outputs, otherwise it holds it's last value
	for(int y=0;y<NUM_CHANNELS;y++) {
	    outputs[GATE1_OUTPUT + y].value = 0;
	}
    }

    // Reset
    if (resetTrigger.process(params[RESET_PARAM].value + inputs[RESET_INPUT].value)) {
	phase = 0.0;
	for (int y = 0; y < NUM_CHANNELS; y++) {
	    channel_index[y] = 0;
	}
	nextStep = true;
	lights[RESET_LIGHT].value = 1.0;
    }
    lights[RESET_LIGHT].value -= lights[RESET_LIGHT].value / lightLambda / gSampleRate;

    // Gate buttons
    for (int i = 0; i < NUM_GATES; i++) {
	if (gateTriggers[i].process(params[GATE1_PARAM + i].value)) {
	    if(lengthMode) {
		currentPattern->length[i/NUM_STEPS] = (i % NUM_STEPS ) + 1;
	    }
	    else
		currentPattern->gates[i] = !currentPattern->gates[i];
	}
	stepLights[i] -= stepLights[i] / lightLambda / gSampleRate;
	lights[GATE_LIGHTS + 2*i].value = (currentPattern->gates[i] >= 1.0) ? 0.7 - stepLights[i] : stepLights[i];
	lights[GATE_LIGHTS + 2*i + 1].value = ( lengthMode && (i % NUM_STEPS + 1) == currentPattern->length[i/NUM_STEPS]) ? 1.0 : 0.0;
    }

    //clock out
    outputs[CLOCK_OUTPUT].value = clockOutPulse.process(1.0/engineGetSampleRate()) ? 10.0 : 0.0;
}

template <typename BASE>
struct BigLight : BASE {
	BigLight() {
	    this->box.size = Vec(17, 17);
	}
};

struct GateSeqWidget : ModuleWidget {
	GateSeqWidget(GateSeq *module);
};

GateSeqWidget::GateSeqWidget(GateSeq *module) : ModuleWidget(module) {
    box.size = Vec(525, 380);

    {
	SVGPanel *panel = new SVGPanel();
	panel->box.size = box.size;
	panel->setBackground(SVG::load(assetPlugin(plugin, "res/GateSeq.svg")));
	addChild(panel);
    }

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

    addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(15, 50), module, GateSeq::CLOCK_PARAM, -2.0, 10.0, 2.0));
    addParam(ParamWidget::create<LEDBezel>(Vec(65, 55), module, GateSeq::RUN_PARAM, 0.0, 1.0, 0.0));
    addChild(ModuleLightWidget::create<BigLight<GreenLight>>(Vec(67.5, 57.5), module, GateSeq::RUNNING_LIGHT));
    addParam(ParamWidget::create<LEDBezel>(Vec(96.5, 55), module, GateSeq::RESET_PARAM, 0.0, 1.0, 0.0));
    addChild(ModuleLightWidget::create<BigLight<GreenLight>>(Vec(99, 57.5), module, GateSeq::RESET_LIGHT));

    addInput(Port::create<PJ301MPort>(Vec(32, 99-1), Port::INPUT, module, GateSeq::CLOCK_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(5, 99-1), Port::INPUT, module, GateSeq::EXT_CLOCK_INPUT));
    addOutput(Port::create<PJ301MPort>(Vec(63.5, 98), Port::OUTPUT, module, GateSeq::CLOCK_OUTPUT));
    addInput(Port::create<PJ301MPort>(Vec(95.0, 98), Port::INPUT, module, GateSeq::RESET_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(133, 98), Port::INPUT, module, GateSeq::PATTERN_INPUT));

    addParam(ParamWidget::create<LEDBezel>(Vec(175, 55), module, GateSeq::COPY_PARAM , 0.0, 1.0, 0.0));
    addChild(ModuleLightWidget::create<BigLight<YellowLight>>(Vec(177.5, 57.5), module, GateSeq::COPY_LIGHT));
    addParam(ParamWidget::create<LEDBezel>(Vec(175, 98), module, GateSeq::INIT_PARAM , 0.0, 1.0, 0.0));

    addParam(ParamWidget::create<LEDBezel>(Vec(205, 98), module, GateSeq::LENGTH_PARAM , 0.0, 1.0, 0.0));
    addChild(ModuleLightWidget::create<BigLight<RedLight>>(Vec(207.5, 100.5), module, GateSeq::LENGTH_LIGHT));
    addParam(ParamWidget::create<CKSS>(Vec(139, 55), module, GateSeq::PATTERN_SWITCH_MODE_PARAM , 0.0, 1.0, 0.0));

    addParam(ParamWidget::create<LEDBezel>(Vec(465, 55), module, GateSeq::MERGE_PARAM , 0.0, 1.0, 0.0));
    addChild(ModuleLightWidget::create<BigLight<RedLight>>(Vec(467.5, 57.5), module, GateSeq::MERGE_LIGHT));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(463, 90), module, GateSeq::MERGE_MODE_PARAM , 0.0, 5.0, 0.0));

    //pattern/bank buttons
    for(int i=0;i<8;i++) {
	addParam(ParamWidget::create<LEDBezel>(Vec(252 + i*24, 55), module, GateSeq::BANK_PARAM + i, 0.0, 1.0, 0.0));
	addChild(ModuleLightWidget::create<BigLight<GreenLight>>(Vec(254.5 + i*24, 57.5), module, GateSeq::BANK_LIGHTS + i));
	addParam(ParamWidget::create<LEDBezel>(Vec(252 + i*24, 98), module, GateSeq::PATTERN_PARAM + i, 0.0, 1.0, 0.0));
	addChild(ModuleLightWidget::create<BigLight<GreenLight>>(Vec(254.5 + i*24, 100.5), module, GateSeq::PATTERN_LIGHTS + i));
    }

    for (int y = 0; y < NUM_CHANNELS; y++) {
	for (int x = 0; x < NUM_STEPS; x++) {
	    int i = y*NUM_STEPS+x;
	    addParam(ParamWidget::create<LEDBezel>(Vec(62 + x*25, 155+y*25), module, GateSeq::GATE1_PARAM + i, 0.0, 1.0, 0.0));
	    addChild(ModuleLightWidget::create<BigLight<GreenRedLight>>(Vec(62 + x*25 + 2.5, 155+y*25+2.5), module, GateSeq::GATE_LIGHTS + 2*i));
	}
	addInput(Port::create<PJ301MPort>(Vec(5, 155+y*25 - 1.5), Port::INPUT, module, GateSeq::CHANNEL_CLOCK_INPUT + y));
	addInput(Port::create<PJ301MPort>(Vec(32, 155+y*25 - 1.5), Port::INPUT, module, GateSeq::CHANNEL_PROB_INPUT + y));
	addOutput(Port::create<PJ301MPort>(Vec(465, 155+y*25 - 1.5), Port::OUTPUT, module, GateSeq::GATE1_OUTPUT + y));
	addParam(ParamWidget::create<Trimpot>(Vec(495, 155+y*25 + 1.5), module, GateSeq::CHANNEL_PROB_PARAM + y, 0.0, 1.0, 1.0));
    }
}

void GateSeq::processPatternSelection() {
    if(initTrigger.process(params[INIT_PARAM].value))
	initializePattern(bank, pattern);

    //copy pattern
    if(copyTrigger.process(params[COPY_PARAM].value)) {
	if(copyMode)
	    copyPattern(sourcePattern, bank, pattern);
	else
	    sourcePattern = 8*bank + pattern;
	copyMode = (!copyMode);
    }
    lights[COPY_LIGHT].value = (copyMode) ? 1.0 : 0.0;

    //merge Mode
    if(mergeTrigger.process(params[MERGE_PARAM].value)) {
	mergeParam = !mergeParam;
    }
    lights[MERGE_LIGHT].value = (mergeParam) ? 1.0 : 0.0;

    //bank
    for(int i=0;i<8;i++) {
	if(bankTriggers[i].process(params[BANK_PARAM + i].value)) {
	    bank = i;
	    //Switch to first pattern in bank (TODO: do i really want this?)
	    pattern = 0;
	    break;
	}
	lights[BANK_LIGHTS + i].value = (bank == i) ? 1.0 : 0.0;
    }
    //pattern
    for(int i=0;i<8;i++) {
	if(inputs[PATTERN_INPUT].active) {
	    int in = clampi(trunc(inputs[PATTERN_INPUT].value),0 , 7);
	    if (in != pattern && params[PATTERN_SWITCH_MODE_PARAM].value) {
		for(int y=0;y<NUM_CHANNELS;y++) {
		    channel_index[y] = -1;
		}
	    }
	    pattern = in;
	}
	else if(patternTriggers[i].process(params[PATTERN_PARAM + i].value)) {
	    if(mergeParam) {
		mergePattern = 8*bank + i;
	    }
	    else {
		pattern = i;
		//reset index
		if(params[PATTERN_SWITCH_MODE_PARAM].value) {
		    for(int y=0;y<NUM_CHANNELS;y++) {
			channel_index[y] = -1;
		    }
		}
	    }
	    break;
	}
    }
    for(int i=0;i<8;i++) {
	lights[PATTERN_LIGHTS + i].value = (pattern == i || (mergeParam && mergePattern == i)) ? 1.0 : 0.0;
    }
    currentPattern = &patterns[8*bank + pattern];
}

/**
   Merge Pattern steps

   @param gate Gate State of the base Pattern
   @param channel
   @param index Position in channel
   @param step true if the index increased in the active cycle (i.e. we have a new step)
*/
bool GateSeq::mergePatterns(bool gate, int channel, int index, bool step) {
    bool out = false;
    mergeMode = params[MERGE_MODE_PARAM].value;
    if(step)
	rand = randomf();

    switch (mergeMode) {
    case MERGE_OR: {
	out = gate || patterns[mergePattern].gates[channel*NUM_STEPS + index];
	break;
    }
    case MERGE_AND: {
	out = gate && patterns[mergePattern].gates[channel*NUM_STEPS + index];
	break;
    }
    case MERGE_XOR: {
	out = gate ^ patterns[mergePattern].gates[channel*NUM_STEPS + index];
	break;
    }
    case MERGE_NOR: {
	out = !(gate || patterns[mergePattern].gates[channel*NUM_STEPS + index]);
	break;
    }
    case MERGE_RAND: {
	if(rand > 0.5)
	    out = gate;
	else
	    out = patterns[mergePattern].gates[channel*NUM_STEPS + index];
	break;
    }
    }
    return out;
}

void GateSeq::initializePattern(int bank, int pattern) {
    for(int i=0;i<NUM_GATES;i++) {
	currentPattern->gates[i] = 0;
    }

    for (int i = 0; i<NUM_CHANNELS; i++) {
	currentPattern->length[i] = 16;
	//currentPatternp->rob[i] = 1;
    }
}

/**
   Copy a pattern

   @param sourcePattern The Source pattern to be copied
   @param bank The bank of the destination pattern
   @param pattern The pattern of the destination pattern
*/
void GateSeq::copyPattern(int sourcePattern, int bank, int pattern) {
    //currentPattern = &patterns[8*bank + pattern];
    printf("Copying pattern: %d to bank: %d, pattern:%d\n", sourcePattern, bank, pattern);
    for (int i=0; i<NUM_GATES; i++) {
	patterns[8*bank + pattern].gates[i] = patterns[sourcePattern].gates[i];
    }
    for(int i=0;i<NUM_CHANNELS;i++) {
	patterns[8*bank + pattern].length[i] = patterns[sourcePattern].length[i];
    }
}

json_t* GateSeq::toJson() {
    json_t *rootJ = json_object();

    //patterns
    json_t *patternsJ = json_array();
    json_t *lengthsJ = json_array();

    for(int y=0;y<64;y++) {
	// Gate values
	json_t *gatesJ = json_array();
	for (int i = 0; i < NUM_GATES; i++) {
	    json_t *gateJ = json_integer((int) patterns[y].gates[i]);
	    json_array_append_new(gatesJ, gateJ);
	}
	json_array_append_new(patternsJ, gatesJ);

	//second array for lengths to keep things simple
	json_t *pLengthJ = json_array();
	for(int i=0;i<NUM_CHANNELS;i++) {
	    json_t* lengthJ = json_integer(patterns[y].length[i]);
	    json_array_append_new(pLengthJ, lengthJ);
	}
	json_array_append_new(lengthsJ, pLengthJ);
    }
    json_object_set_new(rootJ, "patterns", patternsJ);
    json_object_set_new(rootJ, "lengths", lengthsJ);

    json_t *activePatternJ = json_integer(pattern);
    json_object_set_new(rootJ, "pattern", activePatternJ);
    json_t *activeBankJ = json_integer(bank);
    json_object_set_new(rootJ, "bank", activeBankJ);
    return rootJ;
}

void GateSeq::fromJson(json_t *rootJ) {
    json_t *patternsJ = json_object_get(rootJ, "patterns");
    json_t *lengthsJ = json_object_get(rootJ, "lengths");

    for(int y=0;y<64;y++) {
	// Gate values
	json_t *gatesJ = json_array_get(patternsJ, y);
	for (int i = 0; i < NUM_GATES; i++) {
	    json_t *gateJ = json_array_get(gatesJ, i);
	    //patterns[y][i] = json_integer_value(gateJ);
	    patterns[y].gates[i] = json_integer_value(gateJ);
	}
	json_t *pLengthsJ = json_array_get(lengthsJ, y);
	for(int i=0;i<NUM_CHANNELS;i++) {
	    json_t *lengthJ = json_array_get(pLengthsJ, i);
	    patterns[y].length[i] = json_integer_value(lengthJ);
	}
    }
    json_t * patternJ = json_object_get(rootJ, "pattern");
    pattern = json_integer_value(patternJ);
    json_t * bankJ = json_object_get(rootJ, "bank");
    bank = json_integer_value(bankJ);

    currentPattern = &patterns[8*bank + pattern];
}

Model *modelGateSeq = Model::create<GateSeq, GateSeqWidget>("Aepelzens Modules", "GateSEQ", "Gate Sequencer", SEQUENCER_TAG);
