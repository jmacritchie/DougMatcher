

bin_PROGRAMS 		= midi2pml

benmidi2pml_SOURCES 	= main.cpp \
			  readmidi.cpp

benmidi2pml_LDADD		= -lpml -lxerces-c 


INCLUDES		= -I/home/jenni/DougMatcher/pmllib -I/usr/include/xercesc
LIBS			= -L/usr/local/lib 

CXXFLAGS                += $(INCLUDES) $(LIBS)
CFLAGS                  += $(INCLUDES) $(LIBS)






