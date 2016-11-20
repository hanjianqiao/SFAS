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
//ȡ�û���ָ��i�����Ƿ� 
inline const char *getmns(int i) {
	return mnemonics[i];
}
*/
//test
// ��str��ָ��Ļ�����Ƿ���ָ���
	// ����ȷ��ָ����ʽ����������Ӧ�Ļ����룬���ڴ����ָ���0xff
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
			//�ҳ����д���һ��������ָ��
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
	������Ĺ��캯��AS: ��Դ�ļ�,��ʼ�����ű�
	*/
SFAS::SFAS()
{
	

	int i;
	//���ű��ڵķ�����ȫ����ʼΪ '????????'
	for (i = 0; i < SYMTABLEN; ++i){
		strcpy_s(symtabhdr[i].symname, 8, "???????");
		symtabhdr[i].datalen = 0;
		symtabhdr[i].type = 0x00;
		symtabhdr[i].symaddr = 0;
		symtabhdr[i].referredtime = 0;
		symtabhdr[i].referencelist = NULL;
		symtabhdr[i].lastreferredlocation = NULL;
	}

	symcnt = 0;				//���ű��з��Ÿ���
	data_count = 0;			//���ݼ���������
	program_count = 0;		//�������������

}


/*
	���������������~AS: �ر�Դ�ļ�
	*/
SFAS::~SFAS()
{
	
	if (src) {
		fclose(src);
	}
	src = NULL;
}


//��һ��ɨ�裬�������ű�
void SFAS::assemble_round1(char *sourcename){ 
	char words[MAXTOKEN];
	CODE opcode;
	tokentype token_type;
	FILE *savedFile;
	bool isoper;		//���ڱ���Ƿ���'='
	int temp_i;
	fopen_s(&src, sourcename, "r");
	if (src == NULL) {
		printf("Could not open input file: %s.\n", sourcename);
		exit(1);
	}
	while (!feof(src))
	{
		token_type = lex(words, opcode);	//����һ������,ȡ�õ��ʵ���������Ϣ


		switch (token_type) {

		case ASMCODE:
			program_count++;
			break;
		case CMNT:						//ע��
			break;

		case LABEL:							//�¶���ı�ţ����������ű�  
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

		case LOCALVAL:						//	�¶���local vlcalval as uint
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
			//No else, ���Է����ض���
			break;

		case SDATA:							//	��������data
			if (strcmp(words, "*") == 0)//���������data
			{
				token_type = lex(words, opcode);
				if (token_type == OPER)
					data_count += (int)opcode;
				else
				{
					;//����
				}

				//���浽��̬������
			}
			else
			{
				//������ name[len] ���� name
				char name[MAXTOKEN];	//data name
				char c_temp_len[3];
				int temp_len = 0;			//data����
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
				//�����name����
				if (!f)
				{
					temp_len = 1;
				}

				//������ű�
				strcpy_s(symtabhdr[symcnt].symname, strlen(name) + 1, name);
				symtabhdr[symcnt].symaddr = data_count;
				symtabhdr[symcnt].datalen = temp_len;
				symtabhdr[symcnt].type = 0x01;
				//symtabhdr[symcnt].firstref = NULL;
				++symcnt;
				data_count += temp_len;
				
				/*
					�Ƿ�Ϊ���� begin
				*/

				fseek(src, -1, SEEK_CUR);

				//����Ƿ��ж��壬�������ַ��Ƿ����'='��
				char ch;
				isoper = 0;//�Ƿ�����'='
				do{
					ch = getc(src);

					//�����'='
					if (ch == '=')
					{
						fseek(src, -1, SEEK_CUR);//�ļ�ָ����ǰ�ƶ�һ���ֽڣ�����ָ��'='
						isoper = 1;
						break;
					}
					//����ǻ��з�
					else if (ch == 0x0a)
					{
						fseek(src, -1, SEEK_CUR);//�ļ�ָ����ǰ�ƶ�һ���ֽڣ�����ָ��'\n'
						break;
					}
					//���ָ��һ��';'
					else if (ch == ';')
					{
						fseek(src, -1, SEEK_CUR);//�ļ�ָ����ǰ�ƶ�һ���ֽڣ�����ָ��';'
						break;
					}
				} while (ch <= ' '&&!feof(src));

				//�����'='
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
					�Ƕ��� end
				*/
				else//���û��'='
				{
					;//ֻ������δ����
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
				�Ƿ�Ϊ���� begin
			*/

			fseek(src, -1, SEEK_CUR);

			//����Ƿ��ж��壬�������ַ��Ƿ����'='��
			char ch;
			isoper = 0;//�Ƿ�����'='
			do{
				ch = getc(src);

				//�����'='
				if (ch == '=')
				{
					fseek(src, -1, SEEK_CUR);//�ļ�ָ����ǰ�ƶ�һ���ֽڣ�����ָ��'='
					isoper = 1;
					break;
				}
				//����ǻ��з�������һ���������
				else if (ch == 0x0a)
				{
					fseek(src, -1, SEEK_CUR);//�ļ�ָ����ǰ�ƶ�һ���ֽڣ�����ָ��'\n'
					break;
				}
				//���ָ��һ��';'������һ���������
				else if (ch == ';')
				{
					fseek(src, -1, SEEK_CUR);//�ļ�ָ����ǰ�ƶ�һ���ֽڣ�����ָ��';'
					break;
				}
			} while (ch <= ' '&&!feof(src));

			//�����'='������һ������
			if (isoper)
			{
			}
			/*
				�Ƕ��� end
			*/
			else//���û��'='
			{
				program_count += 4; //��ŵ�ַռ4���ֽ�	
			}
			break;

		case PNUM:
		case NNUM:
			program_count += 4;//����ռ4���ֽ�
			break;
		case EXTERN:							//�¶���ı�ţ����������ű�  
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
		case GLOBAL:							//�¶���ı�ţ����������ű�  
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
		case UNKNOWN:		//����ʶ��ķ���,���������Ϣ
			break;
		}

	}
	fclose(src);
	src = NULL;			// �ر�������,��ɵ�1��ɨ��,׼����2��ɨ��
}

//�ڶ���ɨ�裬���ɴ��롣
void SFAS::assemble_round2(char *sourcename){
	//��������ָ����Ҫ�������ƫ����
	char ch_jmp[] = "JMP";
	char ch_jtrue[] = "JTRUE";
	char ch_jfalse[] = "JFALSE";
	char ch_jtruepop[] = "JTRUEPOP";
	char ch_jfalsepop[] = "JFALSEPOP";
	FILE *savedFile;
	unsigned char op;			//���ڱ���ָ��
	unsigned char a, b, c, d;		//���ڱ���ָ��Ĳ�����
	int number;					 //���ڱ�����ֵtoken
	int reflableaddr;			//����loacl val
	int labelidx;				// ����ڷ��ű��ڵ����
	bool p;
	int temp_i;
	ADDRNode * newnode;
	bool isoper;//�Ƿ�����'='
	// ���´����������Խ��е�2��ɨ��
	fopen_s(&src, sourcename, "r");
	if (src == NULL) {
		printf("Could not open input file\n");
		exit(1);
	}



	while (!feof(src)) {
		/*
		��ʼ��2��ɨ��,��ϵ�1��ɨ�����ɵķ��ű�,������Ļ��Դ������ɻ���ָ��,
		������뵽Machine��mem��
		*/
		token_type = lex(words, opcode);	//����һ������,ȡ�õ��ʵ���������Ϣ
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
				//��ȡ��һ��token �ж��Ƿ�Ϊ����
				token_type = lex(words, opcode);
				if (token_type == PNUM)
				{
					//�޸Ĵ˱�������ĵ�ֵַ
					symtabhdr[temp_i].symaddr = atoi(words);
				}
				else{
					printf("Do you forget the positive number?\n");
				}

			}
			break;

		case ASMCODE:		//���ָ���

			op = getopcode(words);	// ��ѯ���ָ�����Ļ���ָ��,���ָ����в���������1λ��1
			if (op == BAD_CODE)	// ��ʶ��Ļ��ָ��,���ô����־
			{
				printf("Pass2: %s - Bad mnemonic at %d\n", words, program_count);

			}


			out_program[program_count] = op;	// �洢����ָ��������ڴ���
			++program_count;	// ��ַ��������1
			break;


		case REFLABEL:		//��ַ��ŵ����ô���

			//��ѯ���ű�,�Ƿ��Ѿ��и÷���,�����ڷ��ű��λ��
			labelidx = seeklabel(words);
			if (labelidx == -1)
			{
				strcat_s(words, ":");	//����ַ���ת����䶨�����ʽ
				labelidx = seeklabel(words);//�ٴβ�ѯ���ű�,�Ƿ��Ѿ��и÷���,�����ڷ��ű��λ��
				if (labelidx == -1) {
					printf
						("Pass2: ERROR! No such label to be referenced :%s at %d\n", words, program_count);
					system("pause");
					exit(3);	//δ����ñ��,���ô���!
				}

			}

			fseek(src, -1, SEEK_CUR);


			//�鿴�Ƿ�Ϊdata
			//���ж������û��Ǹ�ֵ

			//����Ƿ��ж��壬�������ַ��Ƿ����'='��
			char ch;
			
			isoper = 0;
			do{
				ch = getc(src);

				//�����'='
				if (ch == '=')
				{
					fseek(src, -1, SEEK_CUR);//�ļ�ָ����ǰ�ƶ�һ���ֽڣ�����ָ��'='
					isoper = 1;
					break;
				}
				//����ǻ��з�
				else if (ch == 0x0a)
				{
					fseek(src, -1, SEEK_CUR);//�ļ�ָ����ǰ�ƶ�һ���ֽڣ�����ָ��'0x0a'
					break;
				}
				//���ָ��һ��';'
				else if (ch == ';')
				{
					fseek(src, -1, SEEK_CUR);//�ļ�ָ����ǰ�ƶ�һ���ֽڣ�����ָ��';'
					break;
				}
			} while (ch <= ' '&&!feof(src));

			//�����'=',��������һ�θ�ֵ
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
				break;//������д��data��������
			}

			//���û����������ʾ��'='�ţ�
			//��tokenΪһ������

			//ȡ�ø÷�����������ڴ��ַ���浽�������ڴ���
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
			�ж�REFLABEL�Ƿ���Ϊ��Ե�ַ��תָ��
			JMP JTRUE JFALSE JTRUEPOP JFALSEPOP
			�Ĳ�����������ǣ��������ƫ������ֵ
			p=0Ϊ��ƫ��
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

		case SDATA:		//DATA����
			if (strcmp(words, "*") == 0)//���������data
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
					;//����
				}

			}
			else
			{
				//������ name[len] ���� name
				char name[MAXTOKEN];	//data name
				bool f = 0;				//f=0��ʾ��[len]
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

				//����Ƿ��ж��壬�������ַ��Ƿ����'='��
				char ch;
				isoper = 0;//�Ƿ�����'='
				do{
					ch = getc(src);

					//�����'='
					if (ch == '=')
					{
						fseek(src, -1, SEEK_CUR);//�ļ�ָ����ǰ�ƶ�һ���ֽڣ�����ָ��'='
						isoper = 1;
						break;
					}
					//����ǻ��з�
					else if (ch == 0x0a)
					{
						fseek(src, -1, SEEK_CUR);//�ļ�ָ����ǰ�ƶ�һ���ֽڣ�����ָ��'0x0a'
						break;
					}
					//���ָ��һ��';'
					else if (ch == ';')
					{
						fseek(src, -1, SEEK_CUR);//�ļ�ָ����ǰ�ƶ�һ���ֽڣ�����ָ��';'
						break;
					}
				} while (ch <= ' '&&!feof(src));

				//�����'='
				if (isoper)
				{
					//��ѯ���ű�,�Ƿ��Ѿ��и÷���,�����ڷ��ű��λ��
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
				else//���û��'='
				{
					++data_count;
					;//ֻ������δ����
				}

			}
			break;

		case INCLUDE:
			savedFile = src;
			src = NULL;
			assemble_round2(words);
			src = savedFile;
			break;
		case PNUM:		//����ֵ����
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

		case NNUM:		//����ֵ����
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
	��ຯ��
	2��ɨ��Դ����,������ű�,���ɻ���ָ��
	*/
void SFAS::assemble(char *sourcename)
{
	
	strcpy_s(srcfile, sourcename);
	//	printf("Assembling code ... \n");
	printf("Round 1...\n");
	assemble_round1(sourcename);
	printf("Round 2...\n");
	//���ٿռ�洢data��program
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


//�����õ��Ļ�����д��bin�ļ�
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
	lex: �ʷ��������������ַ���ȡ������,
	�Կո�Ϊ�ָ����,�����������ַ���ϳɵ���
	words: ���ʷ��Ŵ�����words����
	opcode���õ��ʶ�Ӧ�Ļ���ָ�����opcode
	������ַ�����opcode�����ַ�������
	*/
tokentype SFAS::lex(char *words, CODE & opcode){
	
	char cmpwords[MAXTOKEN];
	char cmds[MAXTOKEN];	//����token
	char ch;				//���ڱ������ĵ����ַ�
	unsigned int cds;		//���ָ���±�
	tokentype tks;			//token������
	int i = 0;
	int temp_i;

	/*
	**	��Դ�ļ���ȡ�ַ�
	**	�������п����ַ�(00<=ASCII<=0x20)
	*/
	do {
		ch = getc(src);
	} while (ch <= ' ' && !feof(src));

	/*
	**	��ַ���/���ָ��:
	**	��ĸ��ͷ��ĸ�����ֵ���������
	*/
	if (isalpha(ch)) {
		do {
			cmds[i] = ch;
			++i;
			ch = getc(src);
		} while (ch > ' ' && (isalpha(ch) || isdigit(ch)) && !feof(src));

		// ��β����":",��Ϊ��ַ��ŵĶ���,Ӧ���䱣��
		if (ch == ':') {
			cmds[i] = ch;
			++i;
		}

		//��ѯ���ָ���,��ȷ���Ƿ�Ϊ���ָ��ǵ�ַ��ŵĶ��弰����
		cmds[i] = 0;
		strcpy_s(words, strlen(cmds) + 1, cmds);
		temp_i = 0;
		while (cmds[temp_i] != '\0'){
			cmpwords[temp_i] = toupper(cmds[temp_i]);
			temp_i++;
 		}
		cmpwords[temp_i] = '\0';
		cds = lookupcmd(cmds);	//��ѯ���ָ���
		opcode = (CODE)cds;
		//tokenû�г�����ָ����У������һ���ַ�Ϊ':'
		if (cds == MAXINSTUCTION && words[strlen(words) - 1] == ':'){
			tks = LABEL;	// ȷ��Ϊ��ַ��ŵĶ���
			words[strlen(words) - 1] = '\0';
		}
		//tokenû�г�����ָ����У������һ���ַ���Ϊ':'
		else if (cds == MAXINSTUCTION && words[strlen(words) - 1] != ':')
		{
			//�Ƿ�Ϊ�ؼ���LOCAL
			//����local num as 1
			if (cds == MAXINSTUCTION && strcmp("LOCAL", cmpwords) == 0)
			{
				//����������num���浽words
				//�ȶ����ַ�����ch_temp
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

					//������ı��������浽words
					strcpy_s(words, strlen(ch_temp) + 1, ch_temp);

					//ƥ��as
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
						//ƥ���� local * as 
						//��local * as����һ��token����
						//����ΪLOCALVAL
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
			//����������num���浽words
			//�ȶ����ַ�����ch_temp
			else if (cds == MAXINSTUCTION && strcmp("DATA", cmpwords) == 0)
			{

				char ch_temp[MAXTOKEN];
				int temp_i = 0;
				do {
					ch = getc(src);
				} while (ch <= ' ' && !feof(src));

				//�������һ�� *
				if (ch == '*')
				{
					ch_temp[temp_i] = ch;
					++temp_i;
					ch_temp[temp_i] = 0;
					/*
					//������"*\0"���浽words
					strcpy_s(words,strlen(ch_temp)+1, ch_temp);
					return SDATA;
					*/
				}

				//�������һ�� ��ĸ
				else if (isalpha(ch)) {
					do {
						ch_temp[temp_i] = ch;
						++temp_i;
						ch = getc(src);
					} while (ch > ' ' && (isalpha(ch) || isdigit(ch) || ch == '[' || ch == ']') && !feof(src));
					ch_temp[temp_i] = 0;
				}
				//������ı��������浽words
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
			else//��Ҫ����ж��ǲ��Ƕ���
				tks = REFLABEL;	//ȷ��Ϊ��ַ��ŵ�����
		}

		else
			tks = ASMCODE;	// ȷ��Ϊ���ָ��
	}
	/*
	**	�����ֵĴ���
	**	���ֿ�ͷ���0~9����������
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

			tks = PNUM;		//ȷ��Ϊ����
			cmds[i] = 0;
			strcpy_s(words, strlen(cmds) + 1, cmds);
		}
		else{
			cmds[i] = ch;
			++i;
			cmds[i] = 0;
			strcpy_s(words, strlen(cmds) + 1, cmds);
			tks = UNKNOWN;		//����ʶ����ַ�
		}

	}
	/*
	**	�����ֵĴ���
	**	���ֿ�ͷ���0~9����������
	*/
	else if (ch == '-') {
		ch = getc(src);
		if (isdigit(ch)){
			do {
				cmds[i] = ch;
				++i;
				ch = getc(src);
			} while (ch > ' ' && isdigit(ch) && !feof(src));

			tks = NNUM;		//ȷ��Ϊ����
			cmds[i] = 0;
			strcpy_s(words, strlen(cmds) + 1, cmds);
		}
		else{
			cmds[i] = ch;
			++i;
			cmds[i] = 0;
			strcpy_s(words, strlen(cmds) + 1, cmds);
			tks = UNKNOWN;		//����ʶ����ַ�
		}

	}

	/*
	**	����ע��:
	**	��';'��ʼ����β�������ַ�����
	*/
	else if (ch == ';') {
		do {
			ch = getc(src);
		} while (ch != 0x0a && !feof(src));//0x0a���п��Ʒ�

		strcpy_s(words, 5, "CMNT");
		opcode = (CODE)BAD_CODE;
		tks = CMNT;		//ʶ��Ϊע���ַ�����
	}
	/*
	**	��ֵ����
	*/
	else if (ch == '=')
	{
		int data_len;
		read_data(data_len, words);
		//����opcode�����ַ�������
		opcode = (CODE)data_len;
		tks = OPER;

	}
	/*
	**	����ʶ����ַ��Ĵ���
	*/
	else {
		cmds[i] = ch;
		++i;
		cmds[i] = 0;
		strcpy_s(words, strlen(cmds) + 1, cmds);
		tks = UNKNOWN;		//����ʶ����ַ�
	}
	return tks;			//��������־
}


/*lookupcmd()
	��ѯ���ָ���,���ض�Ӧ�Ļ���ָ��,����ָ������Ϊָ��������±�
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
	��ѯ���ű������Ŵ��ڣ��������ڷ��ű��е��±�
	�����Ų����ڣ�����-1
	*/
int SFAS::seeklabel(char *lbl)
{
	int i = 0;
	for (i = 0; i < SYMTABLEN && strcmp(symtabhdr[i].symname, lbl); ++i);
	if (i < SYMTABLEN) {
		return i;
	}
	else {
		return -1;		//�޴˱��
	}
}


/*
	���븳ֵ�ź�����ַ���
	������ַ������浽words
	�ַ����ĳ��ȱ��浽data_len
	*/
void SFAS::read_data(int &data_len, char* words)
{
	
	char ch;
	//��=���ֵ���浽words
	//�ȶ����ַ�����ch_temp
	char ch_temp[MAXDATALEN];	//����data��ֵ
	int temp_i = 0;				//data�ĳ���
	do {
		ch = getc(src);
	} while (ch <= ' ' && !feof(src));

	if (isdigit(ch) || ch == '\'') {


		do {
			//������һ��'\''���������ַ�����char����
			//ֱ��������һ��'\''
			//�˳���if���ʱ���ļ�����ָ��ָ��'\''
			//����һ���ַ�
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
							//��������ڸ�ת�����
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

			//��������ֿ�ͷ�������ַ�
			//��������
			//�˳���ifʱ���ļ��ڵ�ָ��ָ��
			//���һ�������ַ�����һλ
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
				fseek(src, -1, SEEK_CUR);//�ļ�ָ����ǰ�ƶ�һ���ֽڣ�����ָ�����һ�������ַ���һλ
			}

			//�������һ��','����ʾ����һ�����ֻ����ַ���
			else if (ch == ',')
			{

			}

			//������һ���ַ�
			//ȥ��ǰ���ո�,��','֮ǰ�Ŀո�
			// 'hello',   0,   0
			do{
				ch = getc(src);
				//����ǻ��з�
				if (ch == 0x0a)
					break;
				//���ָ��һ��';'
				if (ch == ';')
				{
					fseek(src, -1, SEEK_CUR);//�ļ�ָ����ǰ�ƶ�һ���ֽڣ�����ָ��';'
					break;
				}
			} while (ch <= ' '&&!feof(src));

		} while ((isdigit(ch) || ch == '\'' || ch == ',' || ch == '\\') && ch != 0x0a && !feof(src));
	}
	//������ı��������浽words
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