/*
 * Z80SIM  -  a	Z80-CPU	simulator
 *
 * Copyright (C) 1987-2007 by Udo Munk
 *
 * This modul contains the user interface, a full qualified ICE,
 * for the Z80-CPU simulation.
 *
 * History:
 * 28-SEP-87 Development on TARGON/35 with AT&T Unix System V.3
 * 11-JAN-89 Release 1.1
 * 08-FEB-89 Release 1.2
 * 13-MAR-89 Release 1.3
 * 09-FEB-90 Release 1.4  Ported to TARGON/31 M10/30
 * 20-DEC-90 Release 1.5  Ported to COHERENT 3.0
 * 10-JUN-92 Release 1.6  long casting problem solved with COHERENT 3.2
 *			  and some optimization
 * 25-JUN-92 Release 1.7  comments in english and ported to COHERENT 4.0
 * 02-OCT-06 Release 1.8  modified to compile on modern POSIX OS's
 * 18-NOV-06 Release 1.9  modified to work with CP/M sources
 * 08-DEC-06 Release 1.10 modified MMU for working with CP/NET
 * 17-DEC-06 Release 1.11 TCP/IP sockets for CP/NET
 * 25-DEC-06 Release 1.12 CPU speed option
 * 19-FEB-07 Release 1.13 various improvements
 * 06-OCT-07 Release 1.14 bug fixes and improvements
 */

/*
 *	This modul is an ICE type user interface to debug Z80 programs
 *	on a host system.
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <memory.h>
#include <ctype.h>
#include <signal.h>
#include "sim.h"
#include "simglb.h"
#include "guiglb.h"

#define BUFSIZE 256		/* buffer size for file i/o */

extern void cpu(void);
extern void disass(unsigned char **, int);
extern int exatoi(char *);
extern int getkey(void);
extern void int_on(void), int_off(void);

#ifdef WANT_GUI
void Show_All(void);
void Add_to_Log(char *str);
#endif
void do_step(void);
void do_trace(char *);
void do_go(void);
static int handel_break(void);
static void do_dump(char *);
static void do_list(char *);
static void do_modify(char *);
static void do_fill(char *);
static void do_move(char *);
static void do_port(char *);
static void do_reg(char *);
static void print_head(void);
static void print_reg(void);
int do_break(char *);
static void do_hist(char *);
static void do_count(char *);
static void do_clock(void);
static void timeout(int);
static void do_show(void);
int do_getfile(char *);
static int load_mos(int, char *);
static int load_hex(char *);
static int checksum(char *);
static void do_unix(char *);
static void do_help(void);
static void cpu_err_msg(void);

struct termios old_term;

/*
 *	The function "mon()" is the dialog user interface, called
 *	from the simulation just after program start.
 */
void mon(void)
{
	register int eoj = 1;
	static char cmd[LENCMD];

	tcgetattr(0, &old_term);

	if (x_flag) {
		if (do_getfile(xfn) == 0)
			do_go();
	}
	while (eoj) {
		next:
		printf(">>> ");
		fflush(stdout);
		if (fgets(cmd, LENCMD, stdin) == NULL) {
			putchar('\n');
			goto next;
		}
		switch (*cmd) {
		case '\n':
			do_step();
			break;
		case 't':
			do_trace(cmd + 1);
			break;
		case 'g':
			do_go();
			break;
		case 'd':
			do_dump(cmd + 1);
			break;
		case 'l':
			do_list(cmd + 1);
			break;
		case 'm':
			do_modify(cmd +	1);
			break;
		case 'f':
			do_fill(cmd + 1);
			break;
		case 'v':
			do_move(cmd + 1);
			break;
		case 'x':
			do_reg(cmd + 1);
			break;
		case 'p':
			do_port(cmd + 1);
			break;
		case 'b':
			do_break(cmd + 1);
			break;
		case 'h':
			do_hist(cmd + 1);
			break;
		case 'z':
			do_count(cmd + 1);
			break;
		case 'c':
			do_clock();
			break;
		case 's':
			do_show();
			break;
		case '?':
			do_help();
			break;
		case 'r':
			do_getfile(cmd + 1);
			break;
		case '!':
			do_unix(cmd + 1);
			break;
		case 'q':
			eoj = 0;
			break;
		default:
			puts("what??");
			break;
		}
	}
}

/*
 *	Execute a single step
 */
void do_step(void)
{
	BYTE *p;

	cpu_state = SINGLE_STEP;
	cpu_error = NONE;
	cpu();
	if (cpu_error == OPHALT)
		handel_break();
	cpu_err_msg();
	print_head();
	print_reg();
	p = PC;
	disass(&p, p - ram);
}

/*
 *	Execute several steps with trace output
 */
void do_trace(char *s)
{
	register int count, i;

	while (isspace((int)*s))
		s++;
	if (*s == '\0')
		count =	20;
	else
		count =	atoi(s);
	cpu_state = SINGLE_STEP;
	cpu_error = NONE;
	print_head();
	print_reg();
	for (i = 0; i <	count; i++) {
		cpu();
		print_reg();
		if (cpu_error) {
			if (cpu_error == OPHALT) {
				if (!handel_break()) {
					break;
				}
			} else
				break;
		}
	}
	cpu_err_msg();
}

/*
 *	Run the CPU emulation endless
 */
void do_go(void)
{
	cont:
	cpu_state = CONTIN_RUN;
	cpu_error = NONE;
	cpu();
	if (cpu_error == OPHALT)
		if (handel_break())
			if (!cpu_error)
				goto cont;
	cpu_err_msg();
	print_head();
	print_reg();
}

/*
 *	Handling of software breakpoints (HALT opcode):
 *
 *	Output:	0 breakpoint or other HALT opcode reached (stop)
 *		1 breakpoint reached, passcounter not reached (continue)
 */
static int handel_break(void)
{
#ifdef SBSIZE
	register int i;

	for (i = 0; i <	SBSIZE;	i++)	/* search for breakpoint */
		if (soft[i].sb_adr == PC - ram - 1)
			goto was_softbreak;
	return(0);
	was_softbreak:
#ifdef HISIZE
	h_next--;			/* correct history */
	if (h_next < 0)
		h_next = 0;
#endif
	cpu_error = NONE;		/* HALT	was a breakpoint */
	PC--;				/* substitute HALT opcode by */
	*PC = soft[i].sb_oldopc;	/* original opcode */
	cpu_state = SINGLE_STEP;	/* and execute it */
	cpu();
	*(ram +	soft[i].sb_adr)	= 0x76;	/* restore HALT	opcode again */
	soft[i].sb_passcount++;		/* increment passcounter */
	if (soft[i].sb_passcount != soft[i].sb_pass)
		return(1);		/* pass	not reached, continue */
	sprintf(lstr, "Software breakpoint %d reached at %04X\n", i, soft[i].sb_adr);
	Add_to_Log(lstr);
	soft[i].sb_passcount = 0;	/* reset passcounter */
	return(0);			/* pass	reached, stop */
#else
	return(0);
#endif
}

/*
 *	Memory dump
 */
static void do_dump(char *s)
{
	register int i,	j;
	BYTE c;

	while (isspace((int)*s))
		s++;
	if (isxdigit((int)*s))
		wrk_ram	= ram +	exatoi(s) - exatoi(s) %	16;
	printf("Adr    ");
	for (i = 0; i <	16; i++)
		printf("%02x ",	i);
	puts(" ASCII");
	for (i = 0; i <	16; i++) {
		printf("%04x - ", (WORD)(wrk_ram - ram));
		for (j = 0; j <	16; j++) {
			printf("%02x ",	*wrk_ram);
			wrk_ram++;
			if (wrk_ram > ram + 65535)
				wrk_ram	= ram;
		}
		putchar('\t');
		for (j = -16; j	< 0; j++)
			printf("%c", ((c = *(wrk_ram  + j)) >= ' ' && c <= 0x7f)
			       ?  c : '.');
		putchar('\n');
	}
}

/*
 *	Disassemble
 */
static void do_list(char *s)
{
	register int i;

	while (isspace((int)*s))
		s++;
	if (isxdigit((int)*s))
		wrk_ram	= ram +	exatoi(s);
	for (i = 0; i <	10; i++) {
		printf("%04x - ", (WORD)(wrk_ram - ram));
		disass(&wrk_ram, wrk_ram - ram);
		if (wrk_ram > ram + 65535)
			wrk_ram	= ram;
	}
}

/*
 *	Memory modify
 */
static void do_modify(char *s)
{
	static char nv[LENCMD];

	while (isspace((int)*s))
		s++;
	if (isxdigit((int)*s))
		wrk_ram	= ram +	exatoi(s);
	for (;;) {
		printf("%04x = %02x : ", (WORD)(wrk_ram - ram),
		       *wrk_ram);
		fgets(nv, sizeof(nv), stdin);
		if (nv[0] == '\n') {
			wrk_ram++;
			if (wrk_ram > ram + 65535)
				wrk_ram	= ram;
			continue;
		}
		if (!isxdigit((int)nv[0]))
			break;
		*wrk_ram++ = exatoi(nv);
		if (wrk_ram > ram + 65535)
			wrk_ram	= ram;
	}
}

/*
 *	Memory fill
 */
static void do_fill(char *s)
{
	register BYTE *p;
	register int i;
	register BYTE val;

	while (isspace((int)*s))
		s++;
	p = ram	+ exatoi(s);
	while (*s != ',' && *s != '\0')
		s++;
	if (*s) {
		i = exatoi(++s);
	} else {
		puts("count missing");
		return;
	}
	while (*s != ',' && *s != '\0')
		s++;
	if (*s) {
		val = exatoi(++s);
	} else {
		puts("value missing");
		return;
	}
	while (i--) {
		*p++ = val;
		if (p >	ram + 65535)
			p = ram;
	}
}

/*
 *	Memory move
 */
static void do_move(char *s)
{
	register BYTE *p1, *p2;
	register int count;

	
	while (isspace((int)*s))
		s++;
	p1 = ram + exatoi(s);
	while (*s != ',' && *s != '\0')
		s++;
	if (*s) {
		p2 = ram + exatoi(++s);
	} else {
		puts("to missing");
		return;
	}
	while (*s != ',' && *s != '\0')
		s++;
	if (*s) {
		count =	exatoi(++s);
	} else {
		puts("count missing");
		return;
	}
	while (count--)	{
		*p2++ =	*p1++;
		if (p1 > ram + 65535)
			p1 = ram;
		if (p2 > ram + 65535)
			p2 = ram;
	}
}

/*
 *	Port modify
 */
static void do_port(char *s)
{
	register BYTE port;
	static char nv[LENCMD];
	extern BYTE io_out(), io_in();

	while (isspace((int)*s))
		s++;
	port = exatoi(s);
	printf("%02x = %02x : ", port, io_in(port));
	fgets(nv, sizeof(nv), stdin);
	if (isxdigit((int)*nv))
		io_out(port, (BYTE) exatoi(nv));
}

/*
 *	Register modify
 */
static void do_reg(char *s)
{
	static char nv[LENCMD];

	while (isspace((int)*s))
		s++;
	if (*s == '\0')	{
		print_head();
		print_reg();
	} else {
		if (strncmp(s, "bc'", 3) == 0) {
			printf("BC' = %04x : ",	B_ * 256 + C_);
			fgets(nv, sizeof(nv), stdin);
			B_ = (exatoi(nv) & 0xffff) / 256;
			C_ = (exatoi(nv) & 0xffff) % 256;
		} else if (strncmp(s, "de'", 3)	== 0) {
			printf("DE' = %04x : ",	D_ * 256 + E_);
			fgets(nv, sizeof(nv), stdin);
			D_ = (exatoi(nv) & 0xffff) / 256;
			E_ = (exatoi(nv) & 0xffff) % 256;
		} else if (strncmp(s, "hl'", 3)	== 0) {
			printf("HL' = %04x : ",	H_ * 256 + L_);
			fgets(nv, sizeof(nv), stdin);
			H_ = (exatoi(nv) & 0xffff) / 256;
			L_ = (exatoi(nv) & 0xffff) % 256;
		} else if (strncmp(s, "pc", 2) == 0) {
			printf("PC = %04x : ", (WORD)(PC - ram));
			fgets(nv, sizeof(nv), stdin);
			PC = ram + (exatoi(nv) & 0xffff);
		} else if (strncmp(s, "bc", 2) == 0) {
			printf("BC = %04x : ", B * 256 + C);
			fgets(nv, sizeof(nv), stdin);
			B = (exatoi(nv)	& 0xffff) / 256;
			C = (exatoi(nv)	& 0xffff) % 256;
		} else if (strncmp(s, "de", 2) == 0) {
			printf("DE = %04x : ", D * 256 + E);
			fgets(nv, sizeof(nv), stdin);
			D = (exatoi(nv)	& 0xffff) / 256;
			E = (exatoi(nv)	& 0xffff) % 256;
		} else if (strncmp(s, "hl", 2) == 0) {
			printf("HL = %04x : ", H * 256 + L);
			fgets(nv, sizeof(nv), stdin);
			H = (exatoi(nv)	& 0xffff) / 256;
			L = (exatoi(nv)	& 0xffff) % 256;
		} else if (strncmp(s, "ix", 2) == 0) {
			printf("IX = %04x : ", IX);
			fgets(nv, sizeof(nv), stdin);
			IX = exatoi(nv)	& 0xffff;
		} else if (strncmp(s, "iy", 2) == 0) {
			printf("IY = %04x : ", IY);
			fgets(nv, sizeof(nv), stdin);
			IY = exatoi(nv)	& 0xffff;
		} else if (strncmp(s, "sp", 2) == 0) {
			printf("SP = %04x : ", (WORD)(STACK - ram));
			fgets(nv, sizeof(nv), stdin);
			STACK =	ram + (exatoi(nv) & 0xffff);
		} else if (strncmp(s, "fs", 2) == 0) {
			printf("S-FLAG = %c : ", (F & S_FLAG) ?	'1' : '0');
			fgets(nv, sizeof(nv), stdin);
			F = (exatoi(nv)) ? (F |	S_FLAG)	: (F & ~S_FLAG);
		} else if (strncmp(s, "fz", 2) == 0) {
			printf("Z-FLAG = %c : ", (F & Z_FLAG) ?	'1' : '0');
			fgets(nv, sizeof(nv), stdin);
			F = (exatoi(nv)) ? (F |	Z_FLAG)	: (F & ~Z_FLAG);
		} else if (strncmp(s, "fh", 2) == 0) {
			printf("H-FLAG = %c : ", (F & H_FLAG) ?	'1' : '0');
			fgets(nv, sizeof(nv), stdin);
			F = (exatoi(nv)) ? (F |	H_FLAG)	: (F & ~H_FLAG);
		} else if (strncmp(s, "fp", 2) == 0) {
			printf("P-FLAG = %c : ", (F & P_FLAG) ?	'1' : '0');
			fgets(nv, sizeof(nv), stdin);
			F = (exatoi(nv)) ? (F |	P_FLAG)	: (F & ~P_FLAG);
		} else if (strncmp(s, "fn", 2) == 0) {
			printf("N-FLAG = %c : ", (F & N_FLAG) ?	'1' : '0');
			fgets(nv, sizeof(nv), stdin);
			F = (exatoi(nv)) ? (F |	N_FLAG)	: (F & ~N_FLAG);
		} else if (strncmp(s, "fc", 2) == 0) {
			printf("C-FLAG = %c : ", (F & C_FLAG) ?	'1' : '0');
			fgets(nv, sizeof(nv), stdin);
			F = (exatoi(nv)) ? (F |	C_FLAG)	: (F & ~C_FLAG);
		} else if (strncmp(s, "a'", 2) == 0) {
			printf("A' = %02x : ", A_);
			fgets(nv, sizeof(nv), stdin);
			A_ = exatoi(nv)	& 0xff;
		} else if (strncmp(s, "f'", 2) == 0) {
			printf("F' = %02x : ", F_);
			fgets(nv, sizeof(nv), stdin);
			F_ = exatoi(nv)	& 0xff;
		} else if (strncmp(s, "b'", 2) == 0) {
			printf("B' = %02x : ", B_);
			fgets(nv, sizeof(nv), stdin);
			B_ = exatoi(nv)	& 0xff;
		} else if (strncmp(s, "c'", 2) == 0) {
			printf("C' = %02x : ", C_);
			fgets(nv, sizeof(nv), stdin);
			C_ = exatoi(nv)	& 0xff;
		} else if (strncmp(s, "d'", 2) == 0) {
			printf("D' = %02x : ", D_);
			fgets(nv, sizeof(nv), stdin);
			D_ = exatoi(nv)	& 0xff;
		} else if (strncmp(s, "e'", 2) == 0) {
			printf("E' = %02x : ", E_);
			fgets(nv, sizeof(nv), stdin);
			E_ = exatoi(nv)	& 0xff;
		} else if (strncmp(s, "h'", 2) == 0) {
			printf("H' = %02x : ", H_);
			fgets(nv, sizeof(nv), stdin);
			H_ = exatoi(nv)	& 0xff;
		} else if (strncmp(s, "l'", 2) == 0) {
			printf("L' = %02x : ", L_);
			fgets(nv, sizeof(nv), stdin);
			L_ = exatoi(nv)	& 0xff;
		} else if (strncmp(s, "i", 1) == 0) {
			printf("I = %02x : ", I);
			fgets(nv, sizeof(nv), stdin);
			I = exatoi(nv) & 0xff;
		} else if (strncmp(s, "a", 1) == 0) {
			printf("A = %02x : ", A);
			fgets(nv, sizeof(nv), stdin);
			A = exatoi(nv) & 0xff;
		} else if (strncmp(s, "f", 1) == 0) {
			printf("F = %02x : ", F);
			fgets(nv, sizeof(nv), stdin);
			F = exatoi(nv) & 0xff;
		} else if (strncmp(s, "b", 1) == 0) {
			printf("B = %02x : ", B);
			fgets(nv, sizeof(nv), stdin);
			B = exatoi(nv) & 0xff;
		} else if (strncmp(s, "c", 1) == 0) {
			printf("C = %02x : ", C);
			fgets(nv, sizeof(nv), stdin);
			C = exatoi(nv) & 0xff;
		} else if (strncmp(s, "d", 1) == 0) {
			printf("D = %02x : ", D);
			fgets(nv, sizeof(nv), stdin);
			D = exatoi(nv) & 0xff;
		} else if (strncmp(s, "e", 1) == 0) {
			printf("E = %02x : ", E);
			fgets(nv, sizeof(nv), stdin);
			E = exatoi(nv) & 0xff;
		} else if (strncmp(s, "h", 1) == 0) {
			printf("H = %02x : ", H);
			fgets(nv, sizeof(nv), stdin);
			H = exatoi(nv) & 0xff;
		} else if (strncmp(s, "l", 1) == 0) {
			printf("L = %02x : ", L);
			fgets(nv, sizeof(nv), stdin);
			L = exatoi(nv) & 0xff;
		} else
			printf("can't change register %s\n", nv);
		print_head();
		print_reg();
	}
}

/*
 *	Output header for the CPU registers
 */
static void print_head(void)
{

  Add_to_Log("\nPC   A  SZHPNC I  IFF BC   DE   HL   A'F' B'C' D'E' H'L' IX   IY   SP\n");
}

/*
 *	Output all CPU registers
 */
static void print_reg(void)
{
  char tstr[80];

  sprintf(tstr, "%04x %02x ", (WORD)(PC - ram), A);
  sprintf(tstr, "%c", F & S_FLAG	? '1' :	'0');
  sprintf(tstr, "%c", F & Z_FLAG	? '1' :	'0');
  sprintf(tstr, "%c", F & H_FLAG	? '1' :	'0');
  sprintf(tstr, "%c", F & P_FLAG	? '1' :	'0');
  sprintf(tstr, "%c", F & N_FLAG	? '1' :	'0');
  sprintf(tstr, "%c", F & C_FLAG	? '1' :	'0');
  sprintf(tstr, " %02x ", I);
  sprintf(tstr, "%c", IFF & 1 ? '1' : '0');
  sprintf(tstr, "%c", IFF & 2 ? '1' : '0');
  sprintf(tstr, "  %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %04x %04x %04x\n",
		 B, C, D, E, H,	L, A_, F_, B_, C_, D_, E_, H_, L_, IX, IY,
		 (WORD)(STACK - ram));
  Add_to_Log(tstr);
}

/*
 *	Software breakpoints
 */
int do_break(char *s)
{
#ifndef	SBSIZE
	puts("Sorry, no breakpoints available");
	puts("Please recompile with SBSIZE defined in sim.h");
	return(2);
#else
	register int i;

	if (*s == '\n')	{
		puts("No Addr Pass  Counter");
		for (i = 0; i <	SBSIZE;	i++)
			if (soft[i].sb_pass)
				printf("%02d %04x %05d %05d\n",	i,
				       soft[i].sb_adr,soft[i].sb_pass,
				       soft[i].sb_passcount);
		return(0);
	}
	if (isxdigit((int)*s)) {
		i = atoi(s++);
		if (i >= SBSIZE) {
			printf("breakpoint %d not available\n",	i);
			return(2);
		}
	} else {
		i = sb_next++;
		if (sb_next == SBSIZE)
			sb_next	= 0;
	}
	while (isspace((int)*s))
		s++;
	if (*s == 'c') {
		*(ram +	soft[i].sb_adr)	= soft[i].sb_oldopc;
		memset((char *)	&soft[i], 0, sizeof(struct softbreak));
		return(0);
	}
	if (soft[i].sb_pass)
		*(ram +	soft[i].sb_adr)	= soft[i].sb_oldopc;
	soft[i].sb_adr = exatoi(s);
	soft[i].sb_oldopc = *(ram + soft[i].sb_adr);
	*(ram +	soft[i].sb_adr)	= 0x76;
	while (!iscntrl((int)*s) && !ispunct((int)*s))
		s++;
	if (*s != ',')
		soft[i].sb_pass	= 1;
	else
		soft[i].sb_pass	= exatoi(++s);
	soft[i].sb_passcount = 0;
	return(0);
#endif
}

/*
 *	History
 */
static void do_hist(char *s)
{
#ifndef	HISIZE
	puts("Sorry, no history available");
	puts("Please recompile with HISIZE defined in sim.h");
#else
	int i,	l, b, e, c, sa;

	while (isspace((int)*s))
		s++;
	switch (*s) {
	case 'c':
		memset((char *)	his, 0,	sizeof(struct history) * HISIZE);
		h_next = 0;
		h_flag = 0;
		break;
	default:
		if ((h_next == 0) && (h_flag ==	0)) {
			puts("History memory is empty");
			break;
		}
		e = h_next;
		b = (h_flag) ? h_next +	1 : 0;
		l = 0;
		while (isspace((int)*s))
			s++;
		if (*s)
			sa = exatoi(s);
		else
			sa = -1;
		for (i = b; i != e; i++) {
			if (i == HISIZE)
				i = 0;
			if (sa != -1) {
				if (his[i].h_adr < sa)
					continue;
				else
					sa = -1;
			}
			printf("%04x AF=%04x BC=%04x DE=%04x HL=%04x IX=%04x IY=%04x SP=%04x\n",
			       his[i].h_adr, his[i].h_af, his[i].h_bc,
			       his[i].h_de, his[i].h_hl, his[i].h_ix,
			       his[i].h_iy, his[i].h_sp);
			l++;
			if (l == 20) {
				l = 0;
				printf("q = quit, else continue: ");
				c = getkey();
				putchar('\n');
				if (toupper(c) == 'Q')
					break;
			}
		}
		break;
	}
#endif
}

/*
 *	Runtime measurement by counting the executed T states
 */
static void do_count(char *s)
{
#ifndef	WANT_TIM
	puts("Sorry, no t-state count available");
	puts("Please recompile with WANT_TIM defined in sim.h");
#else
	while (isspace((int)*s))
		s++;
	if (*s == '\0')	{
		puts("start  stop  status  T-states");
		printf("%04x   %04x    %s   %lu\n",
		       (WORD)(t_start - ram),
		       (WORD)(t_end - ram),
		       t_flag ? "on ": "off", t_states);
	} else {
		t_start	= ram +	exatoi(s);
		while (*s != ',' && *s != '\0')
			s++;
		if (*s)
			t_end =	ram + exatoi(++s);
		t_states = 0L;
		t_flag = 0;
	}
#endif
}

/*
 *	Calculate the clock frequency of the emulated CPU:
 *	into memory locations 0000H to 0002H the following
 *	code will be stored:
 *		LOOP: JP LOOP
 *	It uses 10 T states for each execution. A 3 secound
 *	timer is started and then the CPU. For every opcode
 *	fetch the R register is incremented by one and after
 *	the timer is down and stopps the emulation, the clock
 *	speed of the CPU is calculated with:
 *		f = R /	300000
 */
static void do_clock(void)
{
	static BYTE save[3];

	save[0]	= *(ram	+ 0x0000);	/* save memory locations */
	save[1]	= *(ram	+ 0x0001);	/* 0000H - 0002H */
	save[2]	= *(ram	+ 0x0002);
	*(ram +	0x0000)	= 0xc3;		/* store opcode JP 0000H at address */
	*(ram +	0x0001)	= 0x00;		/* 0000H */
	*(ram +	0x0002)	= 0x00;
	PC = ram + 0x0000;		/* set PC to this code */
	R = 0L;				/* clear refresh register */
	cpu_state = CONTIN_RUN;		/* initialize CPU */
	cpu_error = NONE;
	signal(SIGALRM,	timeout);	/* initialize timer interrupt handler */
	alarm(3);			/* start 3 secound timer */
	cpu();				/* start CPU */
	*(ram +	0x0000)	= save[0];	/* restore memory locations */
	*(ram +	0x0001)	= save[1];	/* 0000H - 0002H */
	*(ram +	0x0002)	= save[2];
	if (cpu_error == NONE)
		printf("clock frequency = %5.2f Mhz\n",	((float) R) / 300000.0);
	else
		puts("Interrupted by user");
}

/*
 *	This function is the signal handler for the timer interrupt.
 *	The CPU emulation is stopped here.
 */
static void timeout(int sig)
{
	cpu_state = STOPPED;
}

/*
 *	Output informations about compiling options
 */
static void do_show(void)
{
	register int i;

	printf("Release: %s\n",	RELEASE);
#ifdef HISIZE
	i = HISIZE;
#else
	i = 0;
#endif
	printf("No. of entrys in history memory: %d\n",	i);
#ifdef SBSIZE
	i = SBSIZE;
#else
	i = 0;
#endif
	printf("No. of software breakpoints: %d\n", i);
#ifdef WANT_SPC
	i = 1;
#else
	i = 0;
#endif
	printf("Stackpointer turn around %schecked\n", i ? "" :	"not ");
#ifdef WANT_PCC
	i = 1;
#else
	i = 0;
#endif
	printf("Programcounter turn around %schecked\n", i ? ""	: "not ");
#ifdef WANT_TIM
	i = 1;
#else
	i = 0;
#endif
	printf("T-State counting %spossible\n",	i ? "" : "im");
#ifdef CNTL_C
	i = 1;
#else
	i = 0;
#endif
	printf("CPU simulation %sstopped on cntl-c\n", i ? "" :	"not ");
#ifdef CNTL_BS
	i = 1;
#else
	i = 0;
#endif
	printf("CPU simulation %sstopped on cntl-\\\n",	i ? "" : "not ");
}

/*
 *	Read a file into the memory of the emulated CPU.
 *	The following file formats are supported:
 *
 *		binary images with Mostek header
 *		Intel hex
 */
int do_getfile(char *s)
{
	char fn[LENCMD];
	BYTE fileb[5];
	register char *pfn = fn;
	int fd;

	while (isspace((int)*s))
		s++;
	while (*s != ',' && *s != '\n' && *s !=	'\0')
		*pfn++ = *s++;
	*pfn = '\0';
	if (strlen(fn) == 0) {
		puts("no input file given");
		return(1);
	}
	if ((fd	= open(fn, O_RDONLY)) == -1) {
		printf("can't open file %s\n", fn);
		return(1);
	}
	if (*s == ',')
		wrk_ram	= ram +	exatoi(++s);
	else
		wrk_ram	= NULL;
	read(fd, (char *) fileb, 5); /*	read first 5 bytes of file */
	if (*fileb == (BYTE) 0xff) {	/* Mostek header ? */
		lseek(fd, 0l, 0);
		return (load_mos(fd, fn));
	}
	else {
		close(fd);
		return (load_hex(fn));
	}
}

/*
 *	Loader for binary images with Mostek header.
 *	Format of the first 3 bytes:
 *
 *	0xff ll	lh
 *
 *	ll = load address low
 *	lh = load address high
 */
static int load_mos(int fd, char *fn)
{
	BYTE fileb[3];
	unsigned count,	readed;
	int rc = 0;

	read(fd, (char *) fileb, 3);	/* read load address */
	if (wrk_ram == NULL)		/* and set if not given */
		wrk_ram	= ram +	(fileb[2] * 256	+ fileb[1]);
	count =	ram + 65535 - wrk_ram;
	if ((readed = read(fd, (char *)	wrk_ram, count)) == count) {
		puts("Too much to load, stopped at 0xffff");
		rc = 1;
	}
	close(fd);
	sprintf(lstr, "Loader statistics for file %s (MOSTEK) :\nSTART : %04X\nEND   : %04X\nLOADED: %04X\n",
		fn,
		(WORD)(wrk_ram - ram),
		(WORD)(wrk_ram - ram + readed - 1),
		readed);
	Add_to_Log(lstr);
	PC = wrk_ram;
	return(rc);
}

/*
 *	Loader for Intel hex
 */
static int load_hex(char *fn)
{
	register int i;
	FILE *fd;
	char buf[BUFSIZE];
	char *s;
	int count = 0;
	int addr = 0;
	int saddr = 0xffff;
	int eaddr = 0;
	int data;

	if ((fd = fopen(fn, "r")) == NULL) {
		printf("can't open file %s\n", fn);
		return(1);
	}

	while (fgets(&buf[0], BUFSIZE, fd) != NULL) {
		s = &buf[0];
		while (isspace(*s))
			s++;
		if (*s != ':')
			continue;
		if (checksum(s + 1) != 0) {
			printf("invalid checksum in hex record: %s\n", s);
			return(1);
		}
		s++;
		count = (*s <= '9') ? (*s - '0') << 4 :
				      (*s - 'A' + 10) << 4;
		s++;
		count += (*s <= '9') ? (*s - '0') :
				       (*s - 'A' + 10);
		s++;
		if (count == 0)
			break;
		addr = (*s <= '9') ? (*s - '0') << 4 :
				     (*s - 'A' + 10) << 4;
		s++;
		addr += (*s <= '9') ? (*s - '0') :
				      (*s - 'A' + 10);
		s++;
		addr *= 256;
		addr += (*s <= '9') ? (*s - '0') << 4 :
				      (*s - 'A' + 10) << 4;
		s++;
		addr += (*s <= '9') ? (*s - '0') :
				      (*s - 'A' + 10);
		s++;
		if (addr < saddr)
			saddr = addr;
		if (addr > eaddr)
			eaddr = addr + count - 1;
		s += 2;
		for (i = 0; i < count; i++) {
			data = (*s <= '9') ? (*s - '0') << 4 :
					     (*s - 'A' + 10) << 4;
			s++;
			data += (*s <= '9') ? (*s - '0') :
					      (*s - 'A' + 10);
			s++;
			*(ram + addr + i) = data;
		}
	}

	fclose(fd);
	printf("Loader statistics for file %s (INTEL):\n", fn);
	printf("START : %04x\n", saddr);
	printf("END   : %04x\n", eaddr);
	printf("LOADED: %04x\n", eaddr - saddr + 1);
	PC = ram + saddr;

	return(0);
}

/*
 *	Verify checksum of Intel hex records
 */
static int checksum(char *s)
{
	int chk = 0;

	while (*s != '\n') {
		chk += (*s <= '9') ?
			(*s - '0') << 4 :
			(*s - 'A' + 10) << 4;
		s++;
		chk += (*s <= '9') ?
			(*s - '0') :
			(*s - 'A' + 10);
		s++;
	}

	if ((chk & 255) == 0)
		return(0);
	else
		return(1);
}

/*
 *	Call system function from simulator
 */
static void do_unix(char *s)
{
	int_off();
	system(s);
	int_on();
}

/*
 *	Output help text
 */
static void do_help(void)
{
	puts("r filename[,address]      read object into memory");
	puts("d [address]               dump memory");
	puts("l [address]               list memory");
	puts("m [address]               modify memory");
	puts("f address,count,value     fill memory");
	puts("v from,to,count           move memory");
	puts("p address                 show/modify port");
	puts("g [address]               run program");
	puts("t [count]                 trace program");
	puts("return                    single step program");
	puts("x [register]              show/modify register");
	puts("x f<flag>                 modify flag");
	puts("b[no] address[,pass]      set soft breakpoint");
	puts("b                         show soft breakpoints");
	puts("b[no] c                   clear soft breakpoint");
	puts("h [address]               show history");
	puts("h c                       clear history");
	puts("z start,stop              set trigger adr for t-state count");
	puts("z                         show t-state count");
	puts("c                         measure clock frequency");
	puts("s                         show settings");
	puts("! command                 execute UNIX command");
	puts("q                         quit");
}

/*
 *	Error handler after CPU is stopped
 */
static void cpu_err_msg(void)
{
	switch (cpu_error) {
	case NONE:
		break;
	case OPHALT:
		sprintf(lstr, "HALT Op-Code reached at %04X\n",
		       (WORD)(PC - ram - 1));
		Add_to_Log(lstr);
		Show_All();
		break;
	case IOTRAP:
		sprintf(lstr, "I/O Trap at %04X\n", (WORD)(PC - ram));
		Add_to_Log(lstr);
		break;
	case IOERROR:
		sprintf(lstr, "Fatal I/O Error at %04X\n", (WORD)(PC - ram));
		Add_to_Log(lstr);
		break;
	case OPTRAP1:
		sprintf(lstr, "Op-code trap at %04X %02X\n",
		       (WORD)(PC - 1 - ram), *(PC-1));
		Add_to_Log(lstr);
		break;
	case OPTRAP2:
		sprintf(lstr, "Op-code trap at %04X %02X %02X\n",
		       (WORD)(PC - 2 - ram),
		       *(PC-2),	*(PC-1));
		Add_to_Log(lstr);
		break;
	case OPTRAP4:
		sprintf(lstr, "Op-code trap at %04X %02X %02X %02X %02X\n",
		       (WORD)(PC - 4 - ram), *(PC-4), *(PC-3),
		       *(PC-2), *(PC-1));
		Add_to_Log(lstr);
		break;
	case USERINT:
		sprintf(lstr, "User Interrupt\n");
		Add_to_Log(lstr);
		break;
	default:
		sprintf(lstr, "Unknown error %d\n", cpu_error);
		Add_to_Log(lstr);
		break;
	}
}
