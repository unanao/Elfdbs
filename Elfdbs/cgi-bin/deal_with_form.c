#include <stdio.h>
#include <stdlib.h>

int main()
{
	char *data;
	long m,n;
	printf("Content-Type:text/html\n\n");
	printf("<title>multiplication results</title>\n");
	printf("<h3>multiplication results</h3>\n");
	data=getenv("QUERY_STRING");
	if(data==NULL)
		printf("error");
	else if(sscanf(data,"m=%ld&n=%ld",&m,&n)!=2)
		printf("error args !=2");
	else
		printf("the product of %d and %d is %d \n",m,n,m*n);
	return 0;
}
