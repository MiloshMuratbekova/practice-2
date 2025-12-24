# Makefile для практической работы №2
# Параллельная реализация алгоритмов сортировки с OpenMP

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -fopenmp
TARGET = sorting_openmp
SRC = sorting_openmp.cpp

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

# Запуск с разным количеством потоков
run2: $(TARGET)
	OMP_NUM_THREADS=2 ./$(TARGET)

run4: $(TARGET)
	OMP_NUM_THREADS=4 ./$(TARGET)

run8: $(TARGET)
	OMP_NUM_THREADS=8 ./$(TARGET)
