#ifndef __Timer_H__
#define __Timer_H__

#include "Defs.h"

class Timer
{
public:

	// Constructor
	Timer();

	void Start();
	uint32 Read() const;
	float ReadSec() const;

private:
	uint32	startedAt;
};

#endif //__Timer_H__