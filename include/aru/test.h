typedef struct {
	int a;
	short b;
	struct test {
		int a;
		int b;
		int c[20];
	};
} St1;

struct {
	St1 st1;
	St1 st2[2];
	St1 st3[2][2];
} hh;
