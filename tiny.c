#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef PLATFORM_SMS
#include "SMSlib.h" 
#include "libs/console.h"
#include "libs/strings.h"
#else
#include <curses.h>
#include "libs/SMScompat.h"
#endif

//contain ascii table and basic source file
#include "assets/assets.h"


#define MAX_LABEL (50)
#define LABEL_LEN (6 + 1)
#define FOR_NEST (10)
#define SUB_NEST (10)
#define MAX_STR_LEN (40 + 1)
#define MAX_CMD_LEN (6 + 1)
#define MAX_VARIABLES (20)
#define MAX_VAR_NAME (8 + 1)


#define NONE	(00)
#define DELIMITER  (30)
#define VARIABLE  (40)
#define NUMBER    (50)
//#define COMMAND   (4)
#define STRING	  (60)
#define QUOTE	  (70)

#define PRINT (1)
#define INPUT (2)
#define IF    (3)
#define THEN  (4)
#define FOR   (5)
#define NEXT  (6)
#define TO    (7)
#define GOTO  (8)
#define EOL   (9)
#define GOSUB (10)
#define RETURN (11)
#define AND (12)
#define OR (13)
#define NOT (14)
#define END (15)
#define REM (16)



/* builtins */
#define TAB (0)
#define RND (1)
#define INT (2)
#define CHR (3)


char *g_source;  /* holds expression to be analyzed */
const char *g_basic_end;   /* position in basic source*/

struct variables 
{ /* keyword lookup table */
	char name[MAX_VAR_NAME];
	float number;
} g_variables[MAX_VARIABLES];


const char *g_builtins[] = { 
	"TAB", 
	"RND", 
	"INT", 
	"CHR$",
	"REM",
	""	
}; 

const struct commands 
{ /* keyword lookup table */
	const char command[MAX_CMD_LEN];
	const char tok;
} g_table[] = 
{ /* Commands must be entered lowercase */
	{"PRINT", PRINT}, /* in this table. */
	{"INPUT", INPUT},
	{"IF", IF},
	{"THEN", THEN},
	{"GOTO", GOTO},
	{"FOR", FOR},
	{"NEXT", NEXT},
	{"TO", TO},
	{"GOSUB", GOSUB},
	{"RETURN", RETURN},
	{"END", END},
	{"AND", AND},
	{"OR", OR},
	{"NOT", NOT},
	{"", END}  /* mark end of table */
};

char g_token[MAX_STR_LEN];
char g_token_type;
char g_tok;

struct label {
	char name[LABEL_LEN];
	char *position;  /* points to place to go in source file*/
};
struct label g_label_table[MAX_LABEL];

typedef struct for_stack {
	int var; /* index number in variable table of counter variable */
	float target;  /* target value */
	char* loc;
} stack;
stack g_fstack[FOR_NEST]; /* stack for FOR/NEXT loop */

char *g_gstack[SUB_NEST];	/* stack for gosub */


int g_ftos;  /* index to top of FOR stack */
int g_gtos;  /* index to top of GOSUB stack */

float g_randnext = 0.6789; /* random seed */



char level2(float *number, char *string);
char level3(float *number, char *string); 
char level4(float *number, char *string);
char level5(float *number, char *string); 
char level6(float *number, char *string); 
int get_token(void);

void endless(void)
{
#if PLATFORM_SMS
	while (1) ;
#else
	exit(0);
#endif
}

/* Return a token to input stream. */
void putback(void)
{
	char* t;

	t = g_token;
	for (; *t; ++t)
		--g_source;
}

/* print out embedded basic code in token format */
void print_source(char *stop_pos)
{
	/* if stop_pos == null then print whole source code */
	if (!stop_pos)
		stop_pos = g_source + basic_bas_size - 1;
	
	g_source = (char*) basic_bas;
	while (g_source <= stop_pos)
	{
		puts(g_token);
		puts(" ");
		get_token();
	}
}

/* display an error message */
void errortext(int error)
{
	const char* e[] = {
		"syntax error",
		"unbalanced parentheses",
		"no expression present",
		"equals sign expected",
		"not a variable",
		"Label table full", /*5*/
		"duplicate label",
		"undefined label",
		"THEN expected",
		"TO expected",
		"too many nested FOR loops", /*10*/
		"NEXT without FOR",
		"too many nested GOSUBs",
		"RETURN without GOSUB",
		"max number of variables reached",
		"variable not found"
	};

	puts("\nsource:\n");
	print_source(g_source);
	//print_source(g_source);

	puts("\n\nerror:\n");
	puts(e[error]);
	/* loop forever */
	endless();
}


/* Reset variables table */
void init_variables_table(void)
{
	int idx;
	for (idx = 0; idx < MAX_VARIABLES; ++idx)
	{
		g_variables[idx].name[0] = '\0';
		g_variables[idx].number = 0;
	}	

}

/* Return index for variable name */
signed char find_variable(const char *name)
{
	signed char found = -1;
	char idx;
	for (idx = 0; idx < MAX_VARIABLES; ++idx)
		if (strcmp(g_variables[idx].name, name) == 0) 
		{
			found = idx;
			break;
		}
	return found;
}


/* Add name and number to variables table */
signed char add_variable(const char *name, const float number)
{
	signed char found = -1;
	char idx;
	for (idx = 0; idx < MAX_VARIABLES; ++idx)
	{
		if (strcmp(g_variables[idx].name, "") == 0)
		{ 
			found = idx;
			strcpy(g_variables[idx].name, name);
			g_variables[idx].number = number;  
			break;
		}
	}
	if (found == -1)
		errortext(14);
	return found;
}


/* Return index of internal builtin function otherwise return -1. */
signed char find_builtins(void)
{
	signed char found = -1;
	char idx;
	for (idx = 0; g_builtins[idx]; ++idx)
		if (strcmp(g_builtins[idx], g_token) == 0) 
		{
			found = idx; 
			break;
		}
	return found;
}

/* Entry point into parser. */
signed char do_expression(float *number, char *string)
{
	signed char ret = NONE;
	if (!*g_token) 
	{
		errortext(2);
		return ret;
	}
  	ret = level2(number, string);
	return ret;
	
}


/* Initialize the array that holds the labels.
   By convention, a null label name indicates that
   array position is unused.
*/
void label_init(void)
{
	int i;

	for (i = 0; i < MAX_LABEL; ++i) 
		g_label_table[i].name[0] = '\0';
}


/* GOSUB stack push function. */
void gpush(char *s)
{
	++g_gtos;

	if (g_gtos == SUB_NEST) 
	{
		errortext(12);
		return;
	}

	g_gstack[g_gtos] = s;

}


/* GOSUB stack pop function. */
char *gpop(void)
{
	if (g_gtos == 0) 
	{
		errortext(13);
		return 0;
	}

	return (g_gstack[g_gtos--]);
}


/* Return index of next free position in label array.
   A -1 is returned if the array is full.
   A -2 is returned when duplicate label is found.
*/
int get_next_label(char *label_name)
{
	int t;

	for (t = 0; t < MAX_LABEL; ++t) 
	{
		if (g_label_table[t].name[0] == 0) 
			return t;
		if (!strcmp(g_label_table[t].name, label_name)) 
			return -2; /* doublette */
	}
	/* too many labels*/
	return -1;
}

void print_typed(char ret, int number, char* string)
{
	long int_number;

	if (ret == NUMBER)
	{
		int_number = (long)number;
		if (number - int_number < 0.000001)
			itoa(int_number, string, 10);
		else
			ftoa(number, 2, string);
	}
	puts(string);
}


/* Execute a simple version of the BASIC PRINT statement */
void do_print(void)
{
	float number=0.0f;
	char ret='?';
	char string[MAX_STR_LEN];
	char last_delim = ' ';
	
	string[0] = '\0';
	get_token();
	do 
	{
		if ((g_tok == EOL) || (g_source == g_basic_end)) 
			break;
		else if (g_token_type == QUOTE) 
		{ /* is literal */
			ret = do_expression(&number, string);
			print_typed(ret, number, string);
			last_delim = '\"';
			continue;
		}
		else if (*g_token == ';')
		{	/* another print argument*/
			puts(" ");
			last_delim = *g_token;
		}
		else if (*g_token == ':')
		{
			last_delim = *g_token;
			break;
		}
		else 
		{ 
			ret = do_expression(&number, string);
			print_typed(ret, number, string);
			last_delim = 'E';
			continue;
		}
		get_token();
	} while (1);

	if (last_delim != ';')
		puts("\n");
}


/* Find the start of the next line. */
void find_eol(char command_separator)
{
	while (	*g_source != '\n' && 
			(g_source + 1) != g_basic_end && 
			*g_source != command_separator
			)
			{  
		++g_source;
			}
	if (*g_source) 
		++g_source;
}


/* Find location of given label.  A -1 is returned if
   label is not found; otherwise the index of element is returned.
*/
int find_label(const char *label_name)
{
	int idx;

	for (idx = 0; idx < MAX_LABEL; ++idx)
		if (!strcmp(g_label_table[idx].name, label_name)) 
			return idx;
	return -1; /* error condition */
}


/* Find all referenced labels. */
void scan_labels(void)
{
	int addr;
	char *temp;
	char last_token_type;
	int idx;

	label_init();  /* zero all labels */
	temp = g_source;   /* save pointer to top of program */

	/* get all used labels */
	idx = 0;
	do 
	{
		get_token();
		if (g_token_type == GOSUB || g_token_type == GOTO || g_token_type == THEN) 
		{
			last_token_type = g_token_type;
			get_token();

			// when after THEN not a line nummber follows
			if (last_token_type == THEN && g_token_type != NUMBER)
				continue;
			addr = get_next_label(g_token);
			if (addr == -1 || addr == -2)
			{
				if (addr == -1) 
					errortext(5);
				else // line number already marked
					continue;
			}
		    
		    strcpy(g_label_table[addr].name, g_token);
		    g_label_table[addr].position = 0; /* dummy value */
		    idx += 1;

		    //printf("%d", idx);
		}
	} while (g_source < g_basic_end);
	g_source = temp; 

	/* get positions of the used labels */
	do 
	{

		get_token();
		if (g_token_type == NUMBER) 
		{
			idx = find_label(g_token);
		    if (idx >= 0)
				g_label_table[idx].position = g_source; 
		}
		find_eol('\0');
	} while (g_source < g_basic_end);

	g_source = temp;  /* restore to original */
}


/* Execute a GOTO statement. */
void do_goto(void)
{
	int idx;

	get_token(); /* get label to go to */
	/* find the location of the label */
	idx = find_label(g_token);
	if (idx < 0)
		errortext(7); /* label not defined */
	else 
		g_source = g_label_table[idx].position;  /* start program running at that loc */
}


char condition(char not)
{
	char cond = 0;
	float x , y;
	char op[3];

	if (g_tok == NOT)
		condition(!not);
	do_expression(&x, 0); /* get left expression */

	if (!strchr("=<>", *g_token)) 
	{
		errortext(0); /* not a legal operator */
		return 0;
	}
	strcpy(op, g_token);
	get_token();
	do_expression(&y, 0); /* get right expression */

	/* determine the outcome */
	if (strcmp(op, "<") == 0)
		cond = x < y;
	else if (strcmp(op, ">") == 0)		
		cond = x > y;
	else if (strcmp(op, "=") == 0)		
		cond = x == y;			
	else if (strcmp(op, ">=") == 0)		
		cond = x >= y;
	else if (strcmp(op, "<=") == 0)		
		cond = x <= y;
	else if (strcmp(op, "<>") == 0 || strcmp(op, "><") == 0)		
		cond = x != y;

	return (not) ? !cond : cond;	
}


/* Execute an IF statement. */
void do_if(void)
{
	char cond = 1;
	int idx;
	float number;
	char string[MAX_STR_LEN];
	
	get_token();
	cond = condition(0);
	while (1)
	{
		if (g_tok == AND)
			cond = cond && condition(0);
		else if (g_tok == OR)
			cond = cond || condition(0);
		else 
			break;
	} 
	/* skip token, not all BASIC use this */
	if (g_tok == THEN)
		get_token();

	if (cond) 
	{ /* is true so process target of IF */

		if ((g_token_type == NUMBER) || (strcmp(g_token, "GOTO") == 0))
		{
			if (g_token_type != NUMBER)
				get_token();

			idx = find_label(g_token);
			if (idx < 0)
				errortext(7); /* label not defined */
			else 
				g_source = g_label_table[idx].position;  /* start program running at that loc */
		}
		else //TODO: needs additional testing
			do_expression(&number, string); 
	}
	else /* no further evaluation required */
		//TODO: BUT no colon evaluation
		find_eol(':'); /* find start of next line */
}


/* Push function for the FOR stack. */
void fpush(struct for_stack* i)
{
	if (g_ftos > FOR_NEST)
		errortext(10);

	if (g_ftos >= 0)
		g_fstack[g_ftos++] = *i;
}


struct for_stack* fpop()
{
	--g_ftos;
	if (g_ftos < 0) 
		errortext(11);
	return &g_fstack[g_ftos];
}


/* Execute a FOR loop. */
void do_for(void)
{
	struct for_stack i;
	float number;
	char string[MAX_STR_LEN];
	int idx;

	get_token(); 
	/* only int variable allowed */
	if (!isalpha(*g_token)) 
	{
		errortext(4);
		return;
	}

	/* lookup only capital case */
	to_upper(g_token);
	/* save its index */
	idx = find_variable(g_token);
	/* create new variable if unregisted*/
	if (idx == -1)
		idx = add_variable(g_token, 0);
	i.var = idx;

	get_token(); /* read the equals sign */
	if (*g_token != '=') 
	{
		errortext(3);
		return;
	}

	get_token();
	/* get start value */
	do_expression(&number, string); 

	g_variables[i.var].number = number;

	get_token();
	if (g_tok != TO) 
		errortext(9); /* read and discard the TO */

	get_token();
	do_expression(&i.target, string); 

	/* if loop can execute at least once, push info on stack */
	if (number >= g_variables[i.var].number) 
	{
		i.loc = g_source;
		fpush(&i);
	}
	else  /* otherwise, skip loop code altogether */
		while (g_tok != NEXT) 
			get_token();
}

/* Execute a NEXT statement. */
void do_next(void)
{
	struct for_stack i;

	i = *fpop(); /* read the loop info */

	++g_variables[i.var].number; /* increment control variable */
	if (g_variables[i.var].number > i.target) 
		return;  /* all done */
		
	fpush(&i);  /* otherwise, restore the info */
	g_source = i.loc;  /* loop */
}


/* Execute a simple form of the BASIC INPUT command */
void do_input(void)
{
	signed char var = -1;
	float number;
	char str[MAX_STR_LEN];

	str[0] = '\0';
	get_token(); /* see if prompt string is present */
	if (g_token_type == QUOTE) 
	{
		/* todo: allow only literal, no variable*/

		/* if so, print it and check for comma */
		get_token();
		if (*g_token != ',') 
			errortext(1);
		get_token();
	}
	else 
		puts("? "); /* otherwise, prompt with ? */
  
  	to_upper(g_token);

	if (*g_token != '$')
		var = find_variable(g_token); /* get the input var */

	if (var == -1)
		add_variable(g_token, 0);	

	/* read input */
	gets(str);
	number = atof(str);

	g_variables[var].number = number; /* store it */
}

/* Execute a GOSUB command. */
void do_gosub(void)
{
	int idx;
	
	get_token();
	/* find the label to call */
	idx = find_label(g_token);
	if (idx < 0)
		errortext(7); /* label not defined */
	else 
	{
		gpush(g_source); /* save place to return to */
		g_source = g_label_table[idx].position;  /* start program running at that loc */
	}
}


/* Return from GOSUB. */
void do_return(void)
{
	g_source = gpop();
}


/* Look up a a token's internal representation in the
   token table.
*/
int look_up(char *s)
{
	int i;
	char *p;

	/* convert to lowercase */
	p = s;
	to_upper(p);

	/* see if token is in table */
	for (i = 0; *g_table[i].command; ++i)
	{
		if (strcmp(g_table[i].command, s) == 0) 
			return g_table[i].tok;
	}
	
	/*see if token is in builtin table */
	for (i = 0; *g_builtins[i]; ++i)
	{
		if (strcmp(g_builtins[i], s) == 0) 
			return END + 1 + i;
	} 
	return 0; /* unknown command */
}


/* Return 1 if c is space or tab. */
int is_whitespace(char c)
{
	return (c == ' ' || c == '\t') ;
}


/* Reverse the sign. */
void unary(char o, float *r)
{
	if (o == '-') 
		*r = -(*r);
}


/* Perform the specified arithmetic. */
void do_term(char o, float *r, float *h)
{
	float t, ex;

	switch(o) 
	{
		case '-':
			*r = (*r) - (*h);
			break;
		case '+':
			*r = (*r) + (*h);
			break;
		case '*':
			*r = (*r) * (*h);
			break;
		case '/':
			*r = (*r) / (*h);
			break;
		case '%':
			t = (*r) / (*h);
			*r = (*r) - (t * (*h));
			break;
		case '^':
			ex = *r;
			if (*h == 0) 
			{
				*r = 1;
				break;
			}
			for (t = *h - 1; t > 0; t-=1) 
				*r = (*r) * ex;
			break;
	}
}


/* Process integer exponent. */
char level4(float *number, char *string)
{
	float hold;
	char ret;

	ret = level5(number, string);
	if (*g_token ==  '^') 
	{
		get_token();
		ret = level4(&hold, string);
		do_term('^', number, &hold);
	}
	return ret;
}


/* Multiply or divide two factors. */
char level3(float *number, char *string)
{
	char  op;
	float hold;
	char ret;
	
	ret = level4(number, string);
	while ((op = *g_token) == '*' || op == '/' || op == '%') 
	{
		get_token();
		ret = level4(&hold, string);
		do_term(op, number, &hold);
	}
	return ret;
}


/*  Add or subtract two terms. */
char level2(float *number, char *string)
{
	char  op;
	float hold;
	char ret;

	ret = level3(number, string);
	while ((op = *g_token) == '+' || op == '-') 
	{
		get_token();
		ret = level3(&hold, string);
		do_term(op, number, &hold);
	}
	return ret;
}


/* Execute TAB function*/
void builtin_tab(char *string)
{
	char i;	
	float number;

	get_token();
	do_expression(&number, string);
	for (i = 0; i < (int)number; ++i)
		string[i] = ' ';
	string[i] = '\0';
}

/* Execute INT function*/
void builtin_int(float *number)
{
	//puts("int evaluation");
	get_token();
	do_expression(number, 0);
	*number = (float)(int)*number;
}

/* Execute CHR function*/
void builtin_chr(char *string)
{
	float number;

	get_token();
	do_expression(&number, 0);
	string[0] = (char)number;
	string[1] = '\0';
}

/* Execute RND function*/
void builtin_rnd(float *value)
{
	float factor = 1;
	get_token();
	do_expression(&factor, 0);
	if (factor == 1)
		*value = ((g_randnext = g_randnext * 0.1234f) - (int) g_randnext);
	else 
	{
		factor = atof(g_token);
		*value = (float)(int)(((g_randnext = g_randnext * 0.1234f) - (int)g_randnext) * factor);
	}
}


/* Find value of number or variable. */
char primitive(float *number, char *string)
{
	signed char func_idx;
	int idx;
	
	switch(g_token_type) 
	{
		case VARIABLE:
			to_upper(g_token);
			idx = find_variable(g_token);
			if (idx == -1)
				idx = add_variable(g_token, 0);

			*number = g_variables[idx].number;
			get_token();
			return NUMBER;
		case NUMBER:
			*number = (float) atof(g_token);
			get_token();
			return NUMBER;
		case QUOTE:
			strcpy(string, g_token);
			get_token();
			return STRING;
		default:
			func_idx = find_builtins(); 
			if (func_idx < 0)
				errortext(0);
			else
			{
				switch(func_idx)
				{
					case TAB:
						builtin_tab(string);
						return STRING;
					case RND:
						builtin_rnd(number);
						return NUMBER;
					case INT:
						builtin_int(number);
						return NUMBER;
					case CHR:
						builtin_chr(string);
						return STRING;
				} 
			}
			return NONE;
	}
}


/* Process parenthesized expression. */
char level6(float *number, char *string)
{
	char ret;
	
	if ((*g_token == '(') && (g_token_type == DELIMITER)) 
	{
		get_token();
		ret = level2(number, string);
		if (*g_token != ')') 
			errortext(1);
		get_token();
	}
	else
		ret = primitive(number, string);
	
	return ret;
}


/* Is a unary + or -. */
char level5(float *number, char *string)
{
	char  op;
	char ret;

	op = 0;
	if ((g_token_type == DELIMITER) && *g_token == '+' || *g_token == '-') 
	{
		op = *g_token;
		get_token();
	}
	ret = level6(number, string);
	if (op)
		unary(op, number);
	return ret;
}


/* Return true if c is a delimiter. */
int is_delimiter(char c)
{
	return strchr(" ;,:+-<>/*%^=()", c) || c == 9 || c == '\r' || c == 0;
}


/* Get a token. */
int get_token(void)
{
	char *temp;

	g_token_type = 0; 
	g_tok = 0;
	temp = g_token;

	while (is_whitespace(*g_source)) 
		++g_source;  /* skip over white space */

	if (*g_source == '\r') 
	{ /* crlf */
		++g_source; 
		++g_source;
		g_tok = EOL; 
		g_token[0] = '\r'; 
		g_token[1] = '\n'; 
		g_token[2] = '\0';
		return (g_token_type = DELIMITER);
	}

	if (strchr("+-*^/%=;(),><:", *g_source))
	{ /* delimiter */
		*temp = *g_source;
		++g_source; /* advance to next position */
		++temp;
		if (strchr(">=<", *g_source))
		{
			++g_source; /* advance to next position */
			++temp;
		}
		*temp = '\0';
		return (g_token_type = DELIMITER);
	}

	if (*g_source == '"') 
	{ /* quoted string */
		++g_source;
		while (*g_source != '"' && *g_source != '\r') 
			*temp++ = *g_source++;
		if (*g_source == '\r') 
			errortext(1); 
		++g_source;
		*temp = '\0';
		return (g_token_type = QUOTE);
	}

	if (isdigit(*g_source) || *g_source == '.') 
	{ /* number */
		if (*g_source == '.')
		{
			*temp = '0';
			temp++;
		}
		while (!is_delimiter(*g_source)) 
			*temp++ = *g_source++;
		*temp = '\0';
		return (g_token_type = NUMBER);
	}

	if (isalpha(*g_source)) 
	{ /* var or command */
		while (!is_delimiter(*g_source)) 
			*temp++ = *g_source++;
		g_token_type = STRING;
	}

	*temp = '\0';

	/* see if a string is a command or a variable */
	if (g_token_type == STRING) 
	{
		g_tok = look_up(g_token); /* convert to internal rep */
		if (!g_tok)
			g_token_type = VARIABLE;
		else
			if (strcmp(g_token, "REM") == 0)
				g_token_type = g_tok = REM;
			else
				g_token_type = g_tok; /* is a command */
	}
	return g_token_type;
}


/* Assign a variable a value. */
void do_let(void)
{
	float number;
	char ret;
	char string[MAX_STR_LEN];
	char varname[MAX_VAR_NAME];
	int idx;

	/* get the variable name */
	if (!isalpha(*g_token) && *g_token != '$') 
		errortext(4);

	to_upper(g_token);
	strcpy(varname, g_token);

	/* get the equals sign */
	get_token();
	if (*g_token != '=') 
		errortext(3);
	
	get_token();
	/* get the value to assign to var */
	ret = do_expression(&number, string);

	/* assign the value */
	if (ret == NUMBER) 
	{
		/* create new variable record if unknown */
		idx = find_variable(varname);
		if (idx == -1) 
			add_variable(varname, number);
		else
			g_variables[idx].number = number;
	}
}


void load_ascii_tiles()
{
#ifdef PLATFORM_SMS
	SMS_loadTiles(font__tiles__bin, 0, font__tiles__bin_size);
  	SMS_loadBGPalette(font_palette_bin);
#endif
}

void main(void)
{

#ifdef PLATFORM_SMS
	SMS_displayOn();
#else
	WINDOW* stdsrc =initscr();
	raw();
	keypad(stdsrc, TRUE);
	noecho();
#endif

	load_ascii_tiles();

	clrscr();
	puts("BASIC 0.1 by DarkTrym\n");
#ifndef PLATFORM_SMS	
	refresh();
	int ch=getch();
#endif

	/* set pointer to basic code and a pointer to the end */
	g_source = (char *) basic_bas;
	g_basic_end = g_source + basic_bas_size - 1;
	
	/* create jump table for the labels in the program */
	scan_labels(); 
	
	/* initialize the FOR stack index */
	g_ftos = 0; 
	/* initialize the GOSUB stack index */
	g_gtos = 0; 
	for (;;)
  	{
		do
		{
			g_token_type = get_token();
			//puts("TOKEN: %s TOKEN_TYPE: %d\n", g_token, g_token_type);
#ifndef PLATFORM_SMS	
			refresh();
#endif
			/* check for variable assignment */
			if (g_token_type == VARIABLE) 
			{
				/* must be assignment statement */
				do_let();
			}
			else /* is command */
				switch(g_tok) 
				{
					case PRINT:
						do_print();
						break;
					case GOTO:
						do_goto();
						break;
					case IF:
						do_if();
						break;
					case FOR:
						do_for();
						break;
					case NEXT:
						do_next();
						break;
					case INPUT:
						do_input();
						break;
					case GOSUB:
						do_gosub();
						break;
					case RETURN:
						do_return();
						break;
					case REM:
						do 
						{
							get_token(); 
						} while (g_tok != EOL);
						get_token();
						break;
					case END:
						endless();
					default:;
				}
		} while (g_source < g_basic_end);
#ifdef PLATFORM_SMS
	//	SMS_waitForVBlank();
#else
		refresh();
		getch();
		return;
#endif
  	}
}
