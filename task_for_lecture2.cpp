#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_max.h>
#include <cilk/reducer_min.h>
#include <cilk/reducer_vector.h>
#include <chrono>
#include "iostream"

using namespace std::chrono;

/// Функция ReducerMaxTest() определяет максимальный элемент массива,
/// переданного ей в качестве аргумента, и его позицию
/// mass_pointer - указатель исходный массив целых чисел
/// size - количество элементов в массиве
void ReducerMaxTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_max_index<long, int>> maximum;
	cilk_for(long i = 0; i < size; ++i)
	{
		maximum->calc_max(i, mass_pointer[i]);
	}
	printf("Maximal element = %d has index = %d\n\n",
		maximum->get_reference(), maximum->get_index_reference());
}

/// Функция ReducerMinTest() определяет минимальный элемент массива,
/// переданного ей в качестве аргумента, и его позицию
/// mass_pointer - указатель исходный массив целых чисел
/// size - количество элементов в массиве
void ReducerMinTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_min_index<long, int>> minimum;
	cilk_for(long i = 0; i < size; ++i)
	{
		minimum->calc_min(i, mass_pointer[i]);
	}
	printf("Minimal element = %d has index = %d\n\n",
		minimum->get_reference(), minimum->get_index_reference());
}

/// Функция ParallelSort() сортирует массив в порядке возрастания
/// begin - указатель на первый элемент исходного массива
/// end - указатель на последний элемент исходного массива
void ParallelSort(int *begin, int *end)
{
	if (begin != end) 
	{
		--end;
		int *middle = std::partition(begin, end, std::bind2nd(std::less<int>(), *end));
		std::swap(*end, *middle); 
		cilk_spawn ParallelSort(begin, middle);
		ParallelSort(++middle, ++end);
		cilk_sync;
	}
}

/// Функция TimeParallelSort() измеряет время сортировки массива в порядке возрастания
/// begin - указатель на первый элемент исходного массива
/// end - указатель на последний элемент исходного массива
void TimeParallelSort(int *begin, int *end)
{
	high_resolution_clock::time_point t1 = high_resolution_clock::now();

	ParallelSort(begin, end);

	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration<double> duration = (t2 - t1);
	std::cout << "Sorting time: " << duration.count() << " seconds" << std::endl << std::endl;
}

/// Функция CompareForAndCilck_For() выводит время работы for и cilk_for
/// sz - количество элементов в каждом из векторов.
void CompareForAndCilck_For(size_t sz)
{
	std::vector<int> v;
	long i;
	cilk::reducer<cilk::op_vector<int>>red_vec;
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	for (i = 0; i < sz; i++)
	{
		v.push_back(rand() % 20000 + 1);
	}
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration<double> duration = (t2 - t1);
	std::cout << "Duration For is: " << duration.count() << " seconds" << std::endl;

	t1 = high_resolution_clock::now();
	cilk_for(i = 0; i < sz; ++i)
	{
		red_vec->push_back(rand() % 20000 + 1);
	}
	t2 = high_resolution_clock::now();
	duration = (t2 - t1);
	std::cout << "Duration Cilk_For is: " << duration.count() << " seconds" << std::endl;
}

int main()
{
	srand((unsigned)time(0));

	// устанавливаем количество работающих потоков = 4
	__cilkrts_set_param("nworkers", "4");

	long i;
	const long mass_size = 10000*100;
	int *mass_begin, *mass_end;
	int *mass = new int[mass_size];
	long sz = 1000000;

	std::cout<<"Array size: "<< mass_size << std::endl;

	for(i = 0; i < mass_size; ++i)
	{
		mass[i] = (rand() % 25000) + 1;
	}
	
	mass_begin = mass;
	mass_end = mass_begin + mass_size;
	ReducerMaxTest(mass, mass_size);
	ReducerMinTest(mass, mass_size);
	
	TimeParallelSort(mass_begin, mass_end);
	ReducerMaxTest(mass, mass_size);
	ReducerMinTest(mass, mass_size);

	std::cout <<   std::endl << "Vector size: " << sz << std::endl;
	CompareForAndCilck_For(sz);

	delete[]mass;
	return 0;
}