COV = gcov
CFLAGS += -fprofile-arcs -ftest-coverage
LDFLAGS += -lgcov --coverage
