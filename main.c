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
#include <time.h>

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

typedef struct {
    char* first_str;
    char* second_str;
} StrPair;

void PrintStr(char* data) {
        printf("[%p] = <%s>\n", data, data);
}

void PrintNStr(char* data[], int nEl) {
    for (int i = 0; i < nEl; i++) {
        printf("[%03d]: ", nEl);
        PrintStr(data[i]); //done
    }
}

size_t CountLenOfTextStrings(const struct TextString data[]){
    if (!data[0].string){
        return 0;
    }
    size_t len = 0;
    while (data[len].string) {
        len++;
    }
    return len;
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

    struct stat baff = {};

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
    strcpy((char*)ptr, buffer);

    return ptr;
}


//OLD
int ReadFileAndKeepIt(const char* filename, char* index_arr[]) {

    const int MAX_BUFFER_LENGTH = 10*1000;

    FILE* file = fopen(filename, "r");

    char buffer[MAX_BUFFER_LENGTH] = ""; // done

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
int ReadFile(const char* filename, struct TextString data[], char** buffer) {

    int fd = open(filename, O_RDONLY);

    size_t flen = GetSizeOfFile(filename);

    void* ptr = calloc(1, flen+8); //8 для запаса
    *buffer = ptr;

    if (fd == -1){ printf(RED "ERROR: Не удалось открыть файл \"%s\"!\n" RESET, filename); return -errno; } //done


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

    FILE* fp = fopen(filename, "a");

    if (fp == NULL){ printf(RED "ERROR: Не удалось открыть файл \"%s\"!\n" RESET, filename); return -errno; }

    int count = 0;
    while ( data[count].string ) {
        if (strcmp(data[count].string, "") == 0){
            count++;
            continue;
        }
        fputs(data[count].string, fp); //done
        fputs("\n", fp);
        count++;
    }
    fputs("\0", fp);

    fclose(fp);
    return 0;
}

int StrCmpRevNew(const struct TextString str1, const struct TextString str2) {

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

        if (count1 >=0 && count2 >=0) {

            int char_difference = str1.string[count1]-str2.string[count2]; //done bus
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


int StrCmpNew(const struct TextString str1, const struct TextString str2) {

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

void PrintArray(struct TextString data[], const size_t len, const int leftPos, const int pivotPos, const int rightPos, const char* comment) {

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
    printf("L и P: %d \n", StrCmpNew(data[leftPos], data[pivotPos]));
    printf("P и R: %d \n", StrCmpNew(data[pivotPos], data[rightPos]));
    printf("L и R: %d \n", StrCmpNew(data[leftPos], data[rightPos]));
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

    PrintArray(data, len, leftPos, pivotPos, rightPos, "Начальное положение новой итерации компановки.");

    while (leftPos <= rightPos - 1) {
        if (DEBUG) {
        printf("\n");
        }
        assert(leftPos < rightPos);

        if (CompareMachine(&data[leftPos], &pivot) > 0) {
            PrintArray(data, len, leftPos, pivotPos, rightPos, "Нашли L, который надо переставить.");

            while (rightPos > pivotPos) {
                assert(rightPos > pivotPos);
                PrintArray(data, len, leftPos, pivotPos, rightPos, "Ищем R, который подойдет");

                if (CompareMachine(&data[rightPos], &pivot) < 0) {
                    struct TextString temp = {};
                    temp = data[rightPos];
                    data[rightPos] = data[leftPos];
                    data[leftPos] = temp;
                    PrintArray(data, len, leftPos, pivotPos, rightPos, "Поменяли L и R");
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

                PrintArray(data, len, leftPos, pivotPos, rightPos, "Поменяли L и R, где R было P");
            }

        }
        else if (leftPos == pivotPos) {

            assert(leftPos == pivotPos);
            PrintArray(data, len, leftPos, pivotPos, rightPos, "Берем L = P.");

            while (rightPos > pivotPos) {
                assert(rightPos > pivotPos);
                PrintArray(data, len, leftPos, pivotPos, rightPos, "Ищем R, который подойдет");

                if (CompareMachine(&data[rightPos], &pivot) < 0) {

                    struct TextString temp = {};
                    temp = data[rightPos];
                    data[rightPos] = data[leftPos];
                    data[leftPos] = temp;

                    pivotPos = rightPos;

                    PrintArray(data, len, leftPos, pivotPos, rightPos, "Поменяли L и R, где L было P.");
                    break;
                }

                rightPos--;
                assert(leftPos < len && rightPos < len);
            }
        }
        else {

            if (leftPos + 1 == rightPos) {
                PrintArray(data, len, leftPos, pivotPos, rightPos, "Конец.");
                break;
            }
            leftPos++;

            assert(leftPos < rightPos);
            assert(leftPos < len && rightPos < len);

            PrintArray(data, len, leftPos, pivotPos, rightPos, "Смотрим следующий L.");

        }
    }
    if (left != pivotPos || right!=pivotPos) {

        quickSortStr(data, len, left, pivotPos, "ЛЕВАЯ", CompareMachine);
        quickSortStr(data, len, pivotPos+1, right, "ПРАВАЯ", CompareMachine);

    }

    return data;
}

// int CompareMachineOld(const void* str1, const void* str2) {
//     return StrCmpNew(*(char**)str1, *(char**)str2);
// }

int CompareMachine(const void* a, const void* b) {

    const struct TextString* str1 = (struct TextString*) a;
    const struct TextString* str2 = (struct TextString*) b;

    return StrCmpNew(*str1, *str2);

}

// int CompareMachine2Old(const void* str1, const void* str2) {
//     return StrCmpRevNew(*(char* *) str1, *(char* *) str2);
// }

int CompareMachine2(const void* a, const void* b) {

    const struct TextString* str1 = a;
    const struct TextString* str2 = b;

    return StrCmpRevNew(*str1, *str2);

}


char getFirstLetterInWordAfterSpace(struct TextString word){
    char* first_str = word.string;
    size_t len_of_first = word.len;
    char first_letter = ' ';

    for (int i = len_of_first; first_str[i] != ' '; i--){
        first_letter = first_str[i];
    }

    return first_letter;
}

char getLastLetterInWord(struct TextString word){
    char* first_str = word.string;
    size_t len_of_first = word.len;

    int i = len_of_first;

    while (!isalpha(first_str[i])){
        i--;
    }

    return first_str[i];
}

char getPreLastLetterInWord(struct TextString word){
    char* first_str = word.string;
    size_t len_of_first = word.len;

    int i = len_of_first;

    while (!isalpha(first_str[i])){
        i--;
    }

    return first_str[i-1];
}

char getPrePreLastLetterInWord(struct TextString word){
    char* first_str = word.string;
    size_t len_of_first = word.len;

    int i = len_of_first;

    while (!isalpha(first_str[i])){
        i--;
    }

    return first_str[i-2];
}


StrPair getPairOfRandomStrings(const struct TextString data[]){

    StrPair pair = {};

    int rsn = ( rand() % 6135 ); //random_string_number

    while (data[rsn].len < 9){
        rsn = ( rand() % 6135 );
    }

    char first_let_in_first_str      = getFirstLetterInWordAfterSpace (data[rsn]);
    char last_let_in_first_str       = getLastLetterInWord            (data[rsn]);
    char prelast_let_in_first_str    = getPreLastLetterInWord         (data[rsn]);
    char preprelast_let_in_first_str = getPrePreLastLetterInWord      (data[rsn]);

    int rsn2 = ( rand() % 6135 );

    while (!
           (
              data[rsn].len > 9
           && getFirstLetterInWordAfterSpace (data[rsn2]) != first_let_in_first_str
           && getLastLetterInWord            (data[rsn2]) == last_let_in_first_str
           && getPreLastLetterInWord         (data[rsn2]) == prelast_let_in_first_str
           && getPrePreLastLetterInWord      (data[rsn2]) == preprelast_let_in_first_str
           )
    )
    {
        rsn2 = ( rand() % 6135 );
    }

    // printf("rsn is %d", rsn);
    // printf("rsn2 is %d", rsn2);


    pair.first_str = data[rsn].string;
    pair.second_str = data[rsn2].string;

    return pair;
}


//передаем массив, отсортированный по окончаниям, так же кол-во двоестиший.
void PrintBred(const struct TextString data[], int nStr){

    StrPair pair = {};
    for (int i = 0; i < nStr; i++){
        pair = getPairOfRandomStrings(data);
        printf("%s\n", pair.first_str);
        printf("%s\n", pair.second_str);
        printf("\n");
    }
    return;
}


int main(int argc, char *argv[]) {
    srand(time(NULL));

    const int MAX_LENGTH = 10*1000;
    char* text_buffer = {}; //done rename

    const char* in_file = (argc >= 2) ? argv[1] : "onegin.txt";
    const char* out_file = (argc >= 3) ? argv[2] : "onegin_out.txt";

    struct TextString index_arr[MAX_LENGTH] = {};
    struct TextString index_backup[MAX_LENGTH] = {};


    if (ReadFile(in_file, index_arr, &text_buffer) != 0 || ReadFile(in_file, index_backup, &text_buffer) != 0){
        return -1;
    }

    CleanFile(out_file);

    size_t len = CountLenOfTextStrings(index_arr);

    quickSortStr(index_arr, len, 0, len-1, "", CompareMachine);

    WriteFile(out_file, index_arr);

    qsort(index_arr, len, sizeof(struct TextString), CompareMachine2);


    WriteFile(out_file, index_arr);
    WriteFile(out_file, index_backup);

    PrintBred(index_arr, 10);

    return 0;
}
