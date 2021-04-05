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
 * @file uuid.cpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#include "aru/sdk/crypto/uuid.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h> // access

namespace aru {
    
namespace sdk {

// https://en.wikipedia.org/wiki/Universally_unique_identifier
struct uuid_t
{
	uint32_t time_low; // integer giving the low 32 bits of the time
	uint16_t time_mid; // integer giving the middle 16 bits of the time
	uint16_t time_hi_and_version; // 4-bit "version" in the most significant bits, followed by the high 12 bits of the time
	uint16_t clock_seq; // 1-3 bit "variant" in the most significant bits, followed by the 13-15 bit clock sequence
	uint8_t  node[6]; // the 48-bit node id
};

static void uuid_string(struct uuid_t* uuid, char s[37])
{
	snprintf(s, 37, "%08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x",
		uuid->time_low, uuid->time_mid, uuid->time_hi_and_version, uuid->clock_seq,
		(unsigned int)uuid->node[0], (unsigned int)uuid->node[1], (unsigned int)uuid->node[2],
		(unsigned int)uuid->node[3], (unsigned int)uuid->node[4], (unsigned int)uuid->node[5]);
}

static int uuid_have_random_source()
{
	return 0 == access("/dev/random", R_OK)
		|| 0 == access("/dev/urandom", R_OK);
}

static int read_random_bytes(void* buf, int len)
{
	int n, r;
	FILE* fp;
	
	fp = fopen("/dev/urandom", "rb");
	if (NULL == fp)
	{
		fp = fopen("/dev/random", "rb");
		if (NULL == fp)
			return -1;
	}

	for (r = n = 0; n < len; n += r)
	{
		r = (int)fread((uint8_t*)buf + n, 1, len - n, fp);
		if (r <= 0)
			break;
	}

	fclose(fp);
	return r <= 0 ? -1 : 0;
}

static void uuid_generate_random(struct uuid_t* uuid)
{
	//random_get_bytes(uuid, sizeof(uuid));
	//getrandom(uuid, sizeof(uuid), GRND_RANDOM|GRND_NONBLOCK);
	read_random_bytes(uuid, sizeof(*uuid));
	uuid->clock_seq = (uuid->clock_seq & 0x3FFF) | 0x8000;
	uuid->time_hi_and_version = (uuid->time_hi_and_version & 0x0FFF) | 0x4000;
}

static void uuid_generate_time(struct uuid_t* uuid)
{
	srand((unsigned int)time(NULL));
	uuid->time_low = ((uint32_t)rand() << 16) | (uint16_t)rand();
	uuid->time_mid = (uint16_t)rand();
	uuid->time_hi_and_version = (uint16_t)rand();
	uuid->clock_seq = (uint16_t)rand();
	uuid->node[0] = (uint8_t)rand();
	uuid->node[1] = (uint8_t)rand();
	uuid->node[2] = (uint8_t)rand();
	uuid->node[3] = (uint8_t)rand();
	uuid->node[4] = (uint8_t)rand();
	uuid->node[5] = (uint8_t)rand();

	uuid->clock_seq = (uuid->clock_seq & 0x3FFF) | 0x8000;
	uuid->time_hi_and_version = (uuid->time_hi_and_version & 0x0FFF) | 0x4000;
}

void uuid_generate_simple(char s[37])
{
	struct uuid_t uuid;
	if (uuid_have_random_source())
		uuid_generate_random(&uuid);
	else
		uuid_generate_time(&uuid);

	uuid_string(&uuid, s);
}

} // namespace sdk

} // namespace aru
