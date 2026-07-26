---
title: message
nav_order: 3.1
---

# message (function)
Logs a message.

## Synopsis
```
message(<mode>, <message>)
```

### mode
determines how the message is handled.

**FATAL_ERROR**: error, stops zolal process.  

**WARNING**: warning, continues processing.

**STATUS**: a message for project builder.

**DEBUG**: a message shown when building current project only.

### message
a simple string of the message being shown.
