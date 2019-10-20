### When you're lucky

    make && ./main

### When you're not

Set `DEBUG` to `1` in `transformer.cc` and

    make && ./main

But most likely, you'll then need to

    make && gdb ./main

### Regen tables

Assuming you have a fully built tree in a sibling directory with both
[fxr/330748](https://fuchsia-review.googlesource.com/c/fuchsia/+/330748) and
[fxr/331213](https://fuchsia-review.googlesource.com/c/fuchsia/+/331213)
you can

    ../fuchsia/out/default/host_x64/fidlc --tables tables.h --files example.fidl
