#include <iostream>
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
const int SIZE_MAS = 100000;
const int T = 10;
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

        PartOfArray* params1 = new PartOfArray;
        params1->start = start;
        params1->end = mid;
        params1->list = list;

        mergeSort(params1);
        PartOfArray* params2 = new PartOfArray;
        params2->start = mid + 1;
        params2->end = end;
        params2->list = list;
        mergeSort(params2);
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

void mergeSort_openmp(PartOfArray* params)
{
    int* list = params->list;
    int start = params->start;
    int end = params->end;

    int mid;
    if (start < end) {

        mid = (start + end) / 2;
#pragma omp parallel sections
        {
#pragma omp section
            {
                PartOfArray* params1 = new PartOfArray;
                params1->start = start;
                params1->end = mid;
                params1->list = list;
                mergeSort_openmp(params1);
            }
#pragma omp section
            {
                PartOfArray* params2 = new PartOfArray;
                params2->start = mid + 1;
                params2->end = end;
                params2->list = list;
                mergeSort_openmp(params2);
            }
            merge(list, start, end, mid);
        }
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

int threadsMax = T;
LONG threadsNow = 1;

DWORD WINAPI mergeSort_winapi(LPVOID p)
{
    PartOfArray* params = (PartOfArray*)p;

    int* list = params->list;
    int start = params->start;
    int end = params->end;

    int count = end - start;

    int mid;
    if (start < end && list!=NULL) {
        mid = (start + end) / 2;
        PartOfArray* params1 = new PartOfArray;
        params1->start = start;
        params1->end = mid;
        params1->list = list;
        PartOfArray* params2 = new PartOfArray;
        params2->start = mid + 1;
        params2->end = end;
        params2->list = list;


        if (threadsNow < threadsMax)
        {
            HANDLE  newThread1;
            HANDLE  newThread2;
            InterlockedIncrement(&threadsNow);

            newThread1 = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)mergeSort_winapi, params1, 0, 0);

            InterlockedIncrement(&threadsNow);

            newThread2 = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)mergeSort_winapi, params2, 0, 0);

            WaitForSingleObject(newThread1, INFINITE);
            WaitForSingleObject(newThread2, INFINITE);
        }
        else {
            mergeSort(params1);
            mergeSort(params2);
        }
        merge(list, start, end, mid);
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
    //std::cout << print_mas(mas_res_normal) << std::endl;
    start = clock(); // начальное время
    mergeSort(mas_res_normal, 0, n - 1);
    finish = clock(); // конечное время
    printf("Массив, отсортированный сортировкой слиянием: \n");
    //std::cout << print_mas(mas_res_normal) << std::endl;
    sTime = ((double)(finish - start) / CLOCKS_PER_SEC);
    printf("Время затраченное на сортировку слиянием стандартную%25.10f с\n\n\n", sTime);

    printf("Начальный массив:\n");
    //std::cout << print_mas(mas_res_openmp) << std::endl;
    
    PartOfArray main_openmp = { mas_res_openmp, 0, n - 1 };
    start = clock(); // начальное время
    mergeSort_openmp(&main_openmp);
    finish = clock(); // конечное время
    printf("Массив, отсортированный сортировкой слиянием: \n");
    //std::cout << print_mas(mas_res_normal) << std::endl;
    ompTime = ((double)(finish - start) / CLOCKS_PER_SEC);
    printf("Время затраченное на сортировку слиянием OpenMP%25.10f с\n", ompTime);

    bool bl2 = true;
    for (int i = 0; i < n; i++) {
        if (mas_res_normal[i] != mas_res_openmp[i])
            bl2 = false;
    }
    if (bl2)
        printf("Массив отсортирован верно с помощью OpenMP\n\n\n");
    else
        printf("Массив отсортирован неверно с помощью OpenMP\n\n\n");

    printf("Начальный массив:\n");
    //std::cout << print_mas(mas_res_winpi) << std::endl;

    PartOfArray main = { mas_res_winpi, 0, n - 1 };
    start = clock(); // начальное время
    mergeSort_winapi(LPVOID(&main));

    finish = clock(); // конечное время
    printf("Массив, отсортированный сортировкой слиянием: \n");
    //std::cout << print_mas(mas_res_winpi) << std::endl;
    winTime = ((double)(finish - start) / CLOCKS_PER_SEC);
    printf("Время затраченное на сортировку слиянием WinAPI%25.10f с\n", winTime);
    
    bool bl = true;
    for (int i = 0; i < n; i++) {
        if (mas_res_normal[i] != mas_res_winpi[i])
            bl = false;
    }
    if (bl)
        printf("Массив отсортирован верно с помощью WinAPI\n\n\n");
    else 
        printf("Массив отсортирован неверно с помощью WinAPI\n\n\n");
}

