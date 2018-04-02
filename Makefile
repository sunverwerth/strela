CC=g++
LNFLAGS=-lffi -ldl
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

.PHONY: clean install install-home test

strelac: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)
	$(CC) $^ $(CXXFLAGS) $(LNFLAGS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@[ -d $(@D) ] || mkdir -p $(@D)
	$(CC) $< $(CXXFLAGS) -c -o $@

install: strelac
	install release/strelac /usr/local/bin/strelac
	install -d /usr/local/lib/strela
	cp -r Std /usr/local/lib/strela

install-home: strelac
	install release/strelac ~/bin/strelac
	install -d ~/.strela/lib/
	cp -r Std ~/.strela/lib

clean:
	rm -rf release debug

test: strelac
	bash ./test.sh

-include ${DEPS}
