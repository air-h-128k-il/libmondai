#define DEBUG
#define TRACE

#define MAIN
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <glib.h>
#include "types.h"

#include "value.h"
#include "getset.h"
#include "Native_v.h"
#include "Text_v.h"
#include "XML_v.h"
#include "OpenCOBOL_v.h"
#include "valueconv.h"
#include "RecParser.h"
#include "numerici.h"
#include "memory_v.h"
#include "misc_v.h"
#include "LBSfunc.h"
#include "others.h"

#include "debug.h"

extern int main(int argc, char **argv) {
  ValueStruct *v1,*v2,*v;
  LargeByteString *lbs;
  size_t size;
  int i;
  char *p;
  const char *str = ""
                    "dummy	{"
                    "id		char(8);"
                    "num	int;"
                    "query	{"
                    "key	char(8);"
                    "value	char(8);"
                    "}[2];"
                    "};";

  RecParserInit();

  fprintf(stderr, "\n\n---- normal\n");
  lbs = NewLBS();
  v1 = RecParseValueMem(str, NULL);
  v2 = RecParseValueMem(str, NULL);
  InitializeValue(v1);
  v = GetItemLongName(v1, "query[0].key");
  SetValueStringWithLength(v, "key0", strlen("key0"), NULL);
  v = GetItemLongName(v1, "query[1].key");
  SetValueStringWithLength(v, "key1", strlen("key1"), NULL);
  v = GetItemLongName(v1, "query[0].value");
  // SetValueStringWithLength(v, "value0", strlen("value0"), NULL);
  SetValueStringWithLength(v, "valvalvalvalvalvalvalue0",
                           strlen("valvalvalvalvalvalue0"), NULL);
  v = GetItemLongName(v1, "query[1].value");
  SetValueStringWithLength(v, "value1", strlen("value1"), NULL);

  fprintf(stderr, "#dump before\n");
  DumpValueStruct(v1);
  fprintf(stderr, "#pack\n");
  size = NativeSizeValue(NULL, v1);
  fprintf(stderr, "#size = %ld\n", (long)size);
  LBS_ReserveSize(lbs, size, FALSE);
  NativePackValue(NULL, LBS_Body(lbs), v1);

  fprintf(stderr, "####\n");
  p = LBS_Body(lbs);
  for (i = 0; i < size; i++) {
    fprintf(stderr, "\\x%X", (unsigned int)*p);
    p++;
  }
  fprintf(stderr, "####\n");

  fprintf(stderr, "#nativeunpack\n");
  NativeUnPackValue(NULL, LBS_Body(lbs), v2);
  fprintf(stderr, "#dump val3\n");
  DumpValueStruct(v2);
  fprintf(stderr, "#end\n");

  return 0;
}
