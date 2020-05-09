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

void Show_All(void);
void Add_to_Log(char *str);
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
}

/*
 *	Handling of software breakpoints (HALT opcode):
 *
 *	Output:	0 breakpoint or other HALT opcode reached (stop)
 *		1 breakpoint reached, passcounter not reached (continue)
 */
static int handel_break(void)
{
	register int i;

	for (i = 0; i <	SBSIZE;	i++)	/* search for breakpoint */
		if (soft[i].sb_adr == PC - ram - 1)
			goto was_softbreak;
	return(0);
	was_softbreak:
	h_next--;			/* correct history */
	if (h_next < 0)
		h_next = 0;
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
 *	Software breakpoints
 */
int do_break(char *s)
{
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

/*
 *      Execute a single step
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
        p = PC;
        disass(&p, p - ram);
}

/*
 *      Execute several steps with trace output
 */
void do_trace(char *s)
{
        register int count, i;

        while (isspace((int)*s))
                s++;
        if (*s == '\0')
                count = 20;
        else
                count = atoi(s);
        cpu_state = SINGLE_STEP;
        cpu_error = NONE;
        for (i = 0; i < count; i++) {
                cpu();
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

