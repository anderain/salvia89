#include <stdio.h>
#include <string.h>
#include "salvia.h"

#define COLOR_NRM   "\x1B[0m"
#define COLOR_RED   "\x1B[31m"
#define COLOR_GRN   "\x1B[32m"
#define COLOR_YEL   "\x1B[33m"
#define COLOR_BLU   "\x1B[34m"
#define COLOR_MAG   "\x1B[35m"
#define COLOR_CYN   "\x1B[36m"
#define COLOR_WHT   "\x1B[37m"

int iTestCount = 0;
int iPassedCount = 0;

struct {
    const char* szFormat;
    int         iValue;
} sIntCases[] = {
    { "|%-+8.5d|",      123 },
    { "|%-+8.5d|",      -123 },
    { "|%+08d|",        123 },
    { "|%+08d|",        -123 },
    { "|%+08.5d|",      123 },
    { "|%+08.5d|",      -123 },
    { "|% +5d|",        123 },
    { "|% +5d|",        -123 },
    { "|%-05d|",        123 },
    { "|%-05d|",        -123 },
    { "|%5.0d|",        0 },
    { "|%0-+12.8d|",    123 },
    { "|%0-+12.8d|",    -123 }
};

int iNumIntCases = sizeof(sIntCases) / sizeof(sIntCases[0]);

struct {
    const char* szFormat;
    const char* szValue;
} sStrCases[] = {
    { "|%15s|",     "HelloWorld" },  
    { "|%-15s|",    "HelloWorld" },  
    { "|%5s|",      "HelloWorld" },  
    { "|%.5s|",     "HelloWorld" },
    { "|%15.5s|",   "HelloWorld" },  
    { "|%-15.5s|",  "HelloWorld" },  
    { "|%015s|",    "HelloWorld" },  
    { "|%015.5s|",  "HelloWorld" },  
    { "|%10s|",     "" },   
    { "|%.5s|",     "" },  
    { "|%10.0s|",   "HelloWorld" }
};

int iNumStrCases = sizeof(sStrCases) / sizeof(sStrCases[0]);

struct {
    const char* szFormat;
    char cValue;
} sCharCases[] = {
    { "|%c|",       'A'  },
    { "|%5c|",      'A'  },
    { "|%-5c|",     'A'  },
    { "|%05c|",     'A'  },
    { "|%.3c|",     'A'  },
    { "|%5.3c|",    'A'  },
    { "|%c|",       ' '  },
    { "|%c|",       '\0' },
    { "|%5c|",      '\0' }
};

int iNumCharCases = sizeof(sCharCases) / sizeof(sCharCases[0]);

struct {
    const char* szFormat;
    double      dValue;
} sFloatCases[] = {
    { "|%f|",           3.14 },
    { "|%f|",           -3.14 },
    { "|%.2f|",         3.14159 },
    { "|%.2f|",         -3.14159 },
    { "|%8.2f|",        3.14 },
    { "|%-8.2f|",       3.14 },
    { "|%08.2f|",       3.14 },
    { "|%+8.2f|",       3.14 },
    { "|%+08.2f|",      3.14 },
    { "|%8.2f|",        -3.14 },
    { "|%-8.2f|",       -3.14 },
    { "|%08.2f|",       -3.14 },
    { "|%+8.2f|",       -3.14 },
    { "|%+08.2f|",      -3.14 },
    { "|%.0f|",         3.14 },
    { "|%.0f|",         3.5 },
    { "|%f|",           0.0 },
    { "|%.2f|",         0.0 },
    { "|%.0f|",         0.0 },
    { "|%8.2f|",        0.0 },
    { "|%+8.2f|",       0.0 },
    { "|%5.3f|",        3.14 },
    { "|%5.3f|",        -3.14 },
    { "|%.5f|",         1.5 },
    { "|%f|",           1.0 },
    { "|%f|",           -1.0 },
    { "|%.2f|",         1.0 },
    { "|%.2f|",         -1.0 },
    { "|%10.3f|",       12.345 },
    { "|%-10.3f|",      12.345 },
    { "|%010.3f|",      12.345 },
    { "|%10.3f|",       -12.345 },
    { "|%+f|",          2.5 },
    { "|%+f|",          -2.5 },
    { "|%0-+12.5f|",    3.14 },
    { "|%0-+12.5f|",    -3.14 },
    { "|% 8.2f|",       3.14 },
    { "|% 8.2f|",       -3.14 },
    { "|%8.0f|",        3.14 },
    { "|%8.0f|",        -3.14 },
    { "|%+8.0f|",       3.14 },
    { "|%+8.0f|",       -3.14 }
};

int iNumFloatCases = sizeof(sFloatCases) / sizeof(sFloatCases[0]);

void testIntCases() {
    char strSalviaBuf[100];
    char strStdBuf[100];
    int i;
    int bIsPassed;

    for (i = 0; i < iNumIntCases; ++i) {
        Salvia_Format(strSalviaBuf, sIntCases[i].szFormat, sIntCases[i].iValue);
        sprintf(strStdBuf, sIntCases[i].szFormat, sIntCases[i].iValue);
        bIsPassed = strcmp(strSalviaBuf, strStdBuf) == 0;
        printf("%s[%s]%s Case %d, format = \"%s\"\n",
            bIsPassed ? COLOR_GRN : COLOR_RED,
            bIsPassed ? "Passed" : "Failed",
            COLOR_NRM,
            i + 1,
            sIntCases[i].szFormat
        );
        printf("    salvia = %s\n", strSalviaBuf);
        printf("    stdio  = %s\n", strStdBuf);
        ++iTestCount;
        if (bIsPassed) ++iPassedCount;
    }
}

void testStrCases() {
    char strSalviaBuf[100];
    char strStdBuf[100];
    int i;
    int bIsPassed;

    for (i = 0; i < iNumStrCases; ++i) {
        Salvia_Format(strSalviaBuf, sStrCases[i].szFormat, sStrCases[i].szValue);
        sprintf(strStdBuf, sStrCases[i].szFormat, sStrCases[i].szValue);
        bIsPassed = strcmp(strSalviaBuf, strStdBuf) == 0;
        printf("%s[%s]%s Case %d, format = \"%s\"\n",
            bIsPassed ? COLOR_GRN : COLOR_RED,
            bIsPassed ? "Passed" : "Failed",
            COLOR_NRM,
            i + 1,
            sStrCases[i].szFormat
        );
        printf("    salvia = %s\n", strSalviaBuf);
        printf("    stdio  = %s\n", strStdBuf);
        ++iTestCount;
        if (bIsPassed) ++iPassedCount;
    }
}

void testCharCases() {
    char strSalviaBuf[100];
    char strStdBuf[100];
    int i;
    int bIsPassed;

    for (i = 0; i < iNumCharCases; ++i) {
        Salvia_Format(strSalviaBuf, sCharCases[i].szFormat, sCharCases[i].cValue);
        sprintf(strStdBuf, sCharCases[i].szFormat, sCharCases[i].cValue);
        bIsPassed = strcmp(strSalviaBuf, strStdBuf) == 0;
        printf("%s[%s]%s Case %d, format = \"%s\"\n",
            bIsPassed ? COLOR_GRN : COLOR_RED,
            bIsPassed ? "Passed" : "Failed",
            COLOR_NRM,
            i + 1,
            sCharCases[i].szFormat
        );
        printf("    salvia = %s\n", strSalviaBuf);
        printf("    stdio  = %s\n", strStdBuf);
        ++iTestCount;
        if (bIsPassed) ++iPassedCount;
    }
}

void testFloatCases() {
    char strSalviaBuf[100];
    char strStdBuf[100];
    int i;
    int bIsPassed;

    for (i = 0; i < iNumFloatCases; ++i) {
        Salvia_Format(strSalviaBuf, sFloatCases[i].szFormat, sFloatCases[i].dValue);
        sprintf(strStdBuf, sFloatCases[i].szFormat, sFloatCases[i].dValue);
        bIsPassed = strcmp(strSalviaBuf, strStdBuf) == 0;
        printf("%s[%s]%s Case %d, format = \"%s\"\n",
            bIsPassed ? COLOR_GRN : COLOR_RED,
            bIsPassed ? "Passed" : "Failed",
            COLOR_NRM,
            i + 1,
            sFloatCases[i].szFormat
        );
        printf("    salvia = %s\n", strSalviaBuf);
        printf("    stdio  = %s\n", strStdBuf);
        ++iTestCount;
        if (bIsPassed) ++iPassedCount;
    }
}

int main(int argc, char* argv[]) {
    testIntCases();
    testStrCases();
    testCharCases();
    testFloatCases();
    printf("Total %d cases, %d passed\n", iTestCount, iPassedCount);
    return 0;
}