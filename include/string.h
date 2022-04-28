#ifndef STRING_H
#define STRING_H

#ifndef NULL
#define NULL ((void *) 0)
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif



char * strcpy(char * dest,const char *src);
char * strncpy(char * dest,const char *src,int count);

char * strcat(char * dest,const char * src);
char * strncat(char * dest,const char * src,int count);

int strcmp(const char * cs,const char * ct);
int strncmp(const char * cs,const char * ct,int count);

char * strchr(const char * s,int c);
char * strrchr(const char * s,int c);

int strspn(const char * cs, const char * ct);
int strcspn(const char * cs, const char * ct);

char * strpbrk(const char * cs,const char * ct);
char * strstr(const char * cs,const char * ct);

int strlen(const char * s);

//extern char * ___strtok;
//char * strtok(char * s,const char * ct);

void * memcpy(void * dest,const void * src, int n);

void * memmove(void * dest,const void * src, int n);

int memcmp(const void * cs,const void * ct,int count);

void * memchr(const void * cs,int c,int count);

void * memset(void * s,int c,int count);


#endif //STRING_H

