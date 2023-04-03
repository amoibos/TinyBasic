from __future__ import division
from math import trunc
from random import random
import re

TOKENS = re.compile(r'(?<=REM).*|\.?\d+|\w+\$?|[():;=+\-*/]|<[=>]?|>=?|"[^"]*"')

class Basic(object):
    def __init__(self, filename):
        self.tokens = []
        self.lines = {}
        for line in open(filename):
            tokens = TOKENS.findall(line)
            self.lines[tokens[0]] = len(self.tokens)
            self.tokens.extend(tokens[1:])
            self.tokens.append(":")
        self.index = 0
        self.vars = {}
        self.stack = []
    
    def next(self):
        token = self.tokens[self.index]
        self.index += 1
        return token
    
    def at(self, token):
        if self.next() == token:
            return True
        self.index -= 1
        return False
    
    def expect(self, token):
        if not self.at(token):
            raise SyntaxError("expected %s, but found %s" % (token, self.next()))

    def goto(self, line):
        self.index = self.lines[line]

    def run(self):
        while not self.at("END"):
            token = self.next()
            getattr(self, "do_" + token, lambda:self.do_LET(token))()
    
    def do_LET(self, token):
        if self.at("="):
            self.vars[token] = self.expression()
            self.expect(":")
        else:
            raise SyntaxError("unknown command %s" % token)
    
    def do_PRINT(self):
        def nstr(n):
            return int(n) if isinstance(n, float) and trunc(n) == n else n
        if not self.at(":"):
            print(nstr(self.expression()), end="")
            while self.at(";"):
                if self.at(":"):
                    return
                print(nstr(self.expression()),end="")
            self.expect(":")
        print()
    
    def do_IF(self):
        cond = self.condition()
        self.expect("THEN")
        line = self.next()
        self.expect(":")
        if cond:
            self.goto(line)
    
    def do_INPUT(self):
        name = self.next()
        self.expect(":")
        self.vars[name] = float(input("? "))
    
    def do_GOSUB(self):
        line = self.next()
        self.expect(":")
        self.stack.append(self.index)
        self.goto(line)
    
    def do_RETURN(self):
        self.expect(":")
        self.index = self.stack.pop()
        
    def do_GOTO(self):
        line = self.next()
        self.expect(":")
        self.goto(line)
    
    def do_REM(self):
        self.next()
        self.expect(":")
    
    def do_FOR(self):
        name = self.next()
        self.expect("=")
        self.vars[name] = self.expression()
        self.expect("TO")
        end = self.expression()
        self.expect(":")
        self.stack.append((name, end, self.index))

    def do_NEXT(self):
        if not self.at(":"):
            self.next()
            self.expect(":")
        name, end, index = self.stack[-1]
        self.vars[name] += 1
        if self.vars[name] >= end:
            self.stack.pop()
        else:
            self.index = index
    
    def condition(self):
        left = self.expression()
        op = self.next()
        right = self.expression()
        if op == "=":
            return left == right
        if op == "<":
            return left < right
        if op == "<=":
            return left <= right
        if op == ">":
            return left > right
        if op == ">=":
            return left >= right
        if op == "<>":
            return left != right
        raise SyntaxError("unknown comparision operator %s" % op)
    
    def expression(self):
        left = self.term()
        while True:
            if self.at("+"):
                left += self.term()
            elif self.at("-"):
                left -= self.term()
            else:
                break
        return left
    
    def term(self):
        left = self.factor()
        while True:
            if self.at("*"):
                left *= self.factor()
            elif self.at("/"):
                left /= self.factor()
            else:
                break
        return left

    def factor(self):
        token = self.next()
        if token == "TAB":
            return " " * int(self.expression())
        if token == "INT":
            return trunc(self.expression())
        if token == "RND":
            self.expression()
            return random()
        if token == "CHR$":
            return chr(int(self.expression()))
        if token == "(":
            v = self.expression()
            self.expect(")")
            return v
        if token.isdigit() or token[0] == ".":
            return float(token)
        if token.isalnum():
            return self.vars[token]
        if token[0] == '"':
            return token[1:-1]
        raise SyntaxError("can't handle %s" % token)

basic = Basic("basic_orig.bas")
basic.run()