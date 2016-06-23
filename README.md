# ELF Database system
In 2008, Elfdbs is a prouct for project of innovation and entrepreneurship, Lanzhou University.

## Structure
![structure](https://github.com/unanao/Elfdbs/blob/master/document/12.png.png)

## Implement
### ELF analysis and how to read it
ELF File Format  
![ELF Format](https://github.com/unanao/Elfdbs/blob/master/document/img/elf_formate.JPG)

#### Read ELF file header
The begin of the file is ELF header, describ the organization of the file."section" include information of "link view", for example command, data, symbol, relocate symbol and so on.

The structure of "ELF header table"   
```c
typedef struct
{
	unsigned char e_ident[EI_NIDENT];     /* Magic number and other info */
	Elf32_Half    e_type;                 /* Object file type */
	Elf32_Half    e_machine;              /* Architecture */
	Elf32_Word    e_version;              /* Object file version */
	Elf32_Addr    e_entry;                /* Entry point virtual address */
	Elf32_Off     e_phoff;                /* Program header table file offset */
	Elf32_Off     e_shoff;                /* Section header table file offset */
	Elf32_Word    e_flags;                /* Processor-specific flags */
	Elf32_Half    e_ehsize;               /* ELF header size in bytes */
	Elf32_Half    e_phentsize;            /* Program header table entry size */
	Elf32_Half    e_phnum;                /* Program header table entry count */
	Elf32_Half    e_shentsize;            /* Section header table entry size */
	Elf32_Half    e_shnum;                /* Section header table entry count */
	Elf32_Half    e_shstrndx;             /* Section header string table index */
} Elf32_Ehdr;
```

Procedure to read ELF header:  
1.Jump to begin of file header

```c
lseek(fd, 0, SEEK_SET)
```

2.Calculate the size of ELF header  
```c 
size = sizeof(Elf32_Ehdr) 
```

3.read ELF header to the buffer  
```c
read(fd, buf, size) 
```

#### Read Program Header table
Data structure of "program header table"  
```c
typedef struct { 
	Elf32_Word p_type; 
	Elf32_Off  p_offset; 
	Elf32_Addr p_vaddr; 
	Elf32_Addr p_paddr; 
	Elf32_Word p_filesz; 
	Elf32_Word p_memsz; 
	Elf32_Word p_flags; 
	Elf32_Word p_align; 
} Elf32_phdr; 
```


We can get size and offset from ELF Header
* e_phentsize and  e_phnum: size of program header
* e_phoff : offset of program header

Howto read "program header table"  

1.Jump to program header  
```c
lseek(fd,Elf32_Ehdr->e_phoff,SEEK_SET)
```

2.Calculate size of program header table  
```c
size=Elf32_e_phentsize * Elf32_e_phnum;
```

3.Read to buffer  
```c
read(fd,buf,size);
```  

#### Read "Section Header table"
Data structure of "section header table"
```c
typedef struct{ 
	Elf32_Word sh_name; 
	Elf32_Word sh_type; 
	Elf32_Word sh_flags; 
	Elf32_Addr sh_addr; 
	Elf32_Off sh_offset; 
	Elf32_Word sh_size; 
	Elf32_Word sh_link; 
	Elf32_Word sh_info; 
	Elf32_Word sh_addralign; 
	Elf32_Word sh_entsize; 
}Elf32_Shdr; 
```

Get size and offset from ELF header
* e_shentsize and e_shnum : size 
* e_shoff: offset

Procedure for reading "Section header table"  
1.Jump to Section header  
```c
lseek(fd, Elf32_Ehdr->e_shoff, SEEK_SET)
```  
2.Calculate size of "Section header table"  
```c  
size=Elf32_e_shentsize * Elf32_e_shnum;
```  
3.Read "Section Header Table"  
```c
read(fd,buf,size);
```  

#### Read ELF Section
We need 2 steps to read "ELF Section"
* Read Section Header Table 
Section Header table describ section, we can get offset from section header table.

![Section Header table](https://github.com/unanao/Elfdbs/blob/master/document/img/section-header-table.png)

e_shoff of "ELF header" is the offset of Section Header Table, Therefore, we read section header table is read esh_num * e_shentsize bytes from e_shoff   

* Read section
e_shnum is the number of section, and e_shentsize is size of section, unit is byte
![Relation between Section Header table and Secion](https://github.com/unanao/Elfdbs/blob/master/document/img/section-header2section.png)

Sysv ABI defination of section entry
```c
typedef struct {
	Elf32_Word    sh_name;
	Elf32_Word    sh_type;
	Elf32_Word    sh_flags;
	Elf32_Addr    sh_addr;
	Elf32_Off     sh_offset;
	Elf32_Word    sh_size;
	Elf32_Word    sh_link;
	Elf32_Word    sh_info;
	Elf32_Word    sh_addralign;
	Elf32_Word    sh_entsize;
} Elf32_Shdr;
```


Section header table is array of  Elf32_Shdr, number is e_shnum. We can tranverse the array to reade all sections

##### Read Section Header Table
1.Define variable, store section header table  
```c
Elf32_Shdr header[MAX];
```    
2.Jump to Section Header table  
```c
lseek(fd, Elf32_Ehdr->e_shoff, SEEK_SET);
```    
3.Caculate size of Section header table.  
We should considerate the align of cpu.  

```c
if(Elf32_Shdr->sh_entsize%2 ! =0)
	size=Elf32_Shdr->sh_entsize+1;
else
	size=Elf32_Shdr->sh_entsize;
```

#### Read Section   
```c
for (i = 0; i < Elf32_Ehdr->e_shnum; i++) {
	read(fd, &header[i], size);
}
```


## Write ELF to database
1.Use mysql's longblob to store binary data  
```c
CREATE table elf(id int,data longblob);
```    
2.escape the special strings
for example: NUL(ASCII 0)、'\n'、'\r'、'\'’、'''、'" and Control-  
```c
*end++='\'';
end+=mysql_real_escape_string(conn,end,buf,n);
*end++='\'';
*end++=')';
```


## Strip the ELF File
### Find the content can be striped
`
unanao@debian:~/c$ readelf -S hello 
There are 37 section headers, starting at offset 0xd80: 


	Section Headers: 
	  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al 
	  [ 0]                   NULL            00000000 000000 000000 00      0   0  0 
	  [ 1] .interp           PROGBITS        08048134 000134 000013 00   A  0   0  1 
	  [ 2] .note.ABI-tag     NOTE            08048148 000148 000020 00   A  0   0  4 
	  [ 3] .hash             HASH            08048168 000168 000030 04   A  5   0  4 
	  [ 4] .gnu.hash         GNU_HASH        08048198 000198 000024 04   A  5   0  4 
	  [ 5] .dynsym           DYNSYM          080481bc 0001bc 000070 10   A  6   1  4 
	  [ 6] .dynstr           STRTAB          0804822c 00022c 0000a5 00   A  0   0  1 
	  [ 7] .gnu.version      VERSYM          080482d2 0002d2 00000e 02   A  5   0  2 
	  [ 8] .gnu.version_r    VERNEED         080482e0 0002e0 000040 00   A  6   2  4 
	  [ 9] .rel.dyn          REL             08048320 000320 000008 08   A  5   0  4 
	  [10] .rel.plt          REL             08048328 000328 000020 08   A  5  12  4 
	  [11] .init             PROGBITS        08048348 000348 000030 00  AX  0   0  4 
	  [12] .plt              PROGBITS        08048378 000378 000050 04  AX  0   0  4 
	  [13] .text             PROGBITS        080483d0 0003d0 00017c 00  AX  0   0 16 
	  [14] .fini             PROGBITS        0804854c 00054c 00001c 00  AX  0   0  4 
	  [15] .rodata           PROGBITS        08048568 000568 000014 00   A  0   0  4 
	  [16] .eh_frame_hdr     PROGBITS        0804857c 00057c 000014 00   A  0   0  4 
	  [17] .eh_frame         PROGBITS        08048590 000590 000048 00   A  0   0  4 
	  [18] .ctors            PROGBITS        080495d8 0005d8 000008 00  WA  0   0  4 
	  [19] .dtors            PROGBITS        080495e0 0005e0 000008 00  WA  0   0  4 
	  [20] .jcr              PROGBITS        080495e8 0005e8 000004 00  WA  0   0  4 
	  [21] .dynamic          DYNAMIC         080495ec 0005ec 0000e8 08  WA  6   0  4 
	  [22] .got              PROGBITS        080496d4 0006d4 000004 04  WA  0   0  4 
	  [23] .got.plt          PROGBITS        080496d8 0006d8 00001c 04  WA  0   0  4 
	  [24] .data             PROGBITS        080496f4 0006f4 000008 00  WA  0   0  4 
	  [25] .bss              NOBITS          080496fc 0006fc 000008 00  WA  0   0  4 
	  [26] .comment          PROGBITS        00000000 0006fc 0000cb 00      0   0  1 
	  [27] .debug_aranges    PROGBITS        00000000 0007c8 000050 00      0   0  8 
	  [28] .debug_pubnames   PROGBITS        00000000 000818 000025 00      0   0  1 
	  [29] .debug_info       PROGBITS        00000000 00083d 00017f 00      0   0  1 
	  [30] .debug_abbrev     PROGBITS        00000000 0009bc 00006f 00      0   0  1 
	  [31] .debug_line       PROGBITS        00000000 000a2b 000125 00      0   0  1 
	  [32] .debug_str        PROGBITS        00000000 000b50 0000a3 01  MS  0   0  1 
	  [33] .debug_ranges     PROGBITS        00000000 000bf8 000040 00      0   0  8 
	  [34] .shstrtab         STRTAB          00000000 000c38 000147 00      0   0  1 
	  [35] .symtab           SYMTAB          00000000 001348 0004c0 10     36  55  4 
	  [36] .strtab           STRTAB          00000000 001808 00022e 00      0   0  1 
	Key to Flags: 
	  W (write), A (alloc), X (execute), M (merge), S (strings) 
	  I (info), L (link order), G (group), x (unknown) 
	  O (extra OS processing required) o (OS specific), p (processor specific) 


Addr is 0 after .comment, therefore, the content after .comment can be striped. We write other segment into database. We read the it from database will get striped ELF File.

Comparison

| Before Strip | After Strip |
| ------------ | ----------- |
|6710 bytes    | 4548 bytes  |


## Designer and Developer:
* Sun Jianjiao <jianjiaosun@163.com>
* Guo Linli
* Tan Long

## Acknowledgement
Thanks to Wu Zhangjin, give us so much guidence, who founded technical sebsite: [TinyLab] (http://tinylab.org/)
