# Game Boy Emulator

A Game Boy emulator written in C++, built around an implementation of the **Sharp LR35902** CPU.

## About

This project started as a way to explore low-level programming, hardware/software integration, and the internal workings of the original Game Boy. It is also an opportunity to challenge myself by building a substantial project in C++, a language I was relatively unfamiliar with when I started.

I chose C++ for its performance, control over memory management, and suitability for modelling hardware components such as registers, memory and CPU instructions. Working on this project has significantly improved my understanding and confidence with the language.

## Current Progress

### CPU

The CPU currently implements all 245 valid primary opcode instructions and all 256 CB-prefixed instructions, covering **501 valid instruction encodings**.

Implemented functionality includes:

- 8-bit and 16-bit arithmetic and logical operations
- Register and memory addressing
- Stack operations
- Conditional and unconditional branching
- Subroutines and returns
- CB-prefixed bit manipulation, rotations and shifts
- Interrupt infrastructure

### Testing

I've developed a custom Python assembler and CPU stress test to generate programs for the emulator.
The assembler was generated as a first draft by GPT-6, as a way to ensure my focus remained on the program at hand.
I will be rewriting the assembler at a later date in C++

The current test exercises instruction execution across arithmetic, flags, memory access, stack operations, bit manipulation and control flow. It successfully reaches a designated success condition without triggering the failure handler.

The CPU also provides register-state tracing to help debug instruction execution.

## Roadmap

Next I am going to add a proper cycle count<br>
And I am gonna give it a super cool PPU <br>
Then it'll run Tetris

## Project Philosophy

I'm prioritising understanding and implementing the underlying hardware over simply getting games to run as quickly as possible. I want to understand how each component works and how the components interact to produce a functioning system.

Eventually, I'd like to get the emulator running Tetris. Then I can worry about making everything look pretty.

## References

I used a lot of GPT-6 as a means of debugging my stack traces, due to me not wanting to do it <br>
I also used the site https://www.pastraiser.com/cpu/gameboy/gameboy_opcodes.html as a means of working out what needed implementing, and where
