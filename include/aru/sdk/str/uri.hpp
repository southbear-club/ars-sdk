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
 * @file uri.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#pragma once

namespace aru {

namespace sdk {

struct uri_t
{
	char* scheme;

	char* userinfo;
	char* host;
	int port;

	char* path; // encoded/undecode uri path string, default "/"
	char* query; // encoded/undecode uri query string, NULL if no value
	char* fragment; // encoded/undecode uri fragment string, NULL if no value
};

struct uri_query_t
{
	const char* name;
	int n_name;

	const char* value;
	int n_value;
};

/// URI parser
/// e.g: 1. http://usr:pwd@host:port/path?query#fragment
///      2. usr:pwd@host:port/path?query#fragment
///      3. /path?query#fragment
/// @param[in] uri Uniform Resource Identifier
/// @param[in] len uri length
/// @return NULL if parse failed, other-uri_t pointer, free by uri_free
struct uri_t* uri_parse(const char* uri, int len);

/// @param[in] uri return by uri_parse
void uri_free(struct uri_t* uri);

/// uri path with query + fragment
/// e.g. http://host:port/path?query#fragment --> path?query#fragment
/// @return uri path length
int uri_path(const struct uri_t* uri, char* buf, int len);

int uri_userinfo(const struct uri_t* uri, char* usr, int n1, char* pwd, int n2);

int uri_query(const char* query, const char* end, struct uri_query_t** items);

void uri_query_free(struct uri_query_t** items);

} // namespace sdk

} // namespace aru
