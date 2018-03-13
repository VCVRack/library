#include "WavWriter.hpp"
#include "write_wav.h"

#include <samplerate.h>
#include <engine.hpp>

#include <iostream>

std::string WavWriter::getErrorText(Errors const error)
{
	std::string text;

	switch (error)
	{
	case Errors::BufferOverflow:
		text = "buffer overflow";
		break;
	case Errors::UnableToOpenFile:
		text = "unable to open file for writing";
		break;
	default:
		break;
	}
	return text;
}

WavWriter::WavWriter() :
    m_running(false)
{
	// The buffer can store 1 seconds
	m_buffer.reserve(rack::engineGetSampleRate());
}

WavWriter::~WavWriter()
{
	stop();
	finishThread();
}

void WavWriter::start(std::string const& outputFilePath)
{
	finishThread();
	m_error = Errors::NoError;
	m_buffer.clear();
	m_thread = std::thread(&WavWriter::run, this, outputFilePath);
}

void WavWriter::stop()
{
	if (m_running)
	{
		m_running = false;
	}
}

void WavWriter::push(Frame const& frame)
{
	std::unique_lock<std::mutex> lock(m_mutexBuffer);

	m_buffer.push_back(frame);
}

void WavWriter::run(std::string const& outputFilePath)
{
	static std::chrono::milliseconds const WriteTimeInterval{250};
	// The internal buffer can store 1 second of audio.
	std::vector<short> buffer(rack::engineGetSampleRate() * ChannelCount, 0);
	std::chrono::milliseconds elapsedTime{0u};
	WAV_Writer writer;

	if (Audio_WAV_OpenWriter(&writer, outputFilePath.c_str(), rack::engineGetSampleRate(), ChannelCount) < 0)
	{
		m_error = Errors::UnableToOpenFile;
		return;
	}
	else
	{
		m_running = true;
	}
	while (m_running)
	{
		std::this_thread::sleep_for(WriteTimeInterval - elapsedTime);
		auto currentTime = std::chrono::steady_clock::now();
		std::unique_lock<std::mutex> lock(m_mutexBuffer);
		auto const frameCount = m_buffer.size();
		auto const sampleCount = frameCount * ChannelCount;

		if(sampleCount > buffer.size())
		{
			m_running = false;
			m_error = Errors::BufferOverflow;
			break;
		}

		src_float_to_short_array(m_buffer.data()->samples, buffer.data(), sampleCount);
		m_buffer.clear();
		lock.unlock();

		Audio_WAV_WriteShorts(&writer, buffer.data(), sampleCount);

		elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - currentTime);
	}
	Audio_WAV_CloseWriter(&writer);
}

void WavWriter::finishThread()
{
	if (m_thread.joinable())
		m_thread.join();
}
