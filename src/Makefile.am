bin_PROGRAMS = houkai
houkai_SOURCES = main.c akpk/read.c akpk/didx.c akpk/riff.c
houkai_CFLAGS = -std=gnu18 \
-O0 -g -ggdb -gdwarf-5 -DDEBUG \
-Wall -Wextra \
-Wpedantic -Wcast-align -Wcast-qual -Wconversion -Wenum-compare \
-Wfloat-equal -Wredundant-decls -Wsign-conversion -fstack-protector \
-fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=leak \
-fsanitize=undefined -fsanitize-address-use-after-scope \
-I/usr/lib/gcc/x86_64-pc-linux-gnu/11.1.0/include \
-I/usr/local/include \
-I/usr/lib/gcc/x86_64-pc-linux-gnu/11.1.0/include-fixed \
-I/usr/include \
-flto -fuse-linker-plugin -flto=4
houkai_LDFLAGS = -Wl,-O3 -Wl,--sort-common -Wl,--hash-style=gnu -Wl,--enable-new-dtags -Wl,--as-needed -flto=4 -fuse-linker-plugin

CLEANFILES = *~
