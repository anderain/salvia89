# Salvia89

A compact, C89-compliant `sprintf` alternative. Single-file, zero dependencies outside the C standard library.

Designed for legacy platforms and embedded targets that lack a full standard library or have limited `sprintf` implementations. Tested on the following environments:

- **Visual C++ 6.0** (with Windows CE SDK 1.0 / 2.0)
- **fx-9860SDK** (SH SERIES C/C++ Compiler 6.0)
- **Palm OS SDK**

## Features

- Written in strict **ANSI C (C89)** — compiles with `-ansi -Werror -pedantic`
- Supports common format specifiers: `%d`, `%f`, `%s`, `%c`, `%%`
- Flag support: `-` (left-justify), `+` (force sign), `0` (zero-padding)
- Width and precision control (e.g. `%08.2f`, `%-15s`, `%.5d`)
- No dynamic memory allocation — works entirely on stack buffers

## API

```c
int Salvia_Format(char* szBuf, const char* szFormat, ...);
```

Writes a formatted string into `szBuf`, returning the number of characters written (excluding the null terminator).

## Format Specifiers

| Specifier | Description        | Example                           |
|-----------|--------------------|-----------------------------------|
| `%d`      | Signed integer     | `Salvia_Format(buf, "%d", 42)`   |
| `%f`      | Float / Double     | `Salvia_Format(buf, "%.2f", 3.14)` |
| `%s`      | String             | `Salvia_Format(buf, "%s", "hi")` |
| `%c`      | Character          | `Salvia_Format(buf, "%c", 'A')`  |
| `%%`      | Literal percent    | `Salvia_Format(buf, "100%%")`    |

## Supported Flags

`%[flags][width][.precision]specifier`

| Flag | Description                  |
|------|------------------------------|
| `-`  | Left-justify within width    |
| `+`  | Force sign on positive numbers |
| `0`  | Zero-pad to field width       |

## Example

```c
#include "salvia.h"

int main(void) {
    char buf[100];

    Salvia_Format(buf, "Integer: %d", 42);
    /* buf = "Integer: 42" */

    Salvia_Format(buf, "Float: %.2f", 3.14159);
    /* buf = "Float: 3.14" */

    Salvia_Format(buf, "Padded: |%08.2f|", 3.14);
    /* buf = "Padded: |00003.14|" */

    return 0;
}
```

## Building & Testing

```sh
make test
```

Requires GCC. The test suite compares `Salvia_Format` output against the standard `sprintf` for correctness.

```sh
make clean
```

Removes build artifacts.

## License

MIT
