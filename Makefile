CC=g++
LNFLAGS=-lffi -ldl
SRCDIR=./src
SRCDIRS=$(shell find $(SRCDIR) -type d)
SRC=$(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.cpp))

ifndef DEBUG
	CXXFLAGS=-std=c++11 -MMD -MP -O3
	OBJDIR=Release
	EXECUTABLE=Release/strela
else
	CXXFLAGS=-std=c++11 -g -MMD -MP -D _DEBUG
	OBJDIR=Debug
	EXECUTABLE=Debug/strela
endif

OBJDIRS=$(pathsubst $(SRCDIRS)/%,$(OBJDIRS)/%,$(SRCDIRS))
_OBJ=$(SRC:.cpp=.o)
OBJ=$(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(_OBJ))
DEPS = ${OBJ:.o=.d}

.PHONY: clean install install-home test

strela: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)
	$(CC) $^ $(CXXFLAGS) $(LNFLAGS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@[ -d $(@D) ] || mkdir -p $(@D)
	$(CC) $< $(CXXFLAGS) -c -o $@

install: strela
	install Release/strela /usr/local/bin/strela
	install -d /usr/local/lib/strela
	cp -r Std /usr/local/lib/strela

install-home: strela
	install Release/strela ~/bin/strela
	install -d ~/.strela/lib/
	cp -r Std ~/.strela/lib

clean:
	rm -rf Release Debug

test: strela
	bash ./test.sh

-include ${DEPS}
