#ifndef STRING_UTILS_H

#define STRING_UTILS_H

#pragma once
#include <cstring>
#include <iostream>

char* concatenate_strings(uint32_t string_count, const char* const* strings) {
	size_t output_size = 1;
	for (uint32_t i = 0; i != string_count; ++i) {
		output_size += strlen(strings[i]);
	}
	char* result = (char*)malloc(output_size);
	size_t output_length = 0;
	for (uint32_t i = 0; i != string_count; ++i) {
		size_t length = strlen(strings[i]);
		memcpy(result + output_length, strings[i], sizeof(char) * length);
		output_length += length;
	}
	result[output_length] = 0;
	return result;
}

#endif // !STRING_UTILS_H