Introduction to Mustach
=======================

`mustach` is a C implementation of the [mustache](http://mustache.github.io "main site for mustache") template library.

The main site for `mustach` is on [gitlab](https://gitlab.com/jobol/mustach).

The best way to use mustach is to copy the files **mustach.h** and **mustach.c**
directly into your project and use it.

The current source files are:

- **mustach.c** core implementation of mustache in C
- **mustach.h** header file for core definitions
- **mustach-json-c.c** tiny json wrapper of mustach using [json-c](https://github.com/json-c/json-c)
- **mustach-json-c.h** header file for using the tiny JSON wrapper
- **mustach-tool.c** simple tool for applying template files to a JSON file

The file **mustach-json-c.c** is the main example of use of **mustach** core
and it is also a practical implementation that can be used. It uses the library
json-c. (NOTE for Mac OS: available through homebrew).

HELP REQUESTED TO GIVE EXAMPLE BASED ON OTHER LIBRARIES (ex: janson, ...).

The tool **mustach** is build using `make`,  its usage is:

    mustach json template [template]...

It then outputs the result of applying the templates files to the JSON file.

Portability
===========

Some system does not provide *open_memstream*. In that case, tell your
prefered compiler to declare the preprocessor symbol **NO_OPEN_MEMSTREAM**.
Example:

	gcc -DNO_OPEN_MEMSTREAM

Extensions
==========

By default, the current implementation provides the following extensions:

Explicit Substitution
---------------------

This is a core extension implemented in file **mustach.c**.

In somecases the name of the key used for substition begins with a
character reserved for mustach: one of '#', '^', '/', '&', '{', '>' and '='.
This extension introduces the special character ':' to explicitly
tell mustach to just substitute the value. So ':' becomes a new special
character.

Value Testing
-------------

This is a tool extension implmented in file **mustach-json-c.c**.

This extension allows you to test the value of the selected key.
It is allowed to write key=value (matching test) or key=!value
(not matching test) in any query.

Removing Extensions
-------------------

When compiling mustach.c or mustach-json-c.c,
extensions can be removed by defining macros
using option -D.

The possible macros are:

- `NO_COLON_EXTENSION_FOR_MUSTACH`

  This macro remove the ability to use colon (:)
  as explicit command for variable substituion.
  This extension allows to have name starting
  with one of the mustach character :#^/&{=<

- `NO_EQUAL_VALUE_EXTENSION_FOR_MUSTACH`

  This macro allows the program to check the whether
  the actual value is equal to an expected value.
  This is useful in `{{#key=val}}` or `{{^key=val}}`
  with the corresponding `{{/key=val}}`.
  It can also be used in `{{key=val}}` but this
  doesn't seem to be useful.

- `NO_JSON_POINTER_EXTENSION_FOR_MUSTACH`

  This macro removes the possible use of JSON pointers.
  JSON pointers are defined in IETF RFC 6901.
  If not set, any key starting with "/" is a JSON pointer.
  This implies to use the colon to introduce keys.
  So `NO_COLON_EXTENSION_FOR_MUSTACH` implies
  `NO_JSON_POINTER_EXTENSION_FOR_MUSTACH`.
  A special escaping is used for "=" signs when
  values comparison is enabled: "~=" leaves "=" in the key.

- `NO_ALLOW_EMPTY_TAG`

  Generate the error MUSTACH_ERROR_EMPTY_TAG automatically.

- NO_OBJECT_ITERATION_FOR_MUSTACH

  Disable the object iteration extension. That extension allows
  to iterate over the keys of an object. The iteration on object
  is selected by using the selector `{{#key.*}}`. In the context
  of iterating over object keys, the single key `{{*}}` returns the
  key and `{{.}}` returns the value.

- `NO_EXTENSION_FOR_MUSTACH`

  This macro disables any current or future
  extensions.

