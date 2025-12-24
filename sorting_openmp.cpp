#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <omp.h>
#include <random>
#include <vector>

using namespace std;
using namespace std::chrono;

// ==================== ПОСЛЕДОВАТЕЛЬНЫЕ АЛГОРИТМЫ ====================

// Сортировка пузырьком (последовательная)
void bubbleSortSequential(vector<int> &arr) {
  int n = arr.size();
  for (int i = 0; i < n - 1; i++) {
    for (int j = 0; j < n - i - 1; j++) {
      if (arr[j] > arr[j + 1]) {
        swap(arr[j], arr[j + 1]);
      }
    }
  }
}

// Сортировка выбором (последовательная)
void selectionSortSequential(vector<int> &arr) {
  int n = arr.size();
  for (int i = 0; i < n - 1; i++) {
    int minIdx = i;
    for (int j = i + 1; j < n; j++) {
      if (arr[j] < arr[minIdx]) {
        minIdx = j;
      }
    }
    if (minIdx != i) {
      swap(arr[i], arr[minIdx]);
    }
  }
}

// Сортировка вставкой (последовательная)
void insertionSortSequential(vector<int> &arr) {
  int n = arr.size();
  for (int i = 1; i < n; i++) {
    int key = arr[i];
    int j = i - 1;
    while (j >= 0 && arr[j] > key) {
      arr[j + 1] = arr[j];
      j--;
    }
    arr[j + 1] = key;
  }
}

// ==================== ПАРАЛЛЕЛЬНЫЕ АЛГОРИТМЫ ====================

// Параллельная сортировка пузырьком (Odd-Even Transposition Sort)
// Классическая сортировка пузырьком плохо параллелится,
// поэтому используем модификацию - чёт-нечёт сортировку
void bubbleSortParallel(vector<int> &arr) {
  int n = arr.size();
  bool sorted = false;

  while (!sorted) {
    sorted = true;

// Нечётная фаза: сравниваем пары (0,1), (2,3), (4,5), ...
#pragma omp parallel for shared(arr, sorted)
    for (int i = 0; i < n - 1; i += 2) {
      if (arr[i] > arr[i + 1]) {
        swap(arr[i], arr[i + 1]);
        sorted = false;
      }
    }

// Чётная фаза: сравниваем пары (1,2), (3,4), (5,6), ...
#pragma omp parallel for shared(arr, sorted)
    for (int i = 1; i < n - 1; i += 2) {
      if (arr[i] > arr[i + 1]) {
        swap(arr[i], arr[i + 1]);
        sorted = false;
      }
    }
  }
}

// Параллельная сортировка выбором
// Каждый поток ищет минимум в своей части, затем выбираем глобальный минимум
void selectionSortParallel(vector<int> &arr) {
  int n = arr.size();

  for (int i = 0; i < n - 1; i++) {
    int minIdx = i;
    int minVal = arr[i];

// Параллельный поиск минимума с использованием reduction
#pragma omp parallel
    {
      int localMinIdx = i;
      int localMinVal = arr[i];

#pragma omp for nowait
      for (int j = i + 1; j < n; j++) {
        if (arr[j] < localMinVal) {
          localMinVal = arr[j];
          localMinIdx = j;
        }
      }

#pragma omp critical
      {
        if (localMinVal < minVal) {
          minVal = localMinVal;
          minIdx = localMinIdx;
        }
      }
    }

    if (minIdx != i) {
      swap(arr[i], arr[minIdx]);
    }
  }
}

// Параллельная сортировка вставкой
// Используем блочный подход: разделяем массив на блоки,
// сортируем каждый блок параллельно, затем сливаем
void insertionSortBlock(vector<int> &arr, int start, int end) {
  for (int i = start + 1; i <= end; i++) {
    int key = arr[i];
    int j = i - 1;
    while (j >= start && arr[j] > key) {
      arr[j + 1] = arr[j];
      j--;
    }
    arr[j + 1] = key;
  }
}

// Слияние двух отсортированных частей
void merge(vector<int> &arr, int left, int mid, int right) {
  vector<int> temp(right - left + 1);
  int i = left, j = mid + 1, k = 0;

  while (i <= mid && j <= right) {
    if (arr[i] <= arr[j]) {
      temp[k++] = arr[i++];
    } else {
      temp[k++] = arr[j++];
    }
  }

  while (i <= mid) {
    temp[k++] = arr[i++];
  }

  while (j <= right) {
    temp[k++] = arr[j++];
  }

  for (int i = 0; i < k; i++) {
    arr[left + i] = temp[i];
  }
}

void insertionSortParallel(vector<int> &arr) {
  int n = arr.size();
  int numThreads = omp_get_max_threads();
  int blockSize = (n + numThreads - 1) / numThreads;

// Параллельная сортировка блоков
#pragma omp parallel for
  for (int i = 0; i < numThreads; i++) {
    int start = i * blockSize;
    int end = min(start + blockSize - 1, n - 1);
    if (start < n) {
      insertionSortBlock(arr, start, end);
    }
  }

  // Последовательное слияние блоков
  for (int size = blockSize; size < n; size *= 2) {
    for (int left = 0; left < n - size; left += 2 * size) {
      int mid = left + size - 1;
      int right = min(left + 2 * size - 1, n - 1);
      merge(arr, left, mid, right);
    }
  }
}

// ==================== ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ====================

// Генерация случайного массива
vector<int> generateRandomArray(int size) {
  vector<int> arr(size);
  random_device rd;
  mt19937 gen(rd());
  uniform_int_distribution<> dis(1, 1000000);

  for (int i = 0; i < size; i++) {
    arr[i] = dis(gen);
  }
  return arr;
}

// Проверка корректности сортировки
bool isSorted(const vector<int> &arr) {
  for (size_t i = 1; i < arr.size(); i++) {
    if (arr[i] < arr[i - 1]) {
      return false;
    }
  }
  return true;
}

// Измерение времени выполнения сортировки
template <typename SortFunc>
double measureTime(SortFunc sortFunc, vector<int> &arr) {
  auto start = high_resolution_clock::now();
  sortFunc(arr);
  auto end = high_resolution_clock::now();
  return duration_cast<microseconds>(end - start).count() /
         1000.0; // в миллисекундах
}

// ==================== ГЛАВНАЯ ФУНКЦИЯ ====================

int main() {
  // Размеры тестовых массивов
  vector<int> sizes = {1000, 10000, 100000};

  cout << "============================================================"
       << endl;
  cout << "  Сравнение последовательных и параллельных алгоритмов" << endl;
  cout << "  сортировки с использованием OpenMP" << endl;
  cout << "============================================================"
       << endl;
  cout << endl;

  // Вывод информации о потоках
  cout << "Количество доступных потоков: " << omp_get_max_threads() << endl;
  cout << endl;

  for (int size : sizes) {
    cout << "------------------------------------------------------------"
         << endl;
    cout << "Размер массива: " << size << " элементов" << endl;
    cout << "------------------------------------------------------------"
         << endl;
    cout << left << setw(25) << "Алгоритм" << setw(20) << "Последоват. (мс)"
         << setw(20) << "Параллельн. (мс)" << setw(15) << "Ускорение" << endl;
    cout << "------------------------------------------------------------"
         << endl;

    // Генерация эталонного массива
    vector<int> original = generateRandomArray(size);

    // ===== СОРТИРОВКА ПУЗЫРЬКОМ =====
    {
      vector<int> arrSeq = original;
      vector<int> arrPar = original;

      double timeSeq = measureTime(bubbleSortSequential, arrSeq);
      double timePar = measureTime(bubbleSortParallel, arrPar);

      bool correctSeq = isSorted(arrSeq);
      bool correctPar = isSorted(arrPar);

      cout << left << setw(25) << "Пузырьком" << setw(20) << fixed
           << setprecision(2) << timeSeq << setw(20) << timePar << setw(15)
           << (timeSeq / timePar) << (correctSeq && correctPar ? " ✓" : " ✗")
           << endl;
    }

    // ===== СОРТИРОВКА ВЫБОРОМ =====
    {
      vector<int> arrSeq = original;
      vector<int> arrPar = original;

      double timeSeq = measureTime(selectionSortSequential, arrSeq);
      double timePar = measureTime(selectionSortParallel, arrPar);

      bool correctSeq = isSorted(arrSeq);
      bool correctPar = isSorted(arrPar);

      cout << left << setw(25) << "Выбором" << setw(20) << fixed
           << setprecision(2) << timeSeq << setw(20) << timePar << setw(15)
           << (timeSeq / timePar) << (correctSeq && correctPar ? " ✓" : " ✗")
           << endl;
    }

    // ===== СОРТИРОВКА ВСТАВКОЙ =====
    {
      vector<int> arrSeq = original;
      vector<int> arrPar = original;

      double timeSeq = measureTime(insertionSortSequential, arrSeq);
      double timePar = measureTime(insertionSortParallel, arrPar);

      bool correctSeq = isSorted(arrSeq);
      bool correctPar = isSorted(arrPar);

      cout << left << setw(25) << "Вставкой" << setw(20) << fixed
           << setprecision(2) << timeSeq << setw(20) << timePar << setw(15)
           << (timeSeq / timePar) << (correctSeq && correctPar ? " ✓" : " ✗")
           << endl;
    }

    cout << endl;
  }

  // Вывод ответов на контрольные вопросы
  cout << "============================================================"
       << endl;
  cout << "                  КОНТРОЛЬНЫЕ ВОПРОСЫ" << endl;
  cout << "============================================================"
       << endl;
  cout << endl;

  cout << "1. Основные отличия алгоритмов сортировки:" << endl;
  cout << "   - Пузырьком: сравнивает соседние элементы, сдвигая большие вправо"
       << endl;
  cout << "   - Выбором: ищет минимум в неотсортированной части и ставит в "
          "начало"
       << endl;
  cout << "   - Вставкой: вставляет элементы в нужную позицию отсортированной "
          "части"
       << endl;
  cout << endl;

  cout << "2. Параллельная сортировка вставкой сложнее, потому что:" << endl;
  cout << "   - Каждый шаг зависит от результата предыдущего шага" << endl;
  cout << "   - Элемент вставляется в уже отсортированную часть массива"
       << endl;
  cout << "   - Требуется блочный подход с последующим слиянием" << endl;
  cout << endl;

  cout << "3. Использованные директивы OpenMP:" << endl;
  cout << "   - #pragma omp parallel for - распараллеливание циклов" << endl;
  cout << "   - #pragma omp parallel - создание параллельного региона" << endl;
  cout << "   - #pragma omp for nowait - распределение итераций без барьера"
       << endl;
  cout << "   - #pragma omp critical - критическая секция" << endl;
  cout << endl;

  cout << "4. Преимущества и недостатки параллельной сортировки:" << endl;
  cout << "   Преимущества: ускорение на больших массивах, эффективное" << endl;
  cout << "   использование многоядерных процессоров" << endl;
  cout << "   Недостатки: накладные расходы на создание/синхронизацию потоков,"
       << endl;
  cout << "   не все алгоритмы хорошо параллелятся" << endl;
  cout << endl;

  cout << "5. Измерение производительности в C++:" << endl;
  cout << "   - Библиотека <chrono> (high_resolution_clock)" << endl;
  cout << "   - Функция omp_get_wtime() из OpenMP" << endl;
  cout << endl;

  cout << "6. Изменение производительности при увеличении числа потоков:"
       << endl;
  cout << "   - Сначала производительность растёт (до числа ядер)" << endl;
  cout << "   - Затем рост замедляется из-за накладных расходов" << endl;
  cout << "   - Закон Амдала ограничивает максимальное ускорение" << endl;
  cout << endl;

  cout << "7. Когда параллельная сортировка менее эффективна:" << endl;
  cout << "   - На маленьких массивах (накладные расходы > выигрыш)" << endl;
  cout << "   - При большом количестве зависимостей между итерациями" << endl;
  cout << "   - На одноядерных процессорах" << endl;
  cout << "   - При частично отсортированных данных (для insertion sort)"
       << endl;
  cout << endl;

  return 0;
}
