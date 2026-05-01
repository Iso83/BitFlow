#include <BitFlow/core/bitvector/BitVector.h>
#include <TestAssert.h>

using namespace BitFlow::Core::BitVector;

int TestNormalizeAndAccessors() {
    bf_uint v(0xffffffffffffffffULL, 65);
    bf_uint one(1ULL, 65);
    bf_uint sum = v + one;

    BF_TEST(sum.ToUint64() == 0ULL);
    BF_TEST(sum.BitWidth() == 65U);
    BF_TEST(one.ToUint32() == 1U);
    BF_TEST(!one.IsZero());
    BF_TEST(bf_uint(0ULL, 129).IsZero());
    return 0;
}

int TestBitwiseAndShifts() {
    bf_uint one(1ULL, 128);
    bf_uint high = one.Shl(127);

    BF_TEST(high.Shr(127).ToUint64() == 1ULL);
    BF_TEST(high.Shr(64).ToUint64() == (1ULL << 63));

    bf_uint allLow(~0ULL, 128);
    bf_uint mask(0x0f0f0f0f0f0f0f0fULL, 128);
    BF_TEST((allLow & mask).ToUint64() == 0x0f0f0f0f0f0f0f0fULL);
    BF_TEST((allLow ^ mask).ToUint64() == 0xf0f0f0f0f0f0f0f0ULL);
    return 0;
}

int TestRotateArithmeticAndDivision() {
    bf_uint x(0x81ULL, 128);
    BF_TEST(x.RotL(1).ToUint64() == 0x102ULL);
    BF_TEST(x.RotR(1).ToUint64() == 0x40ULL);

    bf_uint max64(~0ULL, 128);
    bf_uint two(2ULL, 128);
    bf_uint prod = max64 * two;
    BF_TEST(prod.ToUint64() == 0xfffffffffffffffeULL);

    bf_uint dividend(100ULL, 128);
    bf_uint divisor(9ULL, 128);
    BF_TEST((dividend / divisor).ToUint64() == 11ULL);
    BF_TEST((dividend % divisor).ToUint64() == 1ULL);
    return 0;
}

int TestComparisonAndCompound() {
    bf_uint a(10ULL, 64);
    bf_uint b(20ULL, 64);

    BF_TEST(a < b);
    BF_TEST(b > a);
    BF_TEST(a != b);

    a += bf_uint(5ULL, 64);
    BF_TEST(a.ToUint64() == 15ULL);

    a <<= 1;
    BF_TEST(a.ToUint64() == 30ULL);

    a >>= 2;
    BF_TEST(a.ToUint64() == 7ULL);

    a |= bf_uint(8ULL, 64);
    BF_TEST(a.ToUint64() == 15ULL);

    return 0;
}

int TestWrapAroundModuloWidth() {
    bf_uint x(0xffULL, 8);
    bf_uint one(1ULL, 8);

    BF_TEST((x + one).ToUint64() == 0ULL);
    BF_TEST((-one).ToUint64() == 0xffULL);
    BF_TEST((x << 1).ToUint64() == 0xfeULL);

    return 0;
}

int TestStringConversions() {
    bf_uint x(255ULL, 16);

    BF_TEST(x.ToBinaryString() == "11111111");
    BF_TEST(x.ToDecimalString() == "255");
    BF_TEST(x.ToHexString() == "ff");

    return 0;
}

int TestLargeShiftCounts() {
    bf_uint x(1ULL, 64);

    auto s64 = x << 64;
    auto s65 = x << 65;
    auto r129 = x >> 129;

    BF_TEST(s64.IsZero());
    BF_TEST(s65.IsZero());
    BF_TEST(r129.IsZero());

    // extra: check dat lagere shifts nog correct werken
    BF_TEST((x << 63).ToUint64() == (1ULL << 63));
    BF_TEST((x << 62).ToUint64() == (1ULL << 62));

    // en dat net over de grens abrupt naar 0 gaat
    BF_TEST((x << 63).IsZero() == false);
    BF_TEST((x << 64).IsZero() == true);

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
    BF_RUN_TEST(TestNormalizeAndAccessors);
    BF_RUN_TEST(TestBitwiseAndShifts);
    BF_RUN_TEST(TestRotateArithmeticAndDivision);
    BF_RUN_TEST(TestComparisonAndCompound);
    BF_RUN_TEST(TestWrapAroundModuloWidth);
    BF_RUN_TEST(TestStringConversions);
    BF_RUN_TEST(TestLargeShiftCounts);
    BF_RUN_TEST(TestExceptions);
    return 0;
}
