#include "ssa.h"
#include <fstream>

bool ssa_translator::deep_decreased () {
    return psr_.deep_decreased ();
}

unsigned ssa_translator::get_deep_change () {
    return psr_.get_deep_change ();
}

void CFG::add_node () 
{ 
    nodes_.push_back (std::deque<const ast::IAST*> ()); 
    edges_.push_back (std::deque<bool> (edges_.size () + 1));
    size_t sz = edges_.size ();
    for (unsigned i = 0; i < sz; ++i)
        edges_[i].push_back (false);
}
    
void CFG::set_edge (unsigned from, unsigned to)
{
    assert (from < edges_.size ()); 
    assert (to < edges_[from].size ());
    edges_[from][to] = true;
}

void CFG::push_in_node (const ast::IAST *node)
{
    assert (node);
    nodes_[nodes_.size () - 1].push_back (node);
}

bool CFG::last_empty () const {
    return nodes_.back ().size () == 0;
}

void CFG::print (const char *file_name) const
{
    assert (file_name);
    std::ofstream out(file_name);
    out << "digraph  {" << std::endl;
    auto count = nodes_.size ();
    unsigned iter = 0;
    while (iter != count)
    {
        auto cur_count = nodes_[iter].size ();
        unsigned iter_1 = 0;
        out << iter << " [label = \"";
        while (iter_1 != cur_count)
        {
            auto cur_node = nodes_[iter][iter_1];
            auto cur_type = cur_node->get_op ();
            switch (cur_type)
            {
                case op::ASSIGN:    out << cur_node->get_left ()->get_var () + " = ";
                                    if (cur_node->get_right()->get_type() == VAL)
                                        out << cur_node->get_right ()->get_val () << "; ";
                                    else
                                        out << "some operations; ";
                                    break;
                case op::IF:        out << "if (..)";
                                    break;
                case op::WHILE:     out << "while (..)";
                                    break;
                default:            out << "default ";
            }
            ++iter_1;
        }
        out << "\"]" << std::endl;
        ++iter;
    }
    for (unsigned i = 0; i < count; ++i)
    {
        for (unsigned j = 0; j < count; ++j) {
            if (edges_[i][j])
                out << i << " -> " << j << std::endl;
        }
    }
    out << "}";
    out.close();
}

void ssa_translator::solve_decrease (const ast::IAST *node)
{
    assert (node);
    graph_.add_node ();
    graph_.push_in_node (node);
    size_t sz = graph_.nodes_size ();
    if (places_.top () == Place::WHILE)
        graph_.set_edge (sz - 2, cond_.top ());
    else
        graph_.set_edge (sz - 2, sz - 1);
    unsigned count = get_deep_change ();
    for (unsigned i = 0; i < count; ++i)
    {
        places_.pop ();
        graph_.set_edge (cond_.top (), sz - 1);
        cond_.pop ();
    }
}

void ssa_translator::add_if (const ast::IAST *node)
{
    assert (node);
    if (deep_decreased ())
    {
        solve_decrease (node);
        graph_.add_node ();
    }
    else
    {
        graph_.push_in_node (node);
        graph_.add_node (); 
    }
    places_.push (Place::IF); 
    graph_.set_edge (graph_.nodes_size () - 2, graph_.nodes_size () - 1);
    psr_.get_next (); psr_.get_next ();
    cond_.push (graph_.nodes_size () - 2);
}

void ssa_translator::add_while (const ast::IAST *node)
{
    assert (node);
    if (deep_decreased ())
    {
        solve_decrease (node);
        graph_.add_node ();
    }
    else
    {
        if (!graph_.last_empty ())
            graph_.add_node ();
        graph_.push_in_node (node);
        graph_.add_node ();
        size_t sz = graph_.nodes_size ();
        graph_.set_edge (sz - 3, sz - 2);
    }
    size_t sz = graph_.nodes_size ();
    graph_.set_edge (sz - 2, sz - 1);
    places_.push (Place::WHILE);
    cond_.push (sz - 2); 
    psr_.get_next (); psr_.get_next ();
}
        
void ssa_translator::build_CFG ()
{
    graph_.add_node ();
    auto cur_node = psr_.get_next ();
    while (cur_node != nullptr)
    {
        op::Operator cur_type = cur_node->get_op ();
        switch (cur_type)
        {
             case op::ASSIGN:if (deep_decreased ())
                                 solve_decrease (cur_node); 
                             else
                                 graph_.push_in_node (cur_node);
                             psr_.get_next (); psr_.get_next ();
                             break;
            
             case op::IF:    add_if (cur_node);
                             break;
            
             case op::WHILE: add_while (cur_node);
                             break;
             
             default:    assert (!"you should not have seen that\n");
        }
        cur_node = psr_.get_next ();
    }
    graph_.print ("graph.dot");
}
