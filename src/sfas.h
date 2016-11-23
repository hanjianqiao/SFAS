
#ifndef SFAS_H
#define SFAS_H

#include<string.h>
#include<stdio.h>
#include<iostream>
#include"common.h"
#define	MAXINSTUCTION	48
#define	MAXSYMLEN		32
#define	SYMTABLEN		50
#define	BAD_CODE		0xff
#define	MAXTOKEN		30		//Token max length
#define	MAXDATALEN		50		//data max length


class SFAS{
public:
	SFAS();

	~SFAS();

	void assemble(char *sourcename);

private:
	struct ADDRNode{
		unsigned int addr;
		ADDRNode *next;
	};
	struct symboltab {
		char symname[MAXSYMLEN];
		unsigned char type;
		unsigned int symaddr;
		unsigned int datalen;
		unsigned int referredtime;
		ADDRNode *referencelist;
		ADDRNode *lastreferredlocation;
	};


	FILE *src;
	char srcfile[20];

	symboltab symtabhdr[SYMTABLEN];
	int symcnt;
	void assemble_round1(char *sourcename);

	void assemble_round2(char *sourcename);

	unsigned char *out_program;
	int program_count;
	unsigned int *out_data;
	int data_count;
	void outToFile(char *filename);

	void read_data(int &data_len, char* words);

	unsigned int getopcode(char *str);

	char words[MAXTOKEN];
	CODE opcode;
	tokentype token_type;
	tokentype lex(char *words, CODE & opcode);


	unsigned int lookupcmd(char *cmd);

	/*
	seeklabel
	*/
	int seeklabel(char *lbl);

};
#endif