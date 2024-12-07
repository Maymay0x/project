hashtable_dir = ../hashtable/
holdall_dir = ../holdall/
word_dir = ../word/
opt_dir = ../opt/
avl_dir = ../avl/
CC = gcc
CFLAGS = -std=c2x \
  -Wall -Wconversion -Werror -Wextra -Wpedantic -Wwrite-strings \
  -O2 \
  -I$(hashtable_dir) -I$(holdall_dir) -I$(word_dir) -I$(opt_dir) -I$(avl_dir)\
  -DHASHTABLE_STATS=0 -DHOLDALL_PUT_TAIL=1
vpath %.c $(hashtable_dir) $(holdall_dir) $(word_dir) $(opt_dir) $(avl_dir)
vpath %.h $(hashtable_dir) $(holdall_dir) $(word_dir) $(opt_dir) $(avl_dir)
objects = main.o hashtable.o holdall.o word.o opt.o bst.o
executable = xwc
makefile_indicator = .\#makefile\#

.PHONY: all clean

all: $(executable)

clean:
	$(RM) $(objects) $(executable)
	@$(RM) $(makefile_indicator)

$(executable): $(objects)
	$(CC) $(objects) -o $(executable)

main.o: main.c hashtable.h holdall.h word.h opt.h bst.h
hashtable.o: hashtable.c hashtable.h
word.o: word.c word.h
holdall.o: holdall.c holdall.h
opt.o: opt.c opt.h
bst.o: bst.c bst.h

include $(makefile_indicator)

$(makefile_indicator): makefile
	@touch $@
	@$(RM) $(objects) $(executable)
