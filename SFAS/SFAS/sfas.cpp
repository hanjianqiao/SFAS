#include"sfas.h"

const char *mnemonics[48] = {
	"NOP", "NEG", "ADD", "SUB", "MUL", "DIV", "MOD", "NOT",
	"LESS", "GRT", "LESSE", "GRTE", "EQU", "NEQU", "AND", "OR",
	"BNOT", "BAND", "BOR", "BXOR", "BLSHF", "BRSHF", "BSRSHF", "PUSH",
	"PUSHP", "PUSHV", "PUSHT", "COPYN", "COPYRN", "POP", "STORE", "STOREV",
	"HALT", "JMP", "JTRUE", "JFALSE", "JTRUEPOP", "JFALSEPOP", "CALL", "RET",
	"LPUSHP", "LPUSHV", "LSTORE", "LSTOREV", "IN", "INNUM", "OUT", "OUTNUM",
};

/*
//取得机器指令i的助记符 
inline const char *getmns(int i) {
	return mnemonics[i];
}
*/
//test
// 对str所指向的汇编助记符查指令表，
	// 对正确的指令形式返回其所对应的机器码，对于错误的指令返回0xff
unsigned int SFAS::getopcode(char *str){
	
	for (int i = 0; str[i]; i++)
		str[i] = toupper(str[i]);
	unsigned int temp = NOP;
	while (temp <= MAXINSTUCTION && strcmp(str, mnemonics[temp]))
		temp++;
	if (temp <= MAXINSTUCTION)
	{
		switch (temp)
		{
			//找出所有带有一个参数的指令
		case PUSH:		case PUSHP:		case PUSHT:
		case COPYN:		case COPYRN:	case POP:
		case STORE:		case JMP:		case JTRUE:
		case JFALSE:	case JTRUEPOP:	case JFALSEPOP:
		case CALL:		case RET:		case LPUSHP:
		case LSTORE:

			temp |= 0x80;
			break;

		default:
			break;
		}
		return temp;
	}

	else
		return BAD_CODE;
}


/*
	汇编器的构造函数AS: 打开源文件,初始化符号表
	*/
SFAS::SFAS()
{
	

	int i;
	//符号表内的符号名全部初始为 '????????'
	for (i = 0; i < SYMTABLEN; ++i){
		strcpy_s(symtabhdr[i].symname, 8, "???????");
		symtabhdr[i].datalen = 0;
		symtabhdr[i].type = 0x00;
		symtabhdr[i].symaddr = 0;
		symtabhdr[i].referredtime = 0;
		symtabhdr[i].referencelist = NULL;
		symtabhdr[i].lastreferredlocation = NULL;
	}

	symcnt = 0;				//符号表中符号个数
	data_count = 0;			//数据计数器置零
	program_count = 0;		//程序计数器置零

}


/*
	汇编器的析构函数~AS: 关闭源文件
	*/
SFAS::~SFAS()
{
	
	if (src) {
		fclose(src);
	}
	src = NULL;
}


//第一遍扫描，建立符号表。
void SFAS::assemble_round1(char *sourcename){ 
	char words[MAXTOKEN];
	CODE opcode;
	tokentype token_type;
	FILE *savedFile;
	bool isoper;		//用于标记是否有'='
	int temp_i;
	fopen_s(&src, sourcename, "r");
	if (src == NULL) {
		printf("Could not open input file: %s.\n", sourcename);
		exit(1);
	}
	while (!feof(src))
	{
		token_type = lex(words, opcode);	//读入一个单词,取得单词的类别及相关信息


		switch (token_type) {

		case ASMCODE:
			program_count++;
			break;
		case CMNT:						//注释
			break;

		case LABEL:							//新定义的标号，将其加入符号表  
			temp_i = seeklabel(words);
			if (temp_i == -1) {
				strcpy_s(symtabhdr[symcnt].symname, strlen(words) + 1, words);
				symtabhdr[symcnt].symaddr = program_count;
				symtabhdr[symcnt].type = 0x01;
				++symcnt;
			}
			else if (symtabhdr[temp_i].type == 0x02){
				symtabhdr[temp_i].symaddr = program_count;
				symtabhdr[temp_i].type = 0x03;
			}
			else if (symtabhdr[temp_i].type == 0x04) {
				symtabhdr[temp_i].symaddr = program_count;
				symtabhdr[temp_i].type = 0x05;
			}
			else{
				printf("ERROR when define new label.\n");
			}
			break;

		case LOCALVAL:						//	新定义local vlcalval as uint
			temp_i = seeklabel(words);
			if (temp_i == -1) {
				strcpy_s(symtabhdr[symcnt].symname, strlen(words) + 1, words);
				token_type = lex(words, opcode);
				if (token_type == PNUM)
				{
					symtabhdr[symcnt].symaddr = atoi(words);
				}
				else{
					printf("Cannot be negative number.\n");
				}
				symtabhdr[symcnt].type = 0x08;
				++symcnt;
			}
			//No else, 忽略符号重定义
			break;

		case SDATA:							//	新声明的data
			if (strcmp(words, "*") == 0)//如果是匿名data
			{
				token_type = lex(words, opcode);
				if (token_type == OPER)
					data_count += (int)opcode;
				else
				{
					;//报错
				}

				//保存到静态数据区
			}
			else
			{
				//区分是 name[len] 还是 name
				char name[MAXTOKEN];	//data name
				char c_temp_len[3];
				int temp_len = 0;			//data长度
				bool f = 0;

				unsigned int j;
				for (j = 0; j<strlen(words) + 1; ++j)
				{
					if (words[j] != '[')
						name[j] = words[j];
					else
					{
						f = 1;
						break;
					}
				}
				if (f&&words[j] == '[')
				{
					name[j] = 0;
					int k = 0;
					for (++j; words[j] != ']'; ++j, ++k)
						c_temp_len[k] = words[j];
					c_temp_len[k] = 0;
					temp_len = atoi(c_temp_len);
				}
				//如果是name类型
				if (!f)
				{
					temp_len = 1;
				}

				//加入符号表
				strcpy_s(symtabhdr[symcnt].symname, strlen(name) + 1, name);
				symtabhdr[symcnt].symaddr = data_count;
				symtabhdr[symcnt].datalen = temp_len;
				symtabhdr[symcnt].type = 0x01;
				//symtabhdr[symcnt].firstref = NULL;
				++symcnt;
				data_count += temp_len;
				
				/*
					是否为定义 begin
				*/

				fseek(src, -1, SEEK_CUR);

				//检查是否有定义，即后续字符是否存在'='号
				char ch;
				isoper = 0;//是否遇到'='
				do{
					ch = getc(src);

					//如果是'='
					if (ch == '=')
					{
						fseek(src, -1, SEEK_CUR);//文件指针向前移动一个字节，重新指向'='
						isoper = 1;
						break;
					}
					//如果是换行符
					else if (ch == 0x0a)
					{
						fseek(src, -1, SEEK_CUR);//文件指针向前移动一个字节，重新指向'\n'
						break;
					}
					//如果指向一个';'
					else if (ch == ';')
					{
						fseek(src, -1, SEEK_CUR);//文件指针向前移动一个字节，重新指向';'
						break;
					}
				} while (ch <= ' '&&!feof(src));

				//如果有'='
				if (isoper)
				{
					token_type = lex(words, opcode);
					if (token_type == OPER)
					{
						if ((int)opcode>temp_len)
						{
							printf("overflow   %s    ", name);
							system("pause");
							exit(1);
						}
					}
				}
				/*
					是定义 end
				*/
				else//如果没有'='
				{
					;//只声明，未定义
				}
				
			}
			break;

		case INCLUDE:
			savedFile = src;
			src = NULL;
			assemble_round1(words);
			src = savedFile;
			break;

		case REFLABEL:

			/*
				是否为定义 begin
			*/

			fseek(src, -1, SEEK_CUR);

			//检查是否有定义，即后续字符是否存在'='号
			char ch;
			isoper = 0;//是否遇到'='
			do{
				ch = getc(src);

				//如果是'='
				if (ch == '=')
				{
					fseek(src, -1, SEEK_CUR);//文件指针向前移动一个字节，重新指向'='
					isoper = 1;
					break;
				}
				//如果是换行符，这是一个标号引用
				else if (ch == 0x0a)
				{
					fseek(src, -1, SEEK_CUR);//文件指针向前移动一个字节，重新指向'\n'
					break;
				}
				//如果指向一个';'，这是一个标号引用
				else if (ch == ';')
				{
					fseek(src, -1, SEEK_CUR);//文件指针向前移动一个字节，重新指向';'
					break;
				}
			} while (ch <= ' '&&!feof(src));

			//如果有'='，这是一个定义
			if (isoper)
			{
			}
			/*
				是定义 end
			*/
			else//如果没有'='
			{
				program_count += 4; //标号地址占4个字节	
			}
			break;

		case PNUM:
		case NNUM:
			program_count += 4;//数字占4个字节
			break;
		case EXTERN:							//新定义的标号，将其加入符号表  
			temp_i = seeklabel(words);
			if (temp_i == -1) {
				strcpy_s(symtabhdr[symcnt].symname, strlen(words) + 1, words);
				symtabhdr[symcnt].type = 0x02;
				symtabhdr[symcnt].symaddr = 0xffffffff;
				++symcnt;
			}
			else{
				printf("ERROR when define new label.\n");
			}
			break;
		case GLOBAL:							//新定义的标号，将其加入符号表  
			temp_i = seeklabel(words);
			if (temp_i == -1) {
				strcpy_s(symtabhdr[symcnt].symname, strlen(words) + 1, words);
				symtabhdr[symcnt].type = 0x04;
				symtabhdr[symcnt].symaddr = 0xffffffff;
				++symcnt;
			}
			else{
				printf("ERROR when define new label.\n");
			}
			break;
		case UNKNOWN:		//不可识别的符号,输出错误信息
			break;
		}

	}
	fclose(src);
	src = NULL;			// 关闭输入流,完成第1遍扫描,准备第2遍扫描
}

//第二遍扫描，生成代码。
void SFAS::assemble_round2(char *sourcename){
	//以下五条指令需要计算相对偏移量
	char ch_jmp[] = "JMP";
	char ch_jtrue[] = "JTRUE";
	char ch_jfalse[] = "JFALSE";
	char ch_jtruepop[] = "JTRUEPOP";
	char ch_jfalsepop[] = "JFALSEPOP";
	FILE *savedFile;
	unsigned char op;			//用于保存指令
	unsigned char a, b, c, d;		//用于保存分割后的操作数
	int number;					 //用于保存数值token
	int reflableaddr;			//保存loacl val
	int labelidx;				// 标号在符号表内的序号
	bool p;
	int temp_i;
	ADDRNode * newnode;
	bool isoper;//是否遇到'='
	// 重新打开输入流，以进行第2遍扫描
	fopen_s(&src, sourcename, "r");
	if (src == NULL) {
		printf("Could not open input file\n");
		exit(1);
	}



	while (!feof(src)) {
		/*
		开始第2遍扫描,结合第1遍扫描生成的符号表,将输入的汇编源程序汇编成机器指令,
		将其存入到Machine的mem中
		*/
		token_type = lex(words, opcode);	//读入一个单词,取得单词的类别及相关信息
		//		std::cout << words << '\t' << opcode << '\n';
		switch (token_type) {
		case LOCALVAL:
			temp_i = seeklabel(words);
			if (temp_i == -1) {
				printf("The program should never reach here. If this infomation is displayed, check the error.\n");
				strcpy_s(symtabhdr[symcnt].symname, strlen(words) + 1, words);
				token_type = lex(words, opcode);
				if (token_type == PNUM)
				{
					symtabhdr[symcnt].symaddr = atoi(words);
				}
				//symtabhdr[symcnt].firstref = NULL;
				++symcnt;
			}
			else {
				//读取下一个token 判断是否为数字
				token_type = lex(words, opcode);
				if (token_type == PNUM)
				{
					//修改此变量保存的地址值
					symtabhdr[temp_i].symaddr = atoi(words);
				}
				else{
					printf("Do you forget the positive number?\n");
				}

			}
			break;

		case ASMCODE:		//汇编指令处理

			op = getopcode(words);	// 查询汇编指令代表的机器指令,如果指令带有操作数，第1位是1
			if (op == BAD_CODE)	// 不识别的汇编指令,设置错误标志
			{
				printf("Pass2: %s - Bad mnemonic at %d\n", words, program_count);

			}


			out_program[program_count] = op;	// 存储机器指令到机器的内存中
			++program_count;	// 地址计数器增1
			break;


		case REFLABEL:		//地址标号的引用处理

			//查询符号表,是否已经有该符号,返回在符号表的位置
			labelidx = seeklabel(words);
			if (labelidx == -1)
			{
				strcat_s(words, ":");	//将地址标号转变成其定义的形式
				labelidx = seeklabel(words);//再次查询符号表,是否已经有该符号,返回在符号表的位置
				if (labelidx == -1) {
					printf
						("Pass2: ERROR! No such label to be referenced :%s at %d\n", words, program_count);
					system("pause");
					exit(3);	//未定义该标号,引用错误!
				}

			}

			fseek(src, -1, SEEK_CUR);


			//查看是否为data
			//并判断是引用还是赋值

			//检查是否有定义，即后续字符是否存在'='号
			char ch;
			
			isoper = 0;
			do{
				ch = getc(src);

				//如果是'='
				if (ch == '=')
				{
					fseek(src, -1, SEEK_CUR);//文件指针向前移动一个字节，重新指向'='
					isoper = 1;
					break;
				}
				//如果是换行符
				else if (ch == 0x0a)
				{
					fseek(src, -1, SEEK_CUR);//文件指针向前移动一个字节，重新指向'0x0a'
					break;
				}
				//如果指向一个';'
				else if (ch == ';')
				{
					fseek(src, -1, SEEK_CUR);//文件指针向前移动一个字节，重新指向';'
					break;
				}
			} while (ch <= ' '&&!feof(src));

			//如果有'=',表明这是一次赋值
			if (isoper)
			{
				token_type = lex(words, opcode);
				if (token_type == OPER)
				{
					if ((unsigned int)opcode>symtabhdr[labelidx].datalen)
					{
						printf("overflow   %s    ", symtabhdr[labelidx].symname);
						system("pause");
						exit(1);
					}
					else
					{
						printf("If we reach here, we set value to a referred label and make nosense.\n");
						for (int i = 0; i != symtabhdr[labelidx].datalen; ++i)
						{
							out_data[data_count + i] = words[i];
							++data_count;
						}
						symtabhdr[labelidx].symaddr = data_count - 4;

					}
				}
				break;//将数据写入data区，跳出
			}

			//如果没有跳出，表示无'='号，
			//此token为一个引用

			//取得该符号所代表的内存地址，存到机器的内存中
			reflableaddr = symtabhdr[labelidx].symaddr;
			newnode = new ADDRNode();
			newnode->addr = program_count;
			newnode->next = NULL;
			if (symtabhdr[labelidx].referencelist == NULL){
				symtabhdr[labelidx].referencelist = newnode;
				symtabhdr[labelidx].referredtime = 1;
				symtabhdr[labelidx].lastreferredlocation = symtabhdr[labelidx].referencelist;
			}
			else{
				symtabhdr[labelidx].lastreferredlocation->next = newnode;
				symtabhdr[labelidx].lastreferredlocation = symtabhdr[labelidx].lastreferredlocation->next;
				symtabhdr[labelidx].referredtime++;
			}
			/*
			判断REFLABEL是否作为相对地址跳转指令
			JMP JTRUE JFALSE JTRUEPOP JFALSEPOP
			的操作数，如果是，计算相对偏移量的值
			p=0为负偏移
			*/
			p = 1;
			if (out_program[program_count - 1] == getopcode(ch_jmp)
				|| out_program[program_count - 1] == getopcode(ch_jtrue)
				|| out_program[program_count - 1] == getopcode(ch_jfalse)
				|| out_program[program_count - 1] == getopcode(ch_jtruepop)
				|| out_program[program_count - 1] == getopcode(ch_jfalsepop))
			{
				if (reflableaddr<program_count)
				{
					p = 0;
					reflableaddr = program_count - reflableaddr + 4;
				}

				else
				{
					reflableaddr = reflableaddr - program_count - 4;
					p = 1;
				}
			}
			
			a = (reflableaddr & 0xff000000) >> 24;
			if (p)
				a &= 0x7f;
			else
				a |= 0x80;
			b = (reflableaddr & 0x00ff0000) >> 16;
			c = (reflableaddr & 0x0000ff00) >> 8;
			d = reflableaddr & 0x000000ff;
			out_program[program_count] = d;
			++program_count;
			out_program[program_count] = c;
			++program_count;
			out_program[program_count] = b;
			++program_count;
			out_program[program_count] = a;
			++program_count;
			break;

		case SDATA:		//DATA处理
			if (strcmp(words, "*") == 0)//如果是匿名data
			{
				token_type = lex(words, opcode);
				if (token_type == OPER)
				{
					for (int i = 0; i != opcode; ++i)
					{
						out_data[data_count] = words[i];
						++data_count;
					}
				}

				else
				{
					;//报错
				}

			}
			else
			{
				//区分是 name[len] 还是 name
				char name[MAXTOKEN];	//data name
				bool f = 0;				//f=0表示有[len]
				unsigned int j;
				for (j = 0; j<strlen(words) + 1; ++j)
				{
					if (words[j] != '[')
						name[j] = words[j];
					else
					{
						f = 1;
						break;
					}
				}
				if (f&&words[j] == '[')
				{
					name[j] = 0;
				}


				fseek(src, -1, SEEK_CUR);

				//检查是否有定义，即后续字符是否存在'='号
				char ch;
				isoper = 0;//是否遇到'='
				do{
					ch = getc(src);

					//如果是'='
					if (ch == '=')
					{
						fseek(src, -1, SEEK_CUR);//文件指针向前移动一个字节，重新指向'='
						isoper = 1;
						break;
					}
					//如果是换行符
					else if (ch == 0x0a)
					{
						fseek(src, -1, SEEK_CUR);//文件指针向前移动一个字节，重新指向'0x0a'
						break;
					}
					//如果指向一个';'
					else if (ch == ';')
					{
						fseek(src, -1, SEEK_CUR);//文件指针向前移动一个字节，重新指向';'
						break;
					}
				} while (ch <= ' '&&!feof(src));

				//如果有'='
				if (isoper)
				{
					//查询符号表,是否已经有该符号,返回在符号表的位置
					labelidx = seeklabel(name);
					if (labelidx == -1)
					{
						;//error
					}
					else
					{
						int temp_len = symtabhdr[labelidx].datalen;
						token_type = lex(words, opcode);
						if (token_type == OPER)
						{
							if ((int)opcode>temp_len)
							{
								printf("overflow   %s    ", name);
								system("pause");
								exit(1);
							}
							else
							{
								for (int i = 0; i != temp_len; ++i)
								{
									out_data[data_count] = words[i];
									++data_count;
								}
							}
						}
					}

				}
				else//如果没有'='
				{
					++data_count;
					;//只声明，未定义
				}

			}
			break;

		case INCLUDE:
			savedFile = src;
			src = NULL;
			assemble_round2(words);
			src = savedFile;
			break;
		case PNUM:		//正数值处理
			number = atoi(words);

			a = (number & 0xff000000) >> 24;
			b = (number & 0x00ff0000) >> 16;
			c = (number & 0x0000ff00) >> 8;
			d = number & 0x000000ff;
			out_program[program_count] = d;
			++program_count;
			out_program[program_count] = c;
			++program_count;
			out_program[program_count] = b;
			++program_count;
			out_program[program_count] = a;
			++program_count;
			break;

		case NNUM:		//负数值处理
			number = atoi(words);

			a = (number & 0xff000000 | 0x80000000) >> 24;
			b = (number & 0x00ff0000) >> 16;
			c = (number & 0x0000ff00) >> 8;
			d = number & 0x000000ff;
			out_program[program_count] = d;
			++program_count;
			out_program[program_count] = c;
			++program_count;
			out_program[program_count] = b;
			++program_count;
			out_program[program_count] = a;
			++program_count;
			break;
		}
	}
	fclose(src);
	src = NULL;


}

/*
	assemble
	汇编函数
	2遍扫描源程序,构造符号表,汇编成机器指令
	*/
void SFAS::assemble(char *sourcename)
{
	
	strcpy_s(srcfile, sourcename);
	//	printf("Assembling code ... \n");
	printf("Round 1...\n");
	assemble_round1(sourcename);
	printf("Round 2...\n");
	//开辟空间存储data和program
	out_data = new unsigned int[data_count];
	out_program = new unsigned char[program_count];
	data_count = 0;
	program_count = 0;
	assemble_round2(sourcename);

	//	printf("Binary assembled: %d bytes.\n", lc);

	outToFile(srcfile);

	delete[] out_data;
	delete[] out_program;
}


//将汇编得到的机器码写入bin文件
void SFAS::outToFile(char *filename){
	unsigned int gsize, esize, ssize;
	char binfile[20];
	char sizeintofile[4];
	unsigned char buf[128];
	FILE *bin;
	int i;
	for (i = 0; i < 20; i++){
		binfile[i] = filename[i];
		if (binfile[i] == '.')
			break;
	}
	binfile[++i] = 'b';
	binfile[++i] = 'i';
	binfile[++i] = 'n';
	binfile[++i] = '\0';


	fopen_s(&bin, binfile, "wb");
	gsize = 0;
	esize = 0;
	ssize = 0;
	if (bin == NULL) {
		printf("Binary file ignored.\n");
	}
	else {
		for (int i = 0; i < 4; i++){
			sizeintofile[i] = data_count >> (i * 8);
		}
		fwrite(sizeintofile, 1, 4, bin);
		for (int i = 0; i < 4; i++){
			sizeintofile[i] = program_count >> (i * 8);
		}
		fwrite(sizeintofile, 1, 4, bin);
		fwrite(buf, 1, 32 - 8, bin);

		fwrite(out_data, 1, data_count * 4, bin);
		fwrite(out_program, 1, program_count, bin);

		ADDRNode *ptr;
		printf("Global symbol:\n");
		for (int i = 0; i < SYMTABLEN; i++){
			if (symtabhdr[i].type & 0x04){
				fwrite(symtabhdr[i].symname, 1, strlen(symtabhdr[i].symname), bin);
				buf[0] = ' ';
				fwrite(buf, 1, 1, bin);
				gsize = gsize + 1 + strlen(symtabhdr[i].symname);
				for (int j = 0; j < 4; j++){
					buf[j] = symtabhdr[i].symaddr >> (j * 8);
				}
				fwrite(buf, 1, 4, bin);
				gsize += 4;
				printf("Name: %s\n", symtabhdr[i].symname);
				printf("Location: %d\n", symtabhdr[i].symaddr);
				printf("Referred time: %d\n", symtabhdr[i].referredtime);
				ptr = symtabhdr[i].referencelist;
				while (ptr != NULL){
					printf("Referred at: %d\n", ptr->addr);
					ptr = ptr->next;
				}
			}
		}
		printf("Extern symbol:\n");
		for (int i = 0; i < SYMTABLEN; i++){
			if (symtabhdr[i].type & 0x02){
				fwrite(symtabhdr[i].symname, 1, strlen(symtabhdr[i].symname), bin);
				buf[0] = ' ';
				fwrite(buf, 1, 1, bin);
				esize = esize + 1 + strlen(symtabhdr[i].symname);
				for (int j = 0; j < 4; j++){
					sizeintofile[j] = symtabhdr[i].referredtime >> (j * 8);
				}
				esize += 4;
				fwrite(sizeintofile, 1, 4, bin);
				printf("Name: %s\n", symtabhdr[i].symname);
				printf("Location: %d\n", symtabhdr[i].symaddr);
				printf("Referred time: %d\n", symtabhdr[i].referredtime);
				ptr = symtabhdr[i].referencelist;
				while (ptr != NULL){
					for (int j = 0; j < 4; j++){
						sizeintofile[j] = ptr->addr >> (j * 8);
					}
					esize += 4;
					fwrite(sizeintofile, 1, 4, bin);
					printf("Referred at: %d\n", ptr->addr);
					ptr = ptr->next;
				}
			}
		}
		printf("General symbol:\n");
		for (int i = 0; i < SYMTABLEN; i++){
			if (symtabhdr[i].type & 0x01){
				fwrite(symtabhdr[i].symname, 1, strlen(symtabhdr[i].symname), bin);
				buf[0] = ' ';
				fwrite(buf, 1, 1, bin);
				ssize = ssize + 1 + strlen(symtabhdr[i].symname);
				for (int j = 0; j < 4; j++){
					sizeintofile[j] = symtabhdr[i].symaddr >> (j * 8);
				}
				ssize += 4;
				fwrite(sizeintofile, 1, 4, bin);
				for (int j = 0; j < 4; j++){
					sizeintofile[j] = symtabhdr[i].referredtime >> (j * 8);
				}
				ssize += 4;
				fwrite(sizeintofile, 1, 4, bin);
				printf("Name: %s\n", symtabhdr[i].symname);
				printf("Location: %d\n", symtabhdr[i].symaddr);
				printf("Referred time: %d\n", symtabhdr[i].referredtime);
				ptr = symtabhdr[i].referencelist;
				while (ptr != NULL){
					for (int j = 0; j < 4; j++){
						sizeintofile[j] = ptr->addr >> (j * 8);
					}
					ssize += 4;
					fwrite(sizeintofile, 1, 4, bin);
					printf("Referred at: %d\n", ptr->addr);
					ptr = ptr->next;
				}
			}
		}

		fseek(bin, 8, SEEK_SET);
		for (int i = 0; i < 4; i++){
			sizeintofile[i] = gsize >> (i * 8);
		}
		fwrite(sizeintofile, 1, 4, bin);
		for (int i = 0; i < 4; i++){
			sizeintofile[i] = esize >> (i * 8);
		}
		fwrite(sizeintofile, 1, 4, bin);
		for (int i = 0; i < 4; i++){
			sizeintofile[i] = ssize >> (i * 8);
		}
		fwrite(sizeintofile, 1, 4, bin);
		fclose(bin);
	}
}


/*
	lex: 词法分析函数，逐字符读取输入流,
	以空格为分隔标记,将输入流的字符组合成单词
	words: 单词符号串存入words数组
	opcode：该单词对应的机器指令存入opcode
	如果是字符串，opcode保存字符串长度
	*/
tokentype SFAS::lex(char *words, CODE & opcode){
	
	char cmpwords[MAXTOKEN];
	char cmds[MAXTOKEN];	//保存token
	char ch;				//用于保存读入的单个字符
	unsigned int cds;		//汇编指令下标
	tokentype tks;			//token的类型
	int i = 0;
	int temp_i;

	/*
	**	从源文件读取字符
	**	跳过所有控制字符(00<=ASCII<=0x20)
	*/
	do {
		ch = getc(src);
	} while (ch <= ' ' && !feof(src));

	/*
	**	地址标号/汇编指令:
	**	字母开头字母及数字的有限序列
	*/
	if (isalpha(ch)) {
		do {
			cmds[i] = ch;
			++i;
			ch = getc(src);
		} while (ch > ' ' && (isalpha(ch) || isdigit(ch)) && !feof(src));

		// 若尾随有":",则为地址标号的定义,应将其保留
		if (ch == ':') {
			cmds[i] = ch;
			++i;
		}

		//查询汇编指令表,以确定是否为汇编指令还是地址标号的定义及引用
		cmds[i] = 0;
		strcpy_s(words, strlen(cmds) + 1, cmds);
		temp_i = 0;
		while (cmds[temp_i] != '\0'){
			cmpwords[temp_i] = toupper(cmds[temp_i]);
			temp_i++;
 		}
		cmpwords[temp_i] = '\0';
		cds = lookupcmd(cmds);	//查询汇编指令表
		opcode = (CODE)cds;
		//token没有出现在指令表中，且最后一个字符为':'
		if (cds == MAXINSTUCTION && words[strlen(words) - 1] == ':'){
			tks = LABEL;	// 确定为地址标号的定义
			words[strlen(words) - 1] = '\0';
		}
		//token没有出现在指令表中，且最后一个字符不为':'
		else if (cds == MAXINSTUCTION && words[strlen(words) - 1] != ':')
		{
			//是否为关键字LOCAL
			//例如local num as 1
			if (cds == MAXINSTUCTION && strcmp("LOCAL", cmpwords) == 0)
			{
				//将变量名称num保存到words
				//先读入字符串到ch_temp
				char ch_temp[MAXTOKEN];
				int temp_i = 0;
				do {
					ch = getc(src);
				} while (ch <= ' ' && !feof(src));
				if (isalpha(ch)) {
					do {
						ch_temp[temp_i] = ch;
						++temp_i;
						ch = getc(src);
					} while (ch > ' ' && (isalpha(ch) || isdigit(ch)) && !feof(src));
					ch_temp[temp_i] = 0;

					//将读入的变量名保存到words
					strcpy_s(words, strlen(ch_temp) + 1, ch_temp);

					//匹配as
					char ch_as[3];
					temp_i = 0;
					do {
						ch = toupper(getc(src));
					} while (ch <= ' ' && !feof(src));
					if (isalpha(ch)) {
						do {
							ch_as[temp_i] = ch;
							++temp_i;
							ch = toupper(getc(src));
						} while (ch > ' ' && (isalpha(ch) || isdigit(ch)) && !feof(src));
						ch_as[temp_i] = 0;
						//匹配了 local * as 
						//将local * as当作一个token返回
						//类型为LOCALVAL
						if (strcmp("AS", ch_as) == 0)
						{
							return LOCALVAL;
						}
					}
				}
			}
			//data name[length] = 'asd',0
			//data number = 1
			//data * 'hehe',0
			//将变量名称num保存到words
			//先读入字符串到ch_temp
			else if (cds == MAXINSTUCTION && strcmp("DATA", cmpwords) == 0)
			{

				char ch_temp[MAXTOKEN];
				int temp_i = 0;
				do {
					ch = getc(src);
				} while (ch <= ' ' && !feof(src));

				//如果读入一个 *
				if (ch == '*')
				{
					ch_temp[temp_i] = ch;
					++temp_i;
					ch_temp[temp_i] = 0;
					/*
					//将读入"*\0"保存到words
					strcpy_s(words,strlen(ch_temp)+1, ch_temp);
					return SDATA;
					*/
				}

				//如果读入一个 字母
				else if (isalpha(ch)) {
					do {
						ch_temp[temp_i] = ch;
						++temp_i;
						ch = getc(src);
					} while (ch > ' ' && (isalpha(ch) || isdigit(ch) || ch == '[' || ch == ']') && !feof(src));
					ch_temp[temp_i] = 0;
				}
				//将读入的变量名保存到words
				strcpy_s(words, strlen(ch_temp) + 1, ch_temp);
				tks = SDATA;
			}
			else if (cds == MAXINSTUCTION && strcmp("INCLUDE", cmpwords) == 0){
				do {
					ch = getc(src);
				} while (ch <= ' ' && !feof(src));
				int temp_i = 0;
				while ((ch = getc(src)) != '\"'){
					words[temp_i++] = ch;
				}
				words[temp_i] = 0;
				tks = INCLUDE;
			}
			else if (cds == MAXINSTUCTION && strcmp("EXTERN", cmpwords) == 0){
				do {
					ch = getc(src);
				} while (ch <= ' ' && !feof(src));
				int temp_i = 0;
				while (isdigit(ch) || isalpha(ch)){
					words[temp_i++] = ch;
					ch = getc(src);
				}
				words[temp_i] = 0;
				tks = EXTERN;
			}
			else if (cds == MAXINSTUCTION && strcmp("GLOBAL", cmpwords) == 0){
				do {
					ch = getc(src);
				} while (ch <= ' ' && !feof(src));
				int temp_i = 0;
				while (isdigit(ch) || isalpha(ch)){
					words[temp_i++] = ch;
					ch = getc(src);
				}
				words[temp_i] = 0;
				tks = GLOBAL;
			}
			else//需要检查判定是不是定义
				tks = REFLABEL;	//确定为地址标号的引用
		}

		else
			tks = ASMCODE;	// 确定为汇编指令
	}
	/*
	**	正数字的处理：
	**	数字开头后跟0~9的有限序列
	*/
	else if (isdigit(ch) || ch == '+') {
		if (ch == '+')
			ch = getc(src);
		if (isdigit(ch)){
			do {
				cmds[i] = ch;
				++i;
				ch = getc(src);
			} while (ch > ' ' && isdigit(ch) && !feof(src));

			tks = PNUM;		//确定为数字
			cmds[i] = 0;
			strcpy_s(words, strlen(cmds) + 1, cmds);
		}
		else{
			cmds[i] = ch;
			++i;
			cmds[i] = 0;
			strcpy_s(words, strlen(cmds) + 1, cmds);
			tks = UNKNOWN;		//不可识别的字符
		}

	}
	/*
	**	负数字的处理：
	**	数字开头后跟0~9的有限序列
	*/
	else if (ch == '-') {
		ch = getc(src);
		if (isdigit(ch)){
			do {
				cmds[i] = ch;
				++i;
				ch = getc(src);
			} while (ch > ' ' && isdigit(ch) && !feof(src));

			tks = NNUM;		//确定为数字
			cmds[i] = 0;
			strcpy_s(words, strlen(cmds) + 1, cmds);
		}
		else{
			cmds[i] = ch;
			++i;
			cmds[i] = 0;
			strcpy_s(words, strlen(cmds) + 1, cmds);
			tks = UNKNOWN;		//不可识别的字符
		}

	}

	/*
	**	单行注释:
	**	以';'起始至行尾的任意字符序列
	*/
	else if (ch == ';') {
		do {
			ch = getc(src);
		} while (ch != 0x0a && !feof(src));//0x0a换行控制符

		strcpy_s(words, 5, "CMNT");
		opcode = (CODE)BAD_CODE;
		tks = CMNT;		//识别为注释字符序列
	}
	/*
	**	赋值符号
	*/
	else if (ch == '=')
	{
		int data_len;
		read_data(data_len, words);
		//借用opcode保存字符串长度
		opcode = (CODE)data_len;
		tks = OPER;

	}
	/*
	**	不可识别的字符的处理
	*/
	else {
		cmds[i] = ch;
		++i;
		cmds[i] = 0;
		strcpy_s(words, strlen(cmds) + 1, cmds);
		tks = UNKNOWN;		//不可识别的字符
	}
	return tks;			//返回类别标志
}


/*lookupcmd()
	查询汇编指令表,返回对应的机器指令,机器指令隐含为指令数组的下标
	*/
unsigned int SFAS::lookupcmd(char *cmd)
{
	char buf[64];
	strcpy_s(buf, cmd);
	int i = 0;
	while (buf[i] != '\0'){
		buf[i] = toupper(buf[i]);
		i++;
	}
	i = 0;
	while (i < MAXINSTUCTION && strcmp(mnemonics[i], buf))
		++i;
	return  i;
}



/*
	seeklabel
	查询符号表，如果标号存在，返回其在符号表中的下标
	如果标号不存在，返回-1
	*/
int SFAS::seeklabel(char *lbl)
{
	int i = 0;
	for (i = 0; i < SYMTABLEN && strcmp(symtabhdr[i].symname, lbl); ++i);
	if (i < SYMTABLEN) {
		return i;
	}
	else {
		return -1;		//无此标号
	}
}


/*
	读入赋值号后面的字符串
	读入的字符串保存到words
	字符串的长度保存到data_len
	*/
void SFAS::read_data(int &data_len, char* words)
{
	
	char ch;
	//将=后的值保存到words
	//先读入字符串到ch_temp
	char ch_temp[MAXDATALEN];	//保存data的值
	int temp_i = 0;				//data的长度
	do {
		ch = getc(src);
	} while (ch <= ' ' && !feof(src));

	if (isdigit(ch) || ch == '\'') {


		do {
			//当读入一个'\''，将后续字符当初char处理
			//直到读入另一个'\''
			//退出此if语句时，文件内是指针指向'\''
			//的下一个字符
			if (ch == '\'')
			{
				ch = getc(src);
				while (ch != '\'')
				{
					if (ch == '\\')
					{
						switch (getc(src))
						{
						case 'n':
							ch_temp[temp_i++] = '\n';
							break;
						case 'r':
							ch_temp[temp_i++] = '\r';
							break;
						case 't':
							ch_temp[temp_i++] = '\t';
							break;
						case 'b':
							ch_temp[temp_i++] = '\b';
							break;
						case '\\':
							ch_temp[temp_i++] = '\\';
							break;
						case '\'':
							ch_temp[temp_i++] = '\'';
							break;
						case '"':
							ch_temp[temp_i++] = '\"';
							break;
						case 'a':
							ch_temp[temp_i++] = '\a';
							break;
						case 'f':
							ch_temp[temp_i++] = '\f';
							break;
						case 'v':
							ch_temp[temp_i++] = '\v';
							break;
							//如果不存在该转义符号
						default:
							ch_temp[temp_i++] = '\0';
							break;
						}
					}
					else
					{
						ch_temp[temp_i++] = ch;
					}
					ch = getc(src);
				}
			}

			//如果是数字开头，读入字符
			//保存数字
			//退出此if时，文件内的指针指向
			//最后一个数字字符的下一位
			else if (isdigit(ch))
			{
				char temp_num[10];
				int num_len = 0;

				do{
					temp_num[num_len++] = ch;
					ch = getc(src);
				} while (isdigit(ch) && num_len<9);
				temp_num[num_len] = 0;
				ch_temp[temp_i++] = atoi(temp_num);
				fseek(src, -1, SEEK_CUR);//文件指针向前移动一个字节，重新指向最后一个数字字符下一位
			}

			//如果读入一个','，表示紧跟一个数字或新字符串
			else if (ch == ',')
			{

			}

			//读入下一个字符
			//去除前导空格,如','之前的空格
			// 'hello',   0,   0
			do{
				ch = getc(src);
				//如果是换行符
				if (ch == 0x0a)
					break;
				//如果指向一个';'
				if (ch == ';')
				{
					fseek(src, -1, SEEK_CUR);//文件指针向前移动一个字节，重新指向';'
					break;
				}
			} while (ch <= ' '&&!feof(src));

		} while ((isdigit(ch) || ch == '\'' || ch == ',' || ch == '\\') && ch != 0x0a && !feof(src));
	}
	//将读入的变量名保存到words
	//	strcpy_s(words, temp_i, ch_temp);
	for (int i = 0; i < temp_i; i++)
		words[i] = ch_temp[i];
	data_len = temp_i;
}

int smain(int argc, char* argv[]){
	SFAS mysfas;
	char filename[30];
	std::cout << "Please input asm file name:\n";
	std::cin.getline(filename, 30);
	mysfas.assemble(filename);
	return 0;
}
int main(int argc, char* argv[]){
	SFAS mysfas;
	if (argc <= 1){
		printf("No input file.\n");
		return 1;
	}
	mysfas.assemble(argv[1]);
	return 0;
}