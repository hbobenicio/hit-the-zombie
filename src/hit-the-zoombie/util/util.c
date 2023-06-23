#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int parse_ul(const char* s, unsigned long* out_value)
{
    char* endptr = NULL;

    // From man strtoul:
    //     Since  strtoul() can legitimately return 0 or ULONG_MAX (ULLONG_MAX for
    //     strtoull()) on both success and failure, the calling program should set
    //     errno  to 0 before the call, and then determine if an error occurred by
    //     checking whether errno has a nonzero value after the call
    errno = 0;
    unsigned long value = strtoul(s, &endptr, 10);
    if (errno == 0) {
        *out_value = value;
        return 0;
    }

    int error_code = errno;

    fprintf(
        stderr,
        "error: failed to parse string into unsigned long: %s\n",
        strerror(error_code)
    );

    // we leave `out_value` untouched intentionally in this case
    return 1;
}
