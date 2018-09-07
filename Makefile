CXXFLAGS = -std=c++11
OPTLEVEL = -O3
UNOPTLEVEL = -O0
DEBUG = -g

all:
	$(CXX) main.c endurer.c -o endurer $(CXXFLAGS) $(OPTLEVEL)

debug:
	$(CXX) main.c endurer.c -o endurer $(CXXFLAGS) $(UNOPTLEVEL) $(DEBUG)

clean:
	rm -f endurer

run:
	./endurer

edit:
	vim -p main.c endurer.c endurer.h Makefile
