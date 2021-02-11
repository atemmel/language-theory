### regex parser

```
Grammar:
<SelectionGroup> ::= <Sequence>\O{[0-9]}EOL
               (At least one repetition)
<Sequence> ::= {<Value> | <UnaryExpr> | <BinaryExpr>}-
<Value> ::= <String> | <Wildcard> | <Grouping>
<UnaryExpr> ::= <Counter> | <Repeated> | <Insensitive>
<BinaryExpr> ::= <Either>
<Counter> ::= <Value>{[0-9]} | <UnaryExpr>{[0-9]}
<Repeated> ::= <Value>* | <UnaryExpr>*
<Insensitive> ::= <Value>\I | <UnaryExpr>\I
<Either> ::= <Sequence>+<Sequence>
<Grouping> ::= (<Sequence>)
<String> ::= [A-Za-z]
<Wildcard> ::= .
```
