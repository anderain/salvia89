#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "salvia.h"

#define NUM_STR_MAX 100

/**
 * @brief Format parsing state enumeration
 */
enum {
    STATE_PLAIN = 0,      /** < Plain text state */
    STATE_FLAGS,          /** < Flag parsing state   */
    STATE_WIDTH,          /** < Width parsing state */
    STATE_PRECISION,      /** < Precision parsing state */
    STATE_LENGTH,         /** < Length modifier parsing state */
    STATE_SPECIFIER       /** < Type specifier parsing state */
} ReaderState;

/**
 * @brief Format flag bit enumeration
 */
enum {
    FLAG_NONE           = 0,   /** < No flag */
    FLAG_LEFT_JUSTIFY   = 1,   /** < Left-justify flag (-) */
    FLAG_FORCE_SIGN     = 2,   /** < Force sign flag (+) */
    FLAG_SPACE_PADDING  = 4,   /** < Space before positive number flag ( ) */
    FLAG_ZERO_PADDING   = 8    /** < Zero-padding flag (0) */
} Flags;

/** 
 * @brief Format specifier structure
 * 
 * @details Consistent with C printf format flags:
 * %[flags][width][.precision][length]specifier
 */
typedef struct {
    unsigned int    uFlags;     /** < Flag bit combination */
    int             iMinWidth;  /** < Minimum field width */
    int             iPrecision; /** < Precision (decimal places for numbers, max chars for strings) */
    char            cLength;    /** < Length modifier (h/l/L, etc.) */
} FormatSpec;

#define Salvia_StrLen               strlen                          /** < String length calculation macro */
#define Salvia_StrCpy               strcpy                          /** < String copy macro */
#define Salvia_IsDigit(c)           ((c) >= '0' && (c) <= '9')      /** < Check if character is a digit */

static void     Salvia_ClearFormatSpec      (FormatSpec* pFormatSpec);
static char*    Salvia_WriteString          (char* szBuf, const char* szSource, int iStrLength, char cPadding, int iMaxWrite, int iWidth, int bIsLeftAlign);
static int      Salvia_Atoi                 (const char* pSource);
static char*    Salvia_ItoaWithPadding      (int iNum, char* pStrBuf, int iBase, int bUppercase, int bForceSign, int iMinDigit);
static char*    Salvia_Ftoa                 (char* pStrBuf, double dNum, int iPrecision, int bForceSign, int bZeroPad, int iMinWidth);

/* Reset temporary buffer pointer */
#define ResetTempBuffer() (pCurTempBuf = szTempBuf)

/* Reset state and return to Plain state */
#define ResetFormatState() {                        \
            Salvia_ClearFormatSpec(&sFormatSpec);    \
            ResetTempBuffer();                      \
            iState = STATE_PLAIN;                   \
        } NULL

/**
 * @brief Formatted output string (printf-like)
 * 
 * @param szBuf Destination buffer for storing the formatted string
 * @param szFormat Format string
 * @param ... Variable arguments, pass corresponding arguments according to the format string
 * @return int Number of characters written (excluding the terminating '\0')
 * 
 * @details
 * Supported format specifiers:
 * - %d: Decimal integer
 * - %s: String
 * - %c: Character
 */
 int Salvia_Format(char* szBuf, const char* szFormat, ...) {
    char*       pCurBuf = szBuf;        /* Current write position */
    const char* pCurFormat = szFormat;  /* Current read position */
    const char* pLastFormatStart;       /* Start of the last format specifier */
    char        szTempBuf[NUM_STR_MAX]; /* Temporary buffer for reading width / precision */
    char*       pCurTempBuf;            /* Temporary buffer write pointer */
    char        c;                      /* Single character read from szFormat */
    int         iState;                 /* State machine reading state */
    FormatSpec  sFormatSpec;            /* Format parameters */
    va_list     args;                   /* Variable arguments */

    va_start(args, szFormat);

    ResetFormatState();

    while ((c = *pCurFormat)) {
        switch (iState) {
        default:
        /* Plain character */
        case STATE_PLAIN:
            /* Encountered a format start */
            if (c == '%') {
                pLastFormatStart = pCurFormat;
                pCurFormat++;
                iState = STATE_FLAGS;
            }
            /* Other plain characters, write to destination buffer */
            else {
                *pCurBuf++ = c;
                pCurFormat++;
            }
            break;
        case STATE_FLAGS:
            switch (c) {
            case '-': /* Left-justify */
                sFormatSpec.uFlags |= FLAG_LEFT_JUSTIFY;
                pCurFormat++;
                break;
            case '+': /* Force sign */
                sFormatSpec.uFlags |= FLAG_FORCE_SIGN;
                pCurFormat++;
                break;
            case ' ': /* Space padding */
                sFormatSpec.uFlags |= FLAG_SPACE_PADDING;
                pCurFormat++;
                break;
            case '0': /* Zero padding */
                sFormatSpec.uFlags |= FLAG_ZERO_PADDING;
                pCurFormat++;
                break;
            default: /* Other characters, enter next state */
                iState = STATE_WIDTH;
                break;
            }
            break;
        case STATE_WIDTH:
            /* Is a digit, read digit into buffer */
            if (Salvia_IsDigit(c)) {
                *pCurTempBuf++ = c;
                pCurFormat++;
            }
            /* Not a digit, finish reading, enter next state */
            else {
                *pCurTempBuf = '\0';
                sFormatSpec.iMinWidth = Salvia_Atoi(szTempBuf);
                ResetTempBuffer();
                /* Enter precision reading */
                if (c == '.') {
                    iState = STATE_PRECISION;
                    pCurFormat++;
                }
                /* Enter length modifier reading */
                else {
                    iState = STATE_LENGTH;
                }
            }
            break;
        case STATE_PRECISION:
            /* Is a digit, read digit into buffer */
            if (Salvia_IsDigit(c)) {
                *pCurTempBuf++ = c;
                pCurFormat++;
            }
            /* Not a digit, finish reading, enter next state */
            else {
                *pCurTempBuf = '\0';
                sFormatSpec.iPrecision = Salvia_Atoi(szTempBuf);
                ResetTempBuffer();
                iState = STATE_LENGTH;
            }
            break;
        case STATE_LENGTH:
            /* TODO: Implement length modifier reading */
            iState = STATE_SPECIFIER;
            break;
        case STATE_SPECIFIER:
            switch (c) {
            case 'd': { /* Decimal integer */
                int iPrecision = sFormatSpec.iPrecision;
                int bForceSign = (sFormatSpec.uFlags & FLAG_FORCE_SIGN) ? 1 : 0;
                int intValue = va_arg(args, int);
                /* Edge case */
                if (iPrecision == 0 && intValue == 0) {
                    if (intValue < 0) {
                        szTempBuf[0] = '-';
                        szTempBuf[1] = '\0';
                    }
                    else if (bForceSign) {
                        szTempBuf[0] = '+';
                        szTempBuf[1] = '\0';
                    }
                    else {
                        szTempBuf[0] = '\0';
                    }
                }
                /* Normal case */
                else {
                    if (iPrecision < 0 &&
                        (sFormatSpec.uFlags & FLAG_ZERO_PADDING) &&
                        !(sFormatSpec.uFlags & FLAG_LEFT_JUSTIFY) &&
                        sFormatSpec.iMinWidth > 0
                    ) {
                        iPrecision = sFormatSpec.iMinWidth;
                        if (intValue < 0 || bForceSign) {
                            iPrecision--;
                        }
                    }
                    Salvia_ItoaWithPadding(intValue, szTempBuf, 10, 0, bForceSign, iPrecision);
                }
                pCurBuf = Salvia_WriteString(
                    pCurBuf,
                    szTempBuf,
                    Salvia_StrLen(szTempBuf),
                    ' ',
                    -1,
                    sFormatSpec.iMinWidth,
                    (sFormatSpec.uFlags & FLAG_LEFT_JUSTIFY) ? 1 : 0
                );
                break;
            }
            case 'f': { /* Float / Double */
                double dblValue = va_arg(args, double);
                int bForceSign = (sFormatSpec.uFlags & FLAG_FORCE_SIGN) ? 1 : 0;
                int iPrecision = sFormatSpec.iPrecision;
                int bZeroPad = (sFormatSpec.uFlags & FLAG_ZERO_PADDING) && !(sFormatSpec.uFlags & FLAG_LEFT_JUSTIFY) && sFormatSpec.iMinWidth > 0;

                if (iPrecision < 0) {
                    iPrecision = 6;
                }

                Salvia_Ftoa(szTempBuf, dblValue, iPrecision, bForceSign, bZeroPad, sFormatSpec.iMinWidth);

                pCurBuf = Salvia_WriteString(
                    pCurBuf,
                    szTempBuf,
                    Salvia_StrLen(szTempBuf),
                    ' ',
                    -1,
                    sFormatSpec.iMinWidth,
                    (sFormatSpec.uFlags & FLAG_LEFT_JUSTIFY) ? 1 : 0
                );
                break;
            }
            case 's': { /* String */
                const char* pStr = va_arg(args, const char *);
                pCurBuf = Salvia_WriteString(
                    pCurBuf,
                    pStr,
                    Salvia_StrLen(pStr),
                    /* (sFormatSpec.uFlags & FLAG_ZERO_PADDING) ? '0' : ' ', */
                    ' ', /* Consistent with GCC ANSI standard, do not use 0 padding */
                    sFormatSpec.iPrecision,
                    sFormatSpec.iMinWidth,
                    (sFormatSpec.uFlags & FLAG_LEFT_JUSTIFY) ? 1 : 0
                );
                break;
            }
            case 'c':   /* Character */
                szTempBuf[0] = (char)(va_arg(args, int));
                szTempBuf[1] = '\0';
                pCurBuf = Salvia_WriteString(
                    pCurBuf,
                    szTempBuf,
                    1, /* Single character, string length is always 1 */
                    /* (sFormatSpec.uFlags & FLAG_ZERO_PADDING) ? '0' : ' ', */
                    ' ', /* Consistent with GCC ANSI standard, do not use 0 padding */
                    sFormatSpec.iPrecision,
                    sFormatSpec.iMinWidth,
                    (sFormatSpec.uFlags & FLAG_LEFT_JUSTIFY) ? 1 : 0
                );
                break;
            case '%':   /* Percent sign itself */
                *pCurBuf++ = '%';
                break;
            default: {  /* Invalid input, do not format, output content directly */
                    const char* pFmt;
                    for (
                        pFmt = pLastFormatStart;
                        pFmt <= pCurFormat;
                        ++pFmt, ++pCurBuf
                    ) {
                        *pCurBuf = *pFmt;
                    }
                    break;
                }
            }
            ++pCurFormat;
            /* Reset format flags, return to plain character state */
            ResetFormatState();
        }
    }

    /* Format reading incomplete */
    if (iState != STATE_PLAIN) {
        const char* pFmt;
        for (
            pFmt = pLastFormatStart;
            pFmt < pCurFormat;
            ++pFmt, ++pCurBuf
        ) {
            *pCurBuf = *pFmt;
        }
    }

    va_end(args);

    /* Append '\0' terminator at the end */
    *pCurBuf = '\0';
    /* Return the difference between final write position and szBuf, i.e., number of bytes written */
    return pCurBuf - szBuf;
}

/**
 * @brief Write a string into the buffer, supporting alignment and padding
 * 
 * @param szBuf Destination buffer
 * @param szSource Source string 
 * @param cPadding Padding character
 * @param iMaxWrite Maximum number of characters to write (-1 means unlimited)
 * @param iWidth Total field width
 * @param bIsLeftAlign Whether to left-align (1=left-align, 0=right-align)
 * @return char* Returns the buffer position after writing
 * 
 * @details
 * This function adds padding characters before or after the written string
 * according to the specified width and alignment.
 * If the source string length exceeds iMaxWrite, it will be truncated.
 */
static char* Salvia_WriteString(char* szBuf, const char* szSource, int iStrLength, char cPadding, int iMaxWrite, int iWidth, int bIsLeftAlign) {
    int iLengthToWrite;
    int iPadding;
    int i;

    /* Calculate the number of characters to display and padding width */
    if (iMaxWrite < 0) {
        iLengthToWrite = iStrLength;
    }
    else if (iMaxWrite < iStrLength) {
        iLengthToWrite = iMaxWrite;
    }
    else {
        iLengthToWrite = iStrLength;
    }
    iPadding = iWidth - iLengthToWrite;

    /* Right-align, pad before writing the string */
    if (!bIsLeftAlign) {
        for (i = 0; i < iPadding; ++i) {
            *szBuf++ = cPadding;
        }
    }
    /* Write the string */
    for (i = 0; i < iLengthToWrite; ++i) {
        *szBuf++ = szSource[i];
    }
    /* Left-align, pad after writing the string */
    if (bIsLeftAlign) {
        for (i = 0; i < iPadding; ++i) {
            *szBuf++ = cPadding;
        }
    }
    return szBuf;
}

/**
 * @brief Initialize/clear the format specifier structure
 * @param pFormatSpec Pointer to the format specifier to initialize
 * @details Set all fields to default values:
 * - uFlags: FLAG_NONE | FLAG_SPACE_PADDING
 * - iMinWidth: 0
 * - iPrecision: -1
 * - cLength: 0
 */
static void Salvia_ClearFormatSpec(FormatSpec* pFormatSpec) {
    pFormatSpec->uFlags         = FLAG_NONE | FLAG_SPACE_PADDING;
    pFormatSpec->iMinWidth      = 0;
    pFormatSpec->iPrecision     = -1;
    pFormatSpec->cLength        = 0;
}

/**
 * @brief String-to-integer implementation
 * @param pSource Source string
 * @return Converted integer value
 */
static int Salvia_Atoi(const char* pSource) {
    int result = 0;
    while (*pSource != '\0' && Salvia_IsDigit(*pSource)) {
        result = (result << 3) + (result << 1) + (*pSource - '0');
        pSource++;
    }
    return result;
}

static void Salvia_ItoaReverse(char* pStrBuf, int iLength) {
    int iStart = 0;
    int iEnd = iLength -1;
    while (iStart < iEnd) {
        char t = *(pStrBuf + iStart);
        *(pStrBuf + iStart) = *(pStrBuf + iEnd);
        *(pStrBuf + iEnd) = t;
        iStart++;
        iEnd--;
    }
}

/**
 * @brief Integer-to-string conversion with padding and signs
 * @param iNum Integer to convert
 * @param pStrBuf Destination buffer
 * @param iBase Base (2-36)
 * @param bUppercase Whether to use uppercase letters (effective for base 16)
 * @param bForceSign Whether to force a plus sign
 * @param iMinDigit Minimum number of digits (zero-padded if insufficient)
 * @return Returns the destination buffer pointer
 */
static int Salvia_ItoaNoSign(int iNum, char* pStrBuf, int iBase, int bUppercase) {
    int i = 0;
    int bIsNegative = 0;
    char hexDigitStart = bUppercase ? 'A' : 'a';

    if (iNum == 0) {
        pStrBuf[i++] = '0';
        pStrBuf[i] = '\0';
        return bIsNegative;
    }

    if (iNum < 0 && iBase == 10) {
        bIsNegative = 1;
        iNum = -iNum;
    }

    while (iNum != 0) {
        int rem = iNum % iBase;
        pStrBuf[i++] = (rem > 9)? (rem-10) + hexDigitStart : rem + '0';
        iNum = iNum / iBase;
    }

    pStrBuf[i] = '\0';

    Salvia_ItoaReverse(pStrBuf, i);
 
    return bIsNegative;
}

/**
 * @brief Integer-to-string conversion with padding and signs
 * 
 * @param iNum Integer to convert
 * @param pStrBuf Buffer to hold the result
 * @param iBase Base (2-36)
 * @param bUppercase Whether to use uppercase letters (effective for base 16)
 * @param bForceSign Whether to force a plus sign
 * @param iMinDigit Minimum number of digits (zero-padded if insufficient)
 * @return char* Returns the result buffer pointer
 * 
 * @details
 * This function will:
 * 1. Handle the number's sign (negative sign or forced plus sign)
 * 2. Add leading zeros as needed to reach the minimum digit count
 * 3. Support base conversion from 2 to 36
 * 4. Allow uppercase/lowercase formatting for base 16
 */
static char* Salvia_ItoaWithPadding(int iNum, char* pStrBuf, int iBase, int bUppercase, int bForceSign, int iMinDigit) {
    char    szBuf[NUM_STR_MAX];
    char*   pCurBuf = pStrBuf;
    int     bIsNegative;
    int     iStrLength;
    int     iZeroPadding;
    int     i;

    bIsNegative = Salvia_ItoaNoSign(iNum, szBuf, iBase, bUppercase);
    iStrLength = Salvia_StrLen(szBuf);
    iZeroPadding = iMinDigit - iStrLength;
    /* Write the sign */
    if (bIsNegative) {
        *pCurBuf++ = '-';
    }
    else if (bForceSign) {
        *pCurBuf++= '+';
    }
    /* Write zero-padding */
    for (i = 0; i < iZeroPadding; ++i) {
        *pCurBuf++ = '0';
    }
    /* Copy the numeric part */
    Salvia_StrCpy(pCurBuf, szBuf);

    return pStrBuf;
}

/**
 * @brief Double-to-string conversion with precision and signs
 * 
 * @param pStrBuf Buffer to hold the result
 * @param dNum Double to convert
 * @param iPrecision Number of decimal places
 * @param bForceSign Whether to force a plus sign
 * @param bZeroPad Whether to zero-pad between sign and digits
 * @param iMinWidth Minimum field width (for zero-padding)
 * @return char* Returns the result buffer pointer
 */
static char* Salvia_Ftoa(char* pStrBuf, double dNum, int iPrecision, int bForceSign, int bZeroPad, int iMinWidth) {
    char        szIntBuf[NUM_STR_MAX];
    char        szFracBuf[NUM_STR_MAX];
    char        szDigitsBuf[NUM_STR_MAX];
    char*       pCurBuf = pStrBuf;
    char*       pDigitsCur;
    int         bIsNegative = 0;
    int         intPart;
    int         iMultiplier;
    int         i;
    int         j;
    double      frac;
    int         fracPart;
    int         iDigitsLen;
    int         iHasSign;
    int         iZeroPadding;

    if (dNum < 0.0) {
        bIsNegative = 1;
        dNum = -dNum;
    }

    /* Apply rounding */
    if (iPrecision > 0) {
        double rounding = 0.5;
        for (i = 0; i < iPrecision; i++) {
            rounding /= 10.0;
        }
        dNum += rounding;
    } else {
        dNum += 0.5;
    }

    intPart = (int)dNum;

    /* Format the digit part (integer + decimal point + fraction) without sign */
    pDigitsCur = szDigitsBuf;
    Salvia_ItoaNoSign(intPart, szIntBuf, 10, 0);
    Salvia_StrCpy(pDigitsCur, szIntBuf);
    pDigitsCur += Salvia_StrLen(szIntBuf);

    if (iPrecision > 0) {
        iMultiplier = 1;
        for (i = 0; i < iPrecision; i++) {
            iMultiplier *= 10;
        }

        frac = dNum - (double)intPart;
        fracPart = (int)(frac * (double)iMultiplier);

        /* Handle carry-over from rounding */
        if (fracPart >= iMultiplier) {
            fracPart = 0;
        }

        *pDigitsCur++ = '.';

        /* Write fractional part with leading zeros */
        for (j = iPrecision - 1; j >= 0; j--) {
            szFracBuf[j] = (char)('0' + (fracPart % 10));
            fracPart /= 10;
        }
        szFracBuf[iPrecision] = '\0';
        Salvia_StrCpy(pDigitsCur, szFracBuf);
        pDigitsCur += iPrecision;
    }

    *pDigitsCur = '\0';
    iDigitsLen = Salvia_StrLen(szDigitsBuf);

    /* Determine if there will be a sign character */
    iHasSign = (bIsNegative || bForceSign) ? 1 : 0;

    /* Calculate zero-padding needed */
    if (bZeroPad && iMinWidth > iDigitsLen + iHasSign) {
        iZeroPadding = iMinWidth - iDigitsLen - iHasSign;
    } else {
        iZeroPadding = 0;
    }

    /* Write sign */
    if (bIsNegative) {
        *pCurBuf++ = '-';
    } else if (bForceSign) {
        *pCurBuf++ = '+';
    }

    /* Write zero-padding */
    for (i = 0; i < iZeroPadding; i++) {
        *pCurBuf++ = '0';
    }

    /* Copy the digits */
    Salvia_StrCpy(pCurBuf, szDigitsBuf);
    pCurBuf += iDigitsLen;

    *pCurBuf = '\0';

    return pStrBuf;
}