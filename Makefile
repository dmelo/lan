BIN=bin
SRC=src
CC=gcc
CFLAGS= -g -Wall


all: ${BIN}/bbfs ${BIN}/lan


${BIN}/bbfs: ${BIN}/bbfs.o ${BIN}/log.o
	${CC} ${CFLAGS} ${BIN}/bbfs.o ${BIN}/log.o -o ${BIN}/bbfs `pkg-config fuse --libs`

${BIN}/bbfs.o: ${SRC}/bbfs.c ${SRC}/log.h ${SRC}/params.h
	${CC} ${CFLAGS} `pkg-config fuse --cflags` -c ${SRC}/bbfs.c -o ${BIN}/bbfs.o 

${BIN}/log.o: ${SRC}/log.c ${SRC}/log.h ${SRC}/params.h
	${CC} ${CFLAGS} `pkg-config fuse --cflags` -c ${SRC}/log.c -o ${BIN}/log.o

${BIN}/lan: ${SRC}/lan.c ${SRC}/lan.h
	${CC} ${CFLAGS} ${SRC}/lan.c -o ${BIN}/lan

clean:
	rm -f ${BIN}/*
