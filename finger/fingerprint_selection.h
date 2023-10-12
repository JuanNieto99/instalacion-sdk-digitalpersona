 
#pragma once

#include <dpfpdd.h>

#include <stddef.h>

DPFPDD_DEV SelectAndOpenReader(char* szReader, size_t nReaderLen,int *pDPI);
