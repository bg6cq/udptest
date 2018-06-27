


void fill_buffer(char * b, int len)
{
	int i;
	for(i=0;i<len;i++)
		b[i] = i%256;
}

void check_buffer(char *b, int len)
{	int i;
	for(i=0;i<len;i++)
		if( b[i] != i% 256) {
			printf("buf[%d]=%d error, should be %d\n",i,b[i], i%256 );
			return;
		}
	printf("packet check ok\n");
}
			
