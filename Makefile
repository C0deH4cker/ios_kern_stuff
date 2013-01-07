CC = clang
CFLAGS =
LDFLAGS =


all: kern_patcher

kern_patcher: kern_patcher.o kern_utils.o
	clang $(LDFLAGS) -o $@ $^
	ldid -Stfp0.plist $@


%.o: %.c
	clang $(CFLAGS) -c -o $@ $<
