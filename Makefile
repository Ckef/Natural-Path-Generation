
###############################
# If no target was given

help:
	@echo ""
	@echo "Currently OpenGL 4.2 is hardcoded."
	@echo "Dependencies that need to be installed:"
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
	@echo "xorg-dev"
	@echo "libgl1-mesa-dev"
	@echo "GLFW3 (build and install from source)"
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
CFLAGS = -Wall -Wsign-compare -Iinclude -Idepend

# Flags for object files only
OFLAGS = $(CFLAGS) -c -s

# Linker flags
LFLAGS = -lglfw3 -lX11 -lGL -pthread -lm -ldl


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

HEADERS = \
 include/input.h \
 include/output.h

OBJS = \
 $(OUT)/glad.o \
 $(OUT)/input.o \
 $(OUT)/output.o

# Dependencies
$(OUT)/glad.o: depend/glad/glad.c | $(OUT)
	$(CC) $(OFLAGS) $< -o $@

# Object files
$(OUT)/%.o: src/%.c $(HEADERS) | $(OUT)
	$(CC) $(OFLAGS) $< -o $@

# Main binary
main: src/main.c $(OBJS) | $(BIN)
	$(CC) $(CFLAGS) $< $(OBJS) -o $(BIN)/$@ $(LFLAGS)
