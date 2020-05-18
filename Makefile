
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
	@echo "Other dependencies that are included:"
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
	@echo "recp/cglm"
	@echo "Dav1dde/glad"
	@echo ""
	@echo "Available commands:"
	@echo "~~~~~~~~~~~~~~~~~~~"
	@echo "$(MAKE) clean      Clean temporary files."
	@echo "$(MAKE) terr       Build the program."
	@echo ""


###############################
# Compiler/Linker options

# Environment
OUT = obj
CC  = gcc

# Flags for all binaries
CFLAGS = -std=c99 -Wall -Wsign-compare -Iinclude -Idepend

# Flags for object files only
OFLAGS = $(CFLAGS) -c -s

# Linker flags
LFLAGS = -lglfw3 -lX11 -lGL -pthread -lm -ldl


###############################
# Directory management

# Creation
$(OUT):
	@mkdir -p $(OUT)
	@mkdir -p $(OUT)/generators
	@mkdir -p $(OUT)/modifiers

# Cleaning
clean:
	@rm -Rf $(OUT)


###############################
# Builds

HEADERS = \
 include/deps.h \
 include/generators.h \
 include/modifiers.h \
 include/output.h \
 include/patch.h \
 include/scene.h \
 include/shader.h

OBJS = \
 $(OUT)/glad.o \
 $(OUT)/generators/mpd.o \
 $(OUT)/generators/noise.o \
 $(OUT)/modifiers/flatten.o \
 $(OUT)/modifiers/relax_slope.o \
 $(OUT)/modifiers/stats.o \
 $(OUT)/output.o \
 $(OUT)/patch.o \
 $(OUT)/scene.o \
 $(OUT)/shader.o

# Dependencies
$(OUT)/glad.o: depend/glad/glad.c | $(OUT)
	$(CC) $(OFLAGS) $< -o $@

# Object files
$(OUT)/%.o: src/%.c $(HEADERS) | $(OUT)
	$(CC) $(OFLAGS) $< -o $@

# Main binary
terr: src/main.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LFLAGS)
