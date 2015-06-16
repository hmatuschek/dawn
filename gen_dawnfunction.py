from random import Random
from string import Template
import sys


template = """#ifndef __DAWN_FUNCTION_H__
#define __DAWN_FUNCTION_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PROGMEM
#define PROGMEM
#endif

static const uint16_t dawn_func[] PROGMEM = { $data };

#ifdef __cplusplus
}
#endif

#endif
"""

x = map(lambda n: hex(int(0xffff*pow(1.15,30*(n-128.)/128.)/pow(1.15,30))), range(256))
open(sys.argv[1], "w").write(Template(template).substitute(data=", ".join(x)))
