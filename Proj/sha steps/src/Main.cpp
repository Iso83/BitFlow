#include <iostream>
#include "SHA256 Steps.h"
#include <ixalgo/CmdLine_args.h>

using namespace std;

bool Test(unsigned char *input, unsigned steps, HashTicket::TransformHandler transform, const bool printOnFailure = true)
{
	HashTicket t;
	if (!t.Process(input, steps, transform)) {
		if (printOnFailure) {
			print_hash(t.target);
			print_hash(t.result);
		}

		return false;
	}
	else
		return true;
}

#pragma region Explore 
extern void CTest_OrderOfOperations(int argc, char* argv[]);

namespace SHA
{
	namespace Log
	{
		extern void Run(int argc, char* argv[]);
	}
}
#pragma endregion

int main(int argc, char* argv[])
{

#define Process(input, steps, transform) if(!Test(input, steps, transform)) { cout << "Failed on step: " #steps << endl << endl; system("pause"); return EXIT_FAILURE; }
#define ProcessTranform(input, steps) Process(input, steps, Transform_S##steps);

	auto runToken = IXAlgo::Cmdline_args::GetToken(argc, argv, "run");
	if (runToken.IsEmpty()) return EXIT_FAILURE;

	
	if(strcmp(argv[runToken.arg_index+1], "transform") == 0)
	{
		unsigned char input[] = "abc";

		//Process(input, 2, Transform_S2_Ticks);

		Process(input, 64, Transform_S64_Ticks);
	}
	

	if(strcmp(argv[runToken.arg_index+1], "orderOps") == 0)
		CTest_OrderOfOperations(argc, argv);
		

	if(strcmp(argv[runToken.arg_index+1], "log") == 0)
		SHA::Log::Run(argc, argv);


	auto pauseToken = IXAlgo::Cmdline_args::GetToken(argc, argv, "pause");
	if(!pauseToken.IsEmpty())
		system("pause");

	return 0;
}
