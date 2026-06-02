---
title: executable
nav_order: 3.0
---

# executable
this creates the executable or main entry file that runs.

```z
executable = {
    name = <name>,
    sources = [<list-of-sources>],
    target = <list-of-target> | <target>,
    output = <output-file> | <output-dir>,
    <optional-fields>,
};
```

## name
name of executable can be used when chanhging rules or use sources for other build.

## sources
list of source paths for building.

## target
target string or list of them.
targets for llvm like `llvm-x86_64-linux` triplets, or languages `js_es6`.

## output
Output directory if target generates multiple modules with sources name or output file.

## optional-fields
### dependencies
...
