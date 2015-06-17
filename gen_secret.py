from os import urandom
from string import Template
import sys


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


open(sys.argv[1], "w").write(Template(template).substitute(
  key=", ".join(map(lambda x: hex(ord(x)), urandom(16)))))
