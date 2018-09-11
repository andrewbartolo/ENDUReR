CCFLAGS = -std=c11
OPTLEVEL = -O3
UNOPTLEVEL = -O0
DEBUG = -g

all:
	$(CC) main.c endurer.c -o endurer $(CCFLAGS) $(OPTLEVEL)

debug:
	$(CC) main.c endurer.c -o endurer $(CCFLAGS) $(UNOPTLEVEL) $(DEBUG)

clean:
	rm -f endurer

run:
	./endurer

edit:
	vim -p main.c endurer.c endurer.h Makefile
