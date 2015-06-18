from string import Template
import sys
import os

if 1 == len(sys.argv):
    # w/o arguments -> generate random bytes
    print("".join(map(lambda x: hex(ord(x))[2:], os.urandom(16))))
    sys.exit(0)

if 3 != len(sys.argv):
    print("Error: Needs exactly 0 or 2 arguments.")
    sys.exit(1)

# With arguments -> generate header

template = """#ifndef __DAWN_SECRET_H__
#define __DAWN_SECRET_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PROGMEM
#define PROGMEM
#endif

static const uint8_t secret[] PROGMEM = { $key };

#ifdef __cplusplus
}
#endif

#endif
"""

key = ", ".join(map(lambda i: "0x{0}".format(sys.argv[1][i:i+2]), range(0,16,2)))
open(sys.argv[2], "w").write(Template(template).substitute(key=key))
