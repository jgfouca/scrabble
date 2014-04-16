CPP         = mpicxx

#CPPFLAGS    = -g -Wall -Woverloaded-virtual -Wnon-virtual-dtor -Wreturn-type -Wswitch-enum -Wsign-compare -Wfloat-equal -Wparentheses -Wreorder -Wshadow -Wold-style-cast -Wextra 
#CPPFLAGS   += -pg
#CPPFLAGS   += -DVERBOSE
#CPPFLAGS   += -O -Wuninitialized

CPPFLAGS    = -O2 -g -DNDEBUG -std=c++0x        #OPTIMIZATION FLAGS
#CPPFLAGS   += -pg
#Using -O3 means we shouldnt have to worry about inlining.


.PHONY : clean

OBJECTS = $(patsubst %.cc, %.o,$(wildcard *.cc))

scrabble: $(OBJECTS)
	$(CPP) $(CPPFLAGS) $(OBJECTS) -o scrabble

$(OBJECTS): %.o: %.cc
	$(CPP) -c $(CPPFLAGS) $< -o $@
	-@echo 

clean:
	-@rm -f *.o *~ scrabble.exe scrabble core* *stackdump*
