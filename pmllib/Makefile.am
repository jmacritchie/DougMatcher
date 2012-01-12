## Makefile.am -- Process this file with automake to produce Makefile.in

lib_LTLIBRARIES      	= libpml.la

#PML library stuff
pml_sources		= pmldocument.cpp \
		    	pmlbase.cpp \
		        scorepart.cpp \
		        perfpart.cpp \
		        snote.cpp \
		        pnote.cpp \
		        pitchel.cpp \
		        tuningel.cpp \
		    	errorhandler.cpp
		    	

pml_libraries		= -lm -lxerces-c


# tuning library stuff
tuning_sources		= tuning.cpp \
			pitch.cpp
tuning_libraries	= -lm -lqt-mt



#my general stuff
libpml_la_SOURCES 	= xmlsc.cpp \
			$(pml_sources) \
			$(tuning_sources)

libpml_la_LIBADD    	= -lm -lqt-mt -lfftw3 -lsndfile $(pml_libraries) $(tuning_libraries)

INCLUDES		= -I/usr/include/qt3

CXXFLAGS		+= $(INCLUDES) -Wno-deprecated
CFLAGS  		+= $(INCLUDES) -Wno-deprecated

##for 64 bit 
##CXXFLAGS		+= $(INCLUDES) -Wno-deprecated -m64
##CFLAGS  		+= $(INCLUDES) -Wno-deprecated -m64


install-data-local:

	$(INSTALL_DATA)	tunings /usr/local/lib/tunings

