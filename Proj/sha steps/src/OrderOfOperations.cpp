#include <iostream>
#include <ixalgo/numerics/Sequencing_.h>
#include <ixalgo/CmdLine_args.h>

#define a index[0] 
#define b index[1] 
#define c index[2] 
#define d index[3] 
#define e index[4] 
#define f index[5]
#define g index[6]
#define h index[7]

using namespace std;
using namespace IXAlgo;
using namespace IXAlgo::Collections;
using namespace IXAlgo::Numerics;


#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z))) // Majority Logic

#define MaskSL(s) (~unsigned(0) << s)
#define MaskSR(s) (~unsigned(0) >> s)

#define Pow2(exp) IXAlgo::Pow2<unsigned, unsigned>(exp)

#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) >> (-(b))))

// dummy shift ... only 4 bits
#define RR2(x) ROTRIGHT(x, 1) 
#define RR13(x) ROTRIGHT(x, 2)
#define RR22(x) ROTRIGHT(x, 3)
#define RR6(x) ROTRIGHT(x, 1) 
#define RR11(x) ROTRIGHT(x, 2)
#define RR25(x) ROTRIGHT(x, 3)
#define RR7(x) ROTRIGHT(x, 1) 
#define RR18(x) ROTRIGHT(x, 2)
#define R3(x) ROTRIGHT(x, 3)
#define RR17(x) ROTRIGHT(x, 1) 
#define RR19(x) ROTRIGHT(x, 2)
#define R10(x) ROTRIGHT(x, 3)


#define EP0(x) (RR2(x) ^ RR13(x) ^ RR22(x))
#define EP1(x) (RR6(x) ^ RR11(x) ^ RR25(x))
#define SIG0(x) (RR7(x) ^ RR18(x) ^ R3(x))
#define SIG1(x) (RR17(x) ^ RR19(x) ^ R10(x))

#pragma region Check
#define iFunc(deep, bits) [](const Sequence<bits> index[deep])
#define iFuncT(deep, T_SEQ) [](const Sequence<sizeof(T_SEQ::bits)> index[deep])

#define Check_1(func) AllGood<1, Nibble>( \
			target, \
			iFuncT(1, Nibble) { return func ; } );
#define Check_1_Log(log, func) cout << #log << ": " << (AllGood<1, Nibble>( \
			target, \
			iFuncT(1, Nibble) { return func ; }, \
			false) ? "Passed !!" : "**Failed**") << endl;

#define Check_2(func) AllGood<2, Nibble>( \
			target, \
			iFuncT(2, Nibble) { return func ; } );
#define Check_2_Log(log, func) cout << #log << ": " << (AllGood<2, Nibble>( \
			target, \
			iFuncT(2, Nibble) { return func ; }, \
			false) ? "Passed !!" : "**Failed**") << endl;

#define Check_3(func) AllGood<3, Nibble>( \
			target, \
			iFuncT(3, Nibble) { return func ; } );
#define Check_3_Log(log, func) cout << #log << ": " << (AllGood<3, Nibble>( \
			target, \
			iFuncT(3, Nibble) { return func ; }, \
			false) ? "Passed !!" : "**Failed**") << endl;

#define Check_4(func) AllGood<4, Nibble>( \
			target, \
			iFuncT(4, Nibble) { return func ; } );
#define Check_4_Log(log, func) cout << #log << ": " << (AllGood<4, Nibble>( \
			target, \
			iFuncT(4, Nibble) { return func ; }, \
			false) ? "Passed !!" : "**Failed**") << endl;

#define CheckB_8(func) AllGood<8, Sequence<1>>( \
			target, \
			iFunc(8, 1) { return func ; } );
#define CheckB_8_Log(log, func) cout << #log << ": " << (AllGood<8, Sequence<1>>( \
			target, \
			iFunc(8, 1) { return func ; }, \
			false) ? "Passed !!" : "**Failed**") << endl;

#pragma endregion

#pragma region Brackets
//https://en.wikipedia.org/wiki/Order_of_operations
void Test_OrderOps_Brackets()
{
#define Into_Brackets(op1, op2, match) \
				if(match != AllGood<3, Nibble>( \
					iFuncT(3, Nibble) { return a op1 (b op2 c); }, \
					iFuncT(3, Nibble) { return (a op1 b) op2 (a op1 c); }, \
				false)) { throw "failed"; }

	Into_Brackets(&, | , true);
	Into_Brackets(&, ^, true);
	Into_Brackets(&, +, false);

	Into_Brackets(| , &, true);
	Into_Brackets(| , ^, false);
	Into_Brackets(| , +, false);

	Into_Brackets(^, &, false);
	Into_Brackets(^, | , false);
	Into_Brackets(^, +, false);

	Into_Brackets(+, &, false);
	Into_Brackets(+, | , false);
	Into_Brackets(+, ^, false);
}

void Test_InvertedBrackets()
{
	AllGood<2, Nibble>(
		iFuncT(2, Nibble) { return a & b; },
		iFuncT(2, Nibble) { return ~(~a | ~b); });

	AllGood<2, Nibble>(
		iFuncT(2, Nibble) { return a | b; },
		iFuncT(2, Nibble) { return ~(~a & ~b); });
	}

void Test_MixedBrackets()
{
	AllGood<4, Nibble>(
		iFuncT(4, Nibble) { return a | b & c & d; },
		iFuncT(4, Nibble) { return (a | b) & (a | c) & (a | d); });

	AllGood<5, Nibble>(
		iFuncT(5, Nibble) { return a | b | c & d & e; },
		iFuncT(5, Nibble) { return (a | b | c) & (a | b | d) & (a | b | e); });
}
#pragma endregion

#pragma region Order
void Test_Order()

{
	/*
		AND is before OR.
	*/


	AllGood<3, Nibble>(
		iFuncT(3, Nibble) { return a & b | c; },
		iFuncT(3, Nibble) { return c | a & b; });

	// ---------

#define target iFuncT(4, Nibble) { return a & b & c | d; }

	AllGood<4, Nibble>(
		target,
		iFuncT(4, Nibble) { return d | a & b & c; });


	// ----------

#define target iFuncT(4, Nibble) { return a | b | c & d; }

	AllGood<4, Nibble>(
		target,
		iFuncT(4, Nibble) { return a | c & d | b; });
	AllGood<4, Nibble>(
		target,
		iFuncT(4, Nibble) { return c & d | a | b; });

#undef target;
}
#pragma endregion

#pragma region XOR
void Test_XOR()
{
	auto target = iFuncT(2, Nibble) { return a ^ b; };

	Check_2((~a & b) | (a & ~b));
	Check_2(~(~(a & ~(a & b)) & ~(b & ~(a & b))));

	Check_2((a | b) & ~(a & b));
	Check_2((a | b) & (~a | ~b));
	Check_2(~(~(a | b) | (a & b)));

#define target iFuncT(3, Nibble) { return ~(a & b) ^ ~(a & c); }
	Check_3((a & b) ^ (a & c));


#define target iFuncT(3, Nibble) { return a ^ b ^ c; }
	Check_3((~a & b | a & ~b) ^ c);
	Check_3(~(~a & b | a & ~b) & c | (~a & b | a & ~b) & ~c);

	Check_3(~(~a & b | a & ~b) & c | ~a & b & ~c | a & ~b & ~c);
	Check_3(~(~a & b) & ~(a & ~b) & c | ~a & b & ~c | a & ~b & ~c);
	Check_3((a | ~b) & (~a | b) & c | ~a & b & ~c | a & ~b & ~c);

	Check_3(~b & ~a & c | b & a & c | ~a & b & ~c | a & ~b & ~c);


#undef target;
}
#pragma endregion

#pragma region Logic-CH
void Test_CH()
{
	auto target = iFuncT(3, Nibble) { return CH(a, b, c); };

	Check_3(a&b ^ ~a&c);
	Check_3(a&b ^ ~(a | ~c));
	Check_3(~(~a | ~b) ^ ~(a | ~c));
	Check_3((~a | ~b) ^ (a | ~c));



	Check_3((~(a&b) & (~a&c)) | ((a&b) & ~(~a&c)));
	Check_3(((~a | ~b) & (~a&c)) | ((a&b) & (a | ~c)));
	Check_3(((~a | ~b) & ~(a | ~c)) | ((a&b) & (a | ~c)));
	Check_3(~(~(~a | ~b) | (a | ~c)) | ((a&b) & (a | ~c)));

	Check_3((~(a & b) & ~(a | ~c)) | ((a&b) & (a | ~c)));
	Check_3(~(a & b) & ~(a | ~c) | (a&b) & (a | ~c));
	Check_3((~a | ~b) & (~a & c) | (a&b) & (a | ~c));
	Check_3((~a | ~b) & ~a & c | (a&b) & (a | ~c));

	Check_3((~a | ~b) & ~a & c | a & b & (a | ~c));
	Check_3((~a | ~b) & ~a & c | a & b & ~(~a & c));
	Check_3((~a | ~b) & ~a & c | (a & b & ~c | a & b));

	Check_3((~a | ~b) & ~a & c | (a & b & ~c | a & b));

	// remove canceling statements.
	Check_3((~a | ~b) & ~a & c | (a & b));
	Check_3(~(a & b) & ~a & c | (a & b));
	Check_3(~a & c | a & b);

	Check_3((a | ~(a | ~c)) & (b | ~a & c));
	Check_3((a | c) & (b | ~a & c));
	Check_3((a | c) & (b | ~a));



	// Final result
	Check_3(a & b | ~a & c);
	Check_3((~a | b) & (a | c));
}
#pragma endregion

#pragma region Logic-MAJ
void Test_MAJ()
{
	auto target = iFuncT(3, Nibble) { return MAJ(a, b, c); };

	Check_3(a&b ^ a&c ^ b&c);
	Check_3((~(a&b) &(a&c) | (a&b) & ~(a&c)) ^ b&c);
	Check_3((~(a&b) &(a&c) | (a&b) & ~(a&c)) ^ b&c);
	Check_3(~(~(a&b) &(a&c) | (a&b) & ~(a&c)) & b&c | (~(a&b) &(a&c) | (a&b) & ~(a&c)) & ~(b&c));

	Check_3(~(~(a&b) &(a&c) | (a&b) & ~(a&c)) & b&c | (~(a&b) &(a&c) | (a&b) & ~(a&c)) & ~(b&c));
	Check_3(~(~(a&b) & a & c | a & b & (~a | ~c)) & b & c | ~(a&b) &a&c | (a&b) & (~a | ~c) & (~b | ~c));
	Check_3(~((~a | ~b) & a & c | a & b & (~a | ~c)) & b & c | (~a | ~b) & a & c | a & b & (~a | ~c) & (~b | ~c));
	Check_3((~(~a | ~b) | ~a | ~c & ~(~a | ~b | ~(~a | ~c))) & b & c | (~a | ~b) & a & c | a & b & (~a | ~c) & (~b | ~c));

	Check_3((a & b | ~a | ~c & ~(~a | ~b | a & c)) & b & c | (~a | ~b) & a & c | a & b & (~a | ~c) & (~b | ~c));
	Check_3((a & b | ~a | ~c & (a & b & ~a | ~c)) & b & c | (~a | ~b) & a & c | a & b & (~a | ~c) & (~b | ~c));
	Check_3((a & b | ~a | ~c) & b & c | (~a | ~b) & a & c | a & b & ~c);
	Check_3(b & c | ~b & a & c | a & b & ~c);



	// Final result
	Check_3(b & c | a & c | a & b);
}
#pragma endregion

#pragma region Shift
void Test_EP0()
{
	auto target = iFuncT(1, Nibble) { return EP0(a); };
	Check_1(RR2(a) ^ RR13(a) ^ RR22(a));

	Check_1_Log(EP0,
		~RR2(a) & ~RR13(a) &  RR22(a) |
		RR2(a) &  RR13(a) &  RR22(a) |
		~RR2(a) &  RR13(a) & ~RR22(a) |
		RR2(a) & ~RR13(a) & ~RR22(a));



	const unsigned k(0x27b70a85), k2(0x71374491);

#define Check_Target2(x) if(unsigned(x) != (target2)) { cout << "EP0: failed" << endl; return; }

	unsigned target2 = ROTRIGHT32(k, 2) ^ ROTRIGHT32(k, 13) ^ ROTRIGHT32(k, 22);
	Check_Target2(
		(k / Pow2(2) | k * Pow2(32 - 2)) ^
		(k / Pow2(13) | k * Pow2(32 - 13)) ^
		(k / Pow2(22) | k * Pow2(32 - 22))
	);

	Check_Target2(ROTRIGHT32(k ^ ROTRIGHT32(k, 11) ^ ROTRIGHT32(k, 20), 2));
	Check_Target2(
		~(k / Pow2(2) | k * Pow2(32 - 2)) & ~(k / Pow2(13) | k * Pow2(32 - 13)) & (k / Pow2(22) | k * Pow2(32 - 22)) |
		(k / Pow2(2) | k * Pow2(32 - 2)) & (k / Pow2(13) | k * Pow2(32 - 13)) & (k / Pow2(22) | k * Pow2(32 - 22)) |
		~(k / Pow2(2) | k * Pow2(32 - 2)) & (k / Pow2(13) | k * Pow2(32 - 13)) & ~(k / Pow2(22) | k * Pow2(32 - 22)) |
		(k / Pow2(2) | k * Pow2(32 - 2)) & ~(k / Pow2(13) | k * Pow2(32 - 13)) & ~(k / Pow2(22) | k * Pow2(32 - 22))
	);

	Check_Target2(
		(~k / Pow2(2) | ~k * Pow2(32 - 2)) & (~k / Pow2(13) | ~k * Pow2(32 - 13)) & (k / Pow2(22) | k * Pow2(32 - 22)) |
		(k / Pow2(2) | k * Pow2(32 - 2)) & (k / Pow2(13) | k * Pow2(32 - 13)) & (k / Pow2(22) | k * Pow2(32 - 22)) |
		(~k / Pow2(2) | ~k * Pow2(32 - 2)) & (k / Pow2(13) | k * Pow2(32 - 13)) & (~k / Pow2(22) | ~k * Pow2(32 - 22)) |
		(k / Pow2(2) | k * Pow2(32 - 2)) & (~k / Pow2(13) | ~k * Pow2(32 - 13)) & (~k / Pow2(22) | ~k * Pow2(32 - 22))
	);

	Check_Target2(
		~((k / Pow2(2) | ~MaskSR(2)) & (k * Pow2(32 - 2) | ~MaskSL(32 - 2))) &
		~((k / Pow2(13) | ~MaskSR(13)) & (k * Pow2(32 - 13) | ~MaskSL(32 - 13))) &
		(k / Pow2(22) | k * Pow2(32 - 22)) |
		(k / Pow2(2) | k * Pow2(32 - 2)) & (k / Pow2(13) | k * Pow2(32 - 13)) & (k / Pow2(22) | k * Pow2(32 - 22)) |
		(~k / Pow2(2) | ~k * Pow2(32 - 2)) & (k / Pow2(13) | k * Pow2(32 - 13)) & (~k / Pow2(22) | ~k * Pow2(32 - 22)) |
		(k / Pow2(2) | k * Pow2(32 - 2)) & (~k / Pow2(13) | ~k * Pow2(32 - 13)) & (~k / Pow2(22) | ~k * Pow2(32 - 22))
	);

	target2 = ROTRIGHT32((k + k2), 2);
	Check_Target2((k + k2) / Pow2(2) | (k + k2) * Pow2(32 - 2));
	Check_Target2(((k) / Pow2(2) | (k) * Pow2(32 - 2)) + ((k2) / Pow2(2) | (k2)* Pow2(32 - 2)));
}

void Test_Shift()
{
	const unsigned k(0x27b70a85);

#define Check(arg) if(target != (arg)) { cout << "shift: failed" << endl; return; }

	for (int i = 0; i < 32; i++)
	{
		auto target = ROTRIGHT32(k, i);
		Check(k / Pow2(i) | k * Pow2(32 - i));
		Check(~(~(k / Pow2(i)) &  ~(k * Pow2(32 - i))));
	}

	for (int i = 0; i < 32; i++)
	{
		auto target = ~(ROTRIGHT32(k, i));
		Check(ROTRIGHT32(~k, i));
		Check(~k / Pow2(i) | ~k * Pow2(32 - i));
		Check(~(~(~k / Pow2(i)) & ~(~k * Pow2(32 - i))));

		Check(~k / Pow2(i) | ~k<<(32 - i));
		Check(~k / Pow2(i) | (~(k << (32 - i)) & MaskSL(32-i)));

		Check(~(~(~k / Pow2(i)) & ~(~(k << (32 - i)) & MaskSL(32 - i))));
		Check(~(~(~k / Pow2(i)) & ~(~k << (32 - i) & MaskSL(32 - i))));

		Check(~(~(~k / Pow2(i)) & ~(~k * Pow2(32-i) & MaskSL(32 - i))));
		Check(~(~(~k / Pow2(i)) & ~((~(k * Pow2(32 - i)) & MaskSL(32 - i)))));
		Check(~(~(~k / Pow2(i)) & (k * Pow2(32 - i) | ~MaskSL(32 - i))));


		Check(~((k / Pow2(i) | ~MaskSR(i)) & (k * Pow2(32 - i) | ~MaskSL(32 - i))));
	}
}
#pragma endregion

#pragma region Addition
#include <ixalgo/numerics/math/Addition_.h>

inline bool FullAdder(bool b1, bool b2, bool &carry)
{
	bool result = b1 ^ b2;
	bool carryOut = b1 & b2 | result & carry;
	result ^= carry;

	carry = carryOut;
	return result;
}

static Nibble Addition(Nibble n1, Nibble n2)
{
	Nibble r;

	bool carry(false);

	//for (int i = 0; i < 4; i++)
	//{
	//	/// 1
	//	//	//	IXAlgo::Numerics::Math::Addition<bool>(r.bits[i], n1.bits[i], n2.bits[i], carry);
	//	
	//	/// 2 
	//	//	//r.bits[i] = FullAdder(n1.bits[i], n2.bits[i], carry);

	//	/// 3
	//	r.bits[i] = n1.bits[i] ^ n2.bits[i] ^ carry;
	//	carry = n1.bits[i] & n2.bits[i] | (n1.bits[i] ^ n2.bits[i]) & carry;
	//}


	r.bits[0] = n1.bits[0] ^ n2.bits[0];
	r.bits[1] = n1.bits[1] ^ n2.bits[1] ^ n1.bits[0] & n2.bits[0];
	r.bits[2] = n1.bits[2] ^ n2.bits[2] ^ (n1.bits[1] & n2.bits[1] | (n1.bits[1] ^ n2.bits[1]) & n1.bits[0] & n2.bits[0]);
	r.bits[3] = n1.bits[3] ^ n2.bits[3] ^ (n1.bits[2] & n2.bits[2] | (n1.bits[2] ^ n2.bits[2]) & (n1.bits[1] & n2.bits[1] | (n1.bits[1] ^ n2.bits[1]) & n1.bits[0] & n2.bits[0]));


	return r;
}

void Test_Addition()
{
	auto target = iFuncT(4, Nibble) { return CH(a, b, c) + d; };
	Check_4((a & b | ~a & c) + d);

#define target iFuncT(3, Nibble) { return a & (b + c); }
	Check_3(a & b + c);

#define target iFuncT(3, Nibble) { return (a + b) & c; }
	Check_3(a + b & c);
	Check_3(Addition(a, b) & c);

#define target iFuncT(4, Nibble) { return c & d | (c ^ d) & a & b; }
	Check_4(c & d | (c | d) & ~(c & d) & a & b);
	Check_4(a & b & (c | d) & (~c | ~d) | c & d);
	Check_4(a & b & (c | d) & (~c | ~d) | ~(~c | ~d));
	Check_4(a & b & (c | d) & (~c | ~d) | c & d);
	Check_4(a & b & (c | d) | c & d);
	Check_4(a & b & c | a & b & d | c & d);

#define target iFunc(8, 1) { return g ^ h ^ (e & f | (e ^ f) & (c & d | (c ^ d) & a & b)); }
	CheckB_8_Log(addition, g ^ h ^ (e & f | (e ^ f) & (c & d | (c ^ d) & a & b)));
	CheckB_8_Log(addition, g ^ h ^ (e & f | (e ^ f) & (a & b & c | a & b & d | c & d)));



#undef target;
}
#pragma endregion


void CTest_OrderOfOperations(int argc, char* argv[])
{
	auto runToken = Cmdline_args::GetToken(argc, argv, "run");

	if (!runToken.IsEmpty() && runToken.arg_count > 1)
	{
		auto test = argv[runToken.arg_index + 2];

		if (strcmp(test, "brackets") == 0)
			Test_OrderOps_Brackets();

		if (strcmp(test, "bracketsInverted") == 0)
			Test_InvertedBrackets();

		if (strcmp(test, "bracketsMixed") == 0)
			Test_MixedBrackets();

		if (strcmp(test, "order") == 0)
			Test_Order();

		if (strcmp(test, "xor") == 0)
			Test_XOR();

		if (strcmp(test, "ch") == 0)
			Test_CH();

		if (strcmp(test, "maj") == 0)
			Test_MAJ();

		if (strcmp(test, "ep0") == 0)
			Test_EP0();

		if (strcmp(test, "shift") == 0)
			Test_Shift();

		if (strcmp(test, "addition") == 0)
			Test_Addition();
	}
}
