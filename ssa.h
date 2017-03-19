#ifndef SSA_H
#define SSA_H

#include <deque>
#include <cassert>
#include <string>
#include <stack>
#include "parser.h"

enum Place {
    GLOBAL,
    IF,
    WHILE
};

class CFG final {
    std::deque<std::deque<const ast::IAST*>>  nodes_;
    std::deque<std::deque<bool>>        edges_;
    public:
    CFG () {}
    void add_node (); 
    void push_in_node (const ast::IAST *node);
    void set_edge (unsigned from, unsigned to);
    unsigned nodes_size () const { return nodes_.size (); }
    void print (const char *file_name) const;
    bool last_empty () const;
};

class ssa_translator final {
            CFG             graph_;
        parser::Parser      psr_;
    std::stack<unsigned>    cond_;
    std::stack<Place>       places_;
    void add_if (const ast::IAST *node);
    void add_while (const ast::IAST *node);
    void solve_decrease (const ast::IAST* node);
    bool deep_decreased ();
    unsigned get_deep_change ();
    public:
    explicit ssa_translator (const char *filename): psr_(filename, parser::TRANSLATOR) {}
    ~ssa_translator () {}
    void build_CFG ();
};

#endif
