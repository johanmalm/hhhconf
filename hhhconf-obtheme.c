#include "hhh.h"

enum state { INSIDE_TAG, OUTSIDE_TAG };
static enum state state = OUTSIDE_TAG;
static int found_theme_name;
static char theme_name[256] = { 0 };

struct entry {
	char color[16];
};

static struct entry *entries;
static int nr_entries, alloc_entries;

static bool is_duplicate_color(char *color)
{
	int i;

	for (i = 0; i < nr_entries; i++)
		if (!strcmp(entries[i].color, color))
			return true;
	return false;
}

static struct entry *add_entry(void)
{
	struct entry *entry;

	if (nr_entries == alloc_entries) {
		alloc_entries = (alloc_entries + 16) * 2;
		entries = xrealloc(entries, alloc_entries * sizeof(*entry));
	}
	entry = entries + nr_entries;
	(void)memset(entry, 0, sizeof(*entry));
	nr_entries++;
	return entries + nr_entries - 1;
}

void parse_themerc_line(char *line)
{
	char *key = NULL, *value = NULL;
	struct entry *entry;

	split(line, &key, &value, ':');
	if (!key || !value)
		return;
	if (value[0] == '#') {
		if (is_duplicate_color(value))
			return;
		entry = add_entry();
		strlcpy(entry->color, value, sizeof(entry->color));
	}
}

void process_themerc_file(const char *filename)
{
	FILE *fp;
	char line[4096];
	char *p;

	if (!filename)
		die("'%s' no filename", __func__);
	fp = fopen(filename, "r");
	if (!fp)
		die("could not open themerc file");
	while (fgets(line, (int)sizeof(line), fp)) {
		if (line[0] == '\0')
			continue;
		p = strrchr(line, '\n');
		if (!p)
			continue;
		*p = '\0';
		parse_themerc_line(line);
	}
	fclose(fp);
}

static void handle_text(char *text)
{
	if (!found_theme_name)
		return;
	snprintf(theme_name, sizeof(theme_name), "%s", text);
}

static void handle_tag(char *tag)
{
	static int inside_theme;

	if (!tag)
		return;
	if (!strcmp(tag, "theme")) {
		inside_theme = 1;
		return;
	}
	if (!strcmp(tag, "/theme"))
		die("gone past theme");
	if (!inside_theme)
		return;
	if (!strcmp(tag, "name"))
		found_theme_name = 1;
}

static void parse_xml_line(char *line)
{
	char *tag = NULL, *text = NULL;
	size_t i, len = strlen(line);

	for (i = 0; i < len; i++) {
		switch (state) {
		case OUTSIDE_TAG:
			if (!text)
				text = line;
			if (line[i] != '<')
				continue;
			tag = line + i + 1;
			line[i] = '\0';
			handle_text(text);
			state = INSIDE_TAG;
			break;
		case INSIDE_TAG:
			if (!tag)
				tag = line;
			if (line[i] != '>')
				continue;
			text = line + i + 1;
			line[i] = '\0';
			handle_tag(tag);
			state = OUTSIDE_TAG;
			break;
		}
	}
}

void process_xml_file(const char *filename)
{
	FILE *fp;
	char line[4096];
	char *p;

	if (!filename)
		die("'%s' no filename", __func__);
	fp = fopen(filename, "r");
	if (!fp)
		die("could not open rc.xml");
	while (fgets(line, (int)sizeof(line), fp)) {
		if (line[0] == '\0')
			continue;
		p = strrchr(line, '\n');
		if (!p)
			continue;
		*p = '\0';
		parse_xml_line(line);
		if (theme_name[0] != '\0')
			break;
	}
	fclose(fp);
}

int main(int argc, char **argv)
{
	char themerc[1000];
	int i;

	if (argc < 2)
		die("Usage: hhhconf-obtheme <filename>");
	process_xml_file(argv[1]);
	if (theme_name[0] == '\0')
		die("could not find theme name");
	fprintf(stderr, "info: rc.xml obtheme '%s'\n", theme_name);
	snprintf(themerc, sizeof(themerc), "%s/.themes/%s/openbox-3/themerc",
		 getenv("HOME"), theme_name);
	process_themerc_file(themerc);
	for (i = 0; i < nr_entries; i++)
		printf("%s\n", entries[i].color);
	return EXIT_SUCCESS;
}
