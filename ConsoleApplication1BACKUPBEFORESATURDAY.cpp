// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include <omp.h>
#include <iostream>
#include <fstream>
#include <cstdio>
using namespace std;

int number_bacteria;
char** bacteria_name;
long M, M1, M2;

//1d array to  parallel 2d array 
double **t;
long** vector;
long** second;



short code[27] = { 0, 2, 1, 2, 3, 4, 5, 6, 7, -1, 8, 9, 10, 11, -1, 12, 13, 14, 15, 16, 1, 17, 18, 5, 19, 3 };
#define encode(ch)		code[ch-'A']
#define LEN				6
#define AA_NUMBER		20
#define	EPSILON			1e-010
#define THREADS			8

void Init()
{
	M2 = 1;
	for (int i = 0; i<LEN - 2; i++)	// M2 = AA_NUMBER ^ (LEN-2);
		M2 *= AA_NUMBER;
	M1 = M2 * AA_NUMBER;		// M1 = AA_NUMBER ^ (LEN-1);
	M = M1 *AA_NUMBER;			// M  = AA_NUMBER ^ (LEN);\

	/*
		t = (double**)malloc(sizeof(double*) * M);
	for (int i = 0; i<omp_get_num_threads(); i++)
		t[i] = (double*)malloc(sizeof(double) * M);

		OR
		*/
	
		t = new double*[THREADS];
		vector = new long*[THREADS];
		second = new long*[THREADS];


		for (int i = 0; i < THREADS; ++i) {
			t[i] = new double[M];
			vector[i] = new long[M];
			second[i] = new long[M];
		}
		
		printf("Memory has been allocated for the 2d arrays /n");

}




class Bacteria
{
	int threadNumber = omp_get_thread_num();//////////




private:
	//long* vector;
	//long* second;
	long one_l[AA_NUMBER];
	long indexs;
	long total;
	long total_l;
	long complement;

	void InitVectors()
	{
		//vector = new long[M];
		//second = new long[M1];
		memset(vector[threadNumber], 0, M * sizeof(long));
		memset(second[threadNumber], 0, M1 * sizeof(long));
		memset(one_l, 0, AA_NUMBER * sizeof(long));
		total = 0;
		total_l = 0;
		complement = 0;

	}

	void init_buffer(char* buffer)
	{
		complement++;
		indexs = 0;
		for (int i = 0; i<LEN - 1; i++)
		{
			short enc = encode(buffer[i]);
			one_l[enc]++;
			total_l++;
			indexs = indexs * AA_NUMBER + enc;
		}
		second[threadNumber][indexs]++;
	}

	void cont_buffer(char ch)
	{
		short enc = encode(ch);
		one_l[enc]++;
		total_l++;
		long index = indexs * AA_NUMBER + enc;
		vector[threadNumber][index]++;
		total++;
		indexs = (indexs % M2) * AA_NUMBER + enc;
		second[threadNumber][indexs]++;
	}

public:
	long count;
	double* tv;
	long *ti;

	Bacteria(char* filename)
	{
		//omp_get_thread_num(); // O only run the function omp_get_thread_num() once

		FILE * bacteria_file = fopen(filename, "r");
		InitVectors();

		char ch;
		while ((ch = fgetc(bacteria_file)) != EOF)
		{
			if (ch == '>')
			{
				while (fgetc(bacteria_file) != '\n'); // skip rest of line

				char buffer[LEN - 1];
				fread(buffer, sizeof(char), LEN - 1, bacteria_file);
				init_buffer(buffer);
			}
			else if (ch != '\n')
				cont_buffer(ch);
		}

		

		long total_plus_complement = total + complement;
		double total_div_2 = total * 0.5;
		int i_mod_aa_number = 0;
		int i_div_aa_number = 0;
		long i_mod_M1 = 0;
		long i_div_M1 = 0;

		double one_l_div_total[AA_NUMBER];
		for (int i = 0; i<AA_NUMBER; i++)
			one_l_div_total[i] = (double)one_l[i] / total_l;

		double* second_div_total = new double[M1];
		for (int i = 0; i<M1; i++)
			second_div_total[i] = (double)second[threadNumber][i] / total_plus_complement;

		count = 0;
	//	double* t = new double[M]; // move to global

		//t[omp_get_thread_num()];

		
		for (long i = 0; i<M; i++) //M = 64 million
		{
			double p1 = second_div_total[i_div_aa_number]; //TODO:: INITIATE INSIDE LOOP CAUSING MORE WORK?
			double p2 = one_l_div_total[i_mod_aa_number];
			double p3 = second_div_total[i_mod_M1];
			double p4 = one_l_div_total[i_div_M1];
			double stochastic = (p1 * p2 + p3 * p4) * total_div_2;
			
			
			if (i_mod_aa_number == AA_NUMBER - 1)
			{
				i_mod_aa_number = 0;
				i_div_aa_number++;
			}
			else
				i_mod_aa_number++;

			if (i_mod_M1 == M1 - 1)
			{
				i_mod_M1 = 0;
				i_div_M1++;
			}
			else
				i_mod_M1++;



			//WILL ALWAYS OVERWRITE ALL VALUES OF t[i]
			if (stochastic > EPSILON)
			{
				//printf("%d", threadNumber);
				t[threadNumber][i] = (vector[threadNumber][i] - stochastic) / stochastic;
				count++;
			}
			else
				//printf("%d",threadNumber);
				t[threadNumber][i] = 0;
		}

		tv = new double[count];
		ti = new long[count];

		int pos = 0;
		//TODO: SAME LOOP AS THE ONE BEFORE
		//BOTH TAKES ABOUT 1SEC TO RUN
		for (long i = 0; i<M; i++)
		{
			if (t[threadNumber][i] != 0)
			{
				tv[pos] = t[threadNumber][i];
				ti[pos] = i;
				pos++;
			}
		}
		delete second_div_total;
		
		
		//delete vector;
		//delete second;
		
		//delete t; //REMOVED


		//time_t a2 = time(NULL);
		//printf("End loop +%d", a2 - a1);
		fclose(bacteria_file);

		//t[threadNumber][]; dealocate t to prevent memory leak
	}
};

void ReadInputFile(char* input_name)
{
	FILE* input_file = fopen(input_name, "r");
	fscanf(input_file, "%d", &number_bacteria);
	bacteria_name = new char*[number_bacteria];

	fscanf(input_file, "%d", &number_bacteria);

	//This loop does not need to be parallelized 
	for (long i = 0; i<number_bacteria; i++)
	{
		bacteria_name[i] = new char[20];
		fscanf(input_file, "%s", bacteria_name[i]);
		strcat(bacteria_name[i], ".faa");
	}
	fclose(input_file);
	//Creates a list bacteria_name[i] where i is the filename.faa for the data folder
	//

}

double CompareBacteria(Bacteria* b1, Bacteria* b2)
{
	double correlation = 0;
	double vector_len1 = 0;
	double vector_len2 = 0;
	long p1 = 0;
	long p2 = 0;		
	
	
	
	while (p1 < b1->count && p2 < b2->count) //-> pointing to the count value of the object. Type long.
	{
		long n1 = b1->ti[p1];
		long n2 = b2->ti[p2];
		if (n1 < n2)
		{
			double t1 = b1->tv[p1];
			vector_len1 += (t1 * t1);
			p1++;
		}
		else if (n2 < n1)
		{
			double t2 = b2->tv[p2];
			p2++;
			vector_len2 += (t2 * t2);
		}
		else
		{
			double t1 = b1->tv[p1++];
			double t2 = b2->tv[p2++];
			vector_len1 += (t1 * t1);
			vector_len2 += (t2 * t2);
			correlation += t1 * t2;
		}
	}

	//does it make sence to have their own while loops for this
	//or can I include it in the above one for faster computation?
	while (p1 < b1->count)
	{
		long n1 = b1->ti[p1];
		double t1 = b1->tv[p1++];
		vector_len1 += (t1 * t1);
	}
	while (p2 < b2->count)
	{
		long n2 = b2->ti[p2];
		double t2 = b2->tv[p2++];
		vector_len2 += (t2 * t2);
	}


	return correlation / (sqrt(vector_len1) * sqrt(vector_len2));
}

void CompareAllBacteria()
{
	//bacteria_name[i] stores filename.faa for the data folder
	Bacteria** b = new Bacteria*[number_bacteria];

	//TODO1:: parallelize so that Bacteria() runs in multiple instances.
#pragma omp parallel for
	for (int i = 0; i<number_bacteria; i++)
	{
		printf("load %d of %d RUNNING ON THREAD: %d \n", i + 1, number_bacteria, omp_get_thread_num());
		b[i] = new Bacteria(bacteria_name[i]);
	}



	printf("Prossesing and writing data to file...");



	/*
	Kan kanskje gjoore om for lokken her. 
	i=0 men teller bare opp til i<40
	j=i+1 men teller helt opp til j<41
	Noe kan gjoores med denne logikken slik at den blir lettere kanskje j<number kan ta -1? 
	
	i=0 j=1
	i=0 j=2
	i=0 j=3
	i=0 j=4
	i=0 j=5
	i=0 j=6

	i=1 j=1
	i=1 j=2
	i=1 j=3
	i=1 j=4
	i=1 j=5
	i=1 j=6

	i ooker fra 0->5
	i ooker fra 1->6
	alltid 1 verdi hooyere i j. 
	
	sammenligner hver bakterie med den neste. i rekken.
	kanskje jeg kan gjoore om til en for lookke

	Denne looken blir isaafall mye lettere aa parallelisere

	*/

	/* ORIGINAL LOOP
	for (int i = 0; i<number_bacteria - 1; i++)
		for (int j = i + 1; j<number_bacteria; j++)
		{
			//printf("%2d %2d -> ", i, j);

			//Only printed to file
			fprintf(fp, "%2d %2d -> ", i, j);
			double correlation = CompareBacteria(b[i], b[j]);
			fprintf(fp, "%.20lf\n", correlation);

			//printf("%.20lf\n", correlation);
		}
	
	*/

	FILE *fp;
	fp = fopen("thisOutput.txt", "w");


	double**A = (double**) malloc(sizeof(double*) * number_bacteria - 1);
	for (int i = 0; i<number_bacteria-1; i++)
		A[i] = (double*)malloc(sizeof(double) * number_bacteria - 1);


	//A[39][39] = 42;
#pragma omp parallel for
	for (int i = 0; i < number_bacteria - 1; i++) {
		int counter = 0;
		for (int j = i + 1; j<number_bacteria; j++)
		{

			double correlation = CompareBacteria(b[i], b[j]);
			A[i][j+counter] = correlation;
		}
		counter++;
	}
	
	//Output in order of 2d array
	for (int i = 0; i < number_bacteria - 1; i++) {
		int counter = 0;
		for (int j = i + 1; j<number_bacteria; j++)
		{
			fprintf(fp, "%2d %2d -> %.20lf\n", i, j, A[i][j + counter]);
		}
		counter++;
	}

	fclose(fp);


	/*DELETE the global 2d parallel arrays
	TODO:: PREVENT MEMORY LEAK
		
		delete vector;
		delete second;

		delete t;
	*/
	
}

void main(int argc, char * argv[])
{
//#pragma omp parallel for
//	for(int i = 0; i<8; i++)
//	{
//		printf("hello world from thread %d \n",omp_get_thread_num());
//	};
	omp_set_num_threads(THREADS);
	time_t t1 = time(NULL);

	Init();
	ReadInputFile(argv[1]);
	CompareAllBacteria();

	time_t t2 = time(NULL);
	printf("time elapsed: %d seconds\n\n\n", t2 - t1);
	printf("\n\n\n\n Check for difference between original output and parallel output: \n\n");
	//
	system("FC H:\\CAB401\\CAB401_CVTREE_PARALLEL\\ConsoleApplication1\\ConsoleApplication1\\originalOutput.txt H:\\CAB401\\CAB401_CVTREE_PARALLEL\\ConsoleApplication1\\ConsoleApplication1\\thisOutput.txt");
	system("pause");
}