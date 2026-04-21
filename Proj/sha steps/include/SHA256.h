#pragma once

#include <ixalgo/IXA_Core.h>


struct SHA256
{
	unsigned __int8 data[64];
	unsigned __int32 datalen;
	unsigned __int32 bitlen[2];
	unsigned __int32 state[8];

public:
	static void Init(SHA256 *buffer);

	inline void Init() { Init(this); }

private:
	static void Transform(SHA256 *buffer, const unsigned __int8 data[64]);
	static void Transform(SHA256 *buffer, const unsigned __int8 data[64], unsigned steps);

public:
	static void Update(SHA256 *buffer, const unsigned __int8 data[], const unsigned __int32 &len);
	static void Update(SHA256 *buffer, const unsigned __int8 data[], const unsigned __int32 &len, unsigned steps, void(*transform)(SHA256 *buffer, const unsigned __int8 data[64], unsigned steps));

	inline void Update(const unsigned __int8 data[], const unsigned __int32 &len) { Update(this, data, len); }
	inline void Update(const unsigned __int8 data[], const unsigned __int32 &len, unsigned steps) { Update(this, data, len, steps, SHA256::Transform); }

	static void Final(SHA256 *buffer, unsigned __int8 hash[32]);
	static void Final(SHA256 *buffer, unsigned __int8 hash[32], unsigned steps, void(*transform)(SHA256 *buffer, const unsigned __int8 data[64], unsigned steps));

	inline void Final(unsigned __int8 hash[32]) { Final(this, hash); }
	inline void Final(unsigned __int8 hash[32], unsigned steps) { Final(this, hash, steps, SHA256::Transform); }
};

void print_hash(unsigned char hash[32]);
void Test_SHA256();
void Test2_SHA256();