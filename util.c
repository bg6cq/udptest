
void fill_buffer(unsigned char *b, int len)
{
	int i;
	for (i = 0; i < len; i++)
		b[i] = i % 256;
}

int check_buffer(unsigned char *b, int len)
{
	int i;
	for (i = 0; i < len; i++)
		if (b[i] != i % 256) {
			printf("buf[%d]=%d error, should be %d\n", i, b[i], i % 256);
			return -1;
		}
	return 0;
}
