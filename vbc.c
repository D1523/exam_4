#include <stdio.h>
#include <ctype.h>

char *s;
int e;

int f(void)
{
	if (!e++)
		printf(*s ? "Unexpected token '%c'\n" : "Unexpected end of input\n", *s);
	return 0;
}

int p(int l)
{
	int n;

	if (l == 2) {
		if (isdigit(*s)) return *s++ - '0';
		if (*s++ != '(') return s--, f();
		n = p(0);
		return *s == ')' ? (s++, n) : f();
	}
	n = p(l + 1);
	while (!e && *s == "+*"[l] && ++s)
		n = l ? n * p(2) : n + p(1);
	return n;
}

int main(int ac, char **av)
{
	if (ac != 2) return 1;
	s = av[1];
	int n = p(0);
	if (*s) f();
	return e ? 1 : (printf("%d\n", n), 0);
}
