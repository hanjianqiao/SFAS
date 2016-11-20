#ifndef COMMON_H
#define	COMMON_H

// ���������ָ���
enum CODE{
	NOP = 0, NEG, ADD, SUB, MUL, DIV, MOD, NOT,
	LESS, GRT, LESSE, GRTE, EQU, NEQU, AND, OR,
	BNOT, BAND, BOR, BXOR, BLSHF, BRSHF, BSRSHF, PUSH,
	PUSHP, PUSHV, PUSHT, COPYN, COPYRN, POP, STORE, STOREV,
	HALT, JMP, JTRUE, JFALSE, JTRUEPOP, JFALSEPOP, CALL, RET,
	LPUSHP, LPUSHV, LSTORE, LSTOREV, IN, INNUM, OUT, OUTNUM,
};

//��lex�ʷ��������������߷��صļǺ������ö�����Ͷ���
//	ASMCODE:���ָ��		LABEL:�����ַ���			REFLABEL:���õ�ַ���
//	PNUM:�����ֱ�ʶ			NNUM��������ʶ				ID:��ʶ��				
//	CMNT:ע��������ʶ		UNKNOWN:����ʶ��ı��		SDATA:��̬����
//	INCLUDE:include file	EXTERN���ⲿ����			GLOBAL��ȫ�ַ���
enum tokentype {
	ASMCODE, LABEL, REFLABEL,
	PNUM, NNUM, CMNT,
	LOCALVAL, SDATA, OPER,
	UNKNOWN, INCLUDE, EXTERN,
	GLOBAL,
};

#endif // !COMMON_H

