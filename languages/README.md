# Language Syntax Files

Language files define source spellings, separately from accent files that define translated spellings. Use one rule per line:

```text
TOKEN = source spelling
```

The token names are `ADD`, `SUB`, `MULT`, `DIV`, `IF`, `FOR`, `WHILE`, `VAR_DECL`, `LIBRARY_IMPORT`, `FUNC_DECL`, and `COMMENT`. Add another rule with the same token name to recognize an alias. Longer spellings are matched first, and word-like spellings must match whole identifiers. `COMMENT` marks a line comment; the rest of that line is left untouched.

With no arguments, the program uses `languages/cpp.alat`, `test/cpp.alat`, and `test/test.cpp`, in that order. Accent files retain their existing `TOKEN = translated spelling` format.
