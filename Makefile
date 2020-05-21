# Makefile

CC			:= g++
INCDIR		:= -Iinclude
SRCDIR		:= source
CFLAGS		:= $(INCDIR) -Wall
LDFLAGS		:= -lSDL2
CPPFILES	:= $(wildcard $(SRCDIR)/*.cpp)

all: $(CPPFILES)
	$(CC) $(INCDIR) -fsanitize=address -g3 -o gbe $(CPPFILES) $(CFLAGS) $(LDFLAGS)

clean:
	rm gbe
