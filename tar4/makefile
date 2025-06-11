 
C = g++
#CFLAGS = -c -g -Wall -fprofile-arcs -ftest-coverage
#LDFLAGS = -g -fprofile-arcs -ftest-coverage -lgcov
CFLAGS = -c -g -Wall
LDFLAGS = -g
COVERAGE_DIR = coverage_files

OBJECTS1 = ConvexHall.o
OBJECTS2 = Client.o

TARGET1 = ConvexHall
TARGET2 = Client

all: $(TARGET1) $(TARGET2)

$(TARGET1): $(OBJECTS1)
	$(C) -o $(TARGET1) $(OBJECTS1) $(LDFLAGS)

$(TARGET2): $(OBJECTS2)
	$(C) -o $(TARGET2) $(OBJECTS2) $(LDFLAGS)

ConvexHall.o: ConvexHall.cpp ConvexHall.hpp
	$(C) $(CFLAGS) ConvexHall.cpp -o ConvexHall.o

Client.o: Client.cpp
	$(C) $(CFLAGS) Client.cpp -o Client.o

.PHONY: clean all coverage ConvexHall Client

clean:
	rm -rf $(TARGET1) $(TARGET2) *.gcda *.gcno *.gcov $(COVERAGE_DIR) *.o

coverage: $(TARGET)
	mkdir -p $(COVERAGE_DIR)
	mv *.gcda $(COVERAGE_DIR) || true
	mv *.gcno $(COVERAGE_DIR) || true
	mv *.gcov $(COVERAGE_DIR) || true