---
title: Reference Manual
nav_order: 1.0
---

# Getting Started with Zolal
Zolal uses Z minimal language to write `manifest.zm` which is the instructions on how to build a Z project.

Here we don't have types, no allocations no imports, no macros or any complicated features of `Z` but yet the syntax remains the same.

## Sample Executable Build
Imagine we have a `hello_world` project in Z. The base file inside directory is main.zz and we want to write a Zolal manifest to compile it:

```z

let sources = ["main.zz"];

package = {
    name = "hello_world",
    author = "Z Team <ziilang@gmail.com>",
};

executable = {
    name = "main",
    description = "x86_64 binary executable."
    sources = sources,
    output = "bin/hello_world"
    target = "x86_64-llvm-linux"
};

```

In zolal manifest these structures that are written like `name = {}` are called stanzas, some stanzas can be used multiple times but some should be used once.


## Declare a variable
Like Z you can declare simple variables but their type are only `bool`, `integer`, `float`, `string` or `array` and auto induced by zolal.

```z
let bool = false;
let integer = 10;
let float = 10.;
let string = "Hi!";
let array = ["hello", "world"];
```

