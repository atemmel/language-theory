### regex parser

```
Grammar:
               At least one repetition
<Sequence> ::= {<Value> | <UnaryExpr> | <BinaryExpr>}-
<String> ::= [A-Za-z]
<Wildcard> ::= .
<Counter> ::= <Expr>{[0-9]}
<Either> ::= <Expr>+<Expr>
<Repeated> ::= <Expr>*
<Insensitive> ::= <Expr>\I
<Grouping> ::= (<Sequence>)
<SelectionGroup> ::= <Sequence>\O{[0-9]}
<Value> ::= <String> | <Wildcard> | <Grouping>
<UnaryExpr> ::= <Wildcard> | <Counter> | <Repeated> | <Insensitive>
<BinaryExpr> ::= <Either>
<Expr> ::= <Value> | <UnaryExpr> | <BinaryExpr>
```
