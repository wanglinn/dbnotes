新增系统函数，需要向pg_type.h文件中写入相应信息：给定系统函数Oid + 一系列列对应的值。若更改某一个值，需要一个个数，共29列，
不太容易分辨，写个python脚本进行“值”与“列名”进行对照。

需要注意这几项：
provolatile -- 影响函数是否可以下推
proparallel -- 影响是否允许并行
见pg_proc.h 文件有关这两项的说明如下：
/*
 * Symbolic values for provolatile column: these indicate whether the result
 * of a function is dependent *only* on the values of its explicit arguments,
 * or can change due to outside factors (such as parameter variables or
 * table contents).  NOTE: functions having side-effects, such as setval(),
 * must be labeled volatile to ensure they will not get optimized away,
 * even if the actual return value is not changeable.
 */
#define PROVOLATILE_IMMUTABLE	'i' /* never changes for given input */
#define PROVOLATILE_STABLE		's' /* does not change within a scan */
#define PROVOLATILE_VOLATILE	'v' /* can change even within a scan */

/*
 * Symbolic values for proparallel column: these indicate whether a function
 * can be safely be run in a parallel backend, during parallelism but
 * necessarily in the master, or only in non-parallel mode.
 */
#define PROPARALLEL_SAFE		's' /* can run in worker or master */
#define PROPARALLEL_RESTRICTED	'r' /* can run in parallel master only */
#define PROPARALLEL_UNSAFE		'u' /* banned while in parallel mode */


对应pg9.6版本

[wln@localhost1 pg]$ cat pgtype.txt 

NameData        proname;        /* procedure name */
Oid             pronamespace;   /* OID of namespace containing this proc */
Oid             proowner;       /* procedure owner */
Oid             prolang;        /* OID of pg_language entry */
float4          procost;        /* estimated execution cost */
float4          prorows;        /* estimated # of rows out (if proretset) */
Oid             provariadic;    /* element type of variadic array, or 0 */
regproc         protransform;   /* transforms calls to it during planning */
bool            proisagg;       /* is it an aggregate? */
bool            proiswindow;    /* is it a window function? */
bool            prosecdef;      /* security definer */
bool            proleakproof;   /* is it a leak-proof function? */
bool            proisstrict;    /* strict with respect to NULLs? */
bool            proretset;      /* returns a set? */
char            provolatile;    /* see PROVOLATILE_ categories below */
char            proparallel;    /* see PROPARALLEL_ categories below */
int16           pronargs;               /* number of arguments */
int16           pronargdefaults;        /* number of arguments with defaults */
Oid             prorettype;             /* OID of result type */
oidvector       proargtypes;            /* parameter types (excludes OUT params) */
Oid             proallargtypes[1];      /* all param types (NULL if IN only) */
char            proargmodes[1];         /* parameter modes (NULL if IN only) */
text            proargnames[1];         /* parameter names (NULL if no names) */
pg_node_tree    proargdefaults;         /* list of expression trees for argument defaults (NULL if none) */
Oid             protrftypes[1];         /* types for which to apply transforms */
text prosrc     BKI_FORCE_NOT_NULL;     /* procedure source text */
text            probin;                 /* secondary procedure info (can be NULL) */
text            proconfig[1];           /* procedure-local GUC settings */
aclitem         proacl[1];              /* access permissions */



[wln@localhost1 pg]$ cat infotype.py 

#!/bin/env python
import os,sys

pgtypefile = "pgtype.txt"
pgtypelen = 29

# get the file content
def readfile(filename):
	try:
		fd = open(filename, 'r')
	except  IOError,(errno,strerror):
		print "I/O error(%s): %s %s" %(errno, filename, strerror)
		exit(1)
	content = fd.read()
	return  content

# print the information
def printInfo(strArray, contentArray):
	i = 0
	for item in strArray:
		print str(i) + '\t' + strArray[i] + '\t\t' + contentArray[i]
		i = i + 1

def main():
	strArray = []
	strLen = 0
	typeLen = 0

	contentArray = []

	if len(sys.argv) != 2:
		print "input the pg type information string"
		exit(1)

	strarg1 = sys.argv[1]

	#split the input
	strArray = strarg1.split()
	strLen = len(strArray)
	print 'input pg_type information elemets number is ' + str(strLen) + '\n'
	if strLen != pgtypelen:
		print "input the pg type string information is not right, the elements number is not 30"
		exit(1)

	contentArray = readfile(pgtypefile).split('\n')
	#delete the empty element
	while '' in contentArray:
		contentArray.remove('')
	typeLen = len(contentArray)
	if typeLen != pgtypelen:
		print "check the pg type information file " + pgtypefile + ", the elements number is not 30"
		exit(1)

	printInfo(strArray, contentArray)


if __name__=="__main__":
	main()
  
  
  
 示例：
 [wln@localhost1 pg]$ python infotype.py 'num_nulls   PGNSP PGUID 12 1 0 2276 0 f f f f f f i s 1 0 23 "2276" "{2276}" "{v}" _null_ _null_ _null_ pg_num_nulls _null_ _null_ _null_ '
input pg_type information elemets number is 29

0       num_nulls               NameData        proname;        /* procedure name */
1       PGNSP           Oid             pronamespace;   /* OID of namespace containing this proc */
2       PGUID           Oid             proowner;       /* procedure owner */
3       12              Oid             prolang;        /* OID of pg_language entry */
4       1               float4          procost;        /* estimated execution cost */
5       0               float4          prorows;        /* estimated # of rows out (if proretset) */
6       2276            Oid             provariadic;    /* element type of variadic array, or 0 */
7       0               regproc         protransform;   /* transforms calls to it during planning */
8       f               bool            proisagg;       /* is it an aggregate? */
9       f               bool            proiswindow;    /* is it a window function? */
10      f               bool            prosecdef;      /* security definer */
11      f               bool            proleakproof;   /* is it a leak-proof function? */
12      f               bool            proisstrict;    /* strict with respect to NULLs? */
13      f               bool            proretset;      /* returns a set? */
14      i               char            provolatile;    /* see PROVOLATILE_ categories below */
15      s               char            proparallel;    /* see PROPARALLEL_ categories below */
16      1               int16           pronargs;               /* number of arguments */
17      0               int16           pronargdefaults;        /* number of arguments with defaults */
18      23              Oid             prorettype;             /* OID of result type */
19      "2276"          oidvector       proargtypes;            /* parameter types (excludes OUT params) */
20      "{2276}"                Oid             proallargtypes[1];      /* all param types (NULL if IN only) */
21      "{v}"           char            proargmodes[1];         /* parameter modes (NULL if IN only) */
22      _null_          text            proargnames[1];         /* parameter names (NULL if no names) */
23      _null_          pg_node_tree    proargdefaults;         /* list of expression trees for argument defaults (NULL if none) */
24      _null_          Oid             protrftypes[1];         /* types for which to apply transforms */
25      pg_num_nulls            text prosrc     BKI_FORCE_NOT_NULL;     /* procedure source text */
26      _null_          text            probin;                 /* secondary procedure info (can be NULL) */
27      _null_          text            proconfig[1];           /* procedure-local GUC settings */
28      _null_          aclitem         proacl[1];              /* access permissions */

