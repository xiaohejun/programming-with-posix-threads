#ifndef ERRORS_H
#define ERRORS_H

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define err_abort(code, text) \
	do {\
		fprintf(stderr, "%s at \"%s\":%d: %s\n",\
			text, __FILE__, __LINE__, strerror(code));\
		abort();\
	} while(0)

#define err_abort_if(cond, code, text) \
	if (cond) { \
		err_abort(code, text); \
	} \

#define errno_abort(text) \
	do {\
		fprintf(stderr, "%s at \"%s\":%d: %s\n",\
			text, __FILE__, __LINE__, strerror(errno));\
		abort();\
	} while(0)

#define errno_abort_if(cond, text) \
	if (cond) { \
		errno_abort(text); \
	} \

#endif // ERRORS_H