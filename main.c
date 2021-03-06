/*------------------------------*/
/*Autor: Matej Delincak*/
/*Zadanie 1 - Spravca pamati*/
/*------------------------------*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include<time.h>

#define VYPIS 1 //ak nula tak vypisuje co sa deje s pamatou
#define TYPE *(memory-2)    //velkost modu (char, short alebo int)
#define NEXT_P(p) ((p) + TYPE)  //offset dalsieho volneho bloku
#define PREVIOUS_P(p) ((p) + 2*TYPE)    //offset predchadzajuceho volneho bloku
#define CURR_FOOTER(p, size) writeToArr(((p) + abs(size) + TYPE), readFromArr(p)) //nastavi patu podla toho aka je hlavicka
#define NEXT_BLOCK(p) ((p) + 2*TYPE + abs(readFromArr(p)))  //blok vpravo od daneho bloku
#define PREVIOUS_BLOCK(p) ((p) - abs(readFromArr((p)-TYPE)) - 2* TYPE)   //blok vlavo od daneho bloku

//smernik na zaciatok vyhradenej pamate
char* memory = NULL;

//funkcia poskytuje zapis do pamate na zaklade modu(char, short a int)
void writeToArr(long long paOffset, long long paVal)
{
    switch (TYPE)
    {
        case 1:{*((char*)(memory + paOffset))= (char)paVal; break;}
        case 2:{*((short*)(memory + paOffset)) = (short)paVal; break;}
        case 4:{*((int*)(memory + paOffset)) = (int)paVal; break;}
        case 8:{*((long long*)(memory + paOffset)) = (long long)paVal; break;}
    }
}
//funkcia poskytuje citanie z pamate na zaklade modu(char, short a int)
long long readFromArr(long long paOffset)
{
    switch (TYPE)
    {
        case 1:{return *((char*)(memory + paOffset));}
        case 2:{return *((short*)(memory + paOffset));}
        case 4:{return *((int*)(memory + paOffset));}
        case 8:{return *((long long*)(memory + paOffset));}
    }
}
//funkcia vrati cislo konkretneho segregovaneho zoznamu na zaklade velkosti
char blockNumber(long long paSize)
{
    if (paSize < 63) return 0;
    else
        return ((char)(round(log2 (paSize)))-3);
}
//funkcia vymaze blok pamate z jeho priradeneho zoznamu
void deleteBlock(long long paAct)
{
    if (readFromArr(PREVIOUS_P(paAct)) < *(memory - 1) * TYPE)
        writeToArr(readFromArr(PREVIOUS_P(paAct)), readFromArr(NEXT_P(paAct)));
    else
        writeToArr(NEXT_P(readFromArr(PREVIOUS_P(paAct))), readFromArr(NEXT_P(paAct)));

    if (readFromArr(NEXT_P(paAct)) != -1)
        writeToArr(PREVIOUS_P(readFromArr(NEXT_P(paAct))), readFromArr(PREVIOUS_P(paAct)));
    //vymazanie smernikov bloku
    writeToArr(NEXT_P(paAct), -1);
    writeToArr(PREVIOUS_P(paAct), -1);
}
//funkcia zaradi dany blok na zaciatok jeho priradeneho zoznamu
void insertBlock(long long paAct)
{
    int rankOfList = blockNumber(readFromArr(paAct));
    writeToArr(NEXT_P(paAct), readFromArr(rankOfList* TYPE));
    writeToArr(rankOfList* TYPE, paAct);
    if (readFromArr(NEXT_P(paAct)) != -1)
    {
        writeToArr(PREVIOUS_P(readFromArr(NEXT_P(paAct))), paAct);
    }
    writeToArr(PREVIOUS_P(paAct), rankOfList*TYPE);
}
//funckia spoji dva bloky, ktore idu za sebou v pamati
long long mergeBlocks(long long paFirst, long long paSecond)
{
    //ak este nebol vymazany zo zoznamu, tak ho vymaz
    if ((readFromArr(NEXT_P(paFirst)) != -1) || (readFromArr(PREVIOUS_P(paFirst)) != -1))
        deleteBlock(paFirst);
    //spoj bloky
    writeToArr(paFirst, readFromArr(paFirst) + 2*TYPE + readFromArr(paSecond));
    CURR_FOOTER(paFirst, readFromArr(paFirst));
    //vymaz obsah oboch blokov
    memset(memory + paFirst + TYPE, -1, abs(readFromArr(paFirst)));
    return paFirst;
}
//funkcia prejde konkretny zoznam(na zaklade prveho volneho bloku v nom) a najde blok, ktory ma najefektivnejsiu velkost
long long bestFit(long long paSze)
{
    long long best = 0, act = 0;
    //zisti spravny zoznam
    int rankOfList = blockNumber(paSze);
    while (readFromArr(rankOfList* TYPE) == -1)
        rankOfList++;
    if (rankOfList >= *(memory - 1)) return 0;
    best = act = readFromArr(rankOfList* TYPE);
    //********************

    while (readFromArr(NEXT_P(act)) != -1)
    {
        //chod na dalsi blok v zozname
        act = readFromArr(NEXT_P(act));
        if (readFromArr(act) < paSze) continue;
        if (readFromArr(best) < paSze) best = act;
        if (readFromArr(act) == paSze) return act;
        if (readFromArr(act) - paSze < readFromArr(best) - paSze) best = act;
    }
    //ak v zozname nebol dostatocne velky blok, prehladaj este zoznam s vacsimi
    if (readFromArr(best) < paSze)
    {
        rankOfList++;
        while (readFromArr(rankOfList* TYPE) == -1)
            rankOfList++;
        if (rankOfList >= *(memory - 1)) return 0;
        act = best = readFromArr(rankOfList* TYPE);
        while (readFromArr(NEXT_P(act)) != -1)
        {
            //chod na dalsi blok v zozname
            act = readFromArr(NEXT_P(act));
            if (readFromArr(act) - paSze < readFromArr(best) - paSze) best = act;
        }
    }
    return best;
}
//funkcia rozdeli konkretny blok na blok velkosti size a na blok zvysnej pamate
long long split(long long paAct, unsigned int paSize)
{
    long long new = paAct + 2* TYPE + paSize;
    //nastav hlavicku a patu novemu bloku
    writeToArr(new, readFromArr(paAct) - 2* TYPE - paSize);
    CURR_FOOTER(new, readFromArr(new));
    //nastav hlavicku a patu staremu bloku
    writeToArr(paAct, -paSize);
    CURR_FOOTER(paAct, paSize);
    //vymaz stary z konkretneho zoznamu
    deleteBlock(paAct);
    //pridaj novy do spravneho zoznamu
    insertBlock(new);
    return paAct;
}

void *memory_alloc(unsigned int size)
{
    long long act = -1;
    if (size <= 0) return NULL;
    if (size < 8) size = 8;
    if (blockNumber(size) > *(memory-1)) return NULL;
    if ((act = bestFit(size)) == 0)
    {
        if (VYPIS == 0)
            printf("Nealokovane - nedostatok miesta %d\n", size);
        return NULL;
    }

    //ak sa blok da rozdelit, resp. aby novy vzniknuty blok nemal menej ako 8 bajtov
    if (readFromArr(act) >= size + 2* TYPE + 8)
    {
        if (VYPIS == 0)
            printf("Alokovane %d\n", size);
        return (void *) (memory + split(act, size) + TYPE);
    }
    else
    {
        //ak ma presnu velkost tak ho len prirad a nerozdeluj ho
        if (VYPIS == 0)
            printf("Alokovane %d\n", readFromArr(act));
        writeToArr(act, -readFromArr(act));
        CURR_FOOTER(act, readFromArr(act));
        deleteBlock(act);
        return (void*)(memory + act + TYPE);
    }
}
int memory_check(void *ptr)
{
    if ((ptr == NULL) || ((char*)ptr < memory)) return 0;

    char exit = 0;
    long long i = *(memory-1) * TYPE;
    //prejde implicitne vsetky bloky, aby zistil ci smernik ukazuje na nejaky blok
    while (readFromArr(i) != 0) {
        if (memory + i == (char*)ptr - TYPE)
        {
            exit = 1;
            break;
        }
        if (readFromArr(i) < 0)
           i = i - readFromArr(i) + 2 * TYPE;
        else
            i = i + readFromArr(i) + 2 * TYPE;
    }
    if (exit == 0) return 0;
    long long act = ((char*)ptr - TYPE) - memory;//nastavi sa na zaciatok celeho bloku
    //porovna hlavu a patu, ak sa cisla rovnaju, je obrovska pravdepodobnost, ze ide prave o moj spracovavany blok + este nebol uvolneny
    if ((readFromArr(act) == readFromArr(act + TYPE + abs(readFromArr(act)))) && (readFromArr(act) < 0))  return 1;
    return 0;
}
int memory_free(void *valid_ptr)
{
    //nemusim testovat lebo v zadani bolo ze pride vzdy platny, ale aj tak otestujem
    if (!(memory_check(valid_ptr))) return 1;

    long long act = ((char*)valid_ptr - TYPE) - memory;
    long long next = NEXT_BLOCK(act);
    //priprav blok na uvolnenie
    writeToArr(act, -readFromArr(act));
    CURR_FOOTER(act, readFromArr(act));
    if (VYPIS == 0)
        printf("Uvolnene %d    ", readFromArr(act));
    //ak je blok, PRED uvolnovanym blokom, volny, tak ich mergni
    if ((readFromArr(act - TYPE) != 0)&&(act > *(memory-1)* TYPE))
    {
        if (readFromArr(PREVIOUS_BLOCK(act)) > 0)
            act = mergeBlocks(PREVIOUS_BLOCK(act), act);
    }
    //ak je blok, ZA uvolnovanym blokom, volny, tak ich mergni
    if (readFromArr(next) != 0)
    {
        if (readFromArr(next) > 0){
            deleteBlock(next);
            act = mergeBlocks(act, next);
        }

    }
    //pridaj mergnuty blok
    insertBlock(act);
    if (VYPIS == 0)
        printf("Spojene na %d\n", readFromArr(act));
    return 0;
}
void memory_init(void *ptr, unsigned int size)
{
    char numOfLists = blockNumber(size)+1;
    //vymazanie pola
    for (long long i = 0; i < size; i++)
    {
        *((char*)ptr+ i) = -1;
    }
    //zisti s akym modom sa bude pracovat a zapis ho na prve miesto pamate
        if (size < 128)
            *((char*)ptr) = sizeof(char);
        else if (size < 32767)
                *((char*)ptr) = sizeof(short);
            else if(size < 2147483647)
                    *((char*)ptr) = sizeof(int);
                  else
                    *((char*)ptr) = sizeof(long long);
    //***********************************
    //na druhe miesto pamate zapis pocet samostatnych zoznamov
    *((char*)ptr+1) = numOfLists;
    //globalny smernik bude ukazovat az na tretie miesto pamate
    memory = (char*)ptr+2;

    //resetuj zoznamy na "NULL", resp. -1
    for (int i = 0; i < numOfLists; i++)
    {
        writeToArr(i * TYPE, -1);
    }
    //nastav jeden zoznam aby ukazoval na prvy blok
    writeToArr((numOfLists-1) * TYPE,  numOfLists * TYPE);
    //na koniec pamate zapisem nulu, aby som vedel pri free, ze som na konci
    writeToArr(size - 2 -TYPE,  0);

    //vytvorenie prveho bloku
    //blok obsahuje velkost, offset na dalsi volny blok a offset na predchadzajuci volny blok
    long long act = numOfLists * TYPE;
    writeToArr(act, size - 2 - 3* TYPE - numOfLists* TYPE);
    CURR_FOOTER(act, readFromArr(act));
    writeToArr(act + TYPE, -1);
    writeToArr(act + 2 * TYPE, (numOfLists-1)*TYPE);
}

/*TESTOVACIA CAST*/

void print(long long size)
{
    if (VYPIS != 0) return;
    long long shift = 0;
    for (long long i = 0; i < size - (blockNumber(size)+1)*TYPE - 2; i = i + 2*TYPE + abs(shift))
    {
        shift = readFromArr((blockNumber(size)+1)*TYPE + i);
        if (shift > 0) printf("%ld =>", shift);
    }
    printf("\n");
}

/*TEST1 - PAMAT 50 - ROVNAKE BLOKY 8 A ICH UVOLNENIE,
 *                   POTOM ALOKOVANIE 24, 8, 8, UVOLNENIE 24 A ALOKOVANIE ZASE 8*/
void test1(void)
{
    char region[50];
    memory_init(region, 50);

    char* list[5];
    int x;
    print(50);
    for (int i = 0; i < 5; i++) {
        list[i] = (char*)memory_alloc(8);//tu
    }
    for (int i = 0; i < 5; i++) {
        if (list[i] != NULL)
            x=memory_free(list[i]);
        print(50);
    }

    char *pointer = (char *) memory_alloc(24);
    x=memory_check(pointer);
    x=memory_check(region+200);
    x=memory_check(pointer+1);
    x =memory_free(pointer);
    x=memory_check(pointer);
    x=memory_check(pointer+1);
    pointer = (char *) memory_alloc(24);
    char *pointer2 = (char *) memory_alloc(8);
    char *pointer3 = (char *) memory_alloc(8);
    char *pointer4 = (char *) memory_alloc(8);
    char *pointer5 = (char *) memory_alloc(8);
    print(50);
    x =memory_free(pointer);
    pointer = (char *) memory_alloc(8);
    char *pointer6 = (char *) memory_alloc(8);
    print(50);
}

/*TEST2 - PAMAT 100 - BLOKY NAHODNEJ VELKOSTI(8-24) A ICH UVOLNENIE,
 *                   POTOM ALOKOVANIE 80*/
void test2(void){
    char region[100];
    memory_init(region, 100);
    char* list[10];
    srand(time(0));
    int x ;
    for (int i = 0; i < 10; i++) {
        x = rand() % 16 + 8;
        list[i] = (char*)memory_alloc(x);
    }

    for (int i = 0; i < 10; i++) {
        if (list[i] != NULL)
            x = memory_free(list[i]);
    }

    char* pointer = (char*)memory_alloc(80);
}

/*TEST3 - PAMAT 20000 - BLOKY NAHODNEJ VELKOSTI(500-5000) A ICH UVOLNENIE,
 *                   POTOM ALOKOVANIE 15000*/
void test3(void)
{
    char *region = (char*)malloc(20000);
    memory_init(region, 20000);
    char* list[10];
    srand(time(0));
    int x;
    for (int i = 0; i < 10; i++) {
        x = rand() % 5001 + 500;
        list[i] = (char*)memory_alloc(x);//tu
    }

    for (int i = 0; i < 10; i++) {
        if (list[i] != NULL)
            x = memory_free(list[i]);
    }

    char* pointer = (char*)memory_alloc(15000);
}

/*TEST4 - PAMAT 300_000 - BLOKY NAHODNEJ VELKOSTI(8-50000) A ICH UVOLNENIE,
 *                   POTOM ALOKOVANIE 200_000*/
void test4(void)
{
    char *region = (char*)malloc(300000);
    memory_init(region, 300000);
    char* list[15];
    srand(time(0));
    int x ;
    for (int i = 0; i < 15; i++) {
        x = (rand() + rand()) % 50001 + 8;
        list[i] = (char*)memory_alloc(x);//tu
    }

    for (int i = 0; i < 15; i++) {
        if (list[i] != NULL)
            x = memory_free(list[i]);
    }

    char* pointer = (char*)memory_alloc(200000);
    free(region);
}

/*TEST5 - PAMAT 300_000 - NAHODNE PRIDELOVANIE BLOKOV (8-50000) A ICH NAHODNE UVLONOVANIE,
 *                        NAKONIEC VSETKO UVOLNI A PRIRADI BLOK 250_000*/
void test5(void)
{
    char *region = (char*)malloc(300000);
    memory_init(region, 300000);
    char* list[30];
    for (int i =0;i<30;i++)
    {
        list[i] = NULL;
    }
    srand(time(0));
    int x = (rand() + rand()) % 50001 + 8;
    int y, res;
    print(300000);
    for (int i = 0; i < 200; i++)
    {
        y = rand() % 2;
        if (i > 50)  y = rand() % 3;
        if (y == 0)
        {
            y = (rand()) % 30;
            if (list[y] == NULL) {
                list[y] = (char *) memory_alloc(x);
                print(300000);
            }
        }
        else
        {
            y = (rand()) % 30;
            if (list[y] != NULL){
                res = memory_free(list[y]);
                list[y] = NULL;
                print(300000);
            }
        }
        x = (rand() + rand()) % 50001 + 8;
    }
    for (int i =0;i<30;i++)
    {
        if (list[i] != NULL){
            res = memory_free(list[i]);
        }
    }
    char* pointer = (char*)memory_alloc(250000);
    free(region);
}

/*TEST6 - PAMAT 1_000_000 - NAHODNE PRIDELOVANIE BLOKOV (8-50000) A ICH NAHODNE UVLONOVANIE,
 *                        NAKONIEC VSETKO UVOLNI A PRIRADI BLOK 250_000*/
void test6(void)
{
    char *region = (char*)malloc(1000000);
    memory_init(region, 1000000);
    char* list[30];
    for (int i =0;i<30;i++)
    {
        list[i] = NULL;
    }
    srand(time(0));
    int x = (rand() + rand()) % 50001 + 8;
    int y, res;
    print(1000000);
    for (int i = 0; i < 200; i++)
    {
        y = rand() % 2;
        if (i > 50)  y = rand() % 3;
        if (y == 0)
        {
            y = (rand()) % 30;
            if (list[y] == NULL) {
                list[y] = (char *) memory_alloc(x);
                print(1000000);
            }
        }
        else
        {
            y = (rand()) % 30;
            if (list[y] != NULL){
                res = memory_free(list[y]);
                list[y] = NULL;
                print(1000000);
            }
        }
        x = (rand() + rand()) % 50001 + 8;
    }
    for (int i =0;i<30;i++)
    {
        if (list[i] != NULL){
            res = memory_free(list[i]);
        }
    }
    print(1000000);
    char* pointer = (char*)memory_alloc(250000);
    print(1000000);
    free(region);
}
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
//#define _CRT_SECURE_NO_WARNINGS

//nakopirujte nasledujuci kod namiesto funkcie main

void z1_testovac(char *region, char **pointer, int minBlock, int maxBlock, int minMemory, int maxMemory, int testFragDefrag) {
	unsigned int allocated = 0;
	unsigned int mallocated = 0;
	unsigned int allocated_count = 0;
	unsigned int mallocated_count = 0;
	unsigned int i = 0;
	int random_memory = 0;
	int random = 0;
	memset(region, 0, 100000);
	random_memory = (rand() % (maxMemory-minMemory+1)) + minMemory;
	memory_init(region + 500, random_memory);
	if (testFragDefrag) {
		do {
			pointer[i] = memory_alloc(8);
			if (pointer[i])
				i++;
		} while (pointer[i]);
		for (int j = 0; j < i; j++) {
			if (memory_check(pointer[j])) {
				memory_free(pointer[j]);
			}
			else {
				printf("Error: Wrong memory check.\n");
			}
		}
	}
	i = 0;
	while (allocated <= random_memory-minBlock) {
		random = (rand() % (maxBlock-minBlock+1)) + minBlock;
		if (allocated + random > random_memory)
			continue;
		allocated += random;
		allocated_count++;
		pointer[i] = memory_alloc(random);
		if (pointer[i]) {
			i++;
			mallocated_count++;
			mallocated += random;
		}
	}
	for (int j = 0; j < i; j++) {
		if (memory_check(pointer[j])) {
			memory_free(pointer[j]);
		}
		else {
			printf("Error: Wrong memory check.\n");
		}
	}
	memset(region + 500, 0, random_memory);
	for (int j = 0; j < 100000; j++) {
		if (region[j] != 0) {
			region[j] = 0;
			printf("Error: Modified memory outside the managed region. index: %d\n",j-500);
		}
	}
	float result = ((float)mallocated_count / allocated_count) * 100;
	float result_bytes = ((float)mallocated / allocated) * 100;
	printf("Memory size of %d bytes: allocated %.2f%% blocks (%.2f%% bytes).\n", random_memory, result, result_bytes);
}

int main() {
	char region[100000];
	char* pointer[13000];
	srand(time(NULL));
	z1_testovac(region, pointer, 8, 24, 50, 100, 1);
	z1_testovac(region, pointer, 8, 1000, 10000, 20000, 0);
	z1_testovac(region, pointer, 8, 35000, 50000, 99000, 0);
	return 0;
}
/*int main()
{
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    return 0;
}*/