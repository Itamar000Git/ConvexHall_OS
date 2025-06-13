 
C = g++

CFLAGS = -c -g -Wall
LDFLAGS = -g -L../tar5_8 -lreactor
COVERAGE_DIR = coverage_files

OBJECTS1 = ConvexHall.o
OBJECTS2 = Client.o
REACTOR_OBJ = ../tar5_8/ReactorProactor.o
REACTOR_LIB = ../tar5_8/libreactor.a

TARGET1 = ConvexHall
TARGET2 = Client

all: $(REACTOR_LIB) $(TARGET1) $(TARGET2)

$(REACTOR_LIB): ../tar5_8/ReactorProactor.cpp ../tar5_8/ReactorProactor.hpp
	$(C) $(CFLAGS) ../tar5_8/ReactorProactor.cpp -o $(REACTOR_OBJ)
	ar rcs $(REACTOR_LIB) $(REACTOR_OBJ)

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
	rm -rf $(TARGET1) $(TARGET2) *.gcda *.gcno *.gcov $(COVERAGE_DIR) *.o $(REACTOR_OBJ) $(REACTOR_LIB)

coverage: $(TARGET1) $(TARGET2)
	mkdir -p $(COVERAGE_DIR)
	mv *.gcda $(COVERAGE_DIR) || true
	mv *.gcno $(COVERAGE_DIR) || true
	mv *.gcov $(COVERAGE_DIR) || true