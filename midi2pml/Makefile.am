

bin_PROGRAMS 		= benmidi2pml

benmidi2pml_SOURCES 	= main.cpp \
			  readmidi.cpp

benmidi2pml_LDADD		= -lpml -lxerces-c -ldouglet 


INCLUDES		= -I/home/jenni/DougMatcher/pmllib -I/home/jenni/DougMatcher/libdouglet -I/usr/include/xercesc
LIBS			= -L/usr/local/lib 

CXXFLAGS                += $(INCLUDES) $(LIBS)
CFLAGS                  += $(INCLUDES) $(LIBS)






