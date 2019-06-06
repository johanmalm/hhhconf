/*
 * hhhconf-t2.c - set tint2rc key/value pairs
 *
 * Copyright (C) Johan Malm 2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

struct entry {
	char line[1000];
	char key[1000];
	char value[1000];
	int bg_id;
};

static struct entry *entries;
static int nr_entries, alloc_entries;
static int background_id = -1;

static const char hhhconf_t2_usage[] =
"Usage: hhhconf-t2 <filename>\n";

static void usage(void)
{
	printf("%s", hhhconf_t2_usage);
	exit(0);
}

static void die(const char *err, ...)
{
	va_list params;

	fputs("fatal: ", stderr);
	va_start(params, err);
	vfprintf(stderr, err, params);
	va_end(params);
	fputc('\n', stderr);

	exit(1);
}

static void *xrealloc(void *ptr, size_t size)
{
	void *ret = realloc(ptr, size);

	if (!ret)
		die("Out of memory; realloc failed");
	return ret;
}

#define strlcpy _strlcpy
static void _strlcpy(char *dest, const char *src, size_t size)
{
	size_t ret = strlen(src);

	if (size) {
		size_t len = (ret >= size) ? size - 1 : ret;
		memcpy(dest, src, len);
		dest[len] = '\0';
	}
}

static char *trim(char *s)
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

static void split(char *line, char **left, char **right, char delim)
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

static struct entry *add_entry(void)
{
	struct entry *entry;

	if (nr_entries == alloc_entries) {
		alloc_entries = (alloc_entries + 16) * 2;
		entries = xrealloc(entries, alloc_entries * sizeof(*entry));
	}
	entry = entries + nr_entries;
	(void *)memset(entry, 0, sizeof(*entry));
	nr_entries++;
	return entries + nr_entries - 1;
}

static void process_line(char *line)
{
	struct entry *entry;
	char *key = NULL, *value = NULL;

	entry = add_entry();
	strlcpy(entry->line, line, sizeof(entry->line));
	split(line, &key, &value, '#');
	if (!key || !value)
		return;
	strlcpy(entry->key, key, sizeof(entry->key));
	strlcpy(entry->value, value, sizeof(entry->value));

	/*
	 * tint2 uses the key "rounded" to indicate the beginning of a new
	 * background section.
	 */
	if (!strcmp(key, "rounded"))
		++background_id;
	entry->bg_id = background_id;
}

static void read_file(const char *filename)
{
	FILE *fp;
	char line[4096];

	fp = fopen(filename, "r");
	if (!fp)
		die("could not open file %s", filename);
	while (fgets(line, (int)sizeof(line), fp)) {
		char *p;
		if (line[0] == '\0')
			continue;
		p = strrchr(line, '\n');
		if (!p)
			continue;
		*p = '\0';
		process_line(line);
	}
	fclose(fp);
}

static void get_value(const char *key)
{
	int i;

	if (!key)
		die("NULL passed to function '%s'", __func__);
	for (i = 0; i < nr_entries; i++) {
		if (!entries[i].key)
			continue;
		if (!strcmp(entries[i].key, key)) {
			printf("%s\n", entries[i].value);
			break;
		}
	}
}

int main(int argc, char **argv)
{
	if (argc < 2)
		usage();
	read_file(argv[1]);
	get_value("panel_items");
}
