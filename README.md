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

### PPU

The PPU currently allows for full VRAM reading and rendering, with the complete GameBoy render pipeline in place. This includes OAM, drawing, HBlank and VBlank

### Testing

The code currently boots most industry grade cartridges to the display!

In terms of next steps, I will be rewriting the python assembler to an actual C++ assembler, in order to assemble custom assembly programs to the emulator, and run them. 

I will also be running Blargg's test ROMs, hoping for a pass, to verify the quality of the CPU code.

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

## Building

Quick warning - this is the first properly built project of mine, so its not clean building yet <br>
That will come at a later date I promise

# Requirements

- Windows
- Visual Studio with Desktop Development with C++ installed (I used VS 2026, not sure how much it matters)
- MSVC v145 toolset
- Windows 10 SDK
- SDL2 dev libraries

# Setup

1. Clone my repository
   ```GitHub repo
   https://github.com/TriedDevotee/GameBoy-Emulator.git
   ```
2. Install SDL2 and put the dev libraries somewhere on your machine
3. Open 'GameBoyEmulator.slnx' in Visual Studio
4. Update the SDL2 paths in the project properties to match your SDL2 installation:

C/C++ -> General -> Additional Include Directories: Point to the SDL2 include directory.

Linker -> General -> Additional Library Directories: Point to the SDL2 lib/x64 directory.

Linker -> Input -> Additional Dependencies: Ensure SDL2.lib is included.

5. Select the debug configuration and x64
6. Build with Ctrl + Shift + B

The whole linker thing is what I need to sort, I do apologise :D
