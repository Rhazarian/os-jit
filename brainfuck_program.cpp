#include "brainfuck_program.h"

#include <cstdio>
#include <cstring>
#include <cstdint>

#include <sys/mman.h>

const unsigned char brainfuck_program::prologue[] = { 
	0x55,											// push		rbp
	0x48, 0x89, 0xe5,								// mov rbp, rsp
	0x48, 0x81, 0xec, 0x30, 0x75, 0x00, 0x00,		// sub rsp, 0x7530
	0x48, 0x31, 0xdb								// xor		rbx, rbx
};

const unsigned char brainfuck_program::epilogue[] = {
	0xc9,											// leave
	0xc3											// ret
};

const unsigned char brainfuck_program::next[] = {
	0xff, 0xc3										// inc ebx
};

const unsigned char brainfuck_program::prev[] = {
	0xff, 0xcb										// dec ebx
};

const unsigned char brainfuck_program::inc[] = {
	0xfe, 0x84, 0x1d, 0xd0, 0x8a, 0xff, 0xff		// inc BYTE PTR [rbp + rbx*1 - 0x7530]
};

const unsigned char brainfuck_program::dec[] = {
	0xfe, 0x8c, 0x1d, 0xd0, 0x8a, 0xff, 0xff		// dec BYTE PTR [rbp + rbx*1 - 0x7530]
};

const unsigned char brainfuck_program::putchar[] = {
	0x48, 0xc7, 0xc0, 0x01, 0x00, 0x00, 0x00,		// mov rax, 0x1
	0x48, 0xc7, 0xc7, 0x01, 0x00, 0x00, 0x00,		// mov rdi, 0x1
	0x48, 0x8d, 0xb4, 0x1d, 0xd0, 0x8a, 0xff, 0xff,	// lea rsi, [rbp + rbx*1 - 0x7530]
	0x48, 0xc7, 0xc2, 0x01, 0x00, 0x00, 0x00,		// mov rdx, 0x1
	0x0f, 0x05,										// syscall
};

const unsigned char brainfuck_program::getchar[] = {
	0x48, 0xc7, 0xc0, 0x00, 0x00, 0x00, 0x00,		// mov rax, 0x0
	0x48, 0xc7, 0xc7, 0x00, 0x00, 0x00, 0x00,		// mov rdi, 0x0
	0x48, 0x8d, 0xb4, 0x1d, 0xd0, 0x8a, 0xff, 0xff,	// lea rsi, [rbp + rbx*1 - 0x7530]
	0x48, 0xc7, 0xc2, 0x01, 0x00, 0x00, 0x00,		// mov rdx, 0x1
	0x0f, 0x05,										// syscall
};

const unsigned char brainfuck_program::loop_start[] = {
	0x80, 0xbc, 0x1d, 0xd0, 0x8a, 0xff, 0xff, 0x00,	// cmp BYTE PTR [rbp + rbx*1 - 0x7530], 0x0
	0x0f, 0x84, 0x00, 0x00, 0x00, 0x00				// je +0
};

const unsigned char brainfuck_program::loop_end[] = {
	0xe9, 0x00, 0x00, 0x00, 0x00					// jmp +0
};

void brainfuck_program::add_code(const unsigned char* const bytes, const size_t bytes_size)
{
	code_.reserve(code_.size() + bytes_size);
	std::copy(bytes, bytes + bytes_size, std::back_inserter(code_));
}

brainfuck_program::brainfuck_program()
{
	add_code(prologue, sizeof(prologue));
}

void brainfuck_program::add_instruction(const instruction instr)
{
	const unsigned char* bytes = nullptr;
	size_t bytes_size = 0;
	switch (instr)
	{
	case instruction::next:
		bytes = next;
		bytes_size = sizeof(next);
		break;
	case instruction::prev:
		bytes = prev;
		bytes_size = sizeof(prev);
		break;
	case instruction::inc:
		bytes = inc;
		bytes_size = sizeof(inc);
		break;
	case instruction::dec:
		bytes = dec;
		bytes_size = sizeof(dec);
		break;
	case instruction::putchar:
		bytes = putchar;
		bytes_size = sizeof(putchar);
		break;
	case instruction::getchar:
		bytes = getchar;
		bytes_size = sizeof(getchar);
		break;
	case instruction::loop_start:
		bytes = loop_start;
		bytes_size = sizeof(loop_start);
		loops_.push(code_.size() + bytes_size);
		break;
	case instruction::loop_end:
		bytes = loop_end;
		bytes_size = sizeof(loop_end);
		break;
	}
	add_code(bytes, bytes_size);
	if (instr == instruction::loop_end)
	{
		const int32_t jmp_dist = code_.size() - loops_.top();
		memcpy(&code_[loops_.top() - 4], &jmp_dist, 4);
		const int32_t back_jmp_dist = -(jmp_dist + sizeof(loop_start));
		memcpy(&code_[code_.size() - 4], &back_jmp_dist, 4);
		loops_.pop();
	}
}

std::shared_ptr<void()> brainfuck_program::compile()
{
	if (!loops_.empty())
	{
		throw compiler_exception("parenthesis mismatch");
	}
	const auto mem_size = code_.size() + sizeof(epilogue);
	const auto mem = mmap(nullptr, mem_size, PROT_WRITE | PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (mem == MAP_FAILED)
	{
		throw compiler_exception("could not map memory");
	}
	auto ptr = std::shared_ptr<void()>(reinterpret_cast<void(*)()>(mem), [mem, mem_size](void(*mem_ptr)())
	{
		munmap(mem, mem_size);
	});
	memcpy(mem, code_.data(), code_.size());
	memcpy(static_cast<char*>(mem) + code_.size(), epilogue, sizeof(epilogue));
	if (mprotect(mem, mem_size, PROT_EXEC | PROT_READ) == -1) {
		throw compiler_exception(std::string("could not change memory protection: ") + std::strerror(errno));
	}
	return ptr;
}