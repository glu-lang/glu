.PHONY: all clean run

CC = clang
DC = ldc2
ZIG = zig

CFLAGS = -O3
DFLAGS = -O3
ZIGFLAGS = -O ReleaseFast

TARGET = a.out
C_SRC = main.c
D_SRC = random.d
ZIG_SRC = compute.zig

D_OBJ = random.o
ZIG_OBJ = compute.o

all: $(TARGET)

$(D_OBJ): $(D_SRC)
	$(DC) $(DFLAGS) -c $(D_SRC) -of=$(D_OBJ)

$(ZIG_OBJ): $(ZIG_SRC)
	$(ZIG) build-obj $(ZIG_SRC) $(ZIGFLAGS) -femit-bin=$(ZIG_OBJ)

$(TARGET): $(C_SRC) $(D_OBJ) $(ZIG_OBJ)
	$(DC) $(C_SRC) $(D_OBJ) $(ZIG_OBJ) -of=$(TARGET) -L-w

run: $(TARGET)
	time ./$(TARGET)

clean:
	rm -f $(TARGET) $(D_OBJ) $(ZIG_OBJ)
