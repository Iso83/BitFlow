#include "OpsLog.h"
#include <ixalgo/collections/List_.h>
#include <ixalgo/collections/SortedDictionary_.h>
#include <ixalgo/CmdLine_args.h>

#include <iostream>
#include <cstdarg>

using namespace IXAlgo::Collections;
using namespace std;

namespace SHA
{
	namespace Log
	{
		typedef Block::NameIndex NameI;

#pragma region Block
		void PrintValues(Block *b)
		{
			switch (b->type)
			{
			case Block::Dynamic:
				cout << "<?>";
				break;
			case Block::Static:
				cout << "<$>";
				break;

			case Block::Reference:
			{
				assert(b->params.Length == 1);
				cout << b->params[0]->Name();
			}
			break;

			case Block::Operator:
			{
				assert(!b->opGroup.blocks.IsEmpty);

				if (b->opGroup.op == BlockOperator::MultiplicationSingle)
				{
					assert(b->opGroup.opParam > 0);
					assert(b->opGroup.blocks.Length == 1);

					if (b->opGroup.blocks[0]->hideName)
					{
						cout << b->opGroup.opParam << " * ";
						PrintValues(b->opGroup.blocks[0]);
					}
					else
						cout << b->opGroup.opParam << b->opGroup.blocks[0]->Name();

					break;
				}

				if (b->opGroup.op == BlockOperator::Shift_RRR)
				{
					assert(b->opGroup.opParam >= 0);
					assert(b->opGroup.blocks.Length == 1);

					cout << b->opGroup.opParam << " >>> ";
					
					if (b->opGroup.blocks[0]->hideName)
						PrintValues(b->opGroup.blocks[0]);
					else
						cout << b->opGroup.blocks[0]->Name();

					break;
				}

				if (b->opGroup.op == BlockOperator::Shift_RR)
				{
					assert(b->opGroup.opParam >= 0);
					assert(b->opGroup.blocks.Length == 1);

					cout << b->opGroup.opParam << " >> ";

					if (b->opGroup.blocks[0]->hideName)
						PrintValues(b->opGroup.blocks[0]);
					else
						cout << b->opGroup.blocks[0]->Name();

					break;
				}

				string opSymbol;
				switch (b->opGroup.op)
				{
				case BlockOperator::Addition:
					opSymbol = " + ";
					break;
				case BlockOperator::AND:
					opSymbol = "";
					break;
				case BlockOperator::XOR:
					opSymbol = " ^ ";
					break;
				case BlockOperator::OR:
					opSymbol = " | ";
					break;
				}

				bool prev = false;
				for (Block *b : b->opGroup.blocks)
				{
					if (prev)
						cout << opSymbol;
					
					if (b->hideName)
						PrintValues(b);
					else
						cout << b->Name();
					prev = true;
				}
			}
			break;

			case Block::Fuction:
			{
				switch (b->func)
				{
				case Block::CH:
				{
					assert(b->params.Length == 3);
					cout << "CH(" <<
						b->params[0]->Name() << "," <<
						b->params[1]->Name() << "," <<
						b->params[2]->Name() << ")";
				}
				break;

				case Block::MAJ:
				{
					assert(b->params.Length == 3);
					cout << "MAJ(" <<
						b->params[0]->Name() << "," <<
						b->params[1]->Name() << "," <<
						b->params[2]->Name() << ")";
				}
				break;

				case Block::EP0:
				{
					assert(b->params.Length == 1);
					cout << "EP0(" <<
						b->params[0]->Name() << ")";
				}
				break;

				case Block::EP1:
				{
					assert(b->params.Length == 1);
					cout << "EP1(" <<
						b->params[0]->Name() << ")";
				}
				break;

				case Block::SIG0:
				{
					assert(b->params.Length == 1);
					cout << "SIG0(" <<
						b->params[0]->Name() << ")";
				}
				break;

				case Block::SIG1:
				{
					assert(b->params.Length == 1);
					cout << "SIG1(" <<
						b->params[0]->Name() << ")";
				}
				break;
				}
			}
			break;
			
			}
		}
		
#pragma endregion

#pragma region BlockBuffer
		bool BlockBuffer::Release(Block *b)
		{
			if (Contains(b))
			{
				Remove(b);
				delete b;
				
				return true;
			}

			return false;
		}

		void BlockBuffer::ReleaseAll()
		{
			for (auto b : *this)
				delete b;

			dataCEnd = data;
		}

		Block *BlockBuffer::Create()
		{
			Block *b = new Block();

			Add(b);

			return b;
		}

		Block *BlockBuffer::Create_Static(const NameI name)
		{
			Block *r = Create();
			r->type = Block::Static;
			r->name = name.name;
			r->index = name.index;
			r->hideName = name.hide;

			return r;
		}

		Block *BlockBuffer::Create_Dynamic(const NameI name)
		{
			Block *r = Create();
			r->type = Block::Dynamic;
			r->name = name.name;
			r->index = name.index;
			r->hideName = name.hide;

			return r;
		}

		Block *BlockBuffer::Create_Ref(const NameI name, Block *link)
		{
			assert(!(link->name == name.name && link->index == name.index));

			Block *r = Create();
			r->type = Block::Reference;
			r->name = name.name;
			r->index = name.index;
			r->hideName = name.hide;
			r->params.Allocate(1);
			*r->params.dataCEnd++ = link;

			return r;
		}

		Block *BlockBuffer::Create_Op(const NameI name, const BlockOperator::Operator op, int argsCount, Block* ...)
		{
			Block *r = Create();
			r->type = Block::Operator;
			r->name = name.name;
			r->index = name.index;
			r->hideName = name.hide;
			r->opGroup.op = op;
			r->opGroup.blocks.Allocate(argsCount);

			va_list ap;
			va_start(ap, argsCount);
			for (int i = 0; i < argsCount; i++)
			{
				Block *b = va_arg(ap, Block*);

				assert(!(b->name == name.name && b->index == name.index));

				*r->opGroup.blocks.dataCEnd++ = b;
			}

			va_end(ap);

			return r;
		}

		Block *BlockBuffer::Create_Function(const NameI name, const Block::LogicFunction f, Block* ...)
		{
			va_list ap;
			int argsCount = 1;

			switch (f)
			{
			case Block::CH:
			case Block::MAJ:
				va_start(ap, 3);
				argsCount = 3;
				break;

			case Block::EP0:
			case Block::EP1:
			case Block::SIG0:
			case Block::SIG1:
				va_start(ap, 1);
				argsCount = 1;
				break;

			default:
				throw "unknown function type";
			}


			Block *r = Create();
			r->type = Block::Fuction;
			r->name = name.name;
			r->index = name.index;
			r->hideName = name.hide;
			r->func = f;
			r->params.Allocate(argsCount);

			for (int i = 0; i < argsCount; i++)
			{
				Block *b = va_arg(ap, Block*);

				assert(!(b->name == name.name && b->index == name.index));

				*r->params.dataCEnd++ = b;
			}

			va_end(ap);

			return r;
		}

#pragma endregion
		
		void Print(Block *b)
		{
			cout << b->Name() << " = ";
			PrintValues(b);
		}

		inline void Increase(Block *b, SortedDictionary<Block*, unsigned> *list)
		{
			unsigned index;

			if (list->Find(b, index))
				list->data[index].Value++;
			else
				list->Add(b, 1);
		}

		void ListSubs(BlockOperator::Operator op, Block *b, SortedDictionary<Block*, unsigned> *list)
		{
			if (b->type == Block::Reference)
			{
				ListSubs(op, b->params[0], list);
				return;
			}

			if (b->type == Block::Operator && b->opGroup.op == op)
			{
				for (auto o : b->opGroup.blocks)
					ListSubs(op, o, list);

				return;
			}

			Increase(b, list);
		}

		static void ListSubs(Block *opBlock)
		{
			assert(opBlock->type == Block::Operator);

			cout << opBlock->Name() << endl << "---------" << endl;

			SortedDictionary<Block*, unsigned> list;

			for (auto block : opBlock->opGroup.blocks)
				ListSubs(opBlock->opGroup.op, block, &list);

			SortedDictionary<std::string, Block*> sortedKeys;
			sortedKeys.Allocate(list.Length);

			for (auto item : list)
				sortedKeys.Add(item.Key->Name(), item.Key);
			
			for (auto key : sortedKeys)
			{
				unsigned *amount;
				Block *b = key.Value;

				if (list.TryGetValue(b, amount))
					cout << " -> " << b->Name() << "\t " << *amount << endl;
				else
					throw "block is not in sub list";
			}
			
		}

		
		void Run(int argc, char* argv[])
		{
			BlockBuffer buffer;


			Block *k[64];
			for (int i = 0; i < 64; i++)
				k[i] = buffer.Create_Static(NameI("k", i));


			Block *data[16];
			for (int i = 0; i < 16; i++)
				data[i] = buffer.Create_Dynamic(NameI("data", i));


			Block *m[64];
			for (int i = 0; i < 16; i++)
				m[i] = buffer.Create_Ref(NameI("m", i), data[i]);

			for (int i = 16; i < 64; i++)
				m[i] = buffer.Create_Op(NameI("m", i), BlockOperator::Addition, 4,
					buffer.Create_Function(NameI("loadSig1", i, true), Block::SIG1, m[i - 2]),
					m[i - 1],
					buffer.Create_Function(NameI("loadSig0", i, true), Block::SIG0, m[i - 15]),
					m[i - 16]);

			Block *state[8];
			for (int i = 0; i < 8; i++)
				state[i] = buffer.Create_Dynamic(NameI("state", i));


			Block
				*a = state[0],
				*b = state[1],
				*c = state[2],
				*d = state[3],
				*e = state[4],
				*f = state[5],
				*g = state[6],
				*h = state[7],
				*t1, *t2;


			for (int i = 0; i < 64; i++)
			{
				t1 = buffer.Create_Op(NameI("t1", i), BlockOperator::Addition, 5,
					h,
					buffer.Create_Function(NameI("t1_EP1", i), Block::LogicFunction::EP1, e),
					buffer.Create_Function(NameI("t1_CH", i), Block::LogicFunction::CH, e, f, g),
					k[i],
					m[i]);
				t2 = buffer.Create_Op(NameI("t2", i), BlockOperator::Addition, 2,
					buffer.Create_Function(NameI("t2_EP0", i), Block::LogicFunction::EP0, a),
					buffer.Create_Function(NameI("t2_MAJ", i), Block::LogicFunction::MAJ, a, b, c));

				h = g;
				g = f;
				f = e;

				e = buffer.Create_Op(NameI("e", i), BlockOperator::Addition, 2,
					d,
					t1);

				d = c;
				c = b;
				b = a;

				a = buffer.Create_Op(NameI("a", i), BlockOperator::Addition, 2,
					t1,
					t2);
			}

			Block *out[8]
			{
				buffer.Create_Op(NameI("out", 0), BlockOperator::Addition, 2, state[0], a),
				buffer.Create_Op(NameI("out", 1), BlockOperator::Addition, 2, state[1], b),
				buffer.Create_Op(NameI("out", 2), BlockOperator::Addition, 2, state[2], c),
				buffer.Create_Op(NameI("out", 3), BlockOperator::Addition, 2, state[3], d),
				buffer.Create_Op(NameI("out", 4), BlockOperator::Addition, 2, state[4], e),
				buffer.Create_Op(NameI("out", 5), BlockOperator::Addition, 2, state[5], f),
				buffer.Create_Op(NameI("out", 6), BlockOperator::Addition, 2, state[6], g),
				buffer.Create_Op(NameI("out", 7), BlockOperator::Addition, 2, state[7], h)
			};



			auto runToken = IXAlgo::Cmdline_args::GetToken(argc, argv, "run");

			if (runToken.arg_count > 1)
			{
				int i = atoi(argv[runToken.arg_index + 2]);

				ListSubs(out[i]);

				cout << endl;
			}
		}
	}
}
