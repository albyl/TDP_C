#ifndef _MY_STRING_
#define _MY_STRING_

#define MAX_STR 4096

#include <stdlib.h>
#include <stdio.h>

char* strdup(char *s);
int lenstr(char* s);
void copia(char* in, char *da, int lung);
char *input_str();

char* str_dup(char *s)
{
	char *ret, *d;
	ret = (char*)malloc(sizeof(char)*(lenstr(s)+1));
	for(d = ret;(*d = *s); s++, d++);
	return ret;
}

int lenstr(char* s)
{
	char *c;
	for(c=s; *c; c++);
	return (c-s);
}

void copia(char* in, char *da, int lung)
{
    while(lung--) in[lung] = da[lung];
}

char *input_str()
{
	char buffer[4096];
	int i;
	for(i=0; (buffer[i] = getchar()) != '\n'; i++);
	buffer[i] = '\0';

	return str_dup(buffer);
}
#endif
