/*	PANDA -- a simple transaction monitor

Copyright (C) 2000-2002 Ogochan & JMA (Japan Medical Association).

This module is part of PANDA.

	PANDA is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility
to anyone for the consequences of using it or for whether it serves
any particular purpose or works at all, unless he says so in writing.
Refer to the GNU General Public License for full details. 

	Everyone is granted permission to copy, modify and redistribute
PANDA, but only under the conditions described in the GNU General
Public License.  A copy of this license is supposed to have been given
to you along with PANDA so you can know your rights and
responsibilities.  It should be in a file named COPYING.  Among other
things, the copyright notice and this notice must be preserved on all
copies. 
*/

#define	MAIN
/*
#define	DEBUG
#define	TRACE
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<unistd.h>
#include	<glib.h>
#include	"types.h"
#include	"libmondai.h"
#include	"misc.h"
#include	"option.h"
#include	"debug.h"

static	Bool	fNoConv;
static	Bool	fFiller;
static	int		TextSize;
static	int		ArraySize;
static	char	*Prefix;
static	char	*RecName;
static	char	*Lang;

static	int		level;

static	void	COBOL(ValueStruct *val, size_t arraysize, size_t textsize);

static	int		Col;

static	void
PutLevel(
	int		level)
{
	int		n;

	n = 8;
	if		(  level  >  1  ) {
		n += 4 + ( level - 2 ) * 2;
	}
	for	( ; n > 0 ; n -- ) {
		fputc(' ',stdout);
	}
	printf("%02d  ",level);
	Col = 0;
}

static	void
PutTab(
	int		unit)
{
	int		n;

	n = unit - ( Col % unit );
	Col += n;
	for	( ; n > 0 ; n -- ) {
		fputc(' ',stdout);
	}
}

static	void
PutChar(
	int		c)
{
	fputc(c,stdout);
	Col ++;
}

static	void
PutString(
	char	*str)
{
	int		c;

	while	(  *str  !=  0  ) {
		if		(  fNoConv  ) {
			PutChar(*str);
		} else {
			c = toupper(*str);
			if		(  c  ==  '_'  ) {
				c = '-';
			}
			PutChar(c);
		}
		str ++;
	}
}

static	void
PutName(
	char	*name)
{
	char	buff[SIZE_BUFF];

	if		(	(  name           ==  NULL  )
			||	(  !stricmp(name,"filler")  ) ) {
		strcpy(buff,"filler");
	} else {
		sprintf(buff,"%s%s",Prefix,name);
	}
	PutString(buff);
}

static	void
COBOL(
	ValueStruct	*val,
	size_t		arraysize,
	size_t		textsize)
{
	int		i
	,		n;
	ValueStruct	*tmp;
	char	buff[SIZE_BUFF+1];

	if		(  val  ==  NULL  )	return;

	switch	(val->type) {
	  case	GL_TYPE_INT:
		PutString("PIC S9(9)   BINARY");
		break;
	  case	GL_TYPE_BOOL:
		PutString("PIC X");
		break;
	  case	GL_TYPE_BYTE:
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
		sprintf(buff,"PIC X(%d)",val->body.CharData.len);
		PutString(buff);
		break;
	  case	GL_TYPE_NUMBER:
		if		(  val->body.FixedData.slen  ==  0  ) {
			sprintf(buff,"PIC S9(%d)",val->body.FixedData.flen);
		} else {
			sprintf(buff,"PIC S9(%d)V9(%d)",
					(val->body.FixedData.flen - val->body.FixedData.slen),
					val->body.FixedData.slen);
		}
		PutString(buff);
		break;
	  case	GL_TYPE_TEXT:
		sprintf(buff,"PIC X(%d)",textsize);
		PutString(buff);
		break;
	  case	GL_TYPE_ARRAY:
		tmp = val->body.ArrayData.item[0];
		n = val->body.ArrayData.count;
		if		(  n  ==  0  ) {
			n = arraysize;
		}
		if		(  tmp->type  ==  GL_TYPE_RECORD  ) {
			sprintf(buff,"OCCURS  %d TIMES",n);
			PutTab(8);
			PutString(buff);
			COBOL(tmp,arraysize,textsize);
		} else {
			COBOL(tmp,arraysize,textsize);
			sprintf(buff,"OCCURS  %d TIMES",n);
			PutTab(8);
			PutString(buff);
		}
		break;
	  case	GL_TYPE_RECORD:
		level ++;
		for	( i = 0 ; i < val->body.RecordData.count ; i ++ ) {
			printf(".\n");
			PutLevel(level);
			tmp = val->body.RecordData.item[i];
			PutName(val->body.RecordData.names[i]);
			if		(  tmp->type  !=  GL_TYPE_RECORD  ) {
				PutTab(4);
			}
			COBOL(tmp,arraysize,textsize);
		}
		level --;
		break;
	  default:
		break;
	}
}

static	void
SIZE(
	ValueStruct	*val,
	size_t		arraysize,
	size_t		textsize)
{
	char	buff[SIZE_BUFF+1];

	if		(  val  ==  NULL  )	return;
	level ++;
	PutLevel(level);
	PutName("filler");
	PutTab(8);
	sprintf(buff,"PIC X(%d)",SizeValue(val,arraysize,textsize));
	PutString(buff);
	level --;
}

static	void
MakeFromRecord(
	char	*name)
{
	RecordStruct	*rec;

	level = 1;
	DD_ParserInit();
	if		(  ( rec = DD_ParserDataDefines(name) )  !=  NULL  ) {
		PutLevel(level);
		if		(  *RecName  ==  0  ) {
			PutString(rec->name);
		} else {
			PutString(RecName);
		}
		if		(  fFiller  ) {
			printf(".\n");
			SIZE(rec->rec,ArraySize,TextSize);
		} else {
			COBOL(rec->rec,ArraySize,TextSize);
		}
		printf(".\n");
	}
}

static	ARG_TABLE	option[] = {
	{	"lang",		STRING,		TRUE,	(void*)&Lang	,
		"�оݸ���̾"			 						},
	{	"textsize",	INTEGER,	TRUE,	(void*)&TextSize,
		"text�κ���Ĺ"									},
	{	"arraysize",INTEGER,	TRUE,	(void*)&ArraySize,
		"������������κ��緫���֤���"					},
	{	"prefix",	STRING,		TRUE,	(void*)&Prefix,
		"����̾�������ղä���ʸ����"					},
	{	"name",		STRING,		TRUE,	(void*)&RecName,
		"�쥳���ɤ�̾��"								},
	{	"filler",	BOOLEAN,	TRUE,	(void*)&fFiller,
		"�쥳���ɤ���Ȥ�FILLER�ˤ���"					},
	{	"noconv",	BOOLEAN,	TRUE,	(void*)&fNoConv,
		"����̾����ʸ���˲ù����ʤ�"					},
	{	NULL,		0,			FALSE,	NULL,	NULL 	}
};

static	void
SetDefault(void)
{
	fNoConv = FALSE;
	fFiller = FALSE;
	ArraySize = -1;
	TextSize = -1;
	Prefix = "";
	RecName = "";
	Lang = "OpenCOBOL";
}

extern	int
main(
	int		argc,
	char	**argv)
{
	FILE_LIST	*fl;
	char		*name
	,			*ext;

	SetDefault();
	fl = GetOption(option,argc,argv);
	SetLanguage(Lang);
	if		(  fl  !=  NULL  ) {
		name = fl->name;
		ext = GetExt(name);
		if		(	(  !stricmp(ext,".rec")  )
				||	(  !stricmp(ext,".db")  ) ) {
			MakeFromRecord(name);
		}
	}
	return	(0);
}