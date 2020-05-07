	ORG	0000

	LD	HL, str
	LD	C,01
	LD	B,06
loop	LD	A,(HL)
	OUT	(C),A
	INC	HL
	DJNZ	loop
	HALT

	LD	HL,str
	LD	C,02
	LD	B,06
	OTIR
	HALT

	IN	A,(00)
	IN	A,(00)
	IN	A,(00)
	IN	A,(00)
	NOP
	HALT

str	DEFB	'abcdef'
