
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#define SARMAG 8

/* ELF Header */

#define EI_NIDENT       16              /* Size of e_ident[] */

typedef unsigned int bfd_vma;

typedef struct elf_internal_ehdr {
  unsigned char         e_ident[EI_NIDENT]; /* ELF "magic number" */
  unsigned int         e_entry;        /* Entry point virtual address */
  unsigned int         e_phoff;        /* Program header table file offset */
  unsigned int         e_shoff;        /* Section header table file offset */
  unsigned long         e_version;      /* Identifies object file version */
  unsigned long         e_flags;        /* Processor-specific flags */
  unsigned short        e_type;         /* Identifies object file type */
  unsigned short        e_machine;      /* Specifies required architecture */
  unsigned int          e_ehsize;       /* ELF header size in bytes */
  unsigned int          e_phentsize;    /* Program header table entry size */
  unsigned int          e_phnum;        /* Program header table entry count */
  unsigned int          e_shentsize;    /* Section header table entry size */
  unsigned int          e_shnum;        /* Section header table entry count */
  unsigned int          e_shstrndx;     /* Section header string table index */
} Elf_Internal_Ehdr;

/* Section header */

typedef struct elf_internal_shdr {
  unsigned int  sh_name;                /* Section name, index in string tbl */
  unsigned int  sh_type;                /* Type of section */
  unsigned int  sh_flags;               /* Miscellaneous section attributes */
  unsigned int  sh_addr;                /* Section virtual addr at execution */
  unsigned int sh_size;                /* Size of section in bytes */
  unsigned int sh_entsize;             /* Entry size if section holds table */
  unsigned long sh_link;                /* Index of another section */
  unsigned long sh_info;                /* Additional section information */
  unsigned long sh_offset;              /* Section file offset */
  unsigned int  sh_addralign;           /* Section alignment */

  /* The internal rep also has some cached info associated with it. */
  void*    bfd_section;                 /* Associated BFD section.  */
  unsigned char *contents;              /* Section contents.  */
} Elf_Internal_Shdr;

/* Symbol table entry */

struct elf_internal_sym {
  unsigned int  st_value;               /* Value of the symbol */
  unsigned int  st_size;                /* Associated symbol size */
  unsigned long st_name;                /* Symbol name, index in string tbl */
  unsigned char st_info;                /* Type and binding attributes */
  unsigned char st_other;               /* Visibilty, and target specific */
  unsigned int  st_shndx;               /* Associated section index */
};

/* Structure for syminfo section.  */
typedef struct
{
  unsigned short int    si_boundto;
  unsigned short int    si_flags;
} Elf_Internal_Syminfo;


typedef struct elf_internal_sym Elf_Internal_Sym;



/* ELF Header (32-bit implementations) */

typedef struct {
  unsigned char e_ident[16];            /* ELF "magic number" */
  unsigned char e_type[2];              /* Identifies object file type */
  unsigned char e_machine[2];           /* Specifies required architecture */
  unsigned char e_version[4];           /* Identifies object file version */
  unsigned char e_entry[4];             /* Entry point virtual address */
  unsigned char e_phoff[4];             /* Program header table file offset */
  unsigned char e_shoff[4];             /* Section header table file offset */
  unsigned char e_flags[4];             /* Processor-specific flags */
  unsigned char e_ehsize[2];            /* ELF header size in bytes */
  unsigned char e_phentsize[2];         /* Program header table entry size */
  unsigned char e_phnum[2];             /* Program header table entry count */
  unsigned char e_shentsize[2];         /* Section header table entry size */
  unsigned char e_shnum[2];             /* Section header table entry count */
  unsigned char e_shstrndx[2];          /* Section header string table index */
} Elf32_External_Ehdr;

typedef struct {
  unsigned char sh_name[4];             /* Section name, index in string tbl */
  unsigned char sh_type[4];             /* Type of section */
  unsigned char sh_flags[4];            /* Miscellaneous section attributes */
  unsigned char sh_addr[4];             /* Section virtual addr at execution */
  unsigned char sh_offset[4];           /* Section file offset */
  unsigned char sh_size[4];             /* Size of section in bytes */
  unsigned char sh_link[4];             /* Index of another section */
  unsigned char sh_info[4];             /* Additional section information */
  unsigned char sh_addralign[4];        /* Section alignment */
  unsigned char sh_entsize[4];          /* Entry size if section holds table */
} Elf32_External_Shdr;

/* Symbol table entry */

typedef struct {
  unsigned char st_name[4];             /* Symbol name, index in string tbl */
  unsigned char st_value[4];            /* Value of the symbol */
  unsigned char st_size[4];             /* Associated symbol size */
  unsigned char st_info[1];             /* Type and binding attributes */
  unsigned char st_other[1];            /* No defined meaning, 0 */
  unsigned char st_shndx[2];            /* Associated section index */
} Elf32_External_Sym;

typedef struct {
  unsigned char est_shndx[4];           /* Section index */
} Elf_External_Sym_Shndx;


#define SHN_UNDEF       0               /* Undefined section reference */
#define SHN_LORESERVE   0xFF00          /* Begin range of reserved indices */
#define SHN_LOPROC      0xFF00          /* Begin range of appl-specific */
#define SHN_HIPROC      0xFF1F          /* End range of appl-specific */
#define SHN_LOOS        0xFF20          /* OS specific semantics, lo */
#define SHN_HIOS        0xFF3F          /* OS specific semantics, hi */
#define SHN_ABS         0xFFF1          /* Associated symbol is absolute */
#define SHN_COMMON      0xFFF2          /* Associated symbol is in common */
#define SHN_XINDEX      0xFFFF          /* Section index is held elsewhere */
#define SHN_HIRESERVE   0xFFFF          /* End range of reserved indices */
#define SHN_BAD         ((unsigned) -1) /* Used internally by bfd */

/* Values for section header, sh_type field.  */

#define SHT_NULL        0               /* Section header table entry unused */
#define SHT_PROGBITS    1               /* Program specific (private) data */
#define SHT_SYMTAB      2               /* Link editing symbol table */
#define SHT_STRTAB      3               /* A string table */
#define SHT_RELA        4               /* Relocation entries with addends */
#define SHT_HASH        5               /* A symbol hash table */
#define SHT_DYNAMIC     6               /* Information for dynamic linking */
#define SHT_NOTE        7               /* Information that marks file */
#define SHT_NOBITS      8               /* Section occupies no space in file */
#define SHT_REL         9               /* Relocation entries, no addends */
#define SHT_SHLIB       10              /* Reserved, unspecified semantics */
#define SHT_DYNSYM      11              /* Dynamic linking symbol table */

#define SHT_INIT_ARRAY    14            /* Array of ptrs to init functions */
#define SHT_FINI_ARRAY    15            /* Array of ptrs to finish functions */
#define SHT_PREINIT_ARRAY 16            /* Array of ptrs to pre-init funcs */
#define SHT_GROUP         17            /* Section contains a section group */
#define SHT_SYMTAB_SHNDX  18            /* Indicies for SHN_XINDEX entries */

#define STB_LOCAL       0               /* Symbol not visible outside obj */
#define STB_GLOBAL      1               /* Symbol visible outside obj */
#define STB_WEAK        2               /* Like globals, lower precedence */
#define STB_LOOS        10              /* OS-specific semantics */
#define STB_HIOS        12              /* OS-specific semantics */
#define STB_LOPROC      13              /* Application-specific semantics */
#define STB_HIPROC      15              /* Application-specific semantics */

#define STT_NOTYPE      0               /* Symbol type is unspecified */
#define STT_OBJECT      1               /* Symbol is a data object */
#define STT_FUNC        2               /* Symbol is a code object */
#define STT_SECTION     3               /* Symbol associated with a section */
#define STT_FILE        4               /* Symbol gives a file name */
#define STT_COMMON      5               /* An uninitialised common block */
#define STT_TLS         6               /* Thread local data object */
#define STT_RELC        8               /* Complex relocation expression */
#define STT_SRELC       9               /* Signed Complex relocation expression */
#define STT_LOOS        10              /* OS-specific semantics */
#define STT_HIOS        12              /* OS-specific semantics */
#define STT_LOPROC      13              /* Application-specific semantics */
#define STT_HIPROC      15              /* Application-specific semantics */


/* The following constants control how a symbol may be accessed once it has
   become part of an executable or shared library.  */

#define STV_DEFAULT     0               /* Visibility is specified by binding type */
#define STV_INTERNAL    1               /* OS specific version of STV_HIDDEN */
#define STV_HIDDEN      2               /* Can only be seen inside currect component */
#define STV_PROTECTED   3               /* Treat as STB_LOCAL inside current component */



#define ELF_ST_BIND(val)                (((unsigned int)(val)) >> 4)
#define ELF_ST_TYPE(val)                ((val) & 0xF)
#define ELF_ST_INFO(bind,type)          (((bind) << 4) + ((type) & 0xF))

#define ELF_ST_VISIBILITY(v)            ((v) & 0x3)



static char *string_table;
static unsigned long string_table_length;
static Elf_Internal_Ehdr elf_header;
static Elf_Internal_Shdr *section_headers;
static Elf_Internal_Sym *dynamic_symbols;
static Elf_Internal_Syminfo *dynamic_syminfo;
static unsigned long dynamic_syminfo_offset;
static unsigned int dynamic_syminfo_nent;
static char *dynamic_strings;
static Elf_Internal_Shdr *symtab_shndx_hdr;

static unsigned long num_dynamic_syms;
static unsigned int eh_addr_size;
static unsigned long dynamic_strings_length;

unsigned int (*byte_get) (unsigned char *, int);

/* This is just a bit of syntatic sugar.  */
#define streq(a,b)        (strcmp ((a), (b)) == 0)
#define strneq(a,b,n)     (strncmp ((a), (b), (n)) == 0)
#define const_strneq(a,b) (strncmp ((a), (b), sizeof (b) - 1) == 0)

/* Given st_shndx I, map to section_headers index.  */
#define SECTION_HEADER_INDEX(I)                         \
  ((I) < SHN_LORESERVE                                  \
   ? (I)                                                \
   : ((I) <= SHN_HIRESERVE                              \
      ? 0                                               \
      : (I) - (SHN_HIRESERVE + 1 - SHN_LORESERVE)))

/* Reverse of the above.  */
#define SECTION_HEADER_NUM(N)                           \
  ((N) < SHN_LORESERVE                                  \
   ? (N)                                                \
   : (N) + (SHN_HIRESERVE + 1 - SHN_LORESERVE))

#define SECTION_HEADER(I) (section_headers + SECTION_HEADER_INDEX (I))

#define SECTION_NAME(X) \
  ((X) == NULL ? "<none>" \
  : string_table == NULL ? "<no-name>" \
  : ((X)->sh_name >= string_table_length ? "<corrupt>" \
  : string_table + (X)->sh_name))

#define GET_ELF_SYMBOLS(file, section)    get_32bit_elf_symbols (file, section) 


#define BYTE_GET(field) byte_get (field, sizeof (field))


void *
cmalloc (size_t nmemb, size_t size)
{
  /* Check for overflow.  */
  if (nmemb >= ~(size_t) 0 / size)
    return NULL;
  else
    return malloc (nmemb * size);
}

unsigned int
byte_get_little_endian (unsigned char *field, int size)
{
  switch (size)
    {
    case 1:
      return *field;

    case 2:
      return  ((unsigned int) (field[0]))
        |    (((unsigned int) (field[1])) << 8);

    case 4:
      return  ((unsigned long) (field[0]))
        |    (((unsigned long) (field[1])) << 8)
        |    (((unsigned long) (field[2])) << 16)
        |    (((unsigned long) (field[3])) << 24);

    default:
      printf("Unhandled data length: %d\n", size);
      abort ();
    }
}

static void *
get_data (void *var, FILE *file, long offset, size_t size, size_t nmemb)
{
  void *mvar;

  if (size == 0 || nmemb == 0)
    return NULL;

  if (fseek (file, offset, SEEK_SET))
    {
      printf ("Unable to seek to 0x%lx \n", offset);
      return NULL;
    }

  mvar = var;
  if (mvar == NULL)
    {
      /* Check for overflow.  */
      if (nmemb < (~(size_t) 0 - 1) / size)
        /* + 1 so that we can '\0' terminate invalid string table sections.  */
        mvar = malloc (size * nmemb + 1);

      if (mvar == NULL)
        {
          printf ("Out of memory allocating 0x%lx bytes\n", (unsigned long)(size * nmemb));
          return NULL;
        }

      ((char *) mvar)[size * nmemb] = '\0';
    }

  if (fread (mvar, size, nmemb, file) != nmemb)
    {
      printf ("Unable to read in 0x%lx bytes of %s\n", (unsigned long)(size * nmemb));
      if (mvar != var)
        free (mvar);
      return NULL;
    }

  return mvar;
}

static const char *
get_symbol_visibility (unsigned int visibility)
{
  switch (visibility)
    {
    case STV_DEFAULT:   return "DEFAULT";
    case STV_INTERNAL:  return "INTERNAL";
    case STV_HIDDEN:    return "HIDDEN";
    case STV_PROTECTED: return "PROTECTED";
    default: abort ();
    }
}

static const char *
get_symbol_index_type (unsigned int type)
{
  static char buff[32];

  switch (type)
    {
    case SHN_UNDEF:     return "UND";
    case SHN_ABS:       return "ABS";
    case SHN_COMMON:    return "COM";
    default:
        sprintf (buff, "%3d", type);
      break;
    }

  return buff;
}


static const char *
get_symbol_other (unsigned int other)
{
  const char * result = NULL;
  static char buff [32];

  if (other == 0)
    return "";

  snprintf (buff, sizeof buff, "<other>: %x", other);
  return buff;
}



static const char *
get_symbol_binding (unsigned int binding)
{
  static char buff[32];

  switch (binding)
    {
    case STB_LOCAL:     return "LOCAL";
    case STB_GLOBAL:    return "GLOBAL";
    case STB_WEAK:      return "WEAK";
    default:
        snprintf (buff, sizeof (buff), "<unknown>: %d", binding);
      return buff;
    }
}


static const char *
get_symbol_type (unsigned int type)
{
  static char buff[32];

  switch (type)
    {
    case STT_NOTYPE:    return "NOTYPE";
    case STT_OBJECT:    return "OBJECT";
    case STT_FUNC:      return "FUNC";
    case STT_SECTION:   return "SECTION";
    case STT_FILE:      return "FILE";
    case STT_COMMON:    return "COMMON";
    case STT_TLS:       return "TLS";
    case STT_RELC:      return "RELC";
    case STT_SRELC:     return "SRELC";
    default:
        snprintf (buff, sizeof (buff), "<unknown>: %d", type);
      return buff;
    }
}

static int
get_32bit_section_headers (FILE *file, unsigned int num)
{
  Elf32_External_Shdr *shdrs;
  Elf_Internal_Shdr *internal;
  unsigned int i;

  shdrs = get_data (NULL, file, elf_header.e_shoff,
                    elf_header.e_shentsize, num);
  if (!shdrs)
    return 0;

  section_headers = cmalloc (num, sizeof (Elf_Internal_Shdr));

  if (section_headers == NULL)
    {
      printf ("Out of memory\n");
      return 0;
    }

  for (i = 0, internal = section_headers;
       i < num;
       i++, internal++)
    {
      internal->sh_name      = BYTE_GET (shdrs[i].sh_name);
      internal->sh_type      = BYTE_GET (shdrs[i].sh_type);
      internal->sh_flags     = BYTE_GET (shdrs[i].sh_flags);
      internal->sh_addr      = BYTE_GET (shdrs[i].sh_addr);
      internal->sh_offset    = BYTE_GET (shdrs[i].sh_offset);
      internal->sh_size      = BYTE_GET (shdrs[i].sh_size);
      internal->sh_link      = BYTE_GET (shdrs[i].sh_link);
      internal->sh_info      = BYTE_GET (shdrs[i].sh_info);
      internal->sh_addralign = BYTE_GET (shdrs[i].sh_addralign);
      internal->sh_entsize   = BYTE_GET (shdrs[i].sh_entsize);
    }

  free (shdrs);

  return 1;
}

static Elf_Internal_Sym *
get_32bit_elf_symbols (FILE *file, Elf_Internal_Shdr *section);

static int
process_section_headers (FILE *file)
{
  Elf_Internal_Shdr *section;
  unsigned int i;

  section_headers = NULL;

  if (elf_header.e_shnum == 0)
    {
	printf ("\nThere are no sections in this file.\n");
        return 1;
    }

  if (! get_32bit_section_headers (file, elf_header.e_shnum))
    return 0;

  /* Read in the string table, so that we have names to display.  */
  if (elf_header.e_shstrndx != SHN_UNDEF
       && SECTION_HEADER_INDEX (elf_header.e_shstrndx) < elf_header.e_shnum)
    {
      section = SECTION_HEADER (elf_header.e_shstrndx);

      if (section->sh_size != 0)
	{
	  string_table = get_data (NULL, file, section->sh_offset,
				   1, section->sh_size);

	  string_table_length = string_table != NULL ? section->sh_size : 0;
	}
    }

  /* Scan the sections for the dynamic symbol table
     and dynamic string table and debug sections.  */
  dynamic_symbols = NULL;
  dynamic_strings = NULL;
  dynamic_syminfo = NULL;
  symtab_shndx_hdr = NULL;

  eh_addr_size = 4;

  for (i = 0, section = section_headers;
       i < elf_header.e_shnum;
       i++, section++)
    {
      char *name = SECTION_NAME (section);

      if (section->sh_type == SHT_DYNSYM)
	{
	  if (dynamic_symbols != NULL)
	    {
	      printf ("File contains multiple dynamic symbol tables\n");
	      continue;
	    }

	  num_dynamic_syms = section->sh_size / section->sh_entsize;
	  dynamic_symbols = GET_ELF_SYMBOLS (file, section);
	}
      else if (section->sh_type == SHT_STRTAB
	       && streq (name, ".dynstr"))
	{
	  if (dynamic_strings != NULL)
	    {
	      printf ("File contains multiple dynamic string tables\n");
	      continue;
	    }

	  dynamic_strings = get_data (NULL, file, section->sh_offset,
				      1, section->sh_size);
	  dynamic_strings_length = section->sh_size;
	}
      else if (section->sh_type == SHT_SYMTAB_SHNDX)
	{
	  if (symtab_shndx_hdr != NULL)
	    {
	      printf ("File contains multiple symtab shndx tables\n");
	      continue;
	    }
	  symtab_shndx_hdr = section;
	}
    }

  return 1;
}

static Elf_Internal_Sym *
get_32bit_elf_symbols (FILE *file, Elf_Internal_Shdr *section)
{
  unsigned long number;
  Elf32_External_Sym *esyms;
  Elf_External_Sym_Shndx *shndx;
  Elf_Internal_Sym *isyms;
  Elf_Internal_Sym *psym;
  unsigned int j;

  esyms = get_data (NULL, file, section->sh_offset, 1, section->sh_size);
  if (!esyms)
    return NULL;

  shndx = NULL;
  if (symtab_shndx_hdr != NULL
      && (symtab_shndx_hdr->sh_link
	  == (unsigned long) SECTION_HEADER_NUM (section - section_headers)))
    {
      shndx = get_data (NULL, file, symtab_shndx_hdr->sh_offset,
			1, symtab_shndx_hdr->sh_size);
      if (!shndx)
	{
	  free (esyms);
	  return NULL;
	}
    }

  number = section->sh_size / section->sh_entsize;
  isyms = cmalloc (number, sizeof (Elf_Internal_Sym));

  if (isyms == NULL)
    {
      printf ("Out of memory\n");
      if (shndx)
	free (shndx);
      free (esyms);
      return NULL;
    }

  for (j = 0, psym = isyms;
       j < number;
       j++, psym++)
    {
      psym->st_name  = BYTE_GET (esyms[j].st_name);
      psym->st_value = BYTE_GET (esyms[j].st_value);
      psym->st_size  = BYTE_GET (esyms[j].st_size);
      psym->st_shndx = BYTE_GET (esyms[j].st_shndx);
      if (psym->st_shndx == SHN_XINDEX && shndx != NULL)
	psym->st_shndx
	  = byte_get ((unsigned char *) &shndx[j], sizeof (shndx[j]));
      psym->st_info  = BYTE_GET (esyms[j].st_info);
      psym->st_other = BYTE_GET (esyms[j].st_other);
    }

  if (shndx)
    free (shndx);
  free (esyms);

  return isyms;
}

/* How to print a vma value.  */
typedef enum print_mode
{
  HEX,
  DEC,
  DEC_5,
  UNSIGNED,
  PREFIX_HEX,
  FULL_HEX,
  LONG_HEX
}
print_mode;


/* Print a VMA value.  */
static int
print_vma (bfd_vma vma, print_mode mode)
{
    {
      switch (mode)
        {
        case FULL_HEX:
          return printf ("0x%8.8lx", (unsigned long) vma);

        case LONG_HEX:
          return printf ("%8.8lx", (unsigned long) vma);

        case DEC_5:
          if (vma <= 99999)
            return printf ("%5ld", (long) vma);
          /* Drop through.  */

        case PREFIX_HEX:
          return printf ("0x%lx", (unsigned long) vma);

        case HEX:
          return printf ("%lx", (unsigned long) vma);

        case DEC:
          return printf ("%ld", (unsigned long) vma);

        case UNSIGNED:
          return printf ("%lu", (unsigned long) vma);
        }
    }

  return 0;
}

/* Display a symbol on stdout.  If do_wide is not true then
   format the symbol to be at most WIDTH characters,
   truncating as necessary.  If WIDTH is negative then
   format the string to be exactly - WIDTH characters,
   truncating or padding as necessary.  */

static void
print_symbol (int width, const char *symbol)
{
  if (width < 0)
    printf ("%-*.*s", width, width, symbol);
  else
    printf ("%-.*s", width, symbol);
}



/* Dump the symbol table.  */
static int
process_symbol_table (FILE *file)
{
  Elf_Internal_Shdr *section;
    {
      unsigned int i;

      for (i = 0, section = section_headers;
	   i < elf_header.e_shnum;
	   i++, section++)
	{
	  unsigned int si;
	  char *strtab = NULL;
	  unsigned long int strtab_size = 0;
	  Elf_Internal_Sym *symtab;
	  Elf_Internal_Sym *psym;


	  if (section->sh_type != SHT_SYMTAB)
	    continue;

	  printf ("\nSymbol table '%s' contains %lu entries:\n",
		  SECTION_NAME (section),
		  (unsigned long) (section->sh_size / section->sh_entsize));
	  printf ("   Num:    Value  Size Type    Bind   Vis      Ndx Name\n");

	  symtab = GET_ELF_SYMBOLS (file, section);
	  if (symtab == NULL)
	    continue;

	  if (section->sh_link == elf_header.e_shstrndx)
	    {
	      strtab = string_table;
	      strtab_size = string_table_length;
	    }
	  else if (SECTION_HEADER_INDEX (section->sh_link) < elf_header.e_shnum)
	    {
	      Elf_Internal_Shdr *string_sec;

	      string_sec = SECTION_HEADER (section->sh_link);

	      strtab = get_data (NULL, file, string_sec->sh_offset,
				 1, string_sec->sh_size);
	      strtab_size = strtab != NULL ? string_sec->sh_size : 0;
	    }

	  for (si = 0, psym = symtab;
	       si < section->sh_size / section->sh_entsize;
	       si++, psym++)
	    {
	      printf ("%6d: ", si);
	      print_vma (psym->st_value, LONG_HEX);
	      putchar (' ');
	      print_vma (psym->st_size, DEC_5);
	      printf (" %-7s", get_symbol_type (ELF_ST_TYPE (psym->st_info)));
	      printf (" %-6s", get_symbol_binding (ELF_ST_BIND (psym->st_info)));
	      printf (" %-3s", get_symbol_visibility (ELF_ST_VISIBILITY (psym->st_other)));
	      /* Check to see if any other bits in the st_other field are set.
	         Note - displaying this information disrupts the layout of the
	         table being generated, but for the moment this case is very rare.  */
	      if (psym->st_other ^ ELF_ST_VISIBILITY (psym->st_other))
		printf (" [%s] ", get_symbol_other (psym->st_other ^ ELF_ST_VISIBILITY (psym->st_other)));
	      printf (" %4s ", get_symbol_index_type (psym->st_shndx));
	      print_symbol (25, psym->st_name < strtab_size
			    ? strtab + psym->st_name : "<corrupt>");
	      putchar ('\n');
	    }

	  free (symtab);
	  if (strtab != string_table)
	    free (strtab);
	}
    }

  return 1;
}

/* Dump the symbol table.  */
static int
get_symbol_fileoff (FILE *file, unsigned char *reqsymname, unsigned int *symsize)
{
  Elf_Internal_Shdr *section;

      unsigned int i;
      unsigned int found = 0, symaddr;

      for (i = 0, section = section_headers;
	   i < elf_header.e_shnum;
	   i++, section++)
	{
	  unsigned int si;
	  char *strtab = NULL;
	  unsigned long int strtab_size = 0;
	  Elf_Internal_Sym *symtab;
	  Elf_Internal_Sym *psym;


	  if (section->sh_type != SHT_SYMTAB)
	    continue;

	  symtab = GET_ELF_SYMBOLS (file, section);
	  if (symtab == NULL)
	    continue;

	  if (section->sh_link == elf_header.e_shstrndx)
	    {
	      strtab = string_table;
	      strtab_size = string_table_length;
	    }
	  else if (SECTION_HEADER_INDEX (section->sh_link) < elf_header.e_shnum)
	    {
	      Elf_Internal_Shdr *string_sec;

	      string_sec = SECTION_HEADER (section->sh_link);

	      strtab = get_data (NULL, file, string_sec->sh_offset,
				 1, string_sec->sh_size);
	      strtab_size = strtab != NULL ? string_sec->sh_size : 0;
	    }

	  for (si = 0, psym = symtab;
	       si < section->sh_size / section->sh_entsize;
	       si++, psym++)
	    {
              unsigned char *symname = (psym->st_name < strtab_size) ? (strtab + psym->st_name) : "<corrupt>";
              if (!strcmp (reqsymname, symname))
              {
                symaddr = psym->st_value;
                *symsize = psym->st_size;
                found = 1;
                break;
              }
	    }

	  free (symtab);
	  if (strtab != string_table)
	    free (strtab);
          if (found)
            break;
	}
  
  if (!found)
    return 0;

  for (i = 0, section = section_headers; i < elf_header.e_shnum; i++, section++)
  {
     if (section->sh_addr > 0 && symaddr >= section->sh_addr && symaddr < section->sh_addr + section->sh_size)
       return (section->sh_offset + (symaddr - section->sh_addr));
  }

  return 0;
}

#define MAX_LEN 4096
unsigned int patch_buf[MAX_LEN];

main (int argc, char *argv[])
{
  FILE *file;
  char armag[SARMAG];
  int dumpsym = 0;

  if (argc < 2)
  {
    printf ("Usage: %s elf_file [symname [patchval [patchfile]]] \n", argv[0]);
    return 1;
  }
  else if (argc == 2)
    dumpsym = 1;

  file = fopen (argv[1], "r+");
  if (file == NULL)
  {
    printf ("can not open file %s \n", argv[1]);
    return 1;
  }

  /* Read in the identity array.  */
  if (fread (elf_header.e_ident, EI_NIDENT, 1, file) != 1)
  {
    printf ("Read the elf identity error !!!\n");
    goto err_ret;
  }

  byte_get = byte_get_little_endian;
  {
      Elf32_External_Ehdr ehdr32;

      if (fread (ehdr32.e_type, sizeof (ehdr32) - EI_NIDENT, 1, file) != 1)
      {
        printf ("Read the elf e_type error !!!\n");
        goto err_ret;
      }

      elf_header.e_type      = BYTE_GET (ehdr32.e_type);
      elf_header.e_machine   = BYTE_GET (ehdr32.e_machine);
      elf_header.e_version   = BYTE_GET (ehdr32.e_version);
      elf_header.e_entry     = BYTE_GET (ehdr32.e_entry);
      elf_header.e_phoff     = BYTE_GET (ehdr32.e_phoff);
      elf_header.e_shoff     = BYTE_GET (ehdr32.e_shoff);
      elf_header.e_flags     = BYTE_GET (ehdr32.e_flags);
      elf_header.e_ehsize    = BYTE_GET (ehdr32.e_ehsize);
      elf_header.e_phentsize = BYTE_GET (ehdr32.e_phentsize);
      elf_header.e_phnum     = BYTE_GET (ehdr32.e_phnum);
      elf_header.e_shentsize = BYTE_GET (ehdr32.e_shentsize);
      elf_header.e_shnum     = BYTE_GET (ehdr32.e_shnum);
      elf_header.e_shstrndx  = BYTE_GET (ehdr32.e_shstrndx);

//printf ("e_shnum = %d, phnum = %d, ndx = %d \n", elf_header.e_shnum, elf_header.e_phnum, elf_header.e_shstrndx);
   }

    if (! elf_header.e_shoff)
      goto err_ret;

    if (get_32bit_section_headers (file, 1) == 0)
    {
        printf ("read the section headers error !!! \n");
	goto err_ret;
    }
    // process_object, get_file_header


  //printf ("armag = %s \n", armag);
  process_section_headers (file);

  if (dumpsym)
    process_symbol_table (file);
  else
//patch file
  {
    FILE *fp;
    unsigned int symsize, fileoff;
    fileoff = get_symbol_fileoff (file, argv[2], &symsize);

// replace vale
   if (argc == 3)
     printf ("sym name = %s, file off = 0x%x, size = 0x%x\n", argv[2], fileoff, symsize);

   if (! fileoff)
     goto err_ret;

   if (argc == 4)
   {
     unsigned long val;
     char *endptr;
     fseek (file, fileoff, SEEK_SET);
     val = strtoul(argv[3], &endptr, 0);
     fwrite (&val, 4, 1, file);
     printf ("sym name = %s, file off = 0x%x, size = 0x%x, replace: val = 0x%08x\n", argv[2], fileoff, symsize, val);
   }

   if (argc > 4)
   {
     FILE *patch_file = fopen (argv[4], "r");
     unsigned int buflen;
     if (file == NULL)
     {
       printf ("can not open file %s \n", argv[4]);
       return 1;
     }
     buflen = fread (patch_buf, 4, MAX_LEN, patch_file);

     fseek (file, fileoff, SEEK_SET);
     fwrite (patch_buf, 4, buflen, file);
     fclose (patch_file);
     printf ("sym name = %s, file off = 0x%x, size = 0x%x, replace file: %s, replace size = %d\n", 
             argv[2], fileoff, symsize, argv[4], buflen*4);
   }

  }

  fclose (file);
  return 0;
err_ret:
  fclose (file);
  return 1;
  
}

