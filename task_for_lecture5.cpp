#include <vector>
#include <thread>
#include <stdio.h>
#include <exception>
#include <locale.h>
#include <cilk/cilk.h>
#include <cilk/reducer_opadd.h>

/// перечисление, определ€ющее как будет происходить вычисление
/// средних значений матрицы: по строкам или по столбцам
enum class eprocess_type
{
	by_rows = 0,
	by_cols
};

void InitMatrix( double** matrix, const size_t numb_rows, const size_t numb_cols )
{
	for ( size_t i = 0; i < numb_rows; ++i )
	{
		for ( size_t j = 0; j < numb_cols; ++j )
		{
			matrix[i][j] = rand() % 5 + 1;
		}
	}
}

/// ‘ункци€ PrintMatrix() печатает элементы матрицы <i>matrix</i> на консоль;
/// numb_rows - количество строк в исходной матрице <i>matrix</i>
/// numb_cols - количество столбцов в исходной матрице <i>matrix</i>
void PrintMatrix( double** matrix, const size_t numb_rows, const size_t numb_cols )
{
	printf( "Generated matrix:\n" );
	for ( size_t i = 0; i < numb_rows; ++i )
	{
		for ( size_t j = 0; j < numb_cols; ++j )
		{
			printf( "%lf ", matrix[i][j] );
		}
		printf( "\n" );
	}
}

/// ‘ункци€ FindAverageValues() находит средние значени€ в матрице <i>matrix</i>
/// по строкам, либо по столбцам в зависимости от значени€ параметра <i>proc_type</i>;
/// proc_type - признак, в зависимости от которого средние значени€ вычисл€ютс€ 
/// либо по строкам, либо по стобцам исходной матрицы <i>matrix</i>
/// matrix - исходна€ матрица
/// numb_rows - количество строк в исходной матрице <i>matrix</i>
/// numb_cols - количество столбцов в исходной матрице <i>matrix</i>
/// average_vals - массив, куда сохран€ютс€ вычисленные средние значени€
void FindAverageValues( eprocess_type proc_type, double** matrix, const size_t numb_rows, const size_t numb_cols, double* average_vals )
{
	switch ( proc_type )
	{
		case eprocess_type::by_rows:
		{
			for ( size_t i = 0; i < numb_rows; ++i )
			{
				double sum( 0.0 );
				for( size_t j = 0; j < numb_cols; ++j )
				{
					sum += matrix[i][j];
				}
				average_vals[i] = sum / numb_cols;
			}
			break;
		}
		case eprocess_type::by_cols:
		{
			for ( size_t j = 0; j < numb_cols; ++j )
			{
				double sum( 0.0 );
				for( size_t i = 0; i < numb_rows; ++i )
				{
					sum += matrix[i][j];
				}
				average_vals[j] = sum / numb_rows;
			}
			break;
		}
		default:
		{
			throw("Incorrect value for parameter 'proc_type' in function FindAverageValues() call!");
		}
	}
}

/// ‘ункци€ PrintAverageVals() печатает элементы массива <i>average_vals</i> на консоль;
/// proc_type - признак, отвечающий за то, как были вычислены 
/// средние значени€ исходной матрицы по строкам или по столбцам
/// average_vals - массив, хран€щий средние значени€ исходной матрицы,
/// вычисленные по строкам или по столбцам
/// dimension - количество элементов в исходной массиве <i>average_vals</i>
void PrintAverageVals( eprocess_type proc_type, double* average_vals, const size_t dimension )
{
	switch ( proc_type )
	{
		case eprocess_type::by_rows:
		{
			printf( "\nAverage values in rows:\n" );
			for ( size_t i = 0; i < dimension; ++i )
			{
				printf( "Row %u: %lf\n", i, average_vals[i] );
			}
			break;
		}
		case eprocess_type::by_cols:
		{
			printf( "\nAverage values in columns:\n" );
			for ( size_t i = 0; i < dimension; ++i )
			{
				printf( "Column %u: %lf\n", i, average_vals[i] );
			}
			break;
		}
		default:
		{
			throw("Incorrect value for parameter 'proc_type' in function PrintAverageVals() call!");
		}
	}
}


int main()
{
	const unsigned ERROR_STATUS = -1;
	const unsigned OK_STATUS = 0;

	unsigned status = OK_STATUS;

	try
	{
		srand( (unsigned) time( 0 ) );

		const size_t numb_rows = 2;
		const size_t numb_cols = 3;

		double** matrix = new double*[numb_rows];
		for ( size_t i = 0; i < numb_rows; ++i )
		{
			matrix[i] = new double[numb_cols];
		}

		double* average_vals_in_rows = new double[numb_rows];
		double* average_vals_in_cols = new double[numb_cols];

		InitMatrix( matrix, numb_rows, numb_cols );

		PrintMatrix( matrix, numb_rows, numb_cols );

		std::thread first_thr( FindAverageValues, eprocess_type::by_rows, matrix, numb_rows, numb_cols, average_vals_in_rows );
		std::thread second_thr( FindAverageValues, eprocess_type::by_cols, matrix, numb_rows, numb_cols, average_vals_in_cols );

		first_thr.join();
		second_thr.join();

		PrintAverageVals( eprocess_type::by_rows, average_vals_in_rows, numb_rows );
		PrintAverageVals( eprocess_type::by_cols, average_vals_in_cols, numb_cols );
		delete[] average_vals_in_rows;
		delete[] average_vals_in_cols;

		for (size_t i = 0; i < numb_rows; ++i)
			delete[] matrix[i];

		delete[] matrix;
	}
	catch ( std::exception& except )
	{
		printf( "Error occured!\n" );
		except.what();
		status = ERROR_STATUS;
	}
	

	getchar();
	return status;
}