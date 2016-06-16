#include <stdio.h>
#include <string.h>
main()
{
	printf("Contenttype:text/htmlnn");
	printf("<html>n");
	printf("<head><title>An TML Page From a CGI</title></h ead>n");
	printf("<body>n");
	printf("<h2> This is an HTML page generated from with i n a CGI program.. 　　.</h2>n");
	printf("<hr><p>n");
	printf("<a href="hello.cgi"><b> Go back to out put.:html page </b></a>n");
	printf("</body>n");
	printf("</html>n");
	fflush(stdout);
} 
