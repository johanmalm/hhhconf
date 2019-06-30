/*
 * hhhconf-t2.c - set tint2rc key/value pairs
 *
 * Copyright (C) Johan Malm 2019
 */

#include "hhh.h"

struct entry {
	char line[1000];
	char key[1000];
	char value[1000];
	int bg_id;
};

#define DEFAULT_TINT2RC ".config/tint2/tint2rc"

static struct entry *entries;
static int nr_entries, alloc_entries;
static int background_id;
static bool preserve_last_field = false;

static const char hhhconf_t2_usage[] =
"Usage: hhhconf-t2 [options] [<key>] [<value>]\n"
"Get value - if only <key> is provided\n"
"Set key/value pair - if <value> is provided too\n"
"Options:\n"
"  -f <file>     Specify tint2rc filename (default ~/.config/tint2/tint2rc)\n"
"  -F            Add any missing *_font keys except for execp_font and\n"
"                button_font\n"
"  -h            Show help message\n"
"  -p            Preserve last field of value (useful for setting font name\n"
"                whilst preserving font size)\n"
"  -s <section>  Specify section name, e.g. panel, taskbar, task, task_active\n"
"                This is only required for 'background' key/value pairs\n"
"Examples:\n"
"  hhhconf-t2 task_font\n"
"                Get value of task_font (e.g. 'Sans 10')\n"
"  hhhconf-t2 panel_items TCS\n"
"                Set panel_items to TCS\n"
"  hhhconf-t2 -p task_font \"Futura Bk BT\"\n"
"                Set task font name but keep size (e.g. 'Futura Bk BT 10')\n"
"  hhhconf-t2 -s task_active background_color \"#ff0000 100\"\n"
"                Set 'background_color' associated with 'task_active'\n"
"  hhhconf-t2 -F\n"
"                Add missing *_font variables excl 'execp*' and 'button*'\n";


static char *sections[] = {
	"panel", "taskbar", "taskbar_active", "taskbar_name",
	"taskbar_name_active", "task", "task_active", "task_urgent",
	"task_iconified", "systray", "launcher", "launcher_icon", "clock",
	"battery", "tooltip", NULL
};

static char *background_keys[] = {
	"rounded", "border_width", "border_sides", "background_color",
	"border_color", "background_color_hover", "border_color_hover",
	"background_color_pressed", "border_color_pressed", NULL
};

/*
 * We handle "execp_font" and "button_font" differently as they are special in
 * that they can appear multiple times.
 */
static char *font_keys[] = {
	"task_font", "taskbar_name_font", "time1_font", "time2_font",
	"tooltip_font", "bat1_font", "bat2_font", NULL
};

static void usage(void)
{
	printf("%s", hhhconf_t2_usage);
	exit(0);
}

static bool is_background_key(const char *key)
{
	int i;

	if (!key)
		die("NULL passed to function '%s'", __func__);
	for (i = 0; background_keys[i]; i++)
		if (!strcmp(background_keys[i], key))
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
	(void *)memset(entry, 0, sizeof(*entry));
	nr_entries++;
	return entries + nr_entries - 1;
}

static void add_line(char *line)
{
	struct entry *entry;
	char *key = NULL, *value = NULL;

	entry = add_entry();
	strlcpy(entry->line, line, sizeof(entry->line));
	split(line, &key, &value, '=');
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
	if (is_background_key(key))
		entry->bg_id = background_id;
	else
		entry->bg_id = 0;
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
		add_line(line);
	}
	fclose(fp);
}

static void write_file(const char *filename)
{
	FILE *fp;
	int i;

	fp = fopen(filename, "w");
	if (!fp)
		die("could not open file %s", filename);
	for (i = 0; i < nr_entries; i++)
		fprintf(fp, "%s\n", entries[i].line);
	fclose(fp);
}

static bool get_int(int *ret, const char *string)
{
	long res;
	char *endptr;

	if (!string || *string == '\0')
		fprintf(stderr, "warn: %s empty string", __func__);
	errno = 0;
	res = strtol(string, &endptr, 10);
	if (errno != 0) {
		fprintf(stderr, "warn: %s(): failed\n", __func__);
	} else if (*endptr != '\0') {
		fprintf(stderr, "warn: %s(): nonnumeric character\n", __func__);
	} else if (res > INT_MAX || res < INT_MIN) {
		fprintf(stderr, "warn: %s(): integer out of range\n", __func__);
	} else {
		*ret = (int)res;
		return true;
	}
	return false;
}

/*
 * Return the value of key "<section>_background_id"
 * Return -1 if background id cannot be found
 */
static int section_bg_id(const char *section)
{
	int i;
	char _key[1000];

	if (!section)
		return -1;
	snprintf(_key, sizeof(_key), "%s_background_id", section);
	for (i = 0; i < nr_entries; i++) {
		if (!entries[i].key)
			continue;
		if (!strcmp(entries[i].key, _key)) {
			int ret;
			if (!get_int(&ret, entries[i].value))
				die("badness at: %s = %s", _key,
				    entries[i].value);
			return ret;
		}
	}
	fprintf(stderr, "no background id for section '%s'\n", section);
	return -1;
}

/*
 * We compare "entry.key" + "key", and if appropriate also check that the
 * background_id is right
 */
static bool is_match(const char *section, const char *key, struct entry entry)
{
	if (!entry.key)
		return false;
	if (is_background_key(key))
		if (entry.bg_id != section_bg_id(section))
			return false;
	if (!strcmp(entry.key, key))
		return true;
	return false;
}

static char *last_field(char *value)
{
	if (!preserve_last_field)
		return "";
	char *p = strrchr(value, ' ');
	return p ? ++p : "";
}

static void set_value(const char *section, const char *key, const char *value)
{
	int i;
	static int found;

	if (!key || !value)
		die("NULL passed to function '%s'", __func__);
	for (i = 0; i < nr_entries; i++) {
		if (!is_match(section, key, entries[i]))
			continue;
		found = 1;
		snprintf(entries[i].line, sizeof(entries[i].line), "%s = %s %s",
			 key, value, last_field(entries[i].value));
		fprintf(stderr, "info: ");
		if (section)
			fprintf(stderr, "%s.", section);
		fprintf(stderr, "%s\n", entries[i].line);
		strlcpy(entries[i].key, key, sizeof(entries[i].key));
		strlcpy(entries[i].value, value, sizeof(entries[i].value));
	}
	if (found)
		return;
	/*
	 * Let's not warn about {execp,button}_font, as we don't add them with
	 * option -F anyway.
	 */
	if (!strcmp(key, "execp_font") || !strcmp(key, "button_font"))
		return;
	fprintf(stderr, "warn: key '%s' not found\n", key);
}

static void get_value(const char *section, const char *key)
{
	int i;

	if (!key)
		die("NULL passed to function '%s'", __func__);
	for (i = 0; i < nr_entries; i++) {
		if (!is_match(section, key, entries[i]))
			continue;
		printf("%s\n", entries[i].value);
		break;
	}
}

static void add_missing_font_keys(void)
{
	int i, j;
	char buf[1000];

	for (j = 0; font_keys[j]; j++) {
		for (i = 0; i < nr_entries; i++) {
			if (is_match(NULL, font_keys[j], entries[i]))
				goto out;
		}
		fprintf(stderr, "info: add key: %s\n", font_keys[j]);
		/* default font taken from tint2's panel.h#89 */
		snprintf(buf, sizeof(buf), "%s = Sans 10", font_keys[j]);
		add_line(buf);
out:
		;
	}
}

static void validate_section(const char *section)
{
	int i;

	if (!section)
		die("NULL passed to function '%s'", __func__);
	for (i = 0; sections[i]; i++)
		if (!strcmp(sections[i], section))
			return;
	die("section '%s' is not valid", section);
}

int main(int argc, char **argv)
{
	int i;
	char *section = NULL, *key = NULL, *value = NULL;
	char filename[1000];
	enum action { NONE, GET, SET, ADD_MISSING_FONTS };
	enum action action = NONE;

	if (argc < 2)
		usage();
	snprintf(filename, sizeof(filename), "%s/%s",
		 getenv("HOME"), DEFAULT_TINT2RC);
	for (i = 1; i < argc; i++) {
		const char *arg = argv[i];

		if (*arg == '-') {
			switch (arg[1]) {
			case 'f':
				strlcpy(filename, argv[i + 1], sizeof(filename));
				i++;
				continue;
			case 'F':
				action = ADD_MISSING_FONTS;
				i++;
				continue;
			case 'h':
				usage();
				break;
			case 'p':
				preserve_last_field = true;
				continue;
			case 's':
				section = argv[i + 1];
				validate_section(section);
				i++;
				continue;
			}
			die("unknown argument '%s'", arg);
		}
		if (!key) {
			key = (char *)arg;
			continue;
		}
		if (!value) {
			value = (char *)arg;
			continue;
		}
		die("too many arguments '%s'", arg);
	}

	read_file(filename);

	if (key && value)
		action = SET;
	else if (key)
		action = GET;

	switch (action) {
	case NONE:
		usage();
		break;
	case ADD_MISSING_FONTS:
		add_missing_font_keys();
		write_file(filename);
		return EXIT_SUCCESS;
	case GET:
		get_value(section, key);
		break;
	case SET:
		set_value(section, key, value);
		write_file(filename);
		break;
	}
	return EXIT_SUCCESS;
}
