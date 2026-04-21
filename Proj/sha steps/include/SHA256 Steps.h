#pragma once

#include "SHA256.h"
#include <assert.h>
#include <IXAlgo\Shifting_.h>


#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z))) // Majority Logic
#define EP0(x) (ROTRIGHT32(x,2) ^ ROTRIGHT32(x,13) ^ ROTRIGHT32(x,22))
#define EP1(x) (ROTRIGHT32(x,6) ^ ROTRIGHT32(x,11) ^ ROTRIGHT32(x,25))
#define SIG0(x) (ROTRIGHT32(x,7) ^ ROTRIGHT32(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT32(x,17) ^ ROTRIGHT32(x,19) ^ ((x) >> 10))


struct HashTicket
{
	unsigned char
		target[32], result[32];

public:
	static void Normal(const unsigned char* input, unsigned char hash[32], unsigned steps);
	
	typedef void(*TransformHandler)(SHA256 *buffer, const unsigned __int8 data[64], unsigned steps);
	bool Process(unsigned char *input, unsigned steps, void(*transform)(SHA256 *buffer, const unsigned __int8 data[64], unsigned steps));

	bool Passed() const;
};

class PlaceHolder
{
	bool active;
	unsigned __int32 value;

public:
	inline PlaceHolder() :active(false) {}
	inline PlaceHolder(unsigned __int32 v) : active(true), value(v) {}
	
	inline operator unsigned __int32() const
	{
		if (active)
			return value;

		throw "disabled value!";
	}

	inline void operator=(const unsigned __int32 v)
	{
		value = v;
		active = true;
	}

	inline void Disable() { active = false; }
	inline bool Active() const { return active; }
};

#define Disable(ph) ph.Disable();


void Print_LoadSchedule(unsigned steps);
void Print_Schedule(unsigned steps);
void Print_Round(unsigned steps);



#define Def_Transform(step) void Transform_S##step(SHA256 *ctx, const unsigned __int8 data[64], unsigned steps);
#define Def_TransformTicks(step) void Transform_S##step##_Ticks(SHA256 *ctx, const unsigned __int8 data[64], unsigned steps);

Def_Transform(0);
Def_Transform(1);
Def_Transform(2);

Def_TransformTicks(2);
Def_TransformTicks(64);
