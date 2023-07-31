CC = gcc
CFLAGS = -c -Wextra -Werror 
LDFLAGS = lib/ideas_packet.dll -lm -l Ws2_32
INCLUDES = -I src/headers

OBJDIR=obj
SRCDIR=src
BINDIR=bin
EXDIR=exemples

SRCEX = $(notdir $(wildcard exemples/*.c))
OBJEX = $(SRCEX:.c=.o)
EXECEX = $(SRCEX:%.c=%)

SRC = $(notdir $(wildcard $(SRCDIR)/*.c))
OBJ = $(SRC:.c=.o)
EXEC = $(SRC:%.c=%)

$(info objs: $(OBJ)$(OBJEX))

all: $(OBJ) $(OBJEX) $(EXECEX)

%.o: $(SRCDIR)/%.c
	$(CC) $(INCLUDES) $(CFLAGS) $< -o $(OBJDIR)/$@
	
%.o: $(EXDIR)/%.c
	$(CC) $(INCLUDES) $(CFLAGS) $< -o $(OBJDIR)/$@

$(EXECEX): $(addprefix $(OBJDIR)/, $(OBJ))
	$(CC) -o $(BINDIR)/$@ $^ $(OBJDIR)/$@.o $(LDFLAGS) $(INCLUDES)