
###############################
# If no target was given

help:
	@echo ""
	@echo "Available commands:"
	@echo "~~~~~~~~~~~~~~~~~~~"
	@echo "$(MAKE) clean      Clean temporary files."
	@echo "$(MAKE) clean-all  Clean all files make produced."
	@echo "$(MAKE) main       Build the program."
	@echo ""


###############################
# Compiler/Linker options

# Environment
BIN  = bin
OUT  = obj
CC   = gcc

# Flags for all binaries
CFLAGS = -Wall -Wsign-compare -pedantic -Iinclude

# Flags for object files only
OFLAGS = $(CFLAGS) -c -s


###############################
# Directory management

# Creation
$(BIN):
	@mkdir -p $(BIN)

$(OUT):
	@mkdir -p $(OUT)

# Cleaning
clean:
	@rm -Rf $(OUT)

clean-all: clean
	@rm -Rf $(BIN)


###############################
# Builds

# Object files
$(OUT)/%.o: src/%.c $(HEADERS) | $(OUT)
	@$(CC) $(OFLAGS) $< -o $@

# Main binary
main: src/main.c | $(BIN)
	@$(CC) $(CFLAGS) $< $(OBJS) -o $(BIN)/main
