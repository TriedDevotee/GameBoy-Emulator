
class Assembler:
    def __init__(self, origin=0x0000):
        self.origin = origin
        self.code = bytearray()
        self.labels = {}
        self.fixups = []

    @property
    def address(self):
        return self.origin + len(self.code)

    def emit(self, *values):
        for value in values:
            if not 0 <= value <= 0xFF:
                raise ValueError(f"Invalid byte: {value:#x}")
            self.code.append(value)

    def label(self, name):
        if name in self.labels:
            raise ValueError(f"Duplicate label: {name}")
        self.labels[name] = self.address

    def jp(self, opcode, target):
        self.emit(opcode, 0x00, 0x00)
        self.fixups.append(("abs16", len(self.code) - 2, target))

    def call(self, target):
        self.jp(0xCD, target)

    def jr(self, opcode, target):
        self.emit(opcode, 0x00)
        self.fixups.append(("rel8", len(self.code) - 1, target))

    def build(self):
        for kind, position, target in self.fixups:
            if target not in self.labels:
                raise ValueError(f"Undefined label: {target}")

            destination = self.labels[target]

            if kind == "abs16":
                self.code[position] = destination & 0xFF
                self.code[position + 1] = (destination >> 8) & 0xFF

            elif kind == "rel8":
                offset = destination - (self.origin + position + 1)

                if not -128 <= offset <= 127:
                    raise ValueError(
                        f"JR target {target} is out of range: {offset}"
                    )

                self.code[position] = offset & 0xFF

        return bytes(self.code)


# Convenience helper for checking register A.
def check_a(asm, value, failure_label="fail"):
    asm.emit(0xFE, value)       # CP value
    asm.jp(0xC2, failure_label) # JP NZ, fail



asm = Assembler(origin=0x0000)

"""asm.emit(
    0x31, 0x00, 0xD0  # LD SP,D000
)

# ==========================================================
# 1. REGISTER LOADS AND TRANSFERS
# ==========================================================

asm.emit(
    0x06, 0x12,       # LD B,12
    0x0E, 0x34,       # LD C,34
    0x78              # LD A,B
)
check_a(asm, 0x12)

asm.emit(0x79)        # LD A,C
check_a(asm, 0x34)

# ==========================================================
# 2. 8-BIT ARITHMETIC AND FLAGS
# ==========================================================

asm.emit(
    0x3E, 0x42,       # LD A,42
    0xC6, 0x3E        # ADD A,3E -> 80
)
check_a(asm, 0x80)

asm.emit(
    0xCE, 0x7F        # ADC A,7F -> FF
)
check_a(asm, 0xFF)

asm.emit(
    0xC6, 0x01,       # ADD A,01 -> 00
    0xF5,             # PUSH AF
    0xC1,             # POP BC
    0x79              # LD A,C
)
check_a(asm, 0xB0)

# Subtraction
asm.emit(
    0x3E, 0x10,       # LD A,10
    0xD6, 0x01        # SUB 01 -> 0F
)
check_a(asm, 0x0F)

asm.emit(
    0xDE, 0x0E        # SBC A,0E -> 01
)
check_a(asm, 0x01)

# Compare should not modify A.
asm.emit(0xFE, 0x01)  # CP 01
asm.jp(0xC2, "fail")  # Must have Z set

# ==========================================================
# 3. INC / DEC
# ==========================================================

asm.emit(
    0x3E, 0x0F,
    0x3C              # INC A -> 10
)
check_a(asm, 0x10)

asm.emit(0x3D)        # DEC A -> 0F
check_a(asm, 0x0F)

asm.emit(
    0x06, 0xFF,       # LD B,FF
    0x04              # INC B -> 00
)
asm.emit(0x78)        # LD A,B
check_a(asm, 0x00)

asm.emit(0x05)        # DEC B -> FF
asm.emit(0x78)
check_a(asm, 0xFF)

# ==========================================================
# 4. LOGICAL OPERATIONS
# ==========================================================

asm.emit(
    0x3E, 0xF0,
    0xE6, 0x0F        # AND 0F -> 00
)
check_a(asm, 0x00)

asm.emit(
    0x3E, 0xF0,
    0xF6, 0x0F        # OR 0F -> FF
)
check_a(asm, 0xFF)

asm.emit(
    0xEE, 0xFF        # XOR FF -> 00
)
check_a(asm, 0x00)

asm.emit(0x2F)        # CPL -> FF
check_a(asm, 0xFF)

# ==========================================================
# 5. 16-BIT ARITHMETIC
# ==========================================================

asm.emit(
    0x21, 0x34, 0x12, # LD HL,1234
    0x01, 0x11, 0x11, # LD BC,1111
    0x09              # ADD HL,BC -> 2345
)

asm.emit(0x7C)        # LD A,H
check_a(asm, 0x23)

asm.emit(0x7D)        # LD A,L
check_a(asm, 0x45)

asm.emit(
    0x03,             # INC BC -> 1112
    0x0B              # DEC BC -> 1111
)

# ==========================================================
# 6. MEMORY ADDRESSING
# ==========================================================

asm.emit(
    0x21, 0x00, 0xC0, # LD HL,C000
    0x36, 0xAB,       # LD (HL),AB
    0x23,             # INC HL
    0x36, 0xCD,       # LD (HL),CD
    0x2B,             # DEC HL
    0x7E              # LD A,(HL) -> AB
)
check_a(asm, 0xAB)

asm.emit(
    0x22,             # LD (HL+),A -> HL=C001
    0x3E, 0x55,       # LD A,55
    0x3A              # LD A,(HL-) -> CD, HL=C000
)
check_a(asm, 0xCD)

# Check the final HL value.
asm.emit(0x7C)        # LD A,H
check_a(asm, 0xC0)

asm.emit(0x7D)        # LD A,L
check_a(asm, 0x00)

# ==========================================================
# 7. STACK OPERATIONS
# ==========================================================

asm.emit(
    0x31, 0x00, 0xD0, # LD SP,D000
    0x01, 0xEF, 0xBE, # LD BC,BEEF
    0xC5,             # PUSH BC
    0x01, 0x00, 0x00, # LD BC,0000
    0xC1              # POP BC
)

asm.emit(0x78)        # LD A,B
check_a(asm, 0xBE)

asm.emit(0x79)        # LD A,C
check_a(asm, 0xEF)

# ==========================================================
# 8. CB-PREFIXED ROTATIONS AND SHIFTS
# ==========================================================

asm.emit(
    0x3E, 0x81,       # LD A,81
    0xCB, 0x07        # RLC A -> 03
)
check_a(asm, 0x03)

asm.emit(0xCB, 0x0F)  # RRC A -> 81
check_a(asm, 0x81)

asm.emit(0x37, 0xCB, 0x17)  # RL A -> 03
check_a(asm, 0x03)

asm.emit(0x37, 0xCB, 0x1F)  # RR A -> 81
check_a(asm, 0x81)

asm.emit(0xCB, 0x27)  # SLA A -> 02
check_a(asm, 0x02)

asm.emit(0xCB, 0x2F)  # SRA A -> 01
check_a(asm, 0x01)

asm.emit(0xCB, 0x37)  # SWAP A -> 10
check_a(asm, 0x10)

asm.emit(0xCB, 0x3F)  # SRL A -> 08
check_a(asm, 0x08)

# BIT 0,A should set Z, H and clear N.
# BIT 0,A
asm.emit(
    0xCB, 0x47,
    0xF5,
    0xC1,
    0x79
)
check_a(asm, 0xA0)

# SET 0,A -> A1
asm.emit(0xCB, 0xC7)
check_a(asm, 0xA1)

# RES 0,A -> A0
asm.emit(0xCB, 0x87)
check_a(asm, 0xA0)

# ==========================================================
# 9. CONDITIONAL EXECUTION
# ==========================================================

asm.emit(
    0x3E, 0x00,       # LD A,00
    0xFE, 0x00        # CP 00 -> Z
)

asm.jp(0xC2, "fail")  # JP NZ,fail - must NOT jump
asm.jp(0xCA, "zero_ok") # JP Z,zero_ok

asm.jp(0xC3, "fail")  # Should never reach here

asm.label("zero_ok")

asm.emit(
    0x3E, 0x01,       # LD A,01
    0xFE, 0x00        # CP 00 -> NZ
)
asm.jp(0xCA, "fail")  # JP Z,fail - must NOT jump
asm.jp(0xC2, "nonzero_ok")

asm.jp(0xC3, "fail")

asm.label("nonzero_ok")

# Test a relative jump too.
asm.jr(0x18, "relative_ok")
asm.jp(0xC3, "fail")

asm.label("relative_ok")

# ==========================================================
# 10. CALL AND RET
# ==========================================================

asm.emit(0x3E, 0x10)  # LD A,10
asm.call("test_function")
check_a(asm, 0x32)    # Function adds 22

asm.jp(0xC3, "pass")

asm.label("test_function")
asm.emit(
    0xC6, 0x22,       # ADD A,22
    0xC9              # RET
)

# ==========================================================
# 11. SUCCESS / FAILURE HANDLERS
# ==========================================================

asm.label("pass")
asm.emit(
    0x3E, 0xA5,       # LD A,A5
    0xEA, 0x00, 0xC0, # LD (C000),A
    0x76              # HALT
)

asm.label("fail")
asm.emit(
    0x3E, 0xEE,       # LD A,EE
    0xEA, 0x00, 0xC0, # LD (C000),A
    0x76              # HALT
)"""

"""asm.emit(
    0x3E, 0x42,       # LD A,42
    0x06, 0x12,       # LD B,12
    0x0E, 0x34,       # LD C,34
    0x76              # HALT
)"""

"""asm.emit(
    0xF3,             # DI
    0x3E, 0x01,       # LD A,01
    0xEA, 0xFF, 0xFF, # LD (FFFF),A
    0xEA, 0x0F, 0xFF, # LD (FF0F),A
    0x76,             # HALT
    0x06, 0x00,       # LD B,00
    0x0E, 0x99             # HALT
)

asm.emit(
    0x3E, 0x00,       # LD A,00
    0xEA, 0x0F, 0xFF  # LD (FF0F),A ; Clear IF
)

asm.emit(0x76)"""



"""asm.emit(
    # ==========================================
    # 1. Initialise registers
    # ==========================================

    0x31, 0x00, 0xD0, # LD SP,D000
    0x3E, 0x10,       # LD A,10
    0x06, 0x05,       # LD B,05
    0x0E, 0x03,       # LD C,03
    0x16, 0x00,       # LD D,00

    # ==========================================
    # 2. Arithmetic
    # ==========================================

    0x80,             # ADD A,B       A = 15
    0x81,             # ADD A,C       A = 18
    0x90,             # SUB B         A = 13
    0xA1,             # AND C         A = 01
    0xF6, 0x80,       # OR 80         A = 81
    0xEE, 0x01,       # XOR 01        A = 80
    0xFE, 0x80,       # CP 80         Z = 1

    # ==========================================
    # 3. Memory operations
    # ==========================================

    0x21, 0x00, 0xC0, # LD HL,C000
    0x77,             # LD (HL),A
    0x23,             # INC HL
    0x36, 0x42,       # LD (HL),42
    0x7E,             # LD A,(HL)
    0x23,             # INC HL
    0x36, 0x99,       # LD (HL),99
    0x2B,             # DEC HL
    0x7E,             # LD A,(HL)

    # ==========================================
    # 4. Stack operations
    # ==========================================

    0xC5,             # PUSH BC
    0xD5,             # PUSH DE
    0xC1,             # POP BC
    0xD1,             # POP DE

    # ==========================================
    # 5. Loop and conditional jumps
    # ==========================================

    0x06, 0x05,       # LD B,05

    # LOOP:
    0x05,             # DEC B
    0x20, 0xFD,       # JR NZ,LOOP (-3 bytes)

    # ==========================================
    # 6. Finish in a harmless infinite loop
    # ==========================================

    0x18, 0xFE        # JR -2
)"""


"""asm.emit(
    # Initialise
    0x31, 0x00, 0xD0,  # LD SP, D000
    0x3E, 0xFC,        # LD A, FC
    0xE0, 0x47,        # LDH (FF47), A -- initial palette
    0x06, 0xAA,        # LD B, 02 -- wait two frames

    # WAIT_FOR_VBLANK:
    0xF0, 0x44,        # LDH A, (FF44) -- read LY
    0xFE, 0x90,        # CP 144
    0x20, 0xFA,        # JR NZ, WAIT_FOR_VBLANK

    # WAIT_FOR_NEXT_FRAME:
    0xF0, 0x44,        # LDH A, (FF44)
    0xFE, 0x90,        # CP 144
    0x28, 0xFA,        # JR Z, WAIT_FOR_NEXT_FRAME

    # One VBlank has passed
    0x05,              # DEC B
    0x20, 0xF1,        # JR NZ, WAIT_FOR_VBLANK

    # Two frames have passed: invert palette
    0x06, 0xAA,        # LD B, 02
    0xF0, 0x47,        # LDH A, (FF47)
    0xEE, 0xFF,        # XOR FF -- FC <-> 03
    0xE0, 0x47,        # LDH (FF47), A

    0x18, 0xE7         # JR WAIT_FOR_VBLANK
)"""


"""asm.emit(
    # ==========================================
    # 1. GENERATE THE BACKGROUND TILE MAP
    # ==========================================

    0x31, 0x00, 0xD0,  # LD SP, D000
    0x21, 0x00, 0x98,  # LD HL, 9800 -- tile map
    0x06, 0x20,        # LD B, 32 -- 32 rows
    0x16, 0x00,        # LD D, 00 -- starting tile

    # ROW:
    0x7A,              # LD A, D
    0x0E, 0x20,        # LD C, 32

    # COLUMN:
    0x22,              # LD (HL+), A
    0xEE, 0x01,        # XOR 01 -- alternate tile
    0x0D,              # DEC C
    0x20, 0xFA,        # JR NZ, COLUMN

    # Alternate starting tile for next row
    0x7A,              # LD A, D
    0xEE, 0x01,        # XOR 01
    0x57,              # LD D, A
    0x05,              # DEC B
    0x20, 0xF0,        # JR NZ, ROW

    # ==========================================
    # 2. INITIALISE THE DISPLAY
    # ==========================================

    0x3E, 0x00,        # LD A, 00
    0xE0, 0x43,        # LDH (SCX), A
    0xE0, 0x42,        # LDH (SCY), A

    0x3E, 0xE4,        # LD A, E4 -- normal grayscale palette
    0xE0, 0x47,        # LDH (BGP), A

    0x06, 0x00,        # LD B, 00 -- frame counter

    # ==========================================
    # 3. WAIT FOR VBLANK
    # ==========================================

    # WAIT_VBLANK:
    0xF0, 0x44,        # LDH A, (LY)
    0xFE, 0x90,        # CP 144
    0x20, 0xFA,        # JR NZ, WAIT_VBLANK

    # WAIT_END:
    0xF0, 0x44,        # LDH A, (LY)
    0xFE, 0x90,        # CP 144
    0x28, 0xFA,        # JR Z, WAIT_END

    # ==========================================
    # 4. ANIMATE THE BACKGROUND
    # ==========================================

    0xF0, 0x43,        # LDH A, (SCX)
    0x3C,              # INC A
    0xE0, 0x43,        # LDH (SCX), A

    0xF0, 0x42,        # LDH A, (SCY)
    0x3C,              # INC A
    0xE0, 0x42,        # LDH (SCY), A

    # ==========================================
    # 5. INVERT PALETTE EVERY 16 FRAMES
    # ==========================================

    0x04,              # INC B
    0x78,              # LD A, B
    0xE6, 0x0F,        # AND 0F
    0x20, 0x06,        # JR NZ, SKIP_PALETTE

    0xF0, 0x47,        # LDH A, (BGP)
    0xEE, 0xFF,        # XOR FF
    0xE0, 0x47,        # LDH (BGP), A

    # SKIP_PALETTE:
    0x18, 0xDC         # JR WAIT_VBLANK
)"""



asm.emit(
    # Stack pointer
    0x31, 0x00, 0xD0,

    # Enable LCD, background and sprites
    0x3E, 0x93,
    0xEA, 0x40, 0xFF,

    # Sprite palette
    0x3E, 0xE4,
    0xEA, 0x48, 0xFF,

    # Sprite Y = 56 (screen Y = 40)
    0x3E, 0x38,
    0xEA, 0x00, 0xFE,

    # Sprite X = 88 (screen X = 80)
    0x3E, 0x58,
    0xEA, 0x01, 0xFE,

    # Sprite tile = 1
    0x3E, 0x01,
    0xEA, 0x02, 0xFE,

    # Attributes = 0
    0xAF,
    0xEA, 0x03, 0xFE,

    # B = 1 (movement direction)
    0x06, 0x01,

    # LOOP:
    # Read current X
    0xFA, 0x01, 0xFE,

    # Add movement direction
    0x80,

    # Write updated X
    0xEA, 0x01, 0xFE,

    # Have we reached the right boundary?
    0xFE, 0xA0,
    0x20, 0x04,       # JR NZ, CHECK_LEFT

    # Reverse direction: B = -1
    0x06, 0xFF,
    0x18, 0x06,       # JR DELAY

    # CHECK_LEFT:
    0xFE, 0x20,
    0x20, 0x02,       # JR NZ, DELAY

    # Reverse direction: B = +1
    0x06, 0x01,

    # DELAY:
    0x16, 0x20,       # LD D, 32

    # OUTER:
    0x1E, 0xFF,       # LD E, 255

    # INNER:
    0x1D,             # DEC E
    0x20, 0xFD,       # JR NZ, INNER

    0x15,             # DEC D
    0x20, 0xF8,       # JR NZ, OUTER

    # Return to LOOP
    0x18, 0xDF
)






# ==========================================================
# ASSEMBLE AND WRITE
# ==========================================================

program = asm.build()

with open("spriteTest.gb", "wb") as f:
    f.write(program)

print(f"Generated {len(program)} bytes")
print(f"Origin: 0x{asm.origin:04X}")
#print(f"Success handler: 0x{asm.labels['pass']:04X}")
#print(f"Failure handler: 0x{asm.labels['fail']:04X}")
#print(f"Test function: 0x{asm.labels['test_function']:04X}")
