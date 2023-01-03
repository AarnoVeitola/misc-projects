SRC_FILES=project.c
CFLAGS += -std=c99 -g -Wall -Wextra -Wno-missing-field-initializers

all: project

project: $(SRC_FILES)
	gcc $(CFLAGS) $(SRC_FILES) -lm -o main.out

clean:
	rm -f main.out

run: project
	# Running our main function from file project.c
	./main.out
