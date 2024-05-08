#include <cstdio>
#include <cassert>
#include "g3dDebug.h"

void _SetLineInfo(const char *pFileName, int iLine, const char *pFunctionName, const char *pExpression) 
{
	printf("%s(%d)(%s):%s\n", pFileName, iLine, pFunctionName, pExpression);
}

void g3ddbgAssert(bool b, const char *pStr, ...) 
{
	va_list _ArgList;
	__crt_va_start(_ArgList, pStr);
	printf(pStr, _ArgList);
	__crt_va_end(_ArgList);
	assert(b);
}

void g3ddbgPrintf(const char *pStr, ...) 
{
	va_list ArgList;
	__crt_va_start(ArgList, pStr);
	printf(pStr, ArgList);
	__crt_va_end(ArgList);
}

void g3ddbgWarning(bool b, const char *pStr, ...) 
{
	va_list _ArgList;
	__crt_va_start(_ArgList, pStr);
	printf(pStr, _ArgList);
	__crt_va_end(_ArgList);
}
