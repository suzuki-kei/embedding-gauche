SRC = main.c
OUT = a.out
INCLUDE_DIR = `gauche-config -I`
LIBRARY_DIR = `gauche-config -L`
LIBRARIES = `gauche-config -l`

$(OUT): $(SRC)
	gcc -std=gnu99 -Ofast -Wall -Werror -o $(OUT) $(INCLUDE_DIR) $^ $(LIBRARY_DIR) $(LIBRARIES)

run: $(OUT)
	@./$(OUT)

clean:
	rm -f $(OUT)

