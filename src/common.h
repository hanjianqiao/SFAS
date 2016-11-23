#ifndef COMMON_H
#define	COMMON_H

// instruction set
enum CODE{
	NOP = 0, NEG, ADD, SUB, MUL, DIV, MOD, NOT,
	LESS, GRT, LESSE, GRTE, EQU, NEQU, AND, OR,
	BNOT, BAND, BOR, BXOR, BLSHF, BRSHF, BSRSHF, PUSH,
	PUSHP, PUSHV, PUSHT, COPYN, COPYRN, POP, STORE, STOREV,
	HALT, JMP, JTRUE, JFALSE, JTRUEPOP, JFALSEPOP, CALL, RET,
	LPUSHP, LPUSHV, LSTORE, LSTOREV, IN, INNUM, OUT, OUTNUM,
};

//	ASMCODE:asm code			LABEL:label						REFLABEL:reference to a label
//	PNUM:positive number		NNUM£ºnegative number			ID:identifier				
//	CMNT:comment				UNKNOWN:unrecognized			SDATA:static variable
//	INCLUDE:include file		EXTERN£ºextern symbol			GLOBAL£ºglobal symbol
enum tokentype {
	ASMCODE, LABEL, REFLABEL,
	PNUM, NNUM, CMNT,
	LOCALVAL, SDATA, OPER,
	UNKNOWN, INCLUDE, EXTERN,
	GLOBAL,
};

#endif // !COMMON_H

