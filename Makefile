# Basic Makefile

### Compilers
CC  = gcc
CXX = g++

DEBUG_LEVEL    = -g
EXTRA_CCFLAGS  = -W -Wall -std=c++11
CPPFLAGS       = $(DEBUG_LEVEL) $(EXTRA_CCFLAGS)
CCFLAGS        = $(CPPFLAGS)

RM = rm -f
MV = mv

SRCDIR := src
INCDIR := include

### ROOT
ROOTCFLAGS := $(shell root-config --cflags)
ROOTLIBS   := $(shell root-config --libs)

### RAT
RATLIBS  := -L$(RATROOT)/lib -lRATEvent

### BOOST
BOOSTCFLAGS := -I/data/snoplus/home/zsoldos/.local/boost-1.71.0
BOOSTLIBS   := -L/data/snoplus/home/zsoldos/.local/boost-1.71.0/lib -lboost_system -lboost_filesystem
### Python
PYTHONCFLAGS := $(shell python-config --cflags)
PYTHONLIBS   := $(shell python-config --ldflags)

CPPFLAGS  += -I$(INCDIR) $(ROOTCFLAGS) -I$(RATROOT)/include
CPPFLAGS  +=  $(BOOSTCFLAGS)
# CPPFLAGS  +=  $(PYTHONCFLAGS)
EXTRALIBS  = $(ROOTLIBS)
EXTRALIBS += $(RATLIBS)
EXTRALIBS += $(BOOSTLIBS)
# EXTRALIBS += $(PYTHONLIBS)

SRCS = $(wildcard $(SRCDIR)/*.cc)
OBJS = $(subst .cc,.o,$(SRCS))

.PHONY: all clean 
.DEFAULT_GOAL = CreatePDF

help:
	@grep -h -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-15s\033[0m %s\n", $$1, $$2}'

all: CreatePDF

CreatePDF: 
	$(CXX) $(CPPFLAGS) -o CreatePDF CreatePDF.cc $(OBJS) $(EXTRALIBS)
	$(RM) FlattenHits.o $(OBJS)

clean:
	$(RM) $(OBJS) CreatePDF
