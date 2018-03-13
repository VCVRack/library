#include "utils.h"

static long _stepCount = 0;

void log_increase_step_number()
{
    _stepCount++;
}

void write_log(long freq, const char *format, ...)
{
    if (freq == 0)
        freq++;

    if (_stepCount % freq == 0) // log every "freq".  eg pass in 10000 to issue this log every 10k steps
    {
        va_list args;
        va_start(args, format);

        printf("%ld: ", _stepCount);
        vprintf(format, args);
        fflush(stdout);

        va_end(args);
    }
}
