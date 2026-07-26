---
title: Switch And Pin
nav_order: 1.3
---

# Switch And Pin
When building a Z application you can isolate your build environment using switches.

After zolal is initialized:

```bash
zolal setup
```

Asks for the path where it keeps package caches locally and will create a `default` switch
defaulting to the latest `Z` compiler.


## Create a switch

```bash
zolal switch new z-toolchain-nightly --with-z="nightly"
```

## Adding a package

To add a package you can run:

```bash
zolal add https://github.com/zii-lang/http_parser
```
Now we have http_parser in caches.

## Pinning a package
Now if you want to add a package to a switch you have to pin it.
```bash
zolal pin http_parser
```
If a package is not pinned in a switch it's not included as dependency for building
a package you try to build.

## Auto Pin
If a `manifest.zl` has too many dependencies it would be a hard time to pin all of the
dependencies one by one.
So just run:
```bash
zolal pin auto
```
inside the directory with `manifest.zl` to auto-pin packages required, or:
```bash
zolal pin auto --manifest=/path/to/manifest.zl
```
