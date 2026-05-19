/**
 * @file salvia.h
 * @brief Salvia formatted string library header file
 * 
 * Provides printf-like formatting functionality, but more compact and customizable.
 *
 * @author  ForgingMoonLord
 * @date    2025-08-19
 * @version 0.1
 */
#ifndef _SALVIA_H_
#define _SALVIA_H_

/**
 * @brief Write a formatted string into a buffer
 * 
 * @param szBuf Destination buffer that will receive the formatted string
 * @param szFormat Format string, supports:
 *                %d - Integer
 *                %s - String 
 *                %c - Character
 *                %% - Percent sign
 * @param ... Variable arguments corresponding to the placeholders in the format string
 * @return int Number of characters written (excluding the terminating null character)
 * 
 * @example
 *  char buf[100];
 *  Salvia_Format(buf, "Integer:%d，String:%s", 42, "hello");
 */
int Salvia_Format(char* szBuf, const char* szFormat, ...);

#endif /* _SALVIA_H_ */