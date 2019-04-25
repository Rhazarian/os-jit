#ifndef BRAINFUCK_PROGRAM_H
#define BRAINFUCK_PROGRAM_H

#include <vector>
#include <stack>
#include <memory>
#include <stdexcept>

struct brainfuck_program
{
	brainfuck_program();

	enum class instruction
	{
		next,
		prev,
		inc,
		dec,
		putchar,
		getchar,
		loop_start,
		loop_end
	};

	struct compiler_exception : std::runtime_error
	{
		using std::runtime_error::runtime_error;
	};

	void add_instruction(instruction instr);

	std::shared_ptr<void()> compile();

private:
	static const unsigned char prologue[];
	static const unsigned char epilogue[];

	static const unsigned char next[];
	static const unsigned char prev[];
	static const unsigned char inc[];
	static const unsigned char dec[];
	static const unsigned char putchar[];
	static const unsigned char getchar[];
	static const unsigned char loop_start[];
	static const unsigned char loop_end[];

	std::vector<unsigned char> code_{};
	std::stack<size_t> loops_{};

	void add_code(const unsigned char* bytes, size_t bytes_size);
};

#endif // BRAINFUCK_PROGRAM_H