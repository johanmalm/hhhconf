#include "hhh.h"

void die(const char *err, ...)
{
	va_list params;

	fputs("fatal: ", stderr);
	va_start(params, err);
	vfprintf(stderr, err, params);
	va_end(params);
	fputc('\n', stderr);

	exit(1);
}

void *xrealloc(void *ptr, size_t size)
{
	void *ret = realloc(ptr, size);

	if (!ret)
		die("Out of memory; realloc failed");
	return ret;
}

#define strlcpy _strlcpy
void _strlcpy(char *dest, const char *src, size_t size)
{
	size_t ret = strlen(src);

	if (size) {
		size_t len = (ret >= size) ? size - 1 : ret;
		memcpy(dest, src, len);
		dest[len] = '\0';
	}
}

char *trim(char *s)
{
	size_t len;
	char *end;

	len = strlen(s);
	if (!len)
		return s;
	end = s + len - 1;
	while (end >= s && isspace(*end))
		end--;
	*(end + 1) = '\0';
	while (isspace(*s))
		s++;
	return s;
}

void split(char *line, char **left, char **right, char delim)
{
	char *p;

	p = line;
	while ((p[0] == ' ') || (p[0] == '\t'))
		p++;
	if (p[0] == '\n' || p[0] == delim)
		return;
	p = strchr(line, '=');
	if (!p)
		return;
	p[0] = '\0';
	*left = trim(line);
	*right = trim(++p);
	p = strchr(p, '\n');
	if (p)
		p[0] = '\0';
}
