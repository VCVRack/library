#include "Southpole.hpp"
#include "dsp/digital.hpp"

#define NSNAKEBUSS  16
#define NSNAKEPORTS 10

struct Snake : Module {
	enum ParamIds {
		PLUS_PARAM,
		MINUS_PARAM,
		NUM_PARAMS
	};
	enum InputIds {		
		IN_INPUT,
		NUM_INPUTS = IN_INPUT + NSNAKEPORTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS = OUT_OUTPUT + NSNAKEPORTS
	};
	enum LightIds {
		LOCK_LIGHT,
		NUM_LIGHTS = LOCK_LIGHT + 2*NSNAKEPORTS 
	};

	static float  		cable[NSNAKEBUSS][NSNAKEPORTS];
	static int lockid[NSNAKEBUSS][NSNAKEPORTS];
	static int counter[NSNAKEBUSS];
	static int last_buss;

	int buss = 0;
	int id[NSNAKEBUSS];

  	SchmittTrigger plusTrigger;
  	SchmittTrigger minusTrigger;

	Snake() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {

		// first Snake module instantiation
		if (last_buss == -1) {
			buss = 0;
			last_buss = 0;

			for (int b=0; b< NSNAKEBUSS; b++) {

				counter[b] = 0;									
				id[b] 	   = 0;
				for (int i=0; i< NSNAKEPORTS; i++) {
					cable[b][i] = 0.;
					lockid[b][i] = 0;
				}
			}
		}

		buss = last_buss;
		counter[buss]++;
		id[buss] = counter[buss];
	}

	void step() override;
};

float 		  Snake::cable[NSNAKEBUSS][NSNAKEPORTS];
int  Snake::lockid[NSNAKEBUSS][NSNAKEPORTS];
int  Snake::counter[NSNAKEBUSS];
int  Snake::last_buss = -1;

void Snake::step() {

	for (int i=0; i < NSNAKEPORTS; i++) {

		// if active try to lock input
		if ( inputs[IN_INPUT+i].active) {
			if ( lockid[buss][i] == 0 ) {
				lockid[buss][i] = id[buss];
			}

			if ( lockid[buss][i] == id[buss] ) {
				cable[buss][i] = inputs[IN_INPUT+i].value;
			}
		} else if ( lockid[buss][i] == id[buss] ) {
			lockid[buss][i] = 0;
			cable[buss][i]  = 0;
		}

		// operate lights						
		if ( lockid[buss][i] == 0 ) {
			lights[LOCK_LIGHT+2*i  ].setBrightness(0);
			lights[LOCK_LIGHT+2*i+1].setBrightness(0);
		} else if ( lockid[buss][i] == id[buss] ) {
			lights[LOCK_LIGHT+2*i  ].setBrightness(1.0);
			lights[LOCK_LIGHT+2*i+1].setBrightness(0.);
		} else {
			lights[LOCK_LIGHT+2*i  ].setBrightness(0.);			
			lights[LOCK_LIGHT+2*i+1].setBrightness(1.);			
		}

		// set output
		outputs[OUT_OUTPUT+i].value = cable[buss][i];
	}

	// change buss on trigger
    if (plusTrigger.process(params[PLUS_PARAM].value)) {
		if (buss < NSNAKEBUSS-1) {
			for (int i=0; i < NSNAKEPORTS; i++) {
				// free and clean up current buss
				if ( lockid[buss][i] == id[buss] ) {
					lockid[buss][i] = 0;
					cable[buss][i]  = 0;
				}
			}				
			buss++;		
			if ( id[buss] == 0) {
				counter[buss]++;
				id[buss] = counter[buss];
		    } 
	    } 
    }

    if (minusTrigger.process(params[MINUS_PARAM].value)) {
		if (buss > 0) {
			for (int i=0; i < NSNAKEPORTS; i++) {
				// free and clean up current buss
				if ( lockid[buss][i] == id[buss] ) {
					lockid[buss][i] = 0;
					cable[buss][i]  = 0;
				}
			}				
			buss--;
			if ( id[buss] == 0) {
				counter[buss]++;
				id[buss] = counter[buss];
		    } 
		}
    } 
	
}

struct SnakeDisplay : TransparentWidget {
	Snake *module;
	std::shared_ptr<Font> font;

	SnakeDisplay() {
		font = Font::load(assetPlugin(plugin, "res/hdad-segment14-1.002/Segment14.ttf"));
	}

	void draw(NVGcontext *vg) override {

		// Background
		NVGcolor backgroundColor = nvgRGB(0x30, 0x10, 0x10);
		NVGcolor borderColor = nvgRGB(0xd0, 0xd0, 0xd0);
		nvgBeginPath(vg);
		nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
		nvgFillColor(vg, backgroundColor);
		nvgFill(vg);
		nvgStrokeWidth(vg, 1.5);
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);

		nvgFontSize(vg, 20.);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, 2.);

		Vec textPos = Vec(5, 28);
		NVGcolor textColor = nvgRGB(0xff, 0x00, 0x00);
		nvgFillColor(vg, nvgTransRGBA(textColor, 16));
		nvgText(vg, textPos.x, textPos.y, "~~~~", NULL);
		nvgFillColor(vg, textColor);
		char strbuss[4];
		sprintf(strbuss,"%1x",module->buss);
		nvgText(vg, textPos.x, textPos.y, strbuss, NULL);
	}
};


SnakeWidget::SnakeWidget() {

	Snake *module = new Snake();
	setModule(module);

	box.size = Vec(15*4, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Snake.svg")));
		addChild(panel);
	}

	{
		SnakeDisplay *display = new SnakeDisplay();
		display->box.pos = Vec(5., 30.);
		display->box.size = Vec(25., 34.);
		display->module = module;
		addChild(display);
	}

	addParam(createParam<TL1105>(Vec( 40, 30 ), module, Snake::PLUS_PARAM, 0.0, 1.0, 0.0));
  	addParam(createParam<TL1105>(Vec( 40, 50 ), module, Snake::MINUS_PARAM, 0.0, 1.0, 0.0));

	float y1 = 85;	
	float yh = 26;

	for (int i=0; i< NSNAKEPORTS; i++)
	{
		float y = y1+i*yh + floor(i/5)*yh*.4;
		addInput(createInput<sp_Port>(	Vec( 5, y), module, Snake::IN_INPUT + i));
		addOutput(createOutput<sp_Port>(Vec(34, y), module, Snake::OUT_OUTPUT + i));
		addChild(createLight<SmallLight<GreenRedLight>>(Vec(28, y), module, Snake::LOCK_LIGHT + 2*i));
	}


}
