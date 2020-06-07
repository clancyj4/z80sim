/*
 * Z80SIM  -  a	Z80-CPU	simulator
 *
 * Copyright (C) 1987-2008 by Udo Munk
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
 * 06-AUG-08 Release 1.15 many improvements and Windows support via Cygwin
 * 25-AUG-08 Release 1.16 console status I/O loop detection and line discipline
 */

/*
 *	This modul contains the 'main()' function of the simulator,
 *	where the options are checked and variables are initialized.
 *	After initialization of the UNIX interrupts ( int_on() )
 *	and initialization of the I/O simulation ( init_io() )
 *	the user interface ( mon() ) is called.
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <memory.h>
#include "sim.h"
#include "simglb.h"

#define BUFSIZE	256		/* buffer size for file I/O */

static int load_mos(int, char *), load_hex(char *), checksum(char *);
extern void int_on(void), int_off(void), mon(void);
extern void init_io(void), exit_io(void);
extern int exatoi(char *);

/*
 *	Read a file into the memory of the emulated CPU.
 *	The following file formats are supported:
 *
 *		binary images with Mostek header
 *		Intel hex
 */
int load_file(char *s)
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
	printf("\nLoader statistics for file %s:\n", fn);
	printf("START : %04x\n", (WORD)(wrk_ram - ram));
	printf("END   : %04x\n", (WORD)(wrk_ram - ram + readed - 1));
	printf("LOADED: %04x\n\n", readed);
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
	printf("\nLoader statistics for file %s:\n", fn);
	printf("START : %04x\n", saddr);
	printf("END   : %04x\n", eaddr);
	printf("LOADED: %04x\n\n", eaddr - saddr + 1);
	PC = wrk_ram = ram + saddr;

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
