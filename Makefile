DIR=$(PWD)
#BIN=$(DIR)/tcpcali
BIN=md
CFLAG=
LFLAG= -lm
CC=gcc
AR=ar
CO=$(CC)

#SRC = $(wildcard $(DIR)/*.c)
SRC = motion_detect.c libmd.c
OBJ = $(patsubst %.c,%.o,$(SRC))


all:$(BIN)
$(BIN):$(OBJ)
	$(CO) -o $@ $^ $(LFLAG)

$(DIR)/%.o:$(DIR)/%.c
	$(CC) $@ -c $< $(CFLAG)


.PHONY:clean
clean:
	rm $(OBJ) $(BIN)
