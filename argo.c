#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct json {
	enum { MAP, INTEGER, STRING } type;
	union {
		struct { struct pair *data; size_t size; } map;
		int integer;
		char *string;
	};
} json;

typedef struct pair {
	char *key;
	json value;
} pair;

int argo(json *dst, FILE *stream);

int peek(FILE *stream)
{
	int c = getc(stream);
	ungetc(c, stream);
	return c;
}

void unexpected(FILE *stream)
{
	if (peek(stream) != EOF)
		printf("unexpected token '%c'\n", peek(stream));
	else
		printf("unexpected end of input\n");
}

int accept(FILE *stream, char c)
{
	if (peek(stream) == c)
	{
		(void)getc(stream);
		return 1;
	}
	return 0;
}

int expect(FILE *stream, char c)
{
	if (accept(stream, c))
		return 1;
	unexpected(stream);
	return 0;
}

void free_json(json j)
{
	switch (j.type)
	{
		case MAP:
			for (size_t i = 0; i < j.map.size; i++)
			{
				free(j.map.data[i].key);
				free_json(j.map.data[i].value);
			}
			free(j.map.data);
			break;
		case STRING:
			free(j.string);
			break;
		default:
			break;
	}
}

void serialize(json j)
{
	switch (j.type)
	{
		case INTEGER:
			printf("%d", j.integer);
			break;
		case STRING:
			putchar('"');
			for (int i = 0; j.string[i]; i++)
			{
				if (j.string[i] == '\\' || j.string[i] == '"')
					putchar('\\');
				putchar(j.string[i]);
			}
			putchar('"');
			break;
		case MAP:
			putchar('{');
			for (size_t i = 0; i < j.map.size; i++)
			{
				if (i != 0)
					putchar(',');
				serialize((json){.type = STRING, .string = j.map.data[i].key});
				putchar(':');
				serialize(j.map.data[i].value);
			}
			putchar('}');
			break;
	}
}

int main(int argc, char **argv)
{
	if (argc != 2)
		return 1;
	FILE *stream = fopen(argv[1], "r");
	json file;
	if (argo(&file, stream) != 1)
	{
		free_json(file);
		return 1;
	}
	serialize(file);
	printf("\n");
	free_json(file);
}

int str(char **s, FILE *f)
{
	int n = 0, c;

	*s = calloc(1, 1);
	while ((c = getc(f)) != '"') {
		if (c == '\\') c = getc(f);
		if (c == EOF) return unexpected(f), -1;
		*s = realloc(*s, n + 2);
		(*s)[n++] = c, (*s)[n] = 0;
	}
	return 1;
}

int parse(json *d, FILE *f)
{
	*d = (json){0};
	if (accept(f, '"'))
		return d->type = STRING, str(&d->string, f);
	if (isdigit(peek(f)) || peek(f) == '-')
		return d->type = INTEGER, fscanf(f, "%d", &d->integer) == 1 ? 1 : (unexpected(f), -1);
	if (!expect(f, '{')) return -1;
	if (accept(f, '}')) return 1;
	do {
		d->map.data = realloc(d->map.data, ++d->map.size * sizeof(pair));
		pair *p = &d->map.data[d->map.size - 1];
		*p = (pair){0};
		if (!expect(f, '"') || str(&p->key, f) < 0 || !expect(f, ':') || parse(&p->value, f) < 0)
			return -1;
	} while (accept(f, ','));
	return expect(f, '}') ? 1 : -1;
}

int argo(json *dst, FILE *f)
{
	if (parse(dst, f) < 0) return -1;
	return peek(f) == EOF ? 1 : (unexpected(f), -1);
}
