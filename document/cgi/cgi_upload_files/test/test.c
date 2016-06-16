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

void handlePlainText()
{
	char value[81];
	
	cgiFormStringNoNewlines("plaintext", value, 81);
	fprintf(cgiOut, "Plaintext test:: ");
	cgiHtmlEscape(value);
	fprintf(cgiOut, "<BR>\n");
}

void handleMultipleText()
{
	char value[1024];
	
	cgiFormString("suggest", value, 81);
	fprintf(cgiOut, "Multiple-Line Field test: ");
	cgiHtmlEscape(value);
	fprintf(cgiOut, "<BR>\n");
}

void handleSingleCheckBox()
{
	if (cgiFormCheckboxSingle("Firstcome") == cgiFormSuccess) 
	{
		fprintf(cgiOut, "Check Box test:New guest!<BR>\n");
	} 
	else 
	{
		fprintf(cgiOut, "Check Box test:Old guest!<BR>\n");
	}
}

void handleNumbericValue()
{
	double temperature;
	
	cgiFormDoubleBounded("temperature", &temperature, 90.0, 120.0, 100.1);//min,max,default
	fprintf(cgiOut, "Numeric value test: %f.<BR>\n", temperature);
}

void handleIntegerValue()
{
	int pressure;
	
	cgiFormInteger("pressure", &pressure, 0);
	fprintf(cgiOut, "Integer value test:%d<BR>\n",pressure);
}
void handleSelect()
{
	char *colors[] = 
	{
	"Red",
	"Green",
	"Blue"
	};
	int colorChoice;
	
	cgiFormSelectSingle("colors", colors, 3, &colorChoice, 0);
	fprintf(cgiOut, "Select test:%s<BR>\n", colors[colorChoice]);
}
void handleMultipleSelect()
{
	char *animals[] = 
	{
	"chicken",
	"bull",
	"duck"
	};
	int animalChoices[3];//3 animals so there are 3 choices
	int i;
	int result;	
	int invalid;
	
	if (cgiFormSelectMultiple("animals", animals, 3, animalChoices, &invalid)==cgiFormNotFound)
		fprintf(cgiOut, "multiple Select test:No animal is chosen.<p>\n");
	else
	{
		fprintf(cgiOut, "multiple Select test:animals are:\n");
		for (i=0; (i < 3); i++) 
		{
			if (animalChoices[i]) 
				fprintf(cgiOut, "%d. %s\n",i, animals[i]);
		}
		fprintf(cgiOut, "<br>\n");
	}
}

void handleRadioButton()
{
	char *ages[] = 
	{
	"10",
	"20",
	"30",
	"40"
	};
	//method 1
	int ageChoice;
	/* Approach #1: check for one of several valid responses. 
		Good if there are a short list of possible button values and
		you wish to enumerate them. */
	cgiFormRadio("age", ages, 4, &ageChoice, 0);
	fprintf(cgiOut, "Exclusive radio button Test:%s (method #1)<BR>\n",ages[ageChoice]);
	
	//method 2 Joey says:It is more stable than first method...
	char ageText[10];
	/* Approach #2: just get the string. Good
		if the information is not critical or if you wish
		to verify it in some other way. Note that if
		the information is numeric, cgiFormInteger,
		cgiFormDouble, and related functions may be
		used instead of cgiFormString. */	
	cgiFormString("age", ageText, 10);
	fprintf(cgiOut, "Exclusive radio button Test: %s (method #2)<BR>\n", ageText);
}

void handleCheckBoxGroup()
{	
	char *votes[] = 
	{
		"A",
		"B",
		"C",
		"D"
	};
	int voteChoices[4];
	int i;
	int result;	
	int invalid;

	char **responses;

	/* Method #1: check for valid votes. This is a good idea,
		since votes for nonexistent candidates should probably
		be discounted... */
	fprintf(cgiOut, "Votes (method 1):<BR>\n");
	result = cgiFormCheckboxMultiple("vote", votes, 4,voteChoices, &invalid);
	if (result == cgiFormNotFound) 
	{
		fprintf(cgiOut, "Check box group test:None is chosen<p>\n");
	} 
	else 
	{	
		fprintf(cgiOut, "Check box group test::\n");
		fprintf(cgiOut, "<ul>\n");
		for (i=0; (i < 4); i++) 
		{
			if (voteChoices[i]) 
			{
				fprintf(cgiOut, "<li>%s\n", votes[i]);
			}
		}
		fprintf(cgiOut, "</ul>\n");
	}

	/* Method #2: get all the names voted for and trust them.
		This is good if the form will change more often
		than the code and invented responses are not a danger
		or can be checked in some other way. */
	fprintf(cgiOut, "Votes (method 2):<BR>\n");
	result = cgiFormStringMultiple("vote", &responses);
	if (result == cgiFormNotFound) 
	{	
		fprintf(cgiOut, "Check box group test:None is chosen<p>\n");
	} 
	else 
	{
		int i = 0;
		fprintf(cgiOut, "Check box group test:\n");
		fprintf(cgiOut, "<ul>\n");
		while (responses[i]) 
		{
			fprintf(cgiOut, "<li>%s\n", responses[i]);
			i++;
		}
		fprintf(cgiOut, "</ul>\n");
	}
	/* We must be sure to free the string array or a memory
		leak will occur. Simply calling free() would free
		the array but not the individual strings. The
		function cgiStringArrayFree() does the job completely. */	
	cgiStringArrayFree(responses);
}

void Cookies()
{
	char **array, **arrayStep;
	char cname[1024], cvalue[1024];
	fprintf(cgiOut, "Cookies Submitted On This Call, With Values (Many Browsers NEVER Submit Cookies):<p>\n");
	if (cgiCookies(&array) != cgiFormSuccess) 
	{
		return;
	}
	arrayStep = array;
	fprintf(cgiOut, "<table border=1>\n");
	fprintf(cgiOut, "<tr><th>Cookie<th>Value</tr>\n");
	while (*arrayStep)
	{
		char value[1024];
		fprintf(cgiOut, "<tr>");
		fprintf(cgiOut, "<td>");
		cgiHtmlEscape(*arrayStep);
		fprintf(cgiOut, "<td>");
		cgiCookieString(*arrayStep, value, sizeof(value));
		cgiHtmlEscape(value);
		fprintf(cgiOut, "\n");
		arrayStep++;
	}
	fprintf(cgiOut, "</table>\n");
	cgiFormString("cname", cname, sizeof(cname));	
	cgiFormString("cvalue", cvalue, sizeof(cvalue));	
	if (strlen(cname)) 
	{
		fprintf(cgiOut, "New Cookie Set On This Call:<p>\n");
		fprintf(cgiOut, "Name: ");	
		cgiHtmlEscape(cname);
		fprintf(cgiOut, "Value: ");	
		cgiHtmlEscape(cvalue);
		fprintf(cgiOut, "<p>\n");
		fprintf(cgiOut, "If your browser accepts cookies (many do not), this new cookie should appear in the above list the next time the form is submitted.<p>\n"); 
	}
	cgiStringArrayFree(array);
}

void CookieSet()
{
	char cname[1024];
	char cvalue[1024];
	/* Must set cookies BEFORE calling cgiHeaderContentType */
	cgiFormString("cname", cname, sizeof(cname));	
	cgiFormString("cvalue", cvalue, sizeof(cvalue));	
	if (strlen(cname))
	 {
		/* Cookie lives for one day (or until browser chooses
			to get rid of it, which may be immediately),
			and applies only to this script on this site. */	
		cgiHeaderCookieSetString(cname, cvalue,86400, cgiScriptName, SERVER_NAME);
	}
}
	
int cgiMain()
{
	char name[81],mail[81],homepage[81],suggest[1024];
	
	CookieSet();//set cookies before we read them
	/* Send the content type, letting the browser know this is HTML */
	cgiHeaderContentType("text/html");
	/* Top of the page */
	fprintf(cgiOut, "<HTML><HEAD>\n");
	fprintf(cgiOut, "<TITLE>CGIC Library test result</TITLE></HEAD>\n");
	fprintf(cgiOut, "<BODY><H1>Responses</H1>\n");
	
	if ((cgiFormSubmitClicked("send") == cgiFormSuccess))
	{
		
		handlePlainText();
		handleMultipleText();
		handleSingleCheckBox();
		handleNumbericValue();
		handleIntegerValue();
		handleSelect();
		handleMultipleSelect();
		handleRadioButton();
		handleCheckBoxGroup();
		Cookies();
		File();
	}
	fprintf(cgiOut, "</BODY></HTML>\n");
	return 0;
}
