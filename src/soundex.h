#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
 
/* for ASCII only */
// static char code[128] = { 0 };
void add_code(const char *, int );
 
void soundex_init();

/* returns a static buffer; user must copy if want to save
   result across calls */
const char* soundex(const char *);
