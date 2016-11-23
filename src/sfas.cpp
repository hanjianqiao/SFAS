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
//get notation of instruction i 
inline const char *getmns(int i) {
	return mnemonics[i];
}
*/

// get machine code for instruction str, return 0xff if not found
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
		//instructions with one parameter
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


SFAS::SFAS()
{
	

	int i;
	//initialize all to '????????'
	for (i = 0; i < SYMTABLEN; ++i){
		strcpy(symtabhdr[i].symname, "???????");
		symtabhdr[i].datalen = 0;
		symtabhdr[i].type = 0x00;
		symtabhdr[i].symaddr = 0;
		symtabhdr[i].referredtime = 0;
		symtabhdr[i].referencelist = NULL;
		symtabhdr[i].lastreferredlocation = NULL;
	}

	symcnt = 0;				//number of symboles
	data_count = 0;			//data counter
	program_count = 0;		//program counter

}


SFAS::~SFAS()
{
	
	if (src) {
		fclose(src);
	}
	src = NULL;
}


//first round scan: build the symbol table
void SFAS::assemble_round1(char *sourcename){ 
	char words[MAXTOKEN];
	CODE opcode;
	tokentype token_type;
	FILE *savedFile;
	bool isoper;		//if with a '='
	int temp_i;
	src = fopen(sourcename, "r");
	if (src == NULL) {
		printf("Could not open input file: %s.\n", sourcename);
		exit(1);
	}
	while (!feof(src))
	{
		token_type = lex(words, opcode);


		switch (token_type) {

		case ASMCODE:
			program_count++;
			break;
		case CMNT:
			break;

		case LABEL:
			temp_i = seeklabel(words);
			if (temp_i == -1) {
				strcpy(symtabhdr[symcnt].symname, words);
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

		case LOCALVAL:
			temp_i = seeklabel(words);
			if (temp_i == -1) {
				strcpy(symtabhdr[symcnt].symname, words);
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
			//No else, ignore symbol redefination
			break;

		case SDATA:
			if (strcmp(words, "*") == 0)// anymous data
			{
				token_type = lex(words, opcode);
				if (token_type == OPER)
					data_count += (int)opcode;
				else
				{
					;// Error
				}
			}
			else
			{
				// name[len] or name
				char name[MAXTOKEN];
				char c_temp_len[3];
				int temp_len = 0;
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
				//name
				if (!f)
				{
					temp_len = 1;
				}

				strcpy(symtabhdr[symcnt].symname, name);
				symtabhdr[symcnt].symaddr = data_count;
				symtabhdr[symcnt].datalen = temp_len;
				symtabhdr[symcnt].type = 0x01;
				++symcnt;
				data_count += temp_len;


				fseek(src, -1, SEEK_CUR);


				char ch;
				isoper = 0;
				do{
					ch = getc(src);

					if (ch == '=')
					{
						fseek(src, -1, SEEK_CUR);
						isoper = 1;
						break;
					}
					else if (ch == 0x0a)
					{
						fseek(src, -1, SEEK_CUR);
						break;
					}
					else if (ch == ';')
					{
						fseek(src, -1, SEEK_CUR);
						break;
					}
				} while (ch <= ' '&&!feof(src));

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
				else
				{
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

			fseek(src, -1, SEEK_CUR);

			char ch;
			isoper = 0;
			do{
				ch = getc(src);

				if (ch == '=')
				{
					fseek(src, -1, SEEK_CUR);
					isoper = 1;
					break;
				}
				else if (ch == 0x0a)
				{
					fseek(src, -1, SEEK_CUR);
					break;
				}
				else if (ch == ';')
				{
					fseek(src, -1, SEEK_CUR);
					break;
				}
			} while (ch <= ' '&&!feof(src));

			if (isoper)
			{
			}
			else
			{
				program_count += 4;
			}
			break;

		case PNUM:
		case NNUM:
			program_count += 4;
			break;
		case EXTERN:
			temp_i = seeklabel(words);
			if (temp_i == -1) {
				strcpy(symtabhdr[symcnt].symname, words);
				symtabhdr[symcnt].type = 0x02;
				symtabhdr[symcnt].symaddr = 0xffffffff;
				++symcnt;
			}
			else{
				printf("ERROR when define new label.\n");
			}
			break;
		case GLOBAL:
			temp_i = seeklabel(words);
			if (temp_i == -1) {
				strcpy(symtabhdr[symcnt].symname, words);
				symtabhdr[symcnt].type = 0x04;
				symtabhdr[symcnt].symaddr = 0xffffffff;
				++symcnt;
			}
			else{
				printf("ERROR when define new label.\n");
			}
			break;
		case UNKNOWN:
			break;
		}

	}
	fclose(src);
	src = NULL;
}

//Scan round 2: generate code
void SFAS::assemble_round2(char *sourcename){
	//need relative offset
	char ch_jmp[] = "JMP";
	char ch_jtrue[] = "JTRUE";
	char ch_jfalse[] = "JFALSE";
	char ch_jtruepop[] = "JTRUEPOP";
	char ch_jfalsepop[] = "JFALSEPOP";
	FILE *savedFile;
	unsigned char op;			//instruction
	unsigned char a, b, c, d;		//instruction data
	int number;					 //number token
	int reflableaddr;			//loacl val
	int labelidx;				// index of symbol in symbol table
	bool p;
	int temp_i;
	ADDRNode * newnode;
	bool isoper;
	src = fopen(sourcename, "r");
	if (src == NULL) {
		printf("Could not open input file\n");
		exit(1);
	}



	while (!feof(src)) {
		token_type = lex(words, opcode);
		//		std::cout << words << '\t' << opcode << '\n';
		switch (token_type) {
		case LOCALVAL:
			temp_i = seeklabel(words);
			if (temp_i == -1) {
				printf("The program should never reach here. If this infomation is displayed, check the error.\n");
				strcpy(symtabhdr[symcnt].symname, words);
				token_type = lex(words, opcode);
				if (token_type == PNUM)
				{
					symtabhdr[symcnt].symaddr = atoi(words);
				}
				//symtabhdr[symcnt].firstref = NULL;
				++symcnt;
			}
			else {
				token_type = lex(words, opcode);
				if (token_type == PNUM)
				{
					symtabhdr[temp_i].symaddr = atoi(words);
				}
				else{
					printf("Do you forget the positive number?\n");
				}

			}
			break;

		case ASMCODE:

			op = getopcode(words);
			if (op == BAD_CODE)
			{
				printf("Pass2: %s - Bad mnemonic at %d\n", words, program_count);

			}


			out_program[program_count] = op;
			++program_count;
			break;


		case REFLABEL:

			labelidx = seeklabel(words);
			if (labelidx == -1)
			{
				strcat(words, ":");
				labelidx = seeklabel(words);
				if (labelidx == -1) {
					printf
						("Pass2: ERROR! No such label to be referenced :%s at %d\n", words, program_count);
					system("pause");
					exit(3);
				}

			}

			fseek(src, -1, SEEK_CUR);

			char ch;
			
			isoper = 0;
			do{
				ch = getc(src);

				if (ch == '=')
				{
					fseek(src, -1, SEEK_CUR);
					isoper = 1;
					break;
				}
				else if (ch == 0x0a)
				{
					fseek(src, -1, SEEK_CUR);
					break;
				}
				else if (ch == ';')
				{
					fseek(src, -1, SEEK_CUR);
					break;
				}
			} while (ch <= ' '&&!feof(src));

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
				break;
			}

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

		case SDATA:
			if (strcmp(words, "*") == 0)
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
				}

			}
			else
			{
				char name[MAXTOKEN];
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
				}


				fseek(src, -1, SEEK_CUR);

				char ch;
				isoper = 0;
				do{
					ch = getc(src);

					if (ch == '=')
					{
						fseek(src, -1, SEEK_CUR);
						isoper = 1;
						break;
					}
					else if (ch == 0x0a)
					{
						fseek(src, -1, SEEK_CUR);
						break;
					}
					else if (ch == ';')
					{
						fseek(src, -1, SEEK_CUR);
						break;
					}
				} while (ch <= ' '&&!feof(src));

				if (isoper)
				{
					labelidx = seeklabel(name);
					if (labelidx == -1)
					{
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
				else
				{
					++data_count;
				}

			}
			break;

		case INCLUDE:
			savedFile = src;
			src = NULL;
			assemble_round2(words);
			src = savedFile;
			break;
		case PNUM:
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

		case NNUM:
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

void SFAS::assemble(char *sourcename)
{
	
	strcpy(srcfile, sourcename);
	printf("Round 1...\n");
	assemble_round1(sourcename);
	printf("Round 2...\n");
	//memory for data and program
	out_data = new unsigned int[data_count];
	out_program = new unsigned char[program_count];
	data_count = 0;
	program_count = 0;
	assemble_round2(sourcename);

	outToFile(srcfile);

	delete[] out_data;
	delete[] out_program;
}

void SFAS::outToFile(char *filename){
	std::cout<<"Output to file: " << filename << std::endl;
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


	bin = fopen(binfile, "wb");
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

tokentype SFAS::lex(char *words, CODE & opcode){
	
	char cmpwords[MAXTOKEN];
	char cmds[MAXTOKEN];
	char ch;
	unsigned int cds;
	tokentype tks;
	int i = 0;
	int temp_i;

	do {
		ch = getc(src);
	} while (ch <= ' ' && !feof(src));

	/*
	**	address or instruction:
	*/
	if (isalpha(ch)) {
		do {
			cmds[i] = ch;
			++i;
			ch = getc(src);
		} while (ch > ' ' && (isalpha(ch) || isdigit(ch)) && !feof(src));

		if (ch == ':') {
			cmds[i] = ch;
			++i;
		}

		cmds[i] = 0;
		strcpy(words, cmds);
		temp_i = 0;
		while (cmds[temp_i] != '\0'){
			cmpwords[temp_i] = toupper(cmds[temp_i]);
			temp_i++;
 		}
		cmpwords[temp_i] = '\0';
		cds = lookupcmd(cmds);
		opcode = (CODE)cds;
		if (cds == MAXINSTUCTION && words[strlen(words) - 1] == ':'){
			tks = LABEL;
			words[strlen(words) - 1] = '\0';
		}
		else if (cds == MAXINSTUCTION && words[strlen(words) - 1] != ':')
		{
			if (cds == MAXINSTUCTION && strcmp("LOCAL", cmpwords) == 0)
			{
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

					strcpy(words, ch_temp);

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
						if (strcmp("AS", ch_as) == 0)
						{
							return LOCALVAL;
						}
					}
				}
			}
			else if (cds == MAXINSTUCTION && strcmp("DATA", cmpwords) == 0)
			{

				char ch_temp[MAXTOKEN];
				int temp_i = 0;
				do {
					ch = getc(src);
				} while (ch <= ' ' && !feof(src));

				if (ch == '*')
				{
					ch_temp[temp_i] = ch;
					++temp_i;
					ch_temp[temp_i] = 0;
				}

				else if (isalpha(ch)) {
					do {
						ch_temp[temp_i] = ch;
						++temp_i;
						ch = getc(src);
					} while (ch > ' ' && (isalpha(ch) || isdigit(ch) || ch == '[' || ch == ']') && !feof(src));
					ch_temp[temp_i] = 0;
				}
				strcpy(words, ch_temp);
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
			else
				tks = REFLABEL;
		}

		else
			tks = ASMCODE;
	}
	/*
	**	positive number
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

			tks = PNUM;
			cmds[i] = 0;
			strcpy(words, cmds);
		}
		else{
			cmds[i] = ch;
			++i;
			cmds[i] = 0;
			strcpy(words, cmds);
			tks = UNKNOWN;
		}

	}
	/*
	**	negtive number
	*/
	else if (ch == '-') {
		ch = getc(src);
		if (isdigit(ch)){
			do {
				cmds[i] = ch;
				++i;
				ch = getc(src);
			} while (ch > ' ' && isdigit(ch) && !feof(src));

			tks = NNUM;
			cmds[i] = 0;
			strcpy(words, cmds);
		}
		else{
			cmds[i] = ch;
			++i;
			cmds[i] = 0;
			strcpy(words, cmds);
			tks = UNKNOWN;
		}

	}

	/*
	**	single line comment
	*/
	else if (ch == ';') {
		do {
			ch = getc(src);
		} while (ch != 0x0a && !feof(src));

		strcpy(words, "CMNT");
		opcode = (CODE)BAD_CODE;
		tks = CMNT;
	}
	/*
	**	'='
	*/
	else if (ch == '=')
	{
		int data_len;
		read_data(data_len, words);
		opcode = (CODE)data_len;
		tks = OPER;

	}
	/*
	**	unrecognized symbol
	*/
	else {
		cmds[i] = ch;
		++i;
		cmds[i] = 0;
		strcpy(words, cmds);
		tks = UNKNOWN;
	}
	return tks;
}

unsigned int SFAS::lookupcmd(char *cmd)
{
	char buf[64];
	strcpy(buf, cmd);
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


int SFAS::seeklabel(char *lbl)
{
	int i = 0;
	for (i = 0; i < SYMTABLEN && strcmp(symtabhdr[i].symname, lbl); ++i);
	if (i < SYMTABLEN) {
		return i;
	}
	else {
		return -1;
	}
}

void SFAS::read_data(int &data_len, char* words)
{
	
	char ch;
	char ch_temp[MAXDATALEN];
	int temp_i = 0;
	do {
		ch = getc(src);
	} while (ch <= ' ' && !feof(src));

	if (isdigit(ch) || ch == '\'') {


		do {
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
				fseek(src, -1, SEEK_CUR);
			}

			else if (ch == ',')
			{

			}

			do{
				ch = getc(src);
				if (ch == 0x0a)
					break;
				if (ch == ';')
				{
					fseek(src, -1, SEEK_CUR);
					break;
				}
			} while (ch <= ' '&&!feof(src));

		} while ((isdigit(ch) || ch == '\'' || ch == ',' || ch == '\\') && ch != 0x0a && !feof(src));
	}
	for (int i = 0; i < temp_i; i++)
		words[i] = ch_temp[i];
	data_len = temp_i;
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