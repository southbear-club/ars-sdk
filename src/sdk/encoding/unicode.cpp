/**
 * Copyright © 2021 <wotsen>.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the “Software”), to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * @file unicode.cpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#include "aru/sdk/encoding/unicode.hpp"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

namespace aru {
    
namespace sdk {

static char s_encode[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

#define char_isnum(c)	('0'<=(c) && '9'>=(c))
#define char_isalpha(c) (('a'<=(c) && 'z'>=(c)) || ('A'<=(c) && 'Z'>=(c)))
#define char_isalnum(c) (char_isnum(c) || char_isalpha(c))
#define char_ishex(c)	(char_isnum(c) || ('a'<=(c)&&'f'>=(c)) || ('A'<=(c) && 'F'>=(c)))

static inline char HEX(char c)
{
	return char_isnum(c) ? c-'0' : (char)tolower(c)-'a'+10;
}

// RFC3986
// 2.2 Reserved Characters
// reserved    = gen-delims / sub-delims
// gen-delims  = ":" / "/" / "?" / "#" / "[" / "]" / "@"
// sub-delims  = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / ";" / "="
// 2.3.  Unreserved Characters
// unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~"
int url_encode(const char* source, int srcBytes, char* target, int tgtBytes)
{
	int i, r;
	const char* p = source;

	r = 0;
	for(i=0; *p && (-1==srcBytes || p<source+srcBytes) && i<tgtBytes; ++p,++i)
	{
		if(char_isalnum(*p) || '-'==*p || '_'==*p || '.'==*p || '~'==*p)
		{
			target[i] = *p;
		}
		else if(' ' == *p)
		{
			target[i] = '+';
		}
		else
		{
			if(i+2 >= tgtBytes)
			{
				r = -1;
				break;
			}

			target[i] = '%';
			target[++i] = s_encode[(*p>>4) & 0xF];
			target[++i] = s_encode[*p & 0xF];
		}
	}

	if(i < tgtBytes)
		target[i] = '\0';
	return r < 0 ? r : i;
}

int url_decode(const char* source, int srcBytes, char* target, int tgtBytes)
{
	int i, r;
	const char* p = source;

	r = 0;
	for(i=0; *p && (-1==srcBytes || p<source+srcBytes) && i<tgtBytes; ++p,++i)
	{
		if('+' == *p)
		{
            // query '+' -> ' '
            // other '+' -> '+'
			target[i] = ' ';
		}
		else if('%' == *p)
		{
			if ((-1==srcBytes || p + 1 < source + srcBytes) && '%' == p[1])
			{
				target[i] = '%';
				p += 1;
				continue;
			}

            // https://tools.ietf.org/html/rfc3986#page-21
            // in the host component %-encoding can only be used for non-ASCII bytes.
            // https://tools.ietf.org/html/rfc6874#section-2
            // introduces %25 being allowed to escape a percent sign in IPv6 scoped-address literals
			if(!char_ishex(p[1]) || !char_ishex(p[2]) || (-1!=srcBytes && p+2>=source+srcBytes))
			{
				r = -1;
				break;
			}
			
			target[i] = (char)(HEX(p[1])<<4) | HEX(p[2]);
			p += 2;
		}
		else
		{
			target[i] = *p;
		}
	}

	if(i < tgtBytes)
		target[i] = 0;
	return r < 0 ? r : i;
}
    
} // namespace sdk

} // namespace aru
