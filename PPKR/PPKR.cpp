﻿#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <ctime>
#include <time.h>
#include <Windows.h>
#include <vector>
#include <string>

struct PartOfArray
{
    int* list; 
    int start; 
    int end;
};

using namespace std;


const int P = 10;//число рабочих процессов (не используется в MPI)
const int SIZE_MAS = 20;
const int T = 50;
const int list_size[9]{ 10, 30, 50, 70, 100, 500, 1000, 10000, 100000 };

void merge(int list[], int start, int end, int mid);

void mergeSort(int list[], int start, int end)
{
    int mid;
    if (start < end) {
        mid = (start + end) / 2;
        mergeSort(list, start, mid);
        mergeSort(list, mid + 1, end);
        merge(list, start, end, mid);
    }
}

void mergeSort(PartOfArray* partOfArra)
{
    int* list = partOfArra->list;
    int start = partOfArra->start;
    int end = partOfArra->end;

    int mid;
    if (start < end) {
        mid = (start + end) / 2;
        mergeSort(list, start, mid);
        mergeSort(list, mid + 1, end);
        merge(list, start, end, mid);
    }
}
void merge(int list[], int start, int end, int mid)
{
    int mergedList[100000];
    int i, j, k;
    i = start;
    k = start;
    j = mid + 1;

    while (i <= mid && j <= end) {
        if (list[i] < list[j]) {
            mergedList[k] = list[i];
            k++;
            i++;
        }
        else {
            mergedList[k] = list[j];
            k++;
            j++;
        }
    }

    while (i <= mid) {
        mergedList[k] = list[i];
        k++;
        i++;
    }

    while (j <= end) {
        mergedList[k] = list[j];
        k++;
        j++;
    }

    for (i = start; i < k; i++) {
        list[i] = mergedList[i];
    }
}

void mergeSort_openmp(int list[], int start, int end)
{
    int mid;
    if (start < end) {

        mid = (start + end) / 2;

#pragma omp parallel sections
        {
#pragma omp section
            {
                mergeSort_openmp(list, start, mid);
            }

#pragma omp section
            {
                mergeSort_openmp(list, mid + 1, end);
            }
        }

        merge(list, start, end, mid);
    }
}
std::string print_mas(int mas[])
{
    std::string s = "";
    for (int i = 0; i < SIZE_MAS; i++)
    {
        s += std::to_string(mas[i]) + " ";
    }
    return s;
}

int threadsMax = 0; //макс число потоків
LONG threadsNow = 0;    //поточне число потоків

DWORD WINAPI mergeSort_winapi(LPVOID p)
{
    PartOfArray* params = (PartOfArray*)p;

    int* list = params->list;
    int start = params->start;
    int end = params->end;

    int mid;
    if (start < end) {
        mid = (start + end) / 2;
        PartOfArray* params1 = new PartOfArray[0];
        params1->start = start;
        params1->end = mid;
        PartOfArray* params2 = new PartOfArray[0];
        params2->start = mid + 1;
        params2->end = end;
        if (threadsNow < threadsMax)
        {
            HANDLE  newThread1;
            HANDLE  newThread2;
            InterlockedIncrement(&threadsNow);

            newThread1 = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)mergeSort_winapi, params1, 0, 0);
            WaitForSingleObject(newThread1, INFINITE);

            newThread2 = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)mergeSort_winapi, params2, 0, 0);
            WaitForSingleObject(newThread2, INFINITE);

        }
        else
        {
            mergeSort(params1);
            mergeSort(params2);
        }
    }

    return 0;
}

int main()
{
    setlocale(LC_ALL, "Russian");
    clock_t start, finish;
    double sTime, ompTime, winTime;

    int n = SIZE_MAS;


    int* mas_res_normal = new int[n];
    int* mas_res_openmp = new int[n];
    int* mas_res_winpi = new int[n];


    for (int i = 0; i < n; i++) {
        mas_res_normal[i] = rand();
        mas_res_openmp[i] = mas_res_normal[i];
        mas_res_winpi[i] = mas_res_normal[i];
    }

    printf("Начальный массив:\n");
    std::cout << print_mas(mas_res_normal) << std::endl;
    start = clock(); // начальное время
    mergeSort(mas_res_normal, 0, n - 1);
    finish = clock(); // конечное время
    printf("Массив, отсортированный сортировкой слиянием: \n");
    std::cout << print_mas(mas_res_normal) << std::endl;
    sTime = ((double)(finish - start) / CLOCKS_PER_SEC);
    printf("Время затраченное на сортировку слиянием стандартную%25.10f с\n", sTime);

    printf("Начальный массив:\n");
    std::cout << print_mas(mas_res_openmp) << std::endl;
    start = clock(); // начальное время
    mergeSort_openmp(mas_res_openmp, 0, n - 1);
    finish = clock(); // конечное время
    printf("Массив, отсортированный сортировкой слиянием: \n");
    std::cout << print_mas(mas_res_normal) << std::endl;
    ompTime = ((double)(finish - start) / CLOCKS_PER_SEC);
    printf("Время затраченное на сортировку слиянием OpenMP%25.10f с\n", ompTime);

    printf("Начальный массив:\n");
    std::cout << print_mas(mas_res_openmp) << std::endl;
    start = clock(); // начальное время

    PartOfArray main = { mas_res_winpi, 0, n - 1 };
    mergeSort_winapi(LPVOID(&main));

    finish = clock(); // конечное время
    printf("Массив, отсортированный сортировкой слиянием: \n");
    std::cout << print_mas(mas_res_winpi) << std::endl;
    winTime = ((double)(finish - start) / CLOCKS_PER_SEC);
    printf("Время затраченное на сортировку слиянием OpenMP%25.10f с\n", ompTime);
    
}
