#ifndef __PerfTimer_H__
#define __PerfTimer_H__

#include "Defs.h"

class PerfTimer
{
public:

	// Constructor
	PerfTimer();

	void Start();
	double ReadMs() const;
	uint64 ReadTicks() const;

private:
	uint64	startedAt;
	static uint64 frequency;
};

#endif //__PerfTimer_H__