---
title: dependencies
nav_order: 2.4
---

# dependencies
Stanza which introduces dependencies of a package, bound to a name.

## Synopsis

```z
dependencies = {
    packages = [
        <package-name> | "<package-name>:<version>", ...
    ],
    external = [
        <external-lib-definition>, ...
    ],
};
```

## external-lib-definition
External library definition changes based on target, for example if you need a python package we
need to know it's a python package, or when it's a binary library for linking, we need to know if we are:
* linking to static or dynamic library?
* building library from Z interface source?
* linking to an already compiled library?
* and ...

### JS: npm
for external npm packages here's how to do it:

```z
external = [
    npm = {
        dependencies = [
            "<package-name>" | "<package-name>:<npm-style-version>"
        ],
        dev_dependencies = [
            "<package-name>" | "<package-name>:<npm-style-version>"
        ],
    },
],
```
