#include <iostream>

#include "brainfuck_program.h"

int main()
{
	brainfuck_program program;
	std::cout << "Enter a brainfuck program to compile & run: " << std::endl;
	for (std::string line; std::getline(std::cin, line); )
	{
		for (const auto ch : line)
		{
			switch (ch)
			{
			case '>':
				program.add_instruction(brainfuck_program::instruction::next);
				break;
			case '<':
				program.add_instruction(brainfuck_program::instruction::prev);
				break;
			case '+':
				program.add_instruction(brainfuck_program::instruction::inc);
				break;
			case '-':
				program.add_instruction(brainfuck_program::instruction::dec);
				break;
			case '.':
				program.add_instruction(brainfuck_program::instruction::putchar);
				break;
			case ',':
				program.add_instruction(brainfuck_program::instruction::getchar);
				break;
			case '[':
				program.add_instruction(brainfuck_program::instruction::loop_start);
				break;
			case ']':
				program.add_instruction(brainfuck_program::instruction::loop_end);
				break;
			default:
				std::cout << "Unexpected symbol: " << ch << std::endl;
				return EXIT_FAILURE;
			}
		}
	}
	try {
		program.compile().get()();
	} catch (const brainfuck_program::compiler_exception& ex)
	{
		std::cout << "Could not compile program: " << ex.what() << std::endl;
		return EXIT_FAILURE;
	}
    return EXIT_SUCCESS;
}