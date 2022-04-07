void phase_1(char *str)
{
	if (strings_not_equal(str, "Public speaking is very easy."))
		explode_bomb();
}

void phase_2(char *str)
{
	int n[6];

	read_six_numbers(str, n);

	if (n[0] != 1)
		explode_bomb();
	for (int i = 1; i != 6; ++i)
		if (n[i] != (i + 1) * n[i - 1])
			explode_bomb();
/*
	1! = 1
	2! = 2 * 1 = 2
	3! = 3 * 2 = 6
	4! = 4 * 6 = 24
	5! = 5 * 24 = 120
	6! = 6 * 120 = 720

*/
}

void phase_3(char *str)
{
	int a, c;
	char b;
	char ref;

	if (sscanf(str, "%d %c %d", &a, &b, &c) < 3)
		explode_bomb();
	switch (a) {
	/* ... */
	case 1:
		ref = 'b';
		if (c != 214) // 0x08048c02 <+106>: cmp DWORD PTR [ebp-0x4],0xd6
			explode_bomb();
		break;
	/* ... */
	default:
		ref = 'x';
		explode_bomb();
	}
	if (b != ref)
		explode_bomb();
}

int func4(int n)
{
	int ret;

	if (n <= 1)
		return 1;
	return func4(n - 2) + func4(n - 1);
}

void phase_4(char *str)
{
	int n;

	if (sscanf(str, "%d", &n) != 1 || n < 1)
		explode_bomb();
	if (func4(n) != 0x37)
		explode_bomb();
}

void phase_5(char *str)
{
	char *charset = "isrveawhobpnutfg";

	if (strlen(str) != 6)
		explode_bomb();
	for (int i = 0; i != 6; ++i)
		str[i] = charset[str[i] & 0xf];
	if (strcmp(str, "giants"))
		explode_bomb();
}

void phase_6(char *str)
{
	int tab[6] = {253, 725, 301, 997, 212, 432};
	int input[6];
	int sorted[6];

	read_six_numbers(str, input);

	for (int i = 0; i != 6; ++i) {
		// check if all numbers are below or equal to 6
		if (input[i] > 6)
			explode_bomb();
		// check if all numbers are distinct
		for (int j = 0; j != i; ++j) {
			if (input[i] == input[j])
				explode_bomb();
		}
		// sort array based on input[6]
		sorted[i] = tab[input[i]-1];
	}

	// check if array is in descending order
	for (int i = 0; i != 5; ++i) {
		if (sorted[i] < sorted[i+1])
			explode_bomb();
	}
}
