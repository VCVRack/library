#if!defined WAVWRITER_HPP
#define WAVWRITER_HPP
#include <dsp/ringbuffer.hpp>
#include <dsp/frame.hpp>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include "write_wav.h"

class WavWriter
{
	static constexpr unsigned int const ChannelCount = 2u;
public:
	using Frame = rack::Frame<ChannelCount>;

	enum class Errors : std::uint8_t
	{
		NoError,
		BufferOverflow,
		UnableToOpenFile
	};

	static std::string getErrorText(Errors const error);

	WavWriter();
	~WavWriter();

	/*! Start writing thread
	 	\param outputFilePath Output file path. This file can be already existing, but in this case it
		must be writable.
	 */
	void start(std::string const& outputFilePath);

	/*! Stop writing */
	void stop();

	bool isRunning()const { return m_running; }
	bool haveError()const { return m_error != Errors::NoError; }
	void clearError() { m_error = Errors::NoError; }
	Errors error()const { return m_error; }

	/*! Push data to the buffer. */
	void push(Frame const& frame);
private:
	void run(std::string const& outputFilePath);
	void finishThread();
private:
	std::vector<Frame> m_buffer;
	std::mutex m_mutexBuffer;
	std::thread m_thread;
	std::atomic<bool> m_running;
	std::atomic<Errors> m_error;
};

#endif
