---
title: library
nav_order: 2.3
---

# library
recipe for creating library.

```z
library = {
    name = <name>,
    version = <version>,
    sources = [<list-of-sources>],
    <optional-fields>,
};
```

## name
name of the library, can be overriden when used in another package.

## version
version of the library release, in standard semver notation.

## sources
list of Z sources for the library.

## optional-fields
### dependencies
An array of other library names given as dependencies in format of:

```z
dependencies = [
    "<library-name>,
    {"<library-name>" = "<version>"},
],
```
