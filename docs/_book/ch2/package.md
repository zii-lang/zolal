---
title: package
nav_order: 2.1
---

# package
`package` stanza define the main properties of a project. Each project should have their own package name so they can be used if a library includes them.

```z
package = {
    name = <name>,
    version = <version>,
    <optional-fields>
};
```

## name
is module name, when __pinned__ using **zolal** you can reference them in other project using their package name.

## version
used as package version should be in standard semantic version format `MAJOR.MINOR.PATCH`.

## optional-fields

### author
name of author.

### license

### description

### releasenotes

### bugreport

### documentation

### homepage
