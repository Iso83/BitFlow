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

int main() {
    BF_RUN_TEST(TestNormalizeAndAccessors);
    BF_RUN_TEST(TestBitwiseAndShifts);
    BF_RUN_TEST(TestRotateArithmeticAndDivision);
    return 0;
}
