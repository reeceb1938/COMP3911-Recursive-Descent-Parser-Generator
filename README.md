# COMP3931-Recursive-Descent-Parser-Generator

## Grammar Definition

To define a grammar as input to this program create a text file in to form described below.

### Terminals

The first line of the grammar definition file should be of the form `T: a, b, c`, where `a, b, c` is the comma separated list of terminals. To include the comma symbol as a terminal escape it with a backslash (\\).

We use the convention that terminals are lower case.

#### Pre-defined Terminals

| Symbol | Meaning |
| - | - |
| epsilon | The empty word
| identifier | Any user specified identifier, e.g. names of variables / classes
| integer_constant | Any valid decimal number
| string_literal | Any string literal

### Non-terminals

The second line of the grammar definition file should be of the form `NT: A, B, C`, where `A, B, C` is the comma separated list of non-terminals. To include the comma symbol as a terminal escape it with a backslash (\\).

We use the convention that non-temrinals are in all upper case.

### Non-Allowed Symbols

For internal use the class uses `eof` when computing the follow sets, so this cannot appear as either a terminal or non-terminal.

### Production Rules & Start Symbol

The block of production rules is declared after the non-terminal declaration and starts with the line `P:` followed by any number of lines of the form `NONTERMINAL ::= PRODUCTION`, where `NONTERMINAL` is a single NONTERMINAL (because this is a regular grammar) and `PRODUCTION` is the set of productions in BNF or EBNF form.

The non-terminal on the left hand side of the first production rule is implicitly the start symbol.

## Behaviour and Assumptions

In this implementation the follow assumptions are made:

- Any non-terminal that does not have any production rules defined for it produces the empty word

## References & Licences

References for external content

| Resource | Usage | Licence |
| - | - | - |
| .gitignore templates | .gitignore | [CC 1.0 Universal](https://github.com/github/gitignore/blob/master/LICENSE) |
| spdlog | Logging library | [MIT](https://github.com/gabime/spdlog/blob/v1.x/LICENSE) |
