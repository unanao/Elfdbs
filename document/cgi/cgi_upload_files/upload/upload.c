#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>

#define SERVER_NAME cgiServerName

void File()
{
	cgiFilePtr file;
	char name[1024];
	char contentType[1024];
	char buffer[1024];
	int size;
	int got;
	FILE *fp;
	char temp[256];
	int realwrite;
	
	if (cgiFormFileName("file", name, sizeof(name)) != cgiFormSuccess) {
		printf("<p>No file was uploaded.<p>\n");
		return;
	} 
	fprintf(cgiOut, "The filename submitted was: ");
	cgiHtmlEscape(name);
	sprintf(temp,"touch /tmp/%s",name);
	system(temp);
	fprintf(cgiOut, "<p>\n");
	cgiFormFileSize("file", &size);
	fprintf(cgiOut, "The file size was: %d bytes<p>\n", size);
	cgiFormFileContentType("file", contentType, sizeof(contentType));
	fprintf(cgiOut, "The alleged content type of the file was: ");
	cgiHtmlEscape(contentType);
	fprintf(cgiOut, "<p>\n");
	if (cgiFormFileOpen("file", &file) != cgiFormSuccess) 
	{
		fprintf(cgiOut, "Could not open the file.<p>\n");
		return;
	}
	fprintf(cgiOut, "<pre>\n");
	
	sprintf(temp,"/tmp/%s",name);
	fp=fopen(temp,"w");
		
	while (cgiFormFileRead(file, buffer, sizeof(buffer), &got) ==cgiFormSuccess)
	{
		
		fwrite(buffer,1,got,fp);
		//cgiHtmlEscapeData(buffer, got);
		//cgiHtmlEscapeData(buffer, got);
	}
	fclose(fp);
	fprintf(cgiOut, "</pre>\n");
	cgiFormFileClose(file);
}

int cgiMain()
{
	char name[81],mail[81],homepage[81],suggest[1024];
	
	/* Send the content type, letting the browser know this is HTML */
	cgiHeaderContentType("text/html");
	/* Top of the page */
	fprintf(cgiOut, "<HTML><HEAD>\n");
	fprintf(cgiOut, "<TITLE>CGIC Library test result</TITLE></HEAD>\n");
	fprintf(cgiOut, "<BODY><H1>Responses</H1>\n");
	
	if ((cgiFormSubmitClicked("send") == cgiFormSuccess))
	{
		
		File();
	}
	fprintf(cgiOut, "</BODY></HTML>\n");
	return 0;
}
