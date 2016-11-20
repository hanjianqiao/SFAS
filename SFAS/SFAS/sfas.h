
#ifndef SFAS_H
#define SFAS_H

#include<string.h>
#include<stdio.h>
#include<iostream>
#include"common.h"
#define	MAXINSTUCTION	48		//�����֧�ֵ�������ָ����
#define	MAXSYMLEN		32		//���ŵ�ַ���������󳤶�
#define	SYMTABLEN		50		//��������ű���������
#define	BAD_CODE		0xff	//����ָ��
#define	MAXTOKEN		30		//Token����󳤶�
#define	MAXDATALEN		50		//data����󳤶�


class SFAS{
public:
	//  ������Ĺ��캯�� ��Դ�ļ�����ʼ�����ű�
	SFAS();

	// ���������ر�Դ�ļ�
	~SFAS();

	// ������̿��ƺ���
	void assemble(char *sourcename);

private:
	struct ADDRNode{
		unsigned int addr;
		ADDRNode *next;
	};
	//���ű�ṹ
	struct symboltab {
		char symname[MAXSYMLEN];	//���ŵ�ַ������
		unsigned char type;
		unsigned int symaddr;		//���ŵ�ַ��������������ڴ��ַ
		unsigned int datalen;		//data�����ַ��������ĳ���
		unsigned int referredtime;
		ADDRNode *referencelist;
		ADDRNode *lastreferredlocation;
	};


	FILE *src;							//Դ�ļ�ָ��
	char srcfile[20];					//����Դ�ļ���

	//��һ��ɨ�躯��
	symboltab symtabhdr[SYMTABLEN];		//���ű�ṹ����
	int symcnt;							//���ű��з��Ÿ���
	void assemble_round1(char *sourcename);

	//�ڶ���ɨ�躯��
	void assemble_round2(char *sourcename);

	//�������ִ���ļ�
	unsigned char *out_program;		//�����ĳ��򻺳���
	int program_count;			//���������
	unsigned int *out_data;			//���������ݻ�����
	int data_count;				//���ݼ�����
	void outToFile(char *filename);

	/* �����ǹ����Ժ��� */

	//��ȡ '='����ַ�
	void read_data(int &data_len, char* words);

	// ��str��ָ��Ļ�����Ƿ���ָ���
	// ����ȷ��ָ����ʽ����������Ӧ�Ļ����룬���ڴ����ָ���0xff
	unsigned int getopcode(char *str);

	/*
	lex:�������Ҫ���õĴʷ������������������������ص�ǰ�����õĴʷ��Ǻŵ����Ͳ������ʷ��Ŵ�
	����words������,ͬʱ�������ָ���,���õ��ʶ�Ӧ�Ļ���ָ�����opcode��������������
	*/
	char words[MAXTOKEN];		//���ڱ���lex�õ���token
	CODE opcode;				//�������ָ�����
	tokentype token_type;		// �Ǻ����
	tokentype lex(char *words, CODE & opcode);


	//lookupcmd: ��ѯ���ָ���,����cmd����Ӧ�Ļ���ָ��
	unsigned int lookupcmd(char *cmd);

	/*
	seeklabel
	��ѯ���ű������Ŵ��ڣ��������ڷ��ű��е��±�
	�����Ų����ڣ�����-1
	*/
	int seeklabel(char *lbl);

};
#endif