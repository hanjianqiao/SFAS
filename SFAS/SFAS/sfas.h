
#ifndef SFAS_H
#define SFAS_H

#include<string.h>
#include<stdio.h>
#include<iostream>
#include"common.h"
#define	MAXINSTUCTION	48		//虚拟机支持的最大机器指令数
#define	MAXSYMLEN		32		//符号地址所允许的最大长度
#define	SYMTABLEN		50		//汇编器符号表的最大容量
#define	BAD_CODE		0xff	//错误指令
#define	MAXTOKEN		30		//Token的最大长度
#define	MAXDATALEN		50		//data的最大长度


class SFAS{
public:
	//  汇编器的构造函数 打开源文件，初始化符号表
	SFAS();

	// 析构函数关闭源文件
	~SFAS();

	// 汇编流程控制函数
	void assemble(char *sourcename);

private:
	struct ADDRNode{
		unsigned int addr;
		ADDRNode *next;
	};
	//符号表结构
	struct symboltab {
		char symname[MAXSYMLEN];	//符号地址定义名
		unsigned char type;
		unsigned int symaddr;		//符号地址定义名所代表的内存地址
		unsigned int datalen;		//data类型字符串声明的长度
		unsigned int referredtime;
		ADDRNode *referencelist;
		ADDRNode *lastreferredlocation;
	};


	FILE *src;							//源文件指针
	char srcfile[20];					//保存源文件名

	//第一遍扫描函数
	symboltab symtabhdr[SYMTABLEN];		//符号表结构数组
	int symcnt;							//符号表中符号个数
	void assemble_round1(char *sourcename);

	//第二遍扫描函数
	void assemble_round2(char *sourcename);

	//输出到可执行文件
	unsigned char *out_program;		//编译后的程序缓冲区
	int program_count;			//代码计数器
	unsigned int *out_data;			//编译后的数据缓冲区
	int data_count;				//数据计数器
	void outToFile(char *filename);

	/* 下面是功能性函数 */

	//读取 '='后的字符
	void read_data(int &data_len, char* words);

	// 对str所指向的汇编助记符查指令表，
	// 对正确的指令形式返回其所对应的机器码，对于错误的指令返回0xff
	unsigned int getopcode(char *str);

	/*
	lex:汇编器所要调用的词法分析函数，给主调函数返回当前所读得的词法记号的类型并将单词符号串
	存入words数组中,同时查机器的指令表,将该单词对应的机器指令存入opcode返回主调函数。
	*/
	char words[MAXTOKEN];		//用于保存lex得到的token
	CODE opcode;				//保存机器指令编码
	tokentype token_type;		// 记号类别
	tokentype lex(char *words, CODE & opcode);


	//lookupcmd: 查询汇编指令表,返回cmd所对应的机器指令
	unsigned int lookupcmd(char *cmd);

	/*
	seeklabel
	查询符号表，如果标号存在，返回其在符号表中的下标
	如果标号不存在，返回-1
	*/
	int seeklabel(char *lbl);

};
#endif