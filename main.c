#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define RED "\033[31m"
#define CYAN "\033[36m"
#define BLUE "\033[34m"
#define YELLOW "\033[33m"
#define PURPLE "\033[35m"

#define BOLD_RED "\033[1;31m"
#define HIGH_BOLD_RED "\033[1;31m"
#define BOLD_CYAN "\033[1;36m"
#define BOLD_BLUE "\033[1;34m"
#define BOLD_YELLOW "\033[1;33m"
#define BOLD_PURPLE "\033[1;35m"
#define BOLD "\033[1m"

#define RESET "\033[0m"

const bool DEBUG = 0;

struct TextString {
    char* string;
    size_t len;
};

void PrintNStr(char* data[], int nEl) {
    for (int i = 0; i < nEl; i++) {
        printf("%d: [%p] = <%s>\n", i, data[i], data[i]);
    }
}

void PrintStr(char* data) {
        printf("[%p] = <%s>\n", data, data);
}

int CleanFile(const char* filename) {

    FILE* file = fopen(filename, "w");

    if (!file) {
        printf(RED "%s(): Неудачное открытие файла!\n" RESET, __FUNCTION__);
        return -errno;
    }

    fclose(file);
    return 0;
}


//NEW
size_t GetSizeOfFile(const char* filename) {

    struct stat baff;

    // при успехе возвращает 0 => рассматрвиаем не успех
    if (stat(filename, &baff)) {
        return 0;
    }

    return baff.st_size;

}

//OLD NEW
void* BuffDup(char buffer[]) {

    size_t row_len = strlen(buffer);
    void* ptr = calloc(row_len+1, sizeof(char));
    strcpy(ptr, buffer);

    return ptr;
}


//OLD
int ReadFileAndKeepIt(const char* filename, char* index_arr[]) {

    FILE* file = fopen(filename, "r");

    char buffer[100000] = "";

    int idx_count = 0;

    if (!file) {
        printf(RED "%s(): Неудачное открытие файла!\n" RESET, __FUNCTION__);
        return -errno;
    }

    while ((fgets(buffer, 256, file))!=NULL) {
        int len = strlen (buffer);

        //замена \n на \0 для удобного дальнейшего дебага
        if (len >= 1 && buffer[len-1] == '\n') buffer[len-1] = '\0';

        void* ptr = BuffDup(buffer);
        index_arr[idx_count] = ptr;
        idx_count++;
    }

    fclose(file);
    return 0;
}

//NEW
int ReadFile(const char* filename, struct TextString data[], char* BUFFER) {

    int fd = open(filename, O_RDONLY);

    size_t flen = GetSizeOfFile(filename);

    void* ptr = calloc(1, flen+8); //8 для запаса
    BUFFER = ptr;

    if (fd == -1){ printf(RED "Неудачное открытие файла!\n" RESET); return -errno; }


    size_t bytesRead = read(fd, ptr, flen);
    int count = 0;
    int index_count = 1;
    int len_count = 0;
    data[0].string = ptr;

    //ЗАПИСЬ

    //пока символ не равен \0
    while (count < bytesRead) {
        if (*((char*)ptr + count + 1)  &&  *((char*)ptr + count) == '\n') {
            *((char*)ptr + count) = '\0'; //не меняем, тк тогда например запись файла будет
            count++;
            data[index_count].string = (char*)ptr + count;
            data[index_count-1].len = len_count;
            len_count = 0;
            index_count++;
            continue;
        }
        len_count++;
        count++;
    }

    data[index_count-1].len = len_count;

    /* count = 1;
    while (data[count].string) {
        // не получилось. думаю из за выравнивания
        // data[count].len = (uintptr_t)data[count].string-(uintptr_t)data[count-1].string;
        count++;
    }
    */

    return 0;
}

//OLD
int WriteToFile(const char* filename, char** data) {

    printf("Открываем файл <%s>\n", filename);
    FILE* file = fopen(filename, "a");
    fputs("\n----------------------------------------НАЧАЛО ЧАСТИ---------------------------------------------\n", file);
    int idx_count = 0;

    if (!file) {
        printf(RED "Неудачное открытие файла!\n" RESET);
        return -errno;
    }

    while (data[idx_count]) {
        fputs("{", file);
        fputs(data[idx_count], file);
        fputs("}", file);
        idx_count++;
    }
    fputs("\n----------------------------------------КОНЕЦ ЧАСТИ---------------------------------------------\n", file);
    fclose(file);
    return 0;

}

// NEW
int WriteFile(const char* filename, struct TextString data[]) {

    int fd = open(filename, O_WRONLY | O_APPEND);

    size_t flen = GetSizeOfFile(filename);

    // void* ptr = calloc(1, flen+8);

    if (fd == -1){ printf(RED "Неудачное открытие файла!\n" RESET); return -errno; }

    int count =0;
    while ( data[count].string ) {
        write(fd, data[count].string, data[count].len);
        write(fd, "\n", 1);
         // ТАК НОРМ ДЕЛАТЬ?
        count++;
    }
    write(fd, "\0", 1);


    return 0;
}

int strcmprev_new(const struct TextString str1, const struct TextString str2) {

    int len1 = str1.len, len2 = str2.len;
    int count1 = len1 - 1, count2 = len2 - 1;
    if (str1.string == NULL && str2.string == NULL) {
        return 0;
    }
    if (str1.string == NULL) {
        return -1;
    }
    if (str2.string == NULL) {
        return 1;
    }


    while (1) {

        while (count1>=0 && !isalpha(str1.string[count1])) {
            count1--;
        }
        while (count2>=0 && !isalpha(str2.string[count2])) {
            count2--;
        }

        int char_difference = str1.string[count1]-str2.string[count2];
        if (count1 >=0 && count2 >=0) {
            if (!char_difference) {
                if ( !str1.string[count1] && !str2.string[count2] ) {
                    return 0;
                }
                else {
                    count1--;
                    count2--;
                    continue;
                }
            }
            else {
                if (char_difference < 0) {
                    return -1;
                }
                else {
                    return 1;
                }
            }
        }

        if (count1 < 0 && count2 < 0) {
            return 0;
        }

        if (count1 < 0) {
            return -1;
        }

        if (count2 < 0) {
            return 1;
        }

    }

}


int strcmp_new(const struct TextString str1, const struct TextString str2) {

    int count = 0;
    int offset1 = 0; int offset2 = 0;

    if (str1.string == NULL && str2.string == NULL) {
        return 0;
    }
    if (str1.string == NULL) {
        return -1;
    }
    if (str2.string == NULL) {
        return 1;
    }

    while (1) {
        while (!isalpha(str1.string[count+offset1]) && str1.string[count+offset1]) {
            offset1++;
        }
        while (!isalpha(str2.string[count+offset2]) && str2.string[count+offset2]) {
            offset2++;
        }
        int char_difference = str1.string[count+offset1]-str2.string[count+offset2];

        if (!char_difference) {
            if (!str1.string[count+offset1]) {
                return 0;
            }
            else {
                count++;
                continue;
            }
        }
        else {
            if (char_difference < 0) {
                return -1;
            }
            else {
                return 1;
            }
        }

    }

}

//OLD
void FreeAllPtrs(char** index_arr) {
    int count = 0;
    while (index_arr[count]) {
        free(index_arr[count]);
        index_arr[count] = 0;
        count++;
    }

}

void printArray(struct TextString data[], const size_t len, const int leftPos, const int pivotPos, const int rightPos, const char* comment) {

    if (DEBUG) {
        //INIT INDEX LINE
    int i = 0;
    for (; i < len; i++) {
        printf(" %011d  ", i);
    }
    printf("\n");

    //FIRST LINE
    i = 0;
    for (; i < leftPos; i++) {
        printf(YELLOW "[%11.11s] " RESET, data[i].string);
    }
    printf(BOLD_RED "[%11.11s] " RESET, data[leftPos].string);
    i++;
    for (; i < pivotPos; i++) {
        printf( "[%11.11s] " RESET, data[i].string);
    }
    if (leftPos != pivotPos && rightPos != pivotPos) {
        printf(BOLD_PURPLE "[%11.11s] " RESET, data[pivotPos].string);
        i++;
    }

    for (; i < rightPos; i++) {
        printf("[%11.11s] " RESET, data[i].string);
    }
    printf(BOLD_BLUE "[%11.11s] " RESET, data[rightPos].string);
    i++;
    for (; i < len; i++) {
        printf(CYAN "[%11.11s] " RESET, data[i].string);
    }
    printf(" %s ", comment);
    printf("\n");

    //SECOND LINE
    i = 0;
    for (; i < leftPos; i++) {
        printf("              ");
    }
    printf("      ^       ");
    i++;
    for (; i < pivotPos; i++) {
        printf("              ");
    }
    if (leftPos != pivotPos && rightPos != pivotPos) {
        printf("      ^       ");
        i++;
    }
    for (; i < rightPos; i++) {
        printf("              ");
    }
    printf("      ^       ");
    printf("\n");

    //THIRD LINE
    i = 0;
    for (; i < leftPos; i++) {
        printf("              ");
    }
    if (leftPos == pivotPos) {
        printf("      LP      ");
        i++;
    }
    else {
        printf("      L       ");
        i++;
    }

    for (; i < pivotPos; i++) {
        printf("              ");
    }
    if (leftPos != pivotPos && rightPos != pivotPos) {
        printf("      P       ");
        i++;
    }

    for (; i < rightPos; i++) {
        printf("              ");
    }
    if (rightPos == pivotPos) {
        printf("      RP      ");
        i++;
    }
    else {
        printf("      R       ");
        i++;
    }
    printf("\n");
    printf("\n");
    printf("L: <%s>\n", data[leftPos].string);
    printf("P: <%s>\n", data[pivotPos].string);
    printf("R: <%s>\n", data[rightPos].string);
    printf("\n");
    printf("L и P: %d \n", strcmp_new(data[leftPos], data[pivotPos]));
    printf("P и R: %d \n", strcmp_new(data[pivotPos], data[rightPos]));
    printf("L и R: %d \n", strcmp_new(data[leftPos], data[rightPos]));
    printf("\n");
    getchar();
    }
    else {
        return;
    }

}


struct TextString* quickSortStr
(struct TextString data[], size_t len, int left, int right,
    const char* text, int (*CompareMachine)(const void* str1, const void* str2))
{

    if (DEBUG) {
        printf("------------------------------------------НОВАЯ ВЕТКА %s [%d, %d]------------------------------------------\n", text, left, right);
    }

    if (left > right) {
        return data;
    }
    int pivotPos = left + (right-left)/2;
    struct TextString pivot = data[pivotPos];
    int leftPos = left;
    int rightPos = right;

    printArray(data, len, leftPos, pivotPos, rightPos, "Начальное положение новой итерации компановки.");

    while (leftPos <= rightPos) {
        if (DEBUG) {
        printf("\n");
        }
        assert(leftPos < rightPos);

        if (CompareMachine(&data[leftPos], &pivot) > 0) {
            printArray(data, len, leftPos, pivotPos, rightPos, "Нашли L, который надо переставить.");

            while (rightPos > pivotPos) {
                assert(rightPos > pivotPos);
                printArray(data, len, leftPos, pivotPos, rightPos, "Ищем R, который подойдет");

                if (CompareMachine(&data[rightPos], &pivot) < 0) {
                    struct TextString temp = {};
                    temp = data[rightPos];
                    data[rightPos] = data[leftPos];
                    data[leftPos] = temp;
                    printArray(data, len, leftPos, pivotPos, rightPos, "Поменяли L и R");
                    break;
                }
                rightPos--;
                assert(leftPos < rightPos);
                assert(leftPos < len && rightPos < len);
            }
            if (rightPos == pivotPos) {
                struct TextString temp = {};
                temp = data[rightPos];
                data[rightPos] = data[leftPos];
                data[leftPos] = temp;

                pivotPos = leftPos;

                printArray(data, len, leftPos, pivotPos, rightPos, "Поменяли L и R, где R было P");
            }

        }
        else if (leftPos == pivotPos) {

            assert(leftPos == pivotPos);
            printArray(data, len, leftPos, pivotPos, rightPos, "Берем L = P.");

            while (rightPos > pivotPos) {
                assert(rightPos > pivotPos);
                printArray(data, len, leftPos, pivotPos, rightPos, "Ищем R, который подойдет");

                if (CompareMachine(&data[rightPos], &pivot) < 0) {

                    struct TextString temp = {};
                    temp = data[rightPos];
                    data[rightPos] = data[leftPos];
                    data[leftPos] = temp;

                    pivotPos = rightPos;

                    printArray(data, len, leftPos, pivotPos, rightPos, "Поменяли L и R, где L было P.");
                    break;
                }

                rightPos--;
                assert(leftPos < len && rightPos < len);
            }
        }
        else {

            if (leftPos + 1 == rightPos) {
                printArray(data, len, leftPos, pivotPos, rightPos, "Конец.");
                break;
            }
            leftPos++;

            assert(leftPos < rightPos);
            assert(leftPos < len && rightPos < len);

            printArray(data, len, leftPos, pivotPos, rightPos, "Смотрим следующий L.");

        }
    }
    if (left != pivotPos || right!=pivotPos) {
        quickSortStr(data, len, left, pivotPos, "ЛЕВАЯ", CompareMachine);
        quickSortStr(data, len, pivotPos+1, right, "ПРАВАЯ", CompareMachine);
    }

    return data;
}

// int CompareMachineOld(const void* str1, const void* str2) {
//     return strcmp_new(*(char**)str1, *(char**)str2);
// }

int CompareMachine(const void* a, const void* b) {
    const struct TextString* str1 = a;
    const struct TextString* str2 = b;
    return strcmp_new(*str1, *str2);
}

// int CompareMachine2Old(const void* str1, const void* str2) {
//     return strcmprev_new(*(char* *) str1, *(char* *) str2);
// }

int CompareMachine2(const void* a, const void* b) {
    const struct TextString* str1 = a;
    const struct TextString* str2 = b;
    return strcmprev_new(*str1, *str2);
}


int main(void) {

    const int MAX_LENGTH = 10*1000;
    char* BUFFER = {};
    const char* IN = "onegin.txt";
    const char* OUT = "onegin_out.txt";
    struct TextString index_arr[MAX_LENGTH] = {};
    struct TextString index_backup[MAX_LENGTH] = {};

    CleanFile(OUT);

    if (ReadFile(IN, index_arr, BUFFER) || ReadFile(IN, index_backup, BUFFER)){
        printf("Ошибка в чтении файла.");
        return -1;
    }

    size_t len = 0;
    while (index_arr[len].string) {
        len++;
    }

    quickSortStr(index_arr, len, 0, len-1, "", CompareMachine);

    WriteFile(OUT, index_arr);

    qsort(index_arr, len, sizeof(struct TextString), CompareMachine2);

    WriteFile(OUT, index_arr);
    WriteFile(OUT, index_backup);

    return 0;
}
