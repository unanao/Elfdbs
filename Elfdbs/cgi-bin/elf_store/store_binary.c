/*should (1)create the database:test
 *       (2)under database test,create table:
 *            	use test
 *          create table:
 *         	create table test(id int , data longblob);
 *                     
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <mysql/mysql.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <asm/elf.h>

#define host          "localhost"
#define username      "root"
#define password      "xxxxx"/* set the password of the root user here */
#define database      "test"
#define MAX 100    //The max of the section the bianry file have

static int id_distinguish=1;
MYSQL *conn;
MYSQL_RES *res_set;
MYSQL_ROW row;
MYSQL_FIELD *field;
FILE *thefile;

unsigned long phdr_offset,shdr_offset,sect_offset[MAX];//the offset of phdr,shdr,section
unsigned long shdr_num;//sectiong header table's amount
static Elf32_Ehdr ehdr;
static Elf32_Phdr *phdrs=NULL;
static Elf32_Shdr shdrs[MAX];

/*delete the old binary from db*/
void del()
{
	conn=mysql_init(NULL);
	if(conn==NULL)
	{
		printf("init mysql in function del\n");
		exit(1);
	}
	if((mysql_real_connect(conn,host,username,password,database,0,NULL,0))==NULL)
	{
		printf("connect mysql in function del\n");
		exit(1);
	}
	if((mysql_query(conn,"delete from test")))
	{
		printf("delete failed in function del\n");
		exit(0);
	}
}

/*store_binary() can store binay_file to mysql database,*/
void store_binary(char *buf,int n)
{
	off_t size;
	int i, flag;
	char *sql;
	FILE *fp;
	char *end;
	/* allocate a memory space to the SQL statement, it need to big enough to store the whole input binary file */
	sql = (char *) malloc(sizeof(char) *n* 2 + 256);    //2n+1+strlen(other sql)
	if (sql == NULL) 
	{
		perror("malloc sql");
		exit(1);
	}

	/* initilize connection to mysql */
	conn = mysql_init(NULL);
	if (conn == NULL) 
	{
		printf("init mysql, %s\n", mysql_error(conn));
		exit(1);
	}

	/* build the SQL statement for storing the input binary file to mysql */
	sprintf(sql, "insert into test(id,data) values('%d',",id_distinguish);
	end = sql;
	end += strlen(sql);    //point sql tail

	//convert NUL(ASCII 0)、'\n'、'\r'、'\'’、'''、'"', Control-Z and so on
	*end++ = '\'';
	end += mysql_real_escape_string(conn, end, buf,n);
	*end++ = '\'';
	*end++ = ')';

	/* connect to mysql with indicated host, username, password and database */
	if ((mysql_real_connect(conn, host, username, password, database, 0, NULL,0)) == NULL) 
	{
		printf("connect mysql, %s\n", mysql_error(conn));
		exit(1);
	}
	
	/* execute the built SQL statement to store the input binary file to MySQL */
	flag = mysql_real_query(conn, sql, (unsigned int) (end - sql));
	if (flag != 0) 
	{
		printf("insert failed, %s\n", mysql_error(conn));
		exit(1);
	}
	mysql_close(conn);
	free(sql);
	sql = NULL;
	id_distinguish++;//let it add 1,record which position is
}

/*read the elf header table*/
void read_ehdr()
{
	char *buf_ehdr;
	int n;
	if ((buf_ehdr = (char *) malloc(sizeof(char) * (sizeof(ehdr)+1))) == NULL) 
	{
		perror("malloc buf_ehdr");
		exit(1);
	}
	
	if ((n=(fread(buf_ehdr,1,sizeof(ehdr),thefile)))<0) 
	{    
		printf("fread Elf header failed\n");
		exit(1);
	}
	fseek(thefile,0,SEEK_SET);
	if ((n=(fread(&ehdr,1,sizeof(ehdr),thefile)))<0) 
	{    
		printf("fread Elf header failed\n");
		exit(1);
	}

	store_binary(buf_ehdr,n);
	free(buf_ehdr);
	buf_ehdr=NULL;
}

/*read out the ELF program header table*/
void read_phdr()
{
	int size;
	char *buf_phdr;
	int n;
	size=ehdr.e_phentsize*ehdr.e_phnum;
	if ((buf_phdr = (char *) malloc(sizeof(char) * (size+1))) == NULL) 
	{
		perror("malloc buf_phdr");
		exit(1);
	}

	fseek(thefile,ehdr.e_phoff,SEEK_SET);
	phdr_offset=ehdr.e_phoff;
	if((n=(fread(buf_phdr,1,size,thefile)))<0)
	{
		printf("read program header failed is not right\n");
		free(buf_phdr);
		exit(1);
	}
	store_binary(buf_phdr,n);
	
	free(buf_phdr);
	buf_phdr=NULL;
}
/*read out the Elf section header table*/
void read_shdr()
{
	int size;
	int n,i;
	char * buf_shdr;

	size=ehdr.e_shentsize*ehdr.e_shnum;
	shdr_offset=ehdr.e_shoff;
	fseek(thefile,ehdr.e_shoff,SEEK_SET);
	if ((buf_shdr = (char *) malloc(sizeof(char) * (size + 1))) == NULL) 
	{
		perror("malloc buf_shdr");
		exit(1);
	}
	if((n=(fread(buf_shdr,1,size,thefile)))<0)
	{
		printf("read section header failed\n");
		free(buf_shdr);
		exit(1);
	}
	fseek(thefile,ehdr.e_shoff,SEEK_SET);
	for(i=0;i<ehdr.e_shnum;i++)
	{
		if((fread(&shdrs[i],1,ehdr.e_shentsize,thefile))<0)
		{
			printf("read section header failed\n");
			exit(1);
		}
	}
	store_binary(buf_shdr,n);
}

/*read out elf section out */
void read_sect()
{
	int i,size;
	char *buf_section;
	char *sect_name;
	for (i=0;i<ehdr.e_shnum;i++)
	{
		// 第一个是空的，没有用的section，你可以看一下readelf -S hello的结果。
		// 现在我们一起看看，这里就是忽视这个section信息。
		sect_offset[i]=shdrs[i].sh_offset;
//		printf("sect_offset[%d]=%d\n",i,sect_offset[i]);
//		if (shdrs[i].sh_offset == 0) continue;			   

		// 下面这个部分，你刚开始写成了 sizeof(shdrs[i].sh_size)，应该是
		// shdrs[i].sh_size
		size=shdrs[i].sh_size;
		if(size%2!=0)
			size=size+1;
		if ((buf_section = (char *) malloc(sizeof(char) * size + 1)) == NULL) 
		{
			perror("malloc buf");
			exit(1);
		}

		// okay,现在我们调试一下，注意用set args设置这个程序的参数为hello。
		// sh_addralign: 4字节: 04 00 00 00 section的对齐调整值
		fseek(thefile,shdrs[i].sh_offset,SEEK_SET);
		if((fread(buf_section,1,size,thefile))<0)
		{
			printf("read section :%d is not right",i+4);
			exit(1);
		}
		sect_name=shdrs[i].sh_name;
//		printf("shdr[%d]=%s\n",i,sect_name);
		store_binary(buf_section,shdrs[i].sh_size);
		free(buf_section);
		buf_section=NULL;
	}
	
}

/* fetch the binary data from MySQL to a file */
void fetch_out()
{
	char *obf="elfdbs_reduced";   //output file's name;
	char sql_select[100];
	int i,j;   //They are all used for recycle
	FILE *fp;
	unsigned long *length;
	/*inital mysql and connect to the mysql*/
	conn = mysql_init(NULL);
	if (conn == NULL) {
		printf("init mysql, %s\n", mysql_error(conn));
		exit(1);
	}
	if ((mysql_real_connect(conn, host, username, password, database, 0, NULL,0)) == NULL) {
		printf("connect mysql, %s\n", mysql_error(conn));
		exit(1);
	}

	for(j=1;j<=id_distinguish;j++)
	{
		sprintf(sql_select, "select data from test where id='%d'",j);
		if ((mysql_real_query(conn,sql_select,strlen(sql_select)))!= 0) 
		{
			printf("select failed, %s\n", mysql_error(conn));
			exit(1);
		}

		res_set = mysql_store_result(conn);
		if(j==1)
		{
			fp = fopen("/tmp/elfdbs_reduced", "wb");
//			printf("j=%d,%d\n",j,ftell(fp));
			while ((row = mysql_fetch_row(res_set)) != NULL) 
			{
				length = mysql_fetch_lengths(res_set);
				for (i = 0; i < mysql_num_fields(res_set); i++) 
				{
					fwrite(row[0], 1, length[0], fp);
				}
			}
			fclose(fp);
			fp = fopen("/tmp/elfdbs_reduced","r+");
		}
		else
		{
			while ((row = mysql_fetch_row(res_set)) != NULL) 
			{
				length = mysql_fetch_lengths(res_set);
				for (i = 0; i < mysql_num_fields(res_set); i++) 
				{
					if(j==2)
						fseek(fp,phdr_offset,SEEK_SET);
					if(j==3)
						fseek(fp,shdr_offset,SEEK_SET);
					if(j>=4)
						fseek(fp,sect_offset[j-4],SEEK_SET);
//					printf("j=%d,%d\n",j,ftell(fp));
					if(j>=29)
						break;
					fwrite(row[0], 1, length[0], fp);
				}
			}
		}
	}
	fclose(fp);
	mysql_close(conn);
	/**in order to make it is easy to use ftp server for download the 
	 * reduced elf file 
	 * */
	system("chown unanao:unanao /tmp/elfdvs_reduced");
	system("cp /tmp/elfdbs_reduced /home/unanao/tmp");
}


/* main function*/
int main(int argc,char *argv[])
{
	size_t  size;//record the size of every segment
	int i;//It is used as counter in a "for" recycle
	int n;

	/* open the input binary file and copy it to the buffer */
	thefile = fopen("/tmp/elfdbs","rb");
	if (!thefile) 
	{
		printf("open file: elfdbs failed");
		exit(1);
	}

	/*delete the old binary file from db*/
	del();
	/*call store_bianry() function to store the binary_file into database*/
	/*different number represent different headers and sectionfs 
	 *1==> ELF header
	 *2==>program Headers table
	 *3==>section Headers
	 *4==>section1
	 *5==>section2
	 *......
	 * */
	//store the ELF header to db id =1
	read_ehdr();		
	//store the program header to db id =2
	read_phdr();
	//store the section header table to db: 3<=id<=ehdr.e_shnmu+2
	read_shdr();
	//store the section to the db:  e_shnum+3<=id<ehdr.e_shnum+2 
	read_sect();
	//close the file
	fclose(thefile);
//fetch out the binary from the db
	fetch_out();
	return 0;
}
