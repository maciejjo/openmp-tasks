CC	= clang
TARGET	= ex1
CFLAGS	= -Wall -Wextra -pedantic -g -fopenmp
LFLAGS	= -lomp
SRCDIR	= src
OBJDIR	= obj
BINDIR	= bin
DEPDIR	= dep
DATDIR	= data

SOURCES	:= $(wildcard $(SRCDIR)/*.c)
OBJECTS	:= $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
DEPS	:= $(OBJECTS:$(OBJDIR)/%.o=$(DEPDIR)/%.d)

REMOVE	:= rm -rf

# Linking
$(BINDIR)/$(TARGET): $(OBJECTS) $(CMN_OBJ)
	mkdir -p $(BINDIR)
	$(CC) $(LFLAGS) -o $@ $(OBJECTS) $(CMN_OBJ)
	@echo "Linking complete"

-include $(DEPS)

# Compilation
$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	mkdir -p $(OBJDIR)
	mkdir -p $(DEPDIR)
	$(CC) -c $(CFLAGS) $< -o $@
	$(CC) -MM -MT '$(OBJDIR)/$*.o' $(SRCDIR)/$*.c > $(DEPDIR)/$*.d
	@echo "Compiled $<"

.PHONY: clean
clean:
	$(REMOVE) $(OBJECTS) $(OBJDIR) $(BINDIR) $(DEPDIR)
	@echo "Deleted $<"

.PHONY: remove
remove:
	$(REMOVE) $(BINDIR)/$(TARGET)
	$(REMOVE) $(OBJECTS)
	$(REMOVE) $(DEPS)
	@echo "Deleted $<"

.PHONY: all
all: $(BINDIR)/$(TARGET)
