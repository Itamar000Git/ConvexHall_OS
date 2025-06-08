 
C = g++
#CFLAGS = -c -g -Wall -fprofile-arcs -ftest-coverage
#LDFLAGS = -g -fprofile-arcs -ftest-coverage -lgcov
CFLAGS = -c -g -Wall
LDFLAGS = -g
COVERAGE_DIR = coverage_files

OBJECTS = ConvexHall.o

TARGET = ConvexHall


all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(C) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

ConvexHall.o: ConvexHall.cpp ConvexHall.hpp
	$(C) $(CFLAGS) ConvexHall.cpp -o ConvexHall.o

.PHONY: clean all coverage ConvexHall

clean:
	rm -rf $(TARGET) *.gcda *.gcno *.gcov $(COVERAGE_DIR) *.o

coverage: $(TARGET)
	mkdir -p $(COVERAGE_DIR)
	mv *.gcda $(COVERAGE_DIR) || true
	mv *.gcno $(COVERAGE_DIR) || true
	mv *.gcov $(COVERAGE_DIR) || true