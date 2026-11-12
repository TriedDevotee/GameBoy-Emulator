#include <iostream>
#include <cstdint>
#include "CPU.h"
#include "interrupts.h"

CPU::CPU(Memory* memory) {
	std::cout << "CPU is Online\n";

	this->memory = memory;

	A = 0;
	F = 0;

	B = 0;
	C = 0;

	D = 0;
	E = 0;

	H = 0;
	L = 0;

	PC = 0;
	SP = 0xFFFE;

	running = true;

	cycles = 0;
}

int CPU::step() {

	uint8_t pending = getPendingInterrupts();

	if (state == CPUState::Halted) {
		if (pending == 0) {
			return 4;
		}

		state = CPUState::Running;
	}

	if (IME && pending) {
		return serviceInterrupt();
		//Yes this always returns 67 cause ha funny 
		//I will have to fix it later though :(
	}

	//This is where I will 1 day deal with HALT states
	//Why did the inline comment ai shit thing just put "and all that jazz"
	//I am slightly disturbed

	uint16_t instructionAddress = PC;
	uint8_t opcode = memory->read(PC);

	if (haltBug) {
		haltBug = false;
	}
	else {
		PC++;
	}

	std::cout << std::hex
		<< "PC: " << instructionAddress
		<< " | Opcode: " << (int)opcode << '\n';

	cycles = execute(opcode);


	if (imeEnableDelay > 0) {
		imeEnableDelay--;
		if (imeEnableDelay == 0) {
			IME = true;
		}
	}

	return cycles;
}

void CPU::printState() {
	std::cout << "A: " << (int)A << "\n";
	std::cout << "F: " << (int)F << "\n";

	std::cout << "B: " << (int)B << "\n";
	std::cout << "C: " << (int)C << "\n";

	std::cout << "D: " << (int)D << "\n";
	std::cout << "E: " << (int)E << "\n";

	std::cout << "H: " << (int)H << "\n";
	std::cout << "L: " << (int)L << "\n";

	std::cout << "PC: " << (int)PC << "\n";
	std::cout << "SP: " << (int)SP << "\n\n";
}

int CPU::execute(uint8_t opcode) {
	switch (opcode) {
		case 0x00: //NOP
			return 4;
		case 0x10: //STOP
			PC++;
			return 4;
		case 0x06:
			Load(B);
			return 8;
		case 0x0E:
			Load(C);
			return 8;
		case 0x16:
			Load(D);
			return 8;
		case 0x1E:
			Load(E);
			return 8;
		case 0x26:
			Load(H);
			return 8;
		case 0x2E:
			Load(L);
			return 8;
		case 0x3E: //LD A, d8
			Load(A);
			return 8;
		case 0x04:
			Inc(B);
			return 4;
		case 0x0C:
			Inc(C);
			return 4;
		case 0x14:
			Inc(D);
			return 4;
		case 0x1C:
			Inc(E);
			return 4;
		case 0x24:
			Inc(H);
			return 4;
		case 0x2C:
			Inc(L);
			return 4;
		case 0x3C: //INC A
			Inc(A);
			return 4;
		case 0x05:
			Dec(B);
			return 4;
		case 0x0D:
			Dec(C);
			return 4;
		case 0x15:
			Dec(D);
			return 4;
		case 0x1D:
			Dec(E);
			return 4;
		case 0x25:
			Dec(H);
			return 4;
		case 0x2D:
			Dec(L);
			return 4;
		case 0x3D:
			Dec(A);
			return 4;
		case 0xC6: //ADD A, val
		{
			uint8_t value = memory->read(PC);
			PC++;

			Add(value);
			return 8;
		}
		case 0x80:
			Add(B);
			return 4;
		case 0x81:
			Add(C);
			return 4;
		case 0x82:
			Add(D);
			return 4;
		case 0x83:
			Add(E);
			return 4;
		case 0x84:
			Add(H);
			return 4;
		case 0x85:
			Add(L);
			return 4;
		case 0x86:
			Add(memory->read(getHL()));
			return 8;
		case 0x87:
			Add(A);
			return 4;
		case 0x76: //HALT
		{
			std::cout << "HALT\n";

			uint8_t pending = getPendingInterrupts();

			if (!IME && pending != 0) {
				haltBug = true;
			} 
			else {
				state = CPUState::Halted;
			}

			return 4;
		}
		case 0xC3: //JP absolute
			jump(true);
			return 16;
		case 0xC2: //JP Not Equal
			jump(!getZeroFlag());
			if (!getZeroFlag()) return 16;
			else return 12;
		case 0xCA: //JP Equal
			jump(getZeroFlag());
			if (getZeroFlag()) return 16;
			else return 12;
		case 0xFE: //CP A, val
		{
			uint8_t value = memory->read(PC);
			PC++;
			compare(value);
			return 8;
		}
		case 0xCD: //CALL val
			call();
			return 24;
		case 0xC9: //RET val
			ret();
			return 16;
		case 0x40:
			B = B;
			return 4;
		case 0x41:
			B = C;
			return 4;
		case 0x42:
			B = D;
			return 4;
		case 0x43:
			B = E;
			return 4;
		case 0x44:
			B = H;
			return 4;
		case 0x45:
			B = L;
			return 4;
		case 0x46:
			B = memory->read(getHL());
			return 8;
		case 0x47:
			B = A;
			return 4;
		case 0x48:
			C = B;
			return 4;
		case 0x49:
			C = C;
			return 4;
		case 0x4A:
			C = D;
			return 4;
		case 0x4B:
			C = E;
			return 4;
		case 0x4C:
			C = H;
			return 4;
		case 0x4D:
			C = L;
			return 4;
		case 0x4E:
			C = memory->read(getHL());
			return 8;
		case 0x4F:
			C = A;
			return 4;
		case 0x50:
			D = B;
			return 4;
		case 0x51:
			D = C;
			return 4;
		case 0x52:
			D = D;
			return 4;
		case 0x53:
			D = E;
			return 4;
		case 0x54:
			D = H;
			return 4;
		case 0x55:
			D = L;
			return 4;
		case 0x56:
			D = memory->read(getHL());
			return 8;
		case 0x57:
			D = A;
			return 4;
		case 0x58:
			E = B;
			return 4;
		case 0x59:
			E = C;
			return 4;
		case 0x5A:
			E = D;
			return 4;
		case 0x5B:
			E = E;
			return 4;
		case 0x5C:
			E = H;
			return 4;
		case 0x5D:
			E = L;
			return 4;
		case 0x5E:
			E = memory->read(getHL());
			return 8;
		case 0x5F:
			E = A;
			return 4;
		case 0x60:
			H = B;
			return 4;
		case 0x61:
			H = C;
			return 4;
		case 0x62:
			H = D;
			return 4;
		case 0x63:
			H = E;
			return 4;
		case 0x64:
			H = H;
			return 4;
		case 0x65:
			H = L;
			return 4;
		case 0x66:
			H = memory->read(getHL());
			return 8;
		case 0x67:
			H = A;
			return 4;
		case 0x68:
			L = B;
			return 4;
		case 0x69:
			L = C;
			return 4;
		case 0x6A:
			L = D;
			return 4;
		case 0x6B:
			L = E;
			return 4;
		case 0x6C:
			L = H;
			return 4;
		case 0x6D:
			L = L;
			return 4;
		case 0x6E:
			L = memory->read(getHL());
			return 8;
		case 0x6F:
			L = A;
			return 4;
		case 0x70:
			memory->write(getHL(), B);
			return 8;
		case 0x71:
			memory->write(getHL(), C);
			return 8;
		case 0x72:
			memory->write(getHL(), D);
			return 8;
		case 0x73:
			memory->write(getHL(), E);
			return 8;
		case 0x74:
			memory->write(getHL(), H);
			return 8;
		case 0x75:
			memory->write(getHL(), L);
			return 8;
		case 0x77:
			memory->write(getHL(), A);
			return 8;
		case 0x78:
			A = B;
			return 4;
		case 0x79:
			A = C;
			return 4;
		case 0x7A:
			A = D;
			return 4;
		case 0x7B:
			A = E;
			return 4;
		case 0x7C:
			A = H;
			return 4;
		case 0x7D:
			A = L;
			return 4;
		case 0x7E:
			A = memory->read(getHL());
			return 8;
		case 0x7F:
			A = A;
			return 4;
		case 0x01:
		{
			uint8_t low = memory->read(PC);
			PC++;
			uint8_t high = memory->read(PC);
			PC++;
			setBC(low | (high << 8));
			return 12;
		}
		case 0x11:
		{
			uint8_t low = memory->read(PC);
			PC++;
			uint8_t high = memory->read(PC);
			PC++;
			setDE(low | (high << 8));
			return 12;
		}
		case 0x21:
		{
			uint8_t low = memory->read(PC);
			PC++;
			uint8_t high = memory->read(PC);
			PC++;
			setHL(low | (high << 8));
			return 12;
		}
		case 0x31:
		{
			uint8_t low = memory->read(PC);
			PC++;
			uint8_t high = memory->read(PC);
			PC++;
			SP = (low | (high << 8));
			return 12;
		}
		case 0x36:
		{
			uint8_t value = memory->read(PC);
			PC++;

			memory->write(getHL(), value);

			return 12;
		}
		case 0x03:
			setBC(getBC() + 1);
			return 8;
		case 0x13:
			setDE(getDE() + 1);
			return 8;
		case 0x23:
			setHL(getHL() + 1);
			return 8;
		case 0x33:
			SP++;
			return 8;
		case 0x0B:
			setBC(getBC() - 1);
			return 8;
		case 0x1B:
			setDE(getDE() - 1);
			return 8;
		case 0x2B:
			setHL(getHL() - 1);
			return 8;
		case 0x3B:
			SP--;
			return 8;
		case 0xC5:
			push(getBC());
			return 16;
		case 0xD5:
			push(getDE());
			return 16;
		case 0xE5:
			push(getHL());
			return 16;
		case 0xF5:
			push(getAF());
			return 16;
		case 0xC1:
			setBC(pop());
			return 12;
		case 0xD1:
			setDE(pop());
			return 12;
		case 0xE1:
			setHL(pop());
			return 12;
		case 0xF1:
			setAF(pop());
			return 12;
		case 0xD6:
			Sub(memory->read(PC));
			PC++;
			return 8;
		case 0x90:
			Sub(B);
			return 4;
		case 0x91:
			Sub(C);
			return 4;
		case 0x92:
			Sub(D);
			return 4;
		case 0x93:
			Sub(E);
			return 4;
		case 0x94:
			Sub(H);
			return 4;
		case 0x95:
			Sub(L);
			return 4;
		case 0x96:
			Sub(memory->read(getHL()));
			return 8;
		case 0x97:
			Sub(A);
			return 4;
		case 0xA0:
			And(B);
			return 4;
		case 0xA1:
			And(C);
			return 4;
		case 0xA2:
			And(D);
			return 4;
		case 0xA3:
			And(E);
			return 4;
		case 0xA4:
			And(H);
			return 4;
		case 0xA5:
			And(L);
			return 4;
		case 0xA6:
			And(memory->read(getHL()));
			return 8;
		case 0xA7:
			And(A);
			return 4;
		case 0xE6:
			And(memory->read(PC));
			PC++;
			return 8;
		case 0xB0:
			Or(B);
			return 4;
		case 0xB1:
			Or(C);
			return 4;
		case 0xB2:
			Or(D);
			return 4;
		case 0xB3:
			Or(E);
			return 4;
		case 0xB4:
			Or(H);
			return 4;
		case 0xB5:
			Or(L);
			return 4;
		case 0xB6:
			Or(memory->read(getHL()));
			return 8;
		case 0xB7:
			Or(A);
			return 4;
		case 0xF6:
			Or(memory->read(PC));
			PC++;
			return 8;
		case 0xA8:
			Xor(B);
			return 4;
		case 0xA9:
			Xor(C);
			return 4;
		case 0xAA:
			Xor(D);
			return 4;
		case 0xAB:
			Xor(E);
			return 4;
		case 0xAC:
			Xor(H);
			return 4;
		case 0xAD:
			Xor(L);
			return 4;
		case 0xAE:
			Xor(memory->read(getHL()));
			return 8;
		case 0xAF:
			Xor(A);
			return 4;
		case 0xEE:
			Xor(memory->read(PC));
			PC++;
			return 8;
		case 0x09:
			AddBig(getBC());
			return 8;
		case 0x19:
			AddBig(getDE());
			return 8;
		case 0x29:
			AddBig(getHL());
			return 8;
		case 0x39:
			AddBig(SP);
			return 8;
		case 0x18:
			jumpRelative(true);
			return 8;
		case 0x20:
			jumpRelative(!getZeroFlag());
			if (!getZeroFlag()) return 12;
			else return 8;
		case 0x28:
			jumpRelative(getZeroFlag());
			if (getZeroFlag()) return 12;
			else return 8;
		case 0x30:
			jumpRelative(!getCarryFlag());
			if (!getCarryFlag()) return 12;
			else return 8;
		case 0x38:
			jumpRelative(getCarryFlag());
			if (getCarryFlag()) return 12;
			else return 8;
		case 0xB8:
			compare(B);
			return 4;
		case 0xB9:
			compare(C);
			return 4;
		case 0xBA:
			compare(D);
			return 4;
		case 0xBB:
			compare(E);
			return 4;
		case 0xBC:
			compare(H);
			return 4;
		case 0xBD:
			compare(L);
			return 4;
		case 0xBE:
			compare(memory->read(getHL()));
			return 8;
		case 0xBF:
			compare(A);
			return 4;
		case 0x88:
			ADC(B);
			return 4;
		case 0x89:
			ADC(C);
			return 4;
		case 0x8A:
			ADC(D);
			return 4;
		case 0x8B:
			ADC(E);
			return 4;
		case 0x8C:
			ADC(H);
			return 4;
		case 0x8D:
			ADC(L);
			return 4;
		case 0x8E:
			ADC(memory->read(getHL()));
			return 8;
		case 0x8F:
			ADC(A);
			return 4;
		case 0xCE:
			ADC(memory->read(PC));
			PC++;
			return 8;
		case 0x98:
			SBC(B);
			return 4;
		case 0x99:
			SBC(C);
			return 4;
		case 0x9A:
			SBC(D);
			return 4;
		case 0x9B:
			SBC(E);
			return 4;
		case 0x9C:
			SBC(H);
			return 4;
		case 0x9D:
			SBC(L);
			return 4;
		case 0x9E:
			SBC(memory->read(getHL()));
			return 8;
		case 0x9F:
			SBC(A);
			return 4;
		case 0xDE:
			SBC(memory->read(PC));
			PC++;
			return 8;
		case 0xCB:
		{
			uint8_t newOpcode = memory->read(PC);
			PC++;
			std::cout << "CB Opcode: " << std::hex << (int)newOpcode << "\n";
			return executeCB(newOpcode);
		}
		case 0x27:
			DAA(A);
			return 4;
		case 0x2F:
			CPL(A);
			return 4;
		case 0x07:
			RLCA(A);
			return 4;
		case 0x0F:
			RRCA(A);
			return 4;
		case 0x17:
			RLA(A);
			return 4;
		case 0x1F:
			RRA(A);
			return 4;
		case 0x37:
			SCF();
			return 4;
		case 0x3F:
			CCF();
			return 4;
		case 0x02:
			memory->write(getBC(), A);
			return 8;
		case 0x0A:
			A = memory->read(getBC());
			return 8;
		case 0x08:
			storeSPtoAddress(readAddress());
			return 20;
		case 0x12:
			memory->write(getDE(), A);
			return 8;
		case 0x1A:
			A = memory->read(getDE());
			return 8;
		case 0x22:
		{
			uint16_t address = getHL();
			memory->write(address, A);
			setHL(address + 1);
			return 8;
		}
		case 0x2A:
		{
			uint16_t address = getHL();
			A = memory->read(address);
			setHL(address + 1);
			return 8;
		}
		case 0x32:
		{
			uint16_t address = getHL();
			memory->write(address, A);
			setHL(address - 1);
			return 8;
		}
		case 0x3A:
		{
			uint16_t address = getHL();
			A = memory->read(address);
			setHL(address - 1);
			return 8;
		}
		case 0xEA:
			memory->write(readAddress(), A);
			return 16;
		case 0xFA:
			A = memory->read(readAddress());
			return 16;
		case 0xE0:
		{
			uint8_t address = memory->read(PC);
			PC++;
			memory->write(0xFF00 + address, A);
			return 12;
		}
		case 0xF0:
		{
			uint8_t address = memory->read(PC);
			PC++;
			A = memory->read(0xFF00 + address);
			return 12;
		}
		case 0xE2:
			memory->write(0xFF00 + C, A);
			return 8;
		case 0xF2:
			A = memory->read(0xFF00 + C);
			return 8;
		case 0xF9:
			SP = getHL();
			return 8;
		case 0x34:
			incrementHL(true);
			return 12;
		case 0x35:
			incrementHL(false);
			return 12;
		case 0xE8:
		{
			int8_t offset = (int8_t)memory->read(PC);
			PC++;
			SP = addToSP(offset);
			return 16;
		}
		case 0xF8:
		{
			int8_t offset = (int8_t)memory->read(PC);
			PC++;
			setHL(addToSP(offset));
			return 12;
		}
		case 0xC4:
			CallConditional(!getZeroFlag());
			if (!getZeroFlag()) return 24;
			else return 12;
		case 0xCC:
			CallConditional(getZeroFlag());
			if (getZeroFlag()) return 24;
			else return 12;
		case 0xD4:
			CallConditional(!getCarryFlag());
			if (!getCarryFlag()) return 24;
			else return 12;
		case 0xDC:
			CallConditional(getCarryFlag());
			if (getCarryFlag()) return 24;
			else return 12;
		case 0xC0:
			RetConditional(!getZeroFlag());
			if (!getZeroFlag()) return 20;
			else return 8;
		case 0xC8:
			RetConditional(getZeroFlag());
			if (getZeroFlag()) return 20;
			else return 8;
		case 0xD0:
			RetConditional(!getCarryFlag());
			if (!getCarryFlag()) return 20;
			else return 8;
		case 0xD8:
			RetConditional(getCarryFlag());
			if (getCarryFlag()) return 20;
			else return 8;
		case 0xC7:
			RST(0x0000);
			return 16;
		case 0xCF:
			RST(0x0008);
			return 16;
		case 0xD7:
			RST(0x0010);
			return 16;
		case 0xDF:
			RST(0x0018);
			return 16;
		case 0xE7:
			RST(0x0020);
			return 16;
		case 0xEF:
			RST(0x0028);
			return 16;
		case 0xF7:
			RST(0x0030);
			return 16;
		case 0xFF:
			RST(0x0038);
			return 16;
		case 0xE9:
			PC = getHL();
			return 4;
		case 0xF3:
			DI();
			return 4;
		case 0xFB:
			EI();
			return 4;
		case 0xD9:
			RETI();
			return 16;
		case 0xD2:
			jump(!getCarryFlag());
			if (!getCarryFlag()) return 16;
			else return 12;
		case 0xDA:
			jump(getCarryFlag()); //HAPPY LAST OPCODE TO BE IMPLEMENTED 🎉🎉🎉
			if (getCarryFlag()) return 16;
			else return 12;


		default: //Yeah something is not right if this runs // Legit just the bloody crash handler
			std::cout << "Unknown opcode:" << (int) opcode << "\n";
			printState();
			execute(0x76); //Halt

	}
}

int CPU::executeCB(uint8_t opcode) {
	switch (opcode) {
		case 0x00:
			RLC(B);
			return 8;
		case 0x01:
			RLC(C);
			return 8;
		case 0x02:
			RLC(D);
			return 8;
		case 0x03:
			RLC(E);
			return 8;
		case 0x04:
			RLC(H);
			return 8;
		case 0x05:
			RLC(L);
			return 8;
		case 0x06:
		{
			uint8_t value = memory->read(getHL());
			RLC(value);
			memory->write(getHL(), value);
			return 16;
		}
		case 0x07:
			RLC(A);
			return 8;
		case 0x08:
			RRC(B);
			return 8;
		case 0x09:
			RRC(C);
			return 8;
		case 0x0A:
			RRC(D);
			return 8;
		case 0x0B:
			RRC(E);
			return 8;
		case 0x0C:
			RRC(H);
			return 8;
		case 0x0D:
			RRC(L);
			return 8;
		case 0x0E:
		{
			uint8_t value = memory->read(getHL());
			RRC(value);
			memory->write(getHL(), value);
			return 16;
		}
		case 0x0F:
			RRC(A);
			return 8;
		case 0x10:
			RL(B);
			return 8;
		case 0x11:
			RL(C);
			return 8;
		case 0x12:
			RL(D);
			return 8;
		case 0x13:
			RL(E);
			return 8;
		case 0x14:
			RL(H);
			return 8;
		case 0x15:
			RL(L);
			return 8;
		case 0x16:
		{
			uint8_t value = memory->read(getHL());
			RL(value);
			memory->write(getHL(), value);
			return 16;
		}
		case 0x17:
			RL(A);
			return 8;
		case 0x18:
			RR(B);
			return 8;
		case 0x19:
			RR(C);
			return 8;
		case 0x1A:
			RR(D);
			return 8;
		case 0x1B:
			RR(E);
			return 8;
		case 0x1C:
			RR(H);
			return 8;
		case 0x1D:
			RR(L);
			return 8;
		case 0x1E:
		{
			uint8_t value = memory->read(getHL());
			RR(value);
			memory->write(getHL(), value);
			return 16;
		}
		case 0x1F:
			RR(A);
			return 8;
		case 0x20:
			SLA(B);
			return 8;
		case 0x21:
			SLA(C);
			return 8;
		case 0x22:
			SLA(D);
			return 8;
		case 0x23:
			SLA(E);
			return 8;
		case 0x24:
			SLA(H);
			return 8;
		case 0x25:
			SLA(L);
			return 8;
		case 0x26:
		{
			uint8_t value = memory->read(getHL());
			SLA(value);
			memory->write(getHL(), value);
			return 16;
		}
			return 8;
		case 0x27:
			SLA(A);
			return 8;
		case 0x28:
			SRA(B);
			return 8;
		case 0x29:
			SRA(C);
			return 8;
		case 0x2A:
			SRA(D);
			return 8;
		case 0x2B:
			SRA(E);
			return 8;
		case 0x2C:
			SRA(H);
			return 8;
		case 0x2D:
			SRA(L);
			return 8;
		case 0x2E:
		{
			uint8_t value = memory->read(getHL());
			SRA(value);
			memory->write(getHL(), value);
			return 16;
		}
		case 0x2F:
			SRA(A);
			return 8;
		case 0x30:
			Swap(B);
			return 8;
		case 0x31:
			Swap(C);
			return 8;
		case 0x32:
			Swap(D);
			return 8;
		case 0x33:
			Swap(E);
			return 8;
		case 0x34:
			Swap(H);
			return 8;
		case 0x35:
			Swap(L);
			return 8;
		case 0x36:
		{
			uint8_t value = memory->read(getHL());
			Swap(value);
			memory->write(getHL(), value);
			return 16;
		}
		case 0x37:
			Swap(A);
			return 8;
		case 0x38:
			SRL(B);
			return 8;
		case 0x39:
			SRL(C);
			return 8;
		case 0x3A:
			SRL(D);
			return 8;
		case 0x3B:
			SRL(E);
			return 8;
		case 0x3C:
			SRL(H);
			return 8;
		case 0x3D:
			SRL(L);
			return 8;
		case 0x3E:
		{
			uint8_t value = memory->read(getHL());
			SRL(value);
			memory->write(getHL(), value);
			return 16;
		}
		case 0x3F:
			SRL(A);
			return 80;
		default:
			uint8_t bit = (opcode >> 3) & 0b111;
			uint8_t reg = opcode & 0b111;

			bool isHL = reg == 6;

			if (opcode >= 0x40 && opcode <= 0x7F) {
				if (!isHL) {
					uint8_t& regRef = getRegisterCB(reg);
					Bit(bit, regRef);

					return 8;
				}
				else {
					uint8_t regRef = memory->read(getHL());
					Bit(bit, regRef);

					return 12;
				}
			}
			else if (opcode >= 0x80 && opcode <= 0xBF) {
				if (!isHL) {
					uint8_t& regRef = getRegisterCB(reg);
					Res(bit, regRef);

					return 8;
				}
				else {
					uint8_t regRef = memory->read(getHL());
					Res(bit, regRef);
					memory->write(getHL(), regRef);

					return 16;
				}
			}
			else if (opcode >= 0xC0 && opcode <= 0xFF) {
				if (!isHL) {
					uint8_t& regRef = getRegisterCB(reg);
					Set(bit, regRef);

					return 8;
				}
				else {
					uint8_t regRef = memory->read(getHL());
					Set(bit, regRef);
					memory->write(getHL(), regRef);

					return 16;
				}
			}
			else {
				throw std::runtime_error("Invalid CB opcode given");
				return 0;
			}
	}
}

bool CPU::getZeroFlag() {
	return F & 0b10000000;
}

bool CPU::getSubtractFlag() {
	return F & 0b01000000;
}

bool CPU::getHalfCarryFlag() {
	return F & 0b00100000;
}

bool CPU::getCarryFlag() {
	return F & 0b00010000;
}

void CPU::setZeroFlag(bool value) {
	if (value) {
		F |= 0b10000000;
	}
	else {
		F &= 0b01111111;
	}
}

void CPU::setSubtractFlag(bool value) {
	if (value) {
		F |= 0b01000000;
	}
	else {
		F &= 0b10111111;
	}
}

void CPU::setHalfCarryFlag(bool value) {
	if (value) {
		F |= 0b00100000;
	}
	else {
		F &= 0b11011111;
	}
}

void CPU::setCarryFlag(bool value) {
	if (value) {
		F |= 0b00010000;
	}
	else {
		F &= 0b11101111;
	}
}

void CPU::Add(uint8_t value) {
	uint16_t result = A + value;
	
	setZeroFlag((result & 0xFF) == 0);
	setSubtractFlag(false);
	setHalfCarryFlag(((A & 0xF) + (value & 0xF)) > 0xF);
	setCarryFlag(result > 0xFF);

	A = result & 0xFF;
}

void CPU::Sub(uint8_t value) {
	uint16_t result = A - value;

	setZeroFlag((result & 0xFF) == 0);
	setSubtractFlag(true);
	setHalfCarryFlag((A & 0xF) < (value & 0xF));
	setCarryFlag(A < value);

	A = result & 0xFF;
}

void CPU::ADC(uint8_t value) {
	uint8_t carry = getCarryFlag() ? 1 : 0;
	uint16_t result = A + value + carry;

	setZeroFlag((result & 0xFF) == 0);
	setSubtractFlag(false);
	setHalfCarryFlag(((A & 0xF) + (value & 0xF) + carry) > 0xF);
	setCarryFlag(result > 0xFF);
	
	A = result & 0xFF;
}

void CPU::SBC(uint8_t value) {
	uint8_t carry = getCarryFlag() ? 1 : 0;
	uint16_t result = A - value - carry;

	setZeroFlag((result & 0xFF) == 0);
	setSubtractFlag(true);;
	setHalfCarryFlag((A & 0xF) < ((value & 0xF) + carry));
	setCarryFlag(A < (value + carry));

	A = result & 0xFF;
}

void CPU::jump(bool condition) {
	uint8_t LowNibble = memory->read(PC);
	PC++;
	uint8_t HighNibble = memory->read(PC);
	PC++;
	uint16_t newLocation = LowNibble | (HighNibble << 8);

	if (condition) {
		PC = newLocation;
	}
}

void CPU::jumpRelative(bool condition) {
	int8_t offset = (int8_t)memory->read(PC);
	PC++;

	if (condition) {
		PC += offset;
	}
}

uint8_t& CPU::getRegisterCB(uint8_t index) {
	switch (index) {
		case 0:
			return B;
			break;
		case 1:
			return C;
			break;
		case 2:
			return D;
			break;
		case 3:
			return E;
			break;
		case 4:
			return H;
			break;
		case 5:
			return L;
			break;
		case 7:
			return A;
			break;
		default:
			throw std::runtime_error("Invalid CB register");
	}
}

void CPU::RLC(uint8_t& reg) {
	uint8_t carry = reg >> 7;
	reg = (reg << 1) | carry;

	setZeroFlag(reg == 0);
	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setCarryFlag(carry);
}

void CPU::RRC(uint8_t& reg)
{
	uint8_t carry = reg & 0x01;

	reg = (reg >> 1) | (carry << 7);

	setZeroFlag(reg == 0);
	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setCarryFlag(carry);
}

void CPU::RL(uint8_t& reg)
{
	uint8_t oldCarry = getCarryFlag() ? 1 : 0;
	uint8_t newCarry = reg >> 7;

	std::cout << "RL input: " << std::hex << (int)reg << "\n";
	std::cout << "Old carry: " << (int)oldCarry << "\n";
	std::cout << "New carry: " << (int)newCarry << "\n";

	reg = (reg << 1) | oldCarry;

	std::cout << "RL output: " << (int)reg << "\n";

	setZeroFlag(reg == 0);
	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setCarryFlag(newCarry);
}

void CPU::RR(uint8_t& reg)
{
	uint8_t oldCarry = getCarryFlag() ? 1 : 0;
	uint8_t newCarry = reg & 0x01;

	reg = (reg >> 1) | (oldCarry << 7);

	setZeroFlag(reg == 0);
	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setCarryFlag(newCarry);
}

void CPU::SLA(uint8_t& reg) {
	uint8_t carry = reg >> 7;
	reg <<= 1;

	setZeroFlag(reg == 0);
	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setCarryFlag(carry);
}

void CPU::SRA(uint8_t& reg)
{
	uint8_t carry = reg & 0x01;
	uint8_t bit7 = reg & 0x80;

	reg >>= 1;
	reg |= bit7;

	setZeroFlag(reg == 0);
	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setCarryFlag(carry);
}

void CPU::SRL(uint8_t& reg)
{
	uint8_t carry = reg & 0x01;

	reg >>= 1;

	setZeroFlag(reg == 0);
	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setCarryFlag(carry);
}

void CPU::Swap(uint8_t& reg) {
	reg = (reg >> 4) | (reg << 4);

	setZeroFlag(reg == 0);
	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setCarryFlag(false);
}

void CPU::Bit(uint8_t bit, uint8_t value) {
	setZeroFlag((value & (1 << bit)) == 0);
	setSubtractFlag(false);
	setHalfCarryFlag(true);
}

void CPU::Res(uint8_t bit, uint8_t& value)
{
	value &= ~(1 << bit);
}

void CPU::Set(uint8_t bit, uint8_t& value)
{
	value |= (1 << bit);
}

void CPU::compare(uint8_t value) {
	uint8_t result = A - value;

	setZeroFlag(result == 0);
	setSubtractFlag(true);
	setCarryFlag(A < value);
	setHalfCarryFlag((A & 0xF) < (value & 0xF));
}

void CPU::push(uint16_t value) {
	SP--;

	memory->write(SP, value >> 8);

	SP--;

	memory->write(SP, value & 0xFF);
}

uint16_t CPU::pop() {
	uint8_t low = memory->read(SP);
	SP++;
	uint8_t high = memory->read(SP);
	SP++;

	return low | (high << 8);
}

void CPU::call() {
	uint8_t low = memory->read(PC);
	PC++;
	uint8_t high = memory->read(PC);
	PC++;

	uint16_t address = low | (high << 8);

	push(PC);
	PC = address;
}

void CPU::ret() {
	PC = pop();
}

uint16_t CPU::getHL() {
	return (H << 8) | L;
}

uint16_t CPU::getBC() {
	return (B << 8) | C;
}

uint16_t CPU::getDE() {
	return (D << 8) | E;
}

uint16_t CPU::getAF() {
	return (A << 8) | F;
}

void CPU::setBC(uint16_t data) {
	B = (data >> 8) & 0xFF;
	C = (data & 0xFF);
}

void CPU::setDE(uint16_t data) {
	D = (data >> 8) & 0xFF;
	E = (data & 0xFF);
}

void CPU::setHL(uint16_t data) {
	H = (data >> 8) & 0xFF;
	L = (data & 0xFF);
}

void CPU::setAF(uint16_t data) {
	A = (data >> 8);;
	F = (data & 0xF0);;;
}

void CPU::Load(uint8_t& reg) {
	reg = memory->read(PC);
	PC++;
}

void CPU::storeSPtoAddress(uint16_t address) {
	memory->write(address, SP & 0xFF);
	memory->write(address + 1, (SP >> 8) & 0xFF);
}

void CPU::Inc(uint8_t& reg) {
	setHalfCarryFlag((reg & 0xF) == 0xF);

	reg++;

	setZeroFlag(reg == 0);
	setSubtractFlag(false);
}

void CPU::Dec(uint8_t& reg) {
	setHalfCarryFlag((reg & 0xF) == 0);;

	reg--;

	setZeroFlag(reg == 0);
	setSubtractFlag(true);
}

void CPU::And(uint8_t value) {
	A &= value;

	setZeroFlag(A == 0);
	setSubtractFlag(false);
	setHalfCarryFlag(true);
	setCarryFlag(false);
}

void CPU::Or(uint8_t value)
{
	A |= value;

	setZeroFlag(A == 0);
	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setCarryFlag(false);
}

void CPU::Xor(uint8_t value)
{
	A ^= value;

	setZeroFlag(A == 0);
	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setCarryFlag(false);
}

void CPU::AddBig(uint16_t value)
{
	uint16_t oldHL = getHL();
	uint32_t result = oldHL + value;

	setSubtractFlag(false);

	setHalfCarryFlag(
		((oldHL & 0xFFF) + (value & 0xFFF)) > 0xFFF
	);

	setCarryFlag(result > 0xFFFF);

	setHL(result & 0xFFFF);
}

void CPU::DAA(uint8_t& reg) {

	bool prevN = getSubtractFlag();
	bool prevC = getCarryFlag();
	bool prevH = getHalfCarryFlag();

	uint8_t correction = 0;

	if (prevN) {
		if (prevH) {
			correction |= 0x06;
		}
		if (prevC) {
			correction |= 0x60;
		}

		reg -= correction;
	}
	else {
		if (prevC || reg > 0x99) {
			correction |= 0x60;
			prevC = true;
		}
		if (prevH || (reg & 0x0F) > 0x09) {
			correction |= 0x06;
		}

		reg += correction;
	}
	
	setZeroFlag(reg == 0);
	setHalfCarryFlag(false);
	setCarryFlag(prevC);
}

void CPU::CPL(uint8_t& reg) {
	reg = ~reg;
	setSubtractFlag(true);
	setHalfCarryFlag(true);
}

void CPU::SCF() {
	setCarryFlag(true);
	setSubtractFlag(false);
	setHalfCarryFlag(false);
}

void CPU::CCF() {
	setCarryFlag(!getCarryFlag());
	setSubtractFlag(false);
	setHalfCarryFlag(false);
}

void CPU::RLCA(uint8_t& reg) {
	CPU::RLC(reg);
	setZeroFlag(false);
}

void CPU::RRCA(uint8_t& reg) {
	CPU::RRC(reg);
	setZeroFlag(false);
}

void CPU::RLA(uint8_t& reg) {
	CPU::RL(reg);
	setZeroFlag(false);
}

void CPU::RRA(uint8_t& reg) {
	CPU::RR(reg);
	setZeroFlag(false);
}

uint16_t CPU::readAddress() {
	uint16_t value = memory->read(PC);
	value |= (memory->read(PC + 1) << 8);
	PC += 2;
	return value;
}

void CPU::incrementHL(bool increment) {
	uint8_t value = memory->read(getHL());
	if (increment) {
		setSubtractFlag(false);
		setHalfCarryFlag((value & 0x0F) == 0x0F);	

		value++;
	}
	else {
		setSubtractFlag(true);
		setHalfCarryFlag((value & 0x0F) == 0x00);

		value--;
	}

	setZeroFlag(value == 0);

	memory->write(getHL(), value);
}

uint16_t CPU::addToSP(int8_t value) {
	uint16_t currentSP = SP;
	currentSP += value;

	uint8_t offset = static_cast<uint8_t>(value);

	bool H = ((SP & 0x0F) + (offset & 0x0F)) > 0x0F;
	bool C = ((SP & 0xFF) + offset) > 0xFF;

	setZeroFlag(false);
	setSubtractFlag(false);
	setHalfCarryFlag(H);
	setCarryFlag(C);

	return currentSP & 0xFFFF;
}

void CPU::CallConditional(bool condition) {
	uint8_t low = memory->read(PC);
	PC++;
	uint8_t high = memory->read(PC);
	PC++;
	uint16_t address = low | (high << 8);
	if (condition) {
		push(PC);
		PC = address;
	}
}

void CPU::RetConditional(bool condition) {
	if (condition) {
		PC = pop();
	}
}

void CPU::RST(uint16_t address) {
	push(PC);
	PC = address;
}

void CPU::DI() {
	IME = false;
	imeEnableDelay = 0;
	enableInterruptsPending = false;
}

void CPU::EI() {
	enableInterruptsPending = true;
	imeEnableDelay = 2;
}

void CPU::RETI() {
	IME = true;
	imeEnableDelay = 0;
	PC = pop();
	enableInterruptsPending = false;
}

uint8_t CPU::getPendingInterrupts() {
	uint8_t enabledInterrupts = memory->read(Interrupts::IE);
	uint8_t requestedInterrupts = memory->read(Interrupts::IF);

	return enabledInterrupts & requestedInterrupts & 0x1F;
}

int CPU::serviceInterrupt() {

	uint8_t pendingInterrupts = getPendingInterrupts();

	for (int i = 0; i < 5; i++) {
		if (pendingInterrupts & (1 << i)) {
			IME = false;
			imeEnableDelay = 0;
			enableInterruptsPending = false;

			uint8_t flags = memory->read(Interrupts::IF);
			flags &= ~(1 << i);
			memory->write(Interrupts::IF, flags);

			push(PC);
			PC = Interrupts::vectors[i];
			
			
			return 20; //I havent got cycles running yet thats subsystem for tomorrow
					   //Its now tomorrow I have to make it do cycle shit :(
		}
	}

	return 0; //Sad return cause no interrupts :( I feel sorry for it
}