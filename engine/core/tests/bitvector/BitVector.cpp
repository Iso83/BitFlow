#include "TestAssert.h"

#include <BitFlow/engine/core/bitvector/BitVector.h>

using namespace BitFlow::Engine::Core::BitVector;

int TestNormalizeAndAccessors() {
    bf_uint v(0xffffffffffffffffULL, 65);
    bf_uint one(1ULL, 65);
    bf_uint sum = v + one;

    CPPTEST_ASSERT(sum.ToChunk() == 0ULL);
    CPPTEST_ASSERT(sum.BitWidth() == 65U);
    CPPTEST_ASSERT(one.ToUint32() == 1U);
    CPPTEST_ASSERT(!one.IsZero());
    CPPTEST_ASSERT(bf_uint(0ULL, 129).IsZero());
    return 0;
}

int TestBitwiseAndShifts() {
    bf_uint one(1ULL, 128);
    bf_uint high = one.Shl(127);

    CPPTEST_ASSERT(high.Shr(127).ToChunk() == 1ULL);
    CPPTEST_ASSERT(high.Shr(64).ToChunk() == (1ULL << 63));

    bf_uint allLow(~0ULL, 128);
    bf_uint mask(0x0f0f0f0f0f0f0f0fULL, 128);
    CPPTEST_ASSERT((allLow & mask).ToChunk() == 0x0f0f0f0f0f0f0f0fULL);
    CPPTEST_ASSERT((allLow ^ mask).ToChunk() == 0xf0f0f0f0f0f0f0f0ULL);
    return 0;
}

int TestRotateArithmeticAndDivision() {
    bf_uint x(0x81ULL, 128);
    CPPTEST_ASSERT(x.RotL(1).ToChunk() == 0x102ULL);
    CPPTEST_ASSERT(x.RotR(1).ToChunk() == 0x40ULL);

    bf_uint max64(~0ULL, 128);
    bf_uint two(2ULL, 128);
    bf_uint prod = max64 * two;
    CPPTEST_ASSERT(prod.ToChunk() == 0xfffffffffffffffeULL);

    bf_uint dividend(100ULL, 128);
    bf_uint divisor(9ULL, 128);
    CPPTEST_ASSERT((dividend / divisor).ToChunk() == 11ULL);
    CPPTEST_ASSERT((dividend % divisor).ToChunk() == 1ULL);
    return 0;
}

int TestComparisonAndCompound() {
    bf_uint a(10ULL, 64);
    bf_uint b(20ULL, 64);

    CPPTEST_ASSERT(a < b);
    CPPTEST_ASSERT(b > a);
    CPPTEST_ASSERT(a != b);

    a += bf_uint(5ULL, 64);
    CPPTEST_ASSERT(a.ToChunk() == 15ULL);

    a <<= 1;
    CPPTEST_ASSERT(a.ToChunk() == 30ULL);

    a >>= 2;
    CPPTEST_ASSERT(a.ToChunk() == 7ULL);

    a |= bf_uint(8ULL, 64);
    CPPTEST_ASSERT(a.ToChunk() == 15ULL);

    return 0;
}

int TestWrapAroundModuloWidth() {
    bf_uint x(0xffULL, 8);
    bf_uint one(1ULL, 8);

    CPPTEST_ASSERT((x + one).ToChunk() == 0ULL);
    CPPTEST_ASSERT((-one).ToChunk() == 0xffULL);
    CPPTEST_ASSERT((x << 1).ToChunk() == 0xfeULL);

    return 0;
}

int TestStringConversions() {
    bf_uint x(255ULL, 16);

    CPPTEST_ASSERT(x.ToBinaryString() == "11111111");
    CPPTEST_ASSERT(x.ToDecimalString() == "255");
    CPPTEST_ASSERT(x.ToHexString() == "ff");

    return 0;
}

int TestLargeShiftCounts() {
    bf_uint x(1ULL, 64);

    auto s64 = x << 64;
    auto s65 = x << 65;
    auto r129 = x >> 129;

    CPPTEST_ASSERT(s64.IsZero());
    CPPTEST_ASSERT(s65.IsZero());
    CPPTEST_ASSERT(r129.IsZero());

    CPPTEST_ASSERT((x << 63).ToChunk() == (1ULL << 63));
    CPPTEST_ASSERT((x << 62).ToChunk() == (1ULL << 62));

    CPPTEST_ASSERT((x << 63).IsZero() == false);
    CPPTEST_ASSERT((x << 64).IsZero() == true);

    return 0;
}

int TestExceptions() {
    try {
        bf_uint a(10, 32);
        bf_uint b(1, 64);
        auto c = a + b;
        (void)c;
        return -1;
    } catch (...) {
    }

    try {
        bf_uint a(10, 32);
        bf_uint z(0, 32);
        auto c = a / z;
        (void)c;
        return -1;
    } catch (...) {
    }

    return 0;
}

int main() {
    CPPTEST_RUN(TestNormalizeAndAccessors);
    CPPTEST_RUN(TestBitwiseAndShifts);
    CPPTEST_RUN(TestRotateArithmeticAndDivision);
    CPPTEST_RUN(TestComparisonAndCompound);
    CPPTEST_RUN(TestWrapAroundModuloWidth);
    CPPTEST_RUN(TestStringConversions);
    CPPTEST_RUN(TestLargeShiftCounts);
    CPPTEST_RUN(TestExceptions);
    return 0;
}
