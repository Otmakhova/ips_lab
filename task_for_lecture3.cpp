#include <stdio.h>
#include <ctime>
#include <cilk/cilk.h>
#include <cilk/reducer_opadd.h>
#include <chrono>
#include <iostream>

using namespace std::chrono;

// количество строк в исходной квадратной матрице
const int MATRIX_SIZE = 1500;
// кол-во строк в матрице, приводимой в качестве примера
const int test_matrix_lines = 4;

/// Функция InitMatrix() заполняет переданную в качестве 
/// параметра квадратную матрицу случайными значениями
/// matrix - исходная матрица СЛАУ
void InitMatrix( double** matrix )
{
	for ( int i = 0; i < MATRIX_SIZE; ++i )
	{
		matrix[i] = new double[MATRIX_SIZE + 1];
	}

	for ( int i = 0; i < MATRIX_SIZE; ++i )
	{
		for ( int j = 0; j <= MATRIX_SIZE; ++j )
		{
			matrix[i][j] = rand() % 2500 + 1;
		}
	}
}

/// Функция InitTestMatrix() заполняет переданную в качестве 
/// параметра квадратную матрицу случайными значениями
/// test_matrix - исходная матрица СЛАУ
void InitTestMatrix(double** test_matrix) 
{
	// цикл по строкам
	for (int i = 0; i < test_matrix_lines; ++i)
	{
		// (test_matrix_lines + 1)- количество столбцов в тестовой матрице,
		// последний столбец матрицы отведен под правые части уравнений, входящих в СЛАУ
		test_matrix[i] = new double[test_matrix_lines + 1];
	}

	// инициализация тестовой матрицы
	test_matrix[0][0] = 2; test_matrix[0][1] = 5;  test_matrix[0][2] = 4;  test_matrix[0][3] = 1;  test_matrix[0][4] = 20;
	test_matrix[1][0] = 1; test_matrix[1][1] = 3;  test_matrix[1][2] = 2;  test_matrix[1][3] = 1;  test_matrix[1][4] = 11;
	test_matrix[2][0] = 2; test_matrix[2][1] = 10; test_matrix[2][2] = 9;  test_matrix[2][3] = 7;  test_matrix[2][4] = 40;
	test_matrix[3][0] = 3; test_matrix[3][1] = 8;  test_matrix[3][2] = 9;  test_matrix[3][3] = 2;  test_matrix[3][4] = 37;
}


/// Функция SerialGaussMethod() решает СЛАУ методом Гаусса 
/// matrix - исходная матрица коэффиициентов уравнений, входящих в СЛАУ,
/// последний столбей матрицы - значения правых частей уравнений
/// rows - количество строк в исходной матрице
/// result - массив ответов СЛАУ
double SerialGaussMethod( double **matrix, const int rows, double* result )
{
	int k;
	double koef;

	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	// прямой ход метода Гаусса
	for ( k = 0; k < rows; ++k )
	{
		for ( int i = k + 1; i < rows; ++i )
		{
			koef = -matrix[i][k] / matrix[k][k];

			for ( int j = k; j <= rows; ++j )
			{
				matrix[i][j] += koef * matrix[k][j];
			}
		}
	}
	high_resolution_clock::time_point t2 = high_resolution_clock::now();

	duration<double> duration = (t2 - t1);
	std::cout << "Duration forward elimination is: " << duration.count() << " seconds" << std::endl;

	// обратный ход метода Гаусса
	result[rows - 1] = matrix[rows - 1][rows] / matrix[rows - 1][rows - 1];

	for ( k = rows - 2; k >= 0; --k )
	{
		result[k] = matrix[k][rows];

		//
		for ( int j = k + 1; j < rows; ++j )
		{
			result[k] -= matrix[k][j] * result[j];
		}

		result[k] /= matrix[k][k];
	}
	return duration.count();
}

/// Функция ParallelGaussMethod() решает СЛАУ методом Гаусса с использованием параллелизма
/// matrix - исходная матрица коэффиициентов уравнений, входящих в СЛАУ,
/// последний столбей матрицы - значения правых частей уравнений
/// rows - количество строк в исходной матрице
/// result - массив ответов СЛАУ
double ParallelGaussMethod(double** matrix, const int rows, double* result)
{
	int k;
	high_resolution_clock::time_point t1 = high_resolution_clock::now();

	// прямой ход метода Гаусса
	for (k = 0; k < rows; ++k)
	{
		cilk_for(int i = k + 1; i < rows; ++i)
		{
			double koef = -matrix[i][k] / matrix[k][k];

			for (int j = k; j <= rows; ++j)
			{
				matrix[i][j] += koef * matrix[k][j];
			}
		}
	}

	high_resolution_clock::time_point t2 = high_resolution_clock::now();

	duration<double> duration = (t2 - t1);
	std::cout << "Duration parallel forward elimination is: " << duration.count() << " seconds" << std::endl;

	// обратный ход метода Гаусса
	result[rows - 1] = matrix[rows - 1][rows] / matrix[rows - 1][rows - 1];

	for (k = rows - 2; k >= 0; --k)
	{
		cilk::reducer_opadd<double> resultTmp(matrix[k][rows]);

		cilk_for(int j = k + 1; j < rows; ++j)
		{
			resultTmp -= matrix[k][j] * result[j];
		}

		result[k] = resultTmp.get_value() / matrix[k][k];
	}

	return duration.count();
}

int main()
{
	srand( (unsigned) time( 0 ) );

	/*
	// массив решений СЛАУ
	double *result = new double[test_matrix_lines];
	double **test_matrix = new double*[test_matrix_lines];
	

	InitTestMatrix(test_matrix);
	SerialGaussMethod( test_matrix, test_matrix_lines, result );

	for ( i = 0; i < test_matrix_lines; ++i )
	{
		delete[]test_matrix[i];
	}

	printf( "Solution:\n" );

	for ( i = 0; i < test_matrix_lines; ++i )
	{
		printf( "x(%d) = %lf\n", i, result[i] );
	}

	delete[] result;
	delete[] test_matrix;
	*/

	double **parallel_matrix = new double*[MATRIX_SIZE];
	double **serial_matrix = new double*[MATRIX_SIZE];

	double *parallel_result = new double[MATRIX_SIZE];
	double *serial_result = new double[MATRIX_SIZE];

	InitMatrix(parallel_matrix);
	InitMatrix(serial_matrix);

	double d1 = ParallelGaussMethod(parallel_matrix, MATRIX_SIZE, parallel_result);
	double d2 = SerialGaussMethod(serial_matrix, MATRIX_SIZE, serial_result);
	std::cout << "Boost: " << d2 / d1 << std::endl;

	for (int i = 0; i < MATRIX_SIZE; ++i)
	{
		delete[]parallel_matrix[i];
		delete[]serial_matrix[i];
	}

	delete[] parallel_matrix;
	delete[] serial_matrix;
	delete[] parallel_result;
	delete[] serial_result;

	return 0;
}