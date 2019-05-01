/* Instruction Opcodes */
#define OP3_SYSTEM 0x0
#define OP3_JUMP_ABSOLUTE 0x1
#define OP3_JUMP_OFFSET 0xb
#define OP3_CALL_SUBROUTINE 0x2
#define OP3_SKIP_NEXT_EQ_IM 0x3
#define OP3_SKIP_NEXT_NEQ_IM 0x4
#define OP3_SKIP_NEXT_EQ 0x5
#define OP3_SKIP_NEXT_NEQ 0x9
#define OP3_LOAD_V 0x6
#define OP3_ADD_IM 0x7
#define OP3_ARITH 0x8
#define OP3_LOAD_I 0xa
#define OP3_RANDOM 0xc
#define OP3_DRAW 0xd
#define OP3_KEYBOARD 0xe
#define OP3_SPECIAL 0xf

#define OP2_SYSTEM 0x0

#define OP10_SYSTEM_CLEAR 0xe0
#define OP10_SYSTEM_RETURN 0xee
#define OP10_KEYBOARD_EQ 0x9e
#define OP10_KEYBOARD_NEQ 0xa1
#define OP10_SPECIAL_LOAD_DELAY 0x07
#define OP10_SPECIAL_LOAD_KEYBOARD 0x0a
#define OP10_SPECIAL_SET_DELAY 0x15
#define OP10_SPECIAL_SET_SOUND 0x18
#define OP10_SPECIAL_ADD_TO_I 0x1e
#define OP10_SPECIAL_LOAD_FONT 0x29
#define OP10_SPECIAL_LOAD_BCD 0x33
#define OP10_SPECIAL_LOAD_AT_I 0x55
#define OP10_SPECIAL_SET_AT_I 0x65

#define OP0_SKIP_NEXT 0x0
#define OP0_ARITH_LOAD 0x0
#define OP0_ARITH_OR 0x1
#define OP0_ARITH_AND 0x2
#define OP0_ARITH_XOR 0x3
#define OP0_ARITH_ADD 0x4
#define OP0_ARITH_SUB 0x5
#define OP0_ARITH_SHR 0x6
#define OP0_ARITH_SUBN 0x7
#define OP0_ARITH_SHL 0xe
