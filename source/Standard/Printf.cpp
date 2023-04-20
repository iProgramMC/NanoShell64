//  ***************************************************************
//  Printf.cpp - Creation date: 05/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
//  
//  Module description:
//      This module implements printf-style functions.
//
//  ***************************************************************

#include <NanoShell.hpp>

#define IS_64_BIT 1

static void UnsignedToString(uint64_t num, char* str, int paddingInfo, char paddingChar)
{
	// print the actual digits themselves
	int i = 0;
	while (num || i == 0)
	{
		str[i++] = '0' + (num % 10);
		str[i]   = '\0';
		num /= 10;
	}

	// append padding too
	for (; i < paddingInfo; )
	{
		str[i++] = paddingChar;
		str[i]   = '\0';
	}

	// reverse the string
	int start = 0, end = i - 1;
	while (start < end)
	{
		char
		temp       = str[start];
		str[start] = str[end];
		str[end]   = temp;
		start++;
		end--;
	}
}

void SignedToString(int64_t num, char* str, int paddingInfo, char paddingChar)
{
	if (num < 0)
	{
		str[0] = '-';
		UnsignedToString((uint64_t)(-num), str + 1, paddingInfo, paddingChar);
	}
	else
		UnsignedToString((uint64_t)  num,  str,     paddingInfo, paddingChar);
}

extern "C" {

int vsnprintf(char* buf, size_t sz, const char* fmt, va_list args)
{
	int  paddingInfo = -1;
	char paddingChar = ' ';
	size_t currentIndex = 0;
	while (*fmt)
	{
		char m = *fmt;
		if (!m) goto finished;
		fmt++;

		if (m == '%')
		{
			m = *(fmt++);

			// if hit end, return
			if (!m) goto finished;

			// handle %0 or %.
			if (m == '0' || m == '.')
			{
				// this by default handles %0<anything> too, though it does nothing
				paddingInfo = 0;
				m = *(fmt++);

				// if hit end, return
				if (!m) goto finished;

				// handle %0D<anything> cases (D = digit)
				if (m >= '0' && m <= '9')
				{
					paddingInfo = m - '0';
					paddingChar = '0';
					m = *(fmt++);
				}
			}
			else if (m >= '1' && m <= '9')
			{
				paddingInfo = m - '0';
				paddingChar = ' ';
				m = *(fmt++);

				// if hit end, return
				if (!m) goto finished;
			}

			switch (m)
			{
				// Format a string
				case 's': case 'S':
				{
					const char* pString = va_arg(args, const char*);

					//allow user to print null
					if (pString == NULL)
						pString = "(null)";

					while (*pString)
					{
						if (currentIndex >= sz - 1)
							goto finished;

						// place this character here
						buf[currentIndex++] = *pString;

						pString++;
					}

					break;
				}
				// Escape a percentage symbol
				case '%':
				{
					if (currentIndex >= sz - 1)
						goto finished;

					buf[currentIndex++] = '%';
					break;
				}
				// Format a char
				case 'c': case 'C':
				{
					// using va_arg(args, char) has undefined behavior, because
					// char arguments will be promoted to int.
					char character = (char)va_arg(args, int);

					if (currentIndex >= sz - 1)
						goto finished;

					buf[currentIndex++] = character;
					break;
				}
				// Format an int
				case 'd': case 'i': case 'D': case 'I':
				{
					int num = va_arg(args, int);
					char buffer[20];
					
					SignedToString(num, buffer, paddingInfo, paddingChar);

					const char* pString = buffer;
					while (*pString)
					{
						if (currentIndex >= sz - 1)
							goto finished;

						// place this character here
						buf[currentIndex++] = *pString;

						pString++;
					}

					break;
				}
				// Format an unsigned int
				#if !IS_64_BIT
				case 'z':
				{
					m = *(fmt++);
					if (m == 0) goto finished;
					//fallthrough intended
					[[fallthrough]];
				}
				#endif
				case 'u': case 'U':
				{
					uint32_t num = va_arg(args, uint32_t);
					char buffer[20];
					
					UnsignedToString(num, buffer, paddingInfo, paddingChar);

					const char* pString = buffer;
					while (*pString)
					{
						if (currentIndex >= sz - 1)
							goto finished;

						// place this character here
						buf[currentIndex++] = *pString;

						pString++;
					}

					break;
				}
				// Format a longer integer.
				#if IS_64_BIT
				case 'z':
				#endif
				case 'l':
				{
					bool longlong = false;
					#if IS_64_BIT
					if (m == 'z')
					{
						longlong = true;
						m = 'u';
						goto __parse_the_thing;
					}
					#endif
					m = *(fmt++);
					if (m == 'l')
					{
						longlong = true;
						m = *(fmt++);
					}
					if (m == 0) goto finished;
					
					#if IS_64_BIT
				__parse_the_thing:
					#endif
					
					const char* pString = NULL;
					char buffer[30];
					buffer[0] = 0;
					pString = buffer;
					
					if (m == 'u')
					{
						unsigned long long num = longlong ? va_arg(args, unsigned long long) : va_arg(args, unsigned long);
						UnsignedToString(num, buffer, paddingInfo, paddingChar);
					}
					if (m == 'd')
					{
						long long num = longlong ? va_arg(args, long long) : va_arg(args, long);
						SignedToString(num, buffer, paddingInfo, paddingChar);
					}
					
					if (!pString) break;
					
					while (*pString)
					{
						if (currentIndex >= sz - 1)
							goto finished;
	
						// place this character here
						buf[currentIndex++] = *pString;
	
						pString++;
					}
					break;
				}
				// Format a uint8_t as lowercase/uppercase hexadecimal
				case 'b':
				case 'B':
				{
					const char* charset = "0123456789abcdef";
					if (m == 'B')
						charset = "0123456789ABCDEF";

					// using va_arg(args, uint8_t) has undefined behavior, because
					// uint8_t arguments will be promoted to int.
					uint8_t p = (uint8_t) va_arg(args, uint32_t);

					if (currentIndex >= sz - 1) goto finished;
					buf[currentIndex++] = charset[(p & 0xF0) >> 4];
					if (currentIndex >= sz - 1) goto finished;
					buf[currentIndex++] = charset[p & 0x0F];

					break;
				}
				// Format a uint16_t as lowercase/uppercase hexadecimal
				case 'w':
				case 'W':
				{
					const char* charset = "0123456789abcdef";
					if (m == 'W')
						charset = "0123456789ABCDEF";

					// using va_arg(args, uint16_t) has undefined behavior, because
					// uint16_t arguments will be promoted to int.
					uint16_t p = (uint16_t) va_arg(args, uint32_t);

					for (uint32_t mask = 0xF000, bitnum = 12; mask; mask >>= 4, bitnum -= 4)
					{
						if (currentIndex >= sz - 1)
							goto finished;

						// place this character here
						buf[currentIndex++] = charset[(p & mask) >> bitnum];
					}

					break;
				}
				// Format a uint32_t as lowercase/uppercase hexadecimal
				case 'x':
				case 'X':
#if !IS_64_BIT
				case 'p': case 'P':
#endif
				{
					const char* charset = "0123456789abcdef";
					if (m == 'X' || m == 'P')
						charset = "0123456789ABCDEF";

					uint32_t p = va_arg(args, uint32_t);

					for (uint32_t mask = 0xF0000000, bitnum = 28; mask; mask >>= 4, bitnum -= 4)
					{
						if (currentIndex >= sz - 1)
							goto finished;

						// place this character here
						buf[currentIndex++] = charset[(p & mask) >> bitnum];
					}

					break;
				}
				// Format a uint64_t as lowercase/uppercase hexadecimal
				case 'q':
				case 'Q':
#if IS_64_BIT
				case 'p': case 'P':
#endif
				{
					const char* charset = "0123456789abcdef";
					if (m == 'Q' || m == 'P')
						charset = "0123456789ABCDEF";

					uint64_t p = va_arg(args, uint64_t);

					for (uint64_t mask = 0xF000000000000000ULL, bitnum = 60; mask; mask >>= 4, bitnum -= 4)
					{
						if (currentIndex >= sz - 1)
							goto finished;

						// place this character here
						buf[currentIndex++] = charset[(p & mask) >> bitnum];
					}

					break;
				}
			}
		}
		else
		{
			if (currentIndex >= sz - 1)
				goto finished;
			buf[currentIndex++] = m;
		}
	}
finished:
	buf[currentIndex] = '\0';
	return (int)currentIndex;
}

int vsprintf(char* buf, const char* fmt, va_list args)
{
	return vsnprintf(buf, SIZE_MAX, fmt, args);
}

int snprintf(char* buf, size_t sz, const char* fmt, ...)
{
	va_list lst;
	va_start(lst, fmt);

	int val = vsnprintf(buf, sz, fmt, lst);

	va_end(lst);

	return val;
}

int sprintf(char* buf, const char* fmt, ...)
{
	va_list lst;
	va_start(lst, fmt);

	int val = vsprintf(buf, fmt, lst);

	va_end(lst);

	return val;
}

}; // extern "C"
