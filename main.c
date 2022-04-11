

extern void trap_init(void);
extern void console_init(void);


int main()
{
	trap_init();

	console_init();
}



