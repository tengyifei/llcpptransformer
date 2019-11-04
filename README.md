### When you're lucky

    make && ./main

### When you're not

Set `DEBUG` to `1` in `transformer.cc` and

    make && ./main

But most likely, you'll then need to

    make && gdb ./main

### Regen tables

You must have a fully built tree in a sibling directory with both
[fxr/330748](https://fuchsia-review.googlesource.com/c/fuchsia/+/330748) and
[fxr/331213](https://fuchsia-review.googlesource.com/c/fuchsia/+/331213).

Then run

    ../fuchsia/out/default/host_x64/fidlc \
      --tables tables.h \
      --json transformer.test.fidl.json \
      --files transformer.test.fidl \
    | sed 's,#include <lib/fidl/internal.h>,#include "fidl.h",' -i tables.h

You can use this one liner to convert ordinals to hex

    echo 'obase=16; 555209418' | bc
