#include "viewer_path.h"

static int32_t path_is_slash(char c) {
	return (c == '/') | (c == '\\');
}

int32_t path_pop_ext(const char* path, char* out, char* ext) {
	int32_t initial_skipped_periods = 0;
	while (*path == '.') {
		++path;
		++initial_skipped_periods;
	}

	const char* period = path;
	char c;
	
	while ((c = *period++)) {
		if (c == '.') break;
	}

	int32_t has_period = c == '.';
	int32_t len = (int32_t)(period - path) - 1 + initial_skipped_periods;
	
	if (len > PATH_MAX_PATH - 1) {
		len = PATH_MAX_PATH - 1;
	}

	if (out) {
		strncpy(out, path - initial_skipped_periods, len);
		out[len] = 0;
	}

	if (ext) {
		if (has_period) {
			strncpy(ext, path - initial_skipped_periods + len + 1, PATH_MAX_EXT);
		}
		else {
			ext[0] = 0;
		}
	}

	return len;
}

int32_t path_pop(const char* path, char* out, char* pop) {
	const char* original = path;
	int32_t total_len = 0;
	while (*path) {
		++total_len;
		++path;
	}

	// ignore trailing slash from input path
	if (path_is_slash(*(path - 1)))	{
		--path;
		total_len -= 1;
	}

	int32_t pop_len = 0; // length of substring to be popped
	while (!path_is_slash(*--path) && pop_len != total_len) {
		++pop_len;
	}

	int32_t len = total_len - pop_len; // length to copy

    // don't ignore trailing slash if it is the first character
    if (len > 1)
    {
		len -= 1;
    }

	if (len > 0) {
		if (out) {
			strncpy(out, original, len);
			out[len] = 0;
		}

		if (pop) {
			strncpy(pop, path + 1, pop_len);
			pop[pop_len] = 0;
		}

		return len;
	}
	else {
		if (out) {
			out[0] = '.';
			out[1] = 0;
		}

		if (pop) {
			*pop = 0;
		}

		return 1;
	}
}

static int32_t path_strncpy(char* dst, const char* src, 
	int32_t n, int32_t max) {
	int32_t c;

	do {
		if (n >= max - 1) {
			dst[max - 1] = 0;
			break;
		}

		c = *src++;
		dst[n] = c;
		++n;
	} while (c);

	return n;
}

void path_concat(const char* path_a, const char* path_b, char* out, 
	int32_t max_buffer_length) {
	int32_t n = path_strncpy(out, path_a, 0, max_buffer_length);
	n = path_strncpy(out, "/", n - 1, max_buffer_length);
	path_strncpy(out, path_b, n - 1, max_buffer_length);
}
