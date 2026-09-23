CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -pedantic -g -O1
CPPFLAGS = -Iinclude
LDLIBS  = -lm

TARGET  = mesh
BUILD   = build

SRC     = exec/main.c src/HeObject.c src/HeOperations.c src/HeGetters.c
OBJ     = $(SRC:%.c=$(BUILD)/%.o)
DEP     = $(OBJ:.o=.d)

all: $(TARGET)

# the executable stays in the main dir, every .o goes under build/
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

# build/exec/main.o, build/src/HeObject.o, ... -- mkdir -p recreates the tree
$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

# rebuild a .c when one of the headers it includes changed
-include $(DEP)

clean:
	rm -rf $(BUILD) $(TARGET)

.PHONY: all clean
