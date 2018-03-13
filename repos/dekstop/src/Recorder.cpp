#include <atomic>
#include <functional>
#include <thread>

#include "dekstop.hpp"
#include "samplerate.h"
#include "../ext/osdialog/osdialog.h"
#include "write_wav.h"
#include "dsp/digital.hpp"
#include "dsp/ringbuffer.hpp"
#include "dsp/frame.hpp"

#define BLOCKSIZE 1024
#define BUFFERSIZE 32*BLOCKSIZE

template <unsigned int ChannelCount>
struct Recorder : Module {
	enum ParamIds {
		RECORD_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		AUDIO1_INPUT,
		NUM_INPUTS = AUDIO1_INPUT + ChannelCount
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		RECORDING_LIGHT,
		NUM_LIGHTS
	};
	
	std::string filename;
	WAV_Writer writer;
	std::atomic_bool isRecording;

	std::mutex mutex;
	std::thread thread;
	RingBuffer<Frame<ChannelCount>, BUFFERSIZE> buffer;
	short writeBuffer[ChannelCount*BUFFERSIZE];

	Recorder() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{
		isRecording = false;
	}
	~Recorder();
	void step();
	void clear();
	void startRecording();
	void stopRecording();
	void saveAsDialog();
	void openWAV();
	void closeWAV();
	void recorderRun();
};

template <unsigned int ChannelCount>
Recorder<ChannelCount>::~Recorder() {
	if (isRecording) stopRecording();
}

template <unsigned int ChannelCount>
void Recorder<ChannelCount>::clear() {
	filename = "";
}

template <unsigned int ChannelCount>
void Recorder<ChannelCount>::startRecording() {
	saveAsDialog();
	if (!filename.empty()) {
		openWAV();
		isRecording = true;
		thread = std::thread(&Recorder<ChannelCount>::recorderRun, this);
	}
}

template <unsigned int ChannelCount>
void Recorder<ChannelCount>::stopRecording() {
	isRecording = false;
	thread.join();
	closeWAV();
}

template <unsigned int ChannelCount>
void Recorder<ChannelCount>::saveAsDialog() {
	std::string dir = filename.empty() ? "." : extractDirectory(filename);
	char *path = osdialog_file(OSDIALOG_SAVE, dir.c_str(), "Output.wav", NULL);
	if (path) {
		filename = path;
		free(path);
	} else {
		filename = "";
	}
}

template <unsigned int ChannelCount>
void Recorder<ChannelCount>::openWAV() {
	#ifdef v_050_dev
	float gSampleRate = engineGetSampleRate();
	#endif
	if (!filename.empty()) {
		fprintf(stdout, "Recording to %s\n", filename.c_str());
		int result = Audio_WAV_OpenWriter(&writer, filename.c_str(), gSampleRate, ChannelCount);
		if (result < 0) {
			isRecording = false;
			char msg[100];
			snprintf(msg, sizeof(msg), "Failed to open WAV file, result = %d\n", result);
			osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, msg);
			fprintf(stderr, "%s", msg);
		} 
	}
}

template <unsigned int ChannelCount>
void Recorder<ChannelCount>::closeWAV() {
	fprintf(stdout, "Stopping the recording.\n");
	int result = Audio_WAV_CloseWriter(&writer);
	if (result < 0) {
		char msg[100];
		snprintf(msg, sizeof(msg), "Failed to close WAV file, result = %d\n", result);
		osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, msg);
		fprintf(stderr, "%s", msg);
	}
	isRecording = false;
}

// Run in a separate thread
template <unsigned int ChannelCount>
void Recorder<ChannelCount>::recorderRun() {
	#ifdef v_050_dev
	float gSampleRate = engineGetSampleRate();
	#endif
	while (isRecording) {
		// Wake up a few times a second, often enough to never overflow the buffer.
		float sleepTime = (1.0 * BUFFERSIZE / gSampleRate) / 2.0;
		std::this_thread::sleep_for(std::chrono::duration<float>(sleepTime));
		if (buffer.full()) {
			fprintf(stderr, "Recording buffer overflow. Can't write quickly enough to disk. Current buffer size: %d\n", BUFFERSIZE);
		}
		// Check if there is data
		int numFrames = buffer.size();
		if (numFrames > 0) {
			// Convert float frames to shorts
			{
				std::lock_guard<std::mutex> lock(mutex); // Lock during conversion
				src_float_to_short_array(static_cast<float*>(buffer.data[0].samples), writeBuffer, ChannelCount*numFrames);
				buffer.start = 0;
				buffer.end = 0;
			}

			fprintf(stdout, "Writing %d frames to disk\n", numFrames);
			int result = Audio_WAV_WriteShorts(&writer, writeBuffer, ChannelCount*numFrames);
			if (result < 0) {
				stopRecording();

				char msg[100];
				snprintf(msg, sizeof(msg), "Failed to write WAV file, result = %d\n", result);
				osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, msg);
				fprintf(stderr, "%s", msg);
			}
		}
	}
}

template <unsigned int ChannelCount>
void Recorder<ChannelCount>::step() {
	lights[RECORDING_LIGHT].value = isRecording ? 1.0 : 0.0;
	if (isRecording) {
		// Read input samples into recording buffer
		std::lock_guard<std::mutex> lock(mutex);
		if (!buffer.full()) {
			Frame<ChannelCount> f;
			for (unsigned int i = 0; i < ChannelCount; i++) {
				f.samples[i] = inputs[AUDIO1_INPUT + i].value / 5.0;
			}
			buffer.push(f);
		}
	}
}

struct RecordButton : LEDButton {
	using Callback = std::function<void()>;

	Callback onPressCallback;
	SchmittTrigger recordTrigger;
	
	void onChange(EventChange &e) override {
		if (recordTrigger.process(value)) {
			onPress(e);
		}
	}
	void onPress(EventChange &e) {
		assert (onPressCallback);
		onPressCallback();
	}
};


template <unsigned int ChannelCount>
RecorderWidget<ChannelCount>::RecorderWidget() {
	Recorder<ChannelCount> *module = new Recorder<ChannelCount>();
	setModule(module);
	box.size = Vec(15*6+5, 380);

	{
		Panel *panel = new LightPanel();
		panel->box.size = box.size;
		addChild(panel);
	}

	float margin = 5;
	float labelHeight = 15;
	float yPos = margin;
	float xPos = margin;

	{
		Label *label = new Label();
		label->box.pos = Vec(xPos, yPos);
		label->text = "Recorder " + std::to_string(ChannelCount);
		addChild(label);
		yPos += labelHeight + margin;

		xPos = 35;
		yPos += 2*margin;
		ParamWidget *recordButton = createParam<RecordButton>(Vec(xPos, yPos-1), module, Recorder<ChannelCount>::RECORD_PARAM, 0.0, 1.0, 0.0);
		RecordButton *btn = dynamic_cast<RecordButton*>(recordButton);
		Recorder<ChannelCount> *recorder = dynamic_cast<Recorder<ChannelCount>*>(module);

		btn->onPressCallback = [=]()
		{
			if (!recorder->isRecording) {
				recorder->startRecording();
			} else {
				recorder->stopRecording();
			}
		};
		addParam(recordButton);
		addChild(createLight<SmallLight<RedLight>>(Vec(xPos+6, yPos+5), module, Recorder<ChannelCount>::RECORDING_LIGHT));
		xPos = margin;
		yPos += recordButton->box.size.y + 3*margin;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Channels";
		addChild(label);
		yPos += labelHeight + margin;
	}

	yPos += 5;
	xPos = 10;
	for (unsigned int i = 0; i < ChannelCount; i++) {
		addInput(createInput<PJ3410Port>(Vec(xPos, yPos), module, i));
		Label *label = new Label();
		label->box.pos = Vec(xPos + 4, yPos + 28);
		label->text = stringf("%d", i + 1);
		addChild(label);

		if (i % 2 ==0) {
			xPos += 37 + margin;
		} else {
			xPos = 10;
			yPos += 40 + margin;
		}
	}
}

Recorder2Widget::Recorder2Widget() :
	RecorderWidget<2u>()
{
}

Recorder8Widget::Recorder8Widget() :
	RecorderWidget<8u>()
{
}
