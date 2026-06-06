---
title: "Building an executable"
nav_order: 1.2
---

# Step 1: Building an executable

First you need to indecate the sources you want to build in `manifest.zl`.

```z

package = {
    name = "hello_world",
    version = "1.0.0",
};

let source_files = ["main.zz"]

executable = {
    name = "hello_world_binary",
    sources = source_files,
    dependencies = {
        external = {
            "libc",
        },
    },
};

create_executable(
    "hello_world_binary", 
    target = "llvm-x86_64",
    output = "hello_world",
    mode = "release",
);

```

The `main.zz` file:

```z
@:includeBaseTypes;

let printf : *i8->..->i32;

let main : void->i32 = func() : i32 {
	printf("Hello world.\n");
	return 0;
};
```

Now we build when inside the project where `manifest.zl` lives:

```bash
zolal -B build -S .
```

This command will create build directory if it doesn't exist, creates preprocessed `hello_world.zz` 
from `main.zz` inside `build` directory and compiles into `hello_world` as `output` suggests, linked
against `libc`.

```bash
build/hello_world
```
```
Hello world.
```

## Adding another target
Now imagine our source is like:

```z
#if target="js-6"
let js = @:import("js-6");
let log : string->void = js::browser::console::log;
#else
@:includeBaseTypes;
let printf : *i8->..->i32;
let log : string->void = func(v:string) {
    printf(v);
};
#end

#if target_kind="binary"
let main : void->i32 = func() : i32 {
#end
    log("Hello world.");
#if target_kind="binary"
    return 0;
};
#end
```

Here we need different rules for building js and binary target so we have to change `manifest.zl`
a bit.

```z

package = {
    name = "hello_world",
    version = "1.0.0",
};

let source_files = "main.zz";

executable = {
    name = "hello_world",
    sources = source_files,
};
]
// For binary.
create_executable(
    name = "hello_world_binary",
    for = "hello_world", 
    target = "llvm-x86_64",
    output = "hello_world",
    mode = "release",
);

add_rule(
    for = "hello_world_binary",
    rule = {
        dependencies = {
            external = {
                "libc",
            },
        },
    },
);

// For js-6 script.
create_executable(
    name = "hello_world_js",
    for = "hello_world",
    target = "js-6",
    output = "hello_world.js",
    mode = "release",
};
```

Here when we run:
```bash
zolal -B build -S .
```

We will have `hello_world_js.zz` and `hello_world_binary.zz` files which are preprocessed files,
and compiled into `hello_world` and `hello_world.js`.

## Exclude target
We can exclude targets using their names with zolal:

```bash
zolal -B build -S . --exclude-executable "hello_world_js"
```
this will not build `hello_world.js` file nor it's preprocessed files.
