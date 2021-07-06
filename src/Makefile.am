bin_PROGRAMS = houkai

houkai_SOURCES = main.c akpk/akpk.c

houkai_CFLAGS = -std=gnu99 \
-O0 -g -ggdb -gdwarf-5 -DDEBUG \
-Wall -Wextra \
-Wpedantic -Wcast-align -Wcast-qual -Wconversion -Wenum-compare \
-Wfloat-equal -Wredundant-decls -Wsign-conversion \
@GTK_CFLAGS@

houkai_LDADD = @GTK_LIBS@
CLEANFILES = *~
