CXXFLAGS = -std=c++11
OPTLEVEL = -O3
DEBUG = # -g

all:
	$(CXX) main.cpp endurer.cpp -o endurer $(CXXFLAGS) $(OPTLEVEL) $(DEBUG)

clean:
	rm -f endurer

run:
	./endurer

edit:
	vim -p main.cpp endurer.cpp endurer.h Makefile
