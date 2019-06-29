#include "hhh.h"

enum state { INSIDE_TAG, OUTSIDE_TAG };
static enum state state = OUTSIDE_TAG;
static int found_theme_name;

static void handle_text(char *text)
{
	if (!found_theme_name)
		return;
	printf("%s\n", text);
	exit(0);
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

static void parse_line(char *line)
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

int main(int argc, char **argv)
{
	FILE *fp;
	char line[4096];
	char *p;

	if (argc < 2)
		die("Usage: hhhconf-obtheme <filename>");
	fp = fopen(argv[1], "r");
	if (!fp)
		die("could not open file");
	while (fgets(line, (int)sizeof(line), fp)) {
		if (line[0] == '\0')
			continue;
		p = strrchr(line, '\n');
		if (!p)
			continue;
		*p = '\0';
		parse_line(line);
	}
	fclose(fp);
	return EXIT_SUCCESS;
}
