#pragma once

#include <cstdint>

#include "Memory.h"

class CPU {
private:

	int execute(uint8_t opcode);

	Memory* memory;

	uint8_t A;
	uint8_t F;

	uint8_t B;
	uint8_t C;

	uint8_t D;
	uint8_t E;

	uint8_t H;
	uint8_t L;

	uint16_t PC;
	uint16_t SP;

	uint32_t cycles;

	bool IME = false;
	bool enableInterruptsPending = false;
	uint8_t imeEnableDelay = 0;


	enum class CPUState {
		Running,
		Halted,
		Stopped
	};

	CPUState state = CPU::CPUState::Running;
	bool haltBug = false;

public:
	bool running;

	CPU(Memory* memory);

	int step();
	void printState();

	bool getZeroFlag();
	bool getSubtractFlag();
	bool getHalfCarryFlag();
	bool getCarryFlag();

	void setZeroFlag(bool val);
	void setSubtractFlag(bool val);
	void setHalfCarryFlag(bool val);
	void setCarryFlag(bool val);

	void Add(uint8_t value);
	void Sub(uint8_t value);
	void Load(uint8_t& reg);
	void Inc(uint8_t& reg);
	void Dec(uint8_t& reg);
	void jump(bool condition);
	void jumpRelative(bool condition);
	void compare(uint8_t value);
	void call();
	void ret();
	void ADC(uint8_t value);
	void SBC(uint8_t value);

	uint16_t getHL();
	uint16_t getBC();
	uint16_t getDE();
	uint16_t getAF();

	void setBC(uint16_t data);
	void setDE(uint16_t data);
	void setHL(uint16_t data);
	void setAF(uint16_t data);

	void push(uint16_t value);
	uint16_t pop();

	void And(uint8_t value);
	void Or(uint8_t value);
	void Xor(uint8_t value);

	void AddBig(uint16_t value);
	void SubBig(uint16_t value);

	int executeCB(uint8_t opcode);
	uint8_t& getRegisterCB(uint8_t index);
	void RLC(uint8_t& reg);
	void RRC(uint8_t& reg);
	void RL(uint8_t& reg);
	void RR(uint8_t& reg);
	void SLA(uint8_t& reg);
	void SRA(uint8_t& reg);
	void SRL(uint8_t& reg);
	void Swap(uint8_t& reg);
	void Bit(uint8_t bit, uint8_t value);
	void Res(uint8_t bit, uint8_t& value);
	void Set(uint8_t bit, uint8_t& value);
	
	void DAA(uint8_t& reg);
	void CPL(uint8_t& reg);
	void SCF();
	void CCF();
	void RLCA(uint8_t& reg);
	void RRCA(uint8_t& reg);
	void RLA(uint8_t& reg);
	void RRA(uint8_t& reg);

	void storeSPtoAddress(uint16_t address);
	uint16_t readAddress();

	void incrementHL(bool increment);
	uint16_t addToSP(int8_t value);

	void CallConditional(bool condition);
	void RetConditional(bool condition);
	void RST(uint16_t address);

	//GODDAMN STUPID BLOODY INTERRUPT CODE I WANT TO WRITE ANYTHING OTHER THAN THIS AHHHHHHHHHHHH

	void DI();
	void EI();
	void RETI();

	uint8_t getPendingInterrupts();
	int serviceInterrupt();
};