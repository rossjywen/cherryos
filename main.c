#include <sys/types.h>

extern void trap_init(void);
extern void console_init(void);

void console_write(char* string, uint32_t nr);


int main()
{
	char *p = "hello world\n";

	trap_init();

	console_init();

	console_write(p, 12);
}



