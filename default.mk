NAME = ${APP_PREFIX}-${APP}
TESTS = $(wildcard t/*.py)
OBJ = ${SRC:.c=.o}
TEST_SRC = $(wildcard t/*.c)
TEST_BIN = ${TEST_SRC:.c=}

all: options ${NAME}

options:
	@echo ${NAME} build options:
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LDFLAGS = ${LDFLAGS}"
	@echo "CC      = ${CC}"
	@echo "COV     = ${COV}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${NAME}: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ $^ ${LDFLAGS}

${TEST_BIN}: ${TEST_BIN}.c
	@echo CC -o $@
	@${CC} -o $@ $^ ${CFLAGS} ${LDFLAGS} -O0

test: all ${TEST_BIN}
	@echo TEST ${NAME}
	@echo
	@for test in ${TESTS} ${TEST_BIN}; do \
		base=$$(basename "$$test" .pl); \
		echo "##################################################"; \
		printf "# %-46s #\n" "$$base"; \
		echo "##################################################"; \
		echo; \
		$$test; \
	done
	@echo
	@echo "##################################################"
	@printf "# %-46s #\n" "coverage"
	@echo "##################################################"
	@echo
	@if test -z "$$COV"; then ${COV} ${SRC} ${TEST_SRC}; fi

clean:
	@echo cleaning
	@rm -f ${NAME} ${OBJ} *.gc?? t/*.gc?? ${TEST_BIN}

.PHONY: all options test clean
