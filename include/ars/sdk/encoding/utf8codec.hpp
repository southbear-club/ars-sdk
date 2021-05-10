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
 * @file utf8codec.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include <assert.h>
#include <string.h>
#include "unicode.hpp"

namespace ars {
    
namespace sdk {

class UTF8Encode
{
public:
	// from mbcs
	UTF8Encode(const char* mbcs) : m_p(NULL)
	{
		assert(mbcs);
		m_null[0] = 0;
		int n = strlen(mbcs) + 1;
		wchar_t* wbuf = new wchar_t[n];
		n = unicode_from_mbcs(mbcs, n, wbuf, sizeof(wchar_t)*n);
		if(n > 0)
		{
			m_p = new char[(n + 1) * 4];
			memset(m_p, 0, (n + 1) * 4);
			unicode_to_utf8(wbuf, n, m_p, (n + 1) * 4);
		}
	}

	// from unicode
	UTF8Encode(const wchar_t* unicode) : m_p(NULL)
	{
		assert(unicode);
		m_null[0] = 0;
		int n = wcslen(unicode) + 1;
		m_p = new char[n * 4];
		memset(m_p, 0, n * 4);
		unicode_to_utf8(unicode, n, m_p, n * 4);
	}

	UTF8Encode(const char* text, const char* encoding) : m_p(NULL)
	{
		assert(text && encoding);
		m_null[0] = 0;
		if(0==strcasecmp("utf-8", encoding) || 0==strcasecmp("utf8", encoding))
		{
			int n = strlen(text) + 1;
			m_p = new char[n];
			strcpy(m_p, text);
		}
		else if(0==strcasecmp("gbk", encoding) || 0==strcasecmp("gb2312", encoding) || 0==strcasecmp("gb18030", encoding))
		{
			int n = strlen(text) + 1;
			wchar_t* wbuf = new wchar_t[n];
			n = unicode_from_gb18030(text, n, wbuf, sizeof(wchar_t)*n);
			if(n > 0)
			{
				m_p = new char[(n + 1) * 4];
				memset(m_p, 0, (n + 1) * 4);
				unicode_to_utf8(wbuf, n, m_p, (n + 1) * 4);
			}
			delete[] wbuf;
		}
		else
		{
			assert(0);
		}
	}

	~UTF8Encode()
	{
		if(m_p)
			delete[] m_p;
	}

	operator const char* () const
	{
		return m_p ? m_p : m_null;
	}

private:
	char* m_p;
	char   m_null[1];
};

class UTF8Decode
{
public:
	UTF8Decode(const char* utf8) : m_pc(NULL), m_pw(NULL)
	{
		assert(utf8);
		size_t n = strlen(utf8) + 1;
		m_pw = new wchar_t[n];
		memset(m_pw, 0, sizeof(wchar_t) * n);
		unicode_from_utf8(utf8, n, m_pw, sizeof(wchar_t) * n);
	}

	~UTF8Decode()
	{
		if(m_pc)
			delete m_pc;
		if(m_pw)
			delete m_pw;
	}

	const char* ToMBCS()
	{
		if (!m_pc && m_pw)
		{
			size_t n = wcslen(m_pw) + 1;
			m_pc = new char[n * 4];
			memset(m_pc, 0, n * 4);
			unicode_to_mbcs(m_pw, n, m_pc, n * 4);
		}
		return m_pc;
	}

	const char* ToGB18030()
	{
		if(!m_pc && m_pw)
		{
			size_t n = wcslen(m_pw) + 1;
			m_pc = new char[n * 4];
			memset(m_pc, 0, n * 4);
			unicode_to_gb18030(m_pw, n, m_pc, n * 4);
		}
		return m_pc;
	}

	// to mbcs
	operator const char* ()
	{
		return ToMBCS();
	}

	// to unicode
	operator const wchar_t* () const
	{
		return m_pw;
	}

private:
	char* m_pc;
	wchar_t* m_pw;
};

} // namespace sdk

} // namespace ars
