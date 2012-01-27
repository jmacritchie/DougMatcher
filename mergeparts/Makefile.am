
bin_PROGRAMS            = mergeparts

mergeparts_SOURCES    = main.cpp 

mergeparts_LDADD      = -ldouglet -lpml -lXm -lX11

INCLUDES                = -I/home/jenni/DougMatcher/pmllib -I/home/jenni/DougMatcher/libdouglet 
LIBS                    += -L/usr/local/lib:/usr/lib/

CXXFLAGS                += $(INCLUDES) $(LIBS)
CFLAGS                  += $(INCLUDES) $(LIBS)

