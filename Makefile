CC=g++
LNFLAGS=
SRCDIR=src
SRCDIRS=$(shell find $(SRCDIR) -type d)
SRC=$(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.cpp))

ifndef DEBUG
	CXXFLAGS=-std=c++11 -MMD -MP -O3
	OBJDIR=release
	EXECUTABLE=release/strelac
else
	CXXFLAGS=-std=c++11 -g -MMD -MP
	OBJDIR=debug
	EXECUTABLE=debug/strelac
endif

OBJDIRS=$(pathsubst $(SRCDIRS)/%,$(OBJDIRS)/%,$(SRCDIRS))
_OBJ=$(SRC:.cpp=.o)
OBJ=$(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(_OBJ))
DEPS = ${OBJ:.o=.d}

.PHONY: clean strelac

strelac: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)
	$(CC) $^ $(CXXFLAGS) $(LNFLAGS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@[ -d $(@D) ] || mkdir -p $(@D)
	$(CC) $< $(CXXFLAGS) -c -o $@

clean:
	rm -rf release debug

test: strelac
	bash ./test.sh

-include ${DEPS}
