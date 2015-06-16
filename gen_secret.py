from random import Random
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

static uint8_t secret[] = { $key };

#ifdef __cplusplus
}
#endif

#endif
"""


rand = Random();
rand.seed();
open(sys.argv[1], "w").write(Template(template).substitute(
  key=", ".join(map(lambda i: hex(rand.randint(0,255)), range(16)))))
