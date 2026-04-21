#pragma once

#include <ixalgo/IXA_Core.h>
#include <ixalgo/collections/List.h>
#include <string>
#include <functional>
#include <assert.h>

namespace SHA
{
	namespace Log
	{
		class Block;
		struct BlockOperator
		{
		public:
			enum Operator
			{
				MultiplicationSingle,
				Shift_RRR,
				Shift_RR,
				Addition,
				AND,
				XOR,
				OR,
			} op;

			int opParam;
			IXAlgo::Collections::Array<Block*, unsigned> blocks;
		};

		struct Block
		{
		public:
			enum TValue
			{
				Dynamic,
				Static,
				Reference,
				Operator,
				Fuction
			} type;

			enum LogicFunction
			{
				CH,
				MAJ,
				EP0,
				EP1,
				SIG0,
				SIG1
			} func;

			std::string name;
			int index = -1;
			bool hideName = false;
			BlockOperator opGroup;
			IXAlgo::Collections::Array<Block*, unsigned> params;

			std::string Name() const;

			struct NameIndex
			{
				std::string name;
				int index;
				bool hide;

			public:
				inline NameIndex(const char *n, const int i = -1, const bool h = false) : 
					name(n),
					index(i),
					hide(h)
				{}
			};

			
		};

		class BlockBuffer : 
			protected IXAlgo::Collections::List<Block*, unsigned>
		{
		public:
			~BlockBuffer();
		protected:
			bool Release(Block *b);

		public:
			void ReleaseAll();

		public:
			Block *Create();

			Block *Create_Static(const Block::NameIndex name);
			Block *Create_Dynamic(const Block::NameIndex name);
			Block *Create_Ref(const Block::NameIndex name, Block* link);
			Block *Create_Op(const Block::NameIndex name, const BlockOperator::Operator op, int argsCount, Block* ...);
			Block *Create_Function(const Block::NameIndex name, const Block::LogicFunction f, Block* ...);
		};

		inline BlockBuffer::~BlockBuffer()
		{
			ReleaseAll();
		}
		

		inline std::string Block::Name() const
		{
			assert(!name.empty());

			if (index < 0)
				return name;
			else
				return name + "[" + std::to_string(index) + "]";
		}
	}
}
