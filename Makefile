z80_srcs = disas.c iosim.c sim0.c sim1.c sim2.c sim3.c sim4.c sim5.c sim6.c sim7.c simfun.c simglb.c simint.c simctl.c 
z80_hdrs = simglb.h sim.h 
gui_srcs = callbacks.c code.c guiglb.c memory.c flags.c log.c ports.c registers.c general.c breaks.c
gui_hdrs = prototypes.h callbacks.h guiglb.h

z80_objs = $(patsubst %.c,obj/%.o,$(z80_srcs))
gui_objs = $(patsubst %.c,obj/%.o,$(gui_srcs))

EXE = z80sim

vpath %.c gui z80

$(shell mkdir -p obj)

# obj/%.o: %.c $(z80_hdrs) $(gui_hdrs)
obj/%.o: %.c
	gcc -g -Wall -Wno-deprecated-declarations -Wno-format-security -c $< `pkg-config --cflags gtk+-3.0` -Igui -Iz80 -o $@

all: $(EXE)

$(EXE): $(z80_objs) $(gui_objs) main.c
	gcc -g -o $@ -Wall -Iz80 -Igui `pkg-config --cflags --libs gtk+-3.0` -export-dynamic $^ `pkg-config --cflags --libs gtk+-3.0`

clean:
	rm -f $(EXE) *.o obj/*.o
