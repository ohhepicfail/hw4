#include "interpreter.h"
#include <list>

namespace ipr {
    struct Node_info {
        const ast::IAST*  node_;
        bool need_to_visit_;
    };
    
    std::stack<Node_info> build_expr_stack (const ast::IAST*  node);


    double Interpreter::run () {
        root_->print ("tree.dot");
        calculate ();

        auto find_res = var_value_.find ("result");
        if (find_res == var_value_.end ()) {
            printf ("No variable 'result'\n");
            abort ();
        }
        return find_res->second;
    }


    void Interpreter::calculate () {
        struct Program_data {
            std::stack <const ast::IAST*> prog_;
            std::unordered_map <std::string, double> var_htable_;
        };
        std::stack <Program_data> scope;
        
        const ast::IAST* cur_node = root_;
        for (;;) {
            if (cur_node->get_type () != type::OP) {
                printf ("Statement has no effect\n");
                abort ();
            }
            switch (cur_node->get_op ()) {

                default:
                    assert (0);
                break;


                case op::CODE: {
                    auto r = cur_node->get_right ();
                    auto l = cur_node->get_left ();
                    assert (r);
                    assert (l);

                    prog_nodes_.push (r);
                    prog_nodes_.push (l);
                } break;


                case op::ASSIGN: {              // assign + tern
                    auto var = cur_node->get_left ();
                    assert (var);
                    assert (var->get_type () == type::VAR);

                    auto r = cur_node->get_right ();
                    decltype (calculate_val (r)) res;

                    if (r->get_type () == type::OP && r->get_op () == op::TERN) {
                        cur_node = r->get_right ();
                        assert (cur_node);

                        if (calculate_cond (r->get_left ()))
                            res = calculate_val (cur_node->get_left ());
                        else
                            res = calculate_val (cur_node->get_right ());      
                    }
                    else
                        res = calculate_val (r);

                    auto find_res = var_value_.find (var->get_var ());
                    if (find_res == var_value_.end ())
                        var_value_.insert (std::make_pair (var->get_var (), res));
                    else
                        find_res->second = res;
                } break;


                case op::WHILE:
                case op::IF: {
                    if (!calculate_cond (cur_node->get_left ()))
                        break;

                    prog_nodes_.push (cur_node);        // we need var_list for update_htable () when the scope 
                                                        // will be decreased
                    cur_node = cur_node->get_right ();
                    assert (cur_node);

                    scope.push (Program_data {prog_nodes_, var_value_});

                    std::stack <const ast::IAST*> ().swap (prog_nodes_);    // clear stack
                    assert (prog_nodes_.empty ());

                    create_htable (cur_node->get_left ());
                    cur_node = cur_node->get_right ();
                    continue;
                } break;

            }

            for (;;) {
                if (prog_nodes_.empty ()) {
                    if (scope.empty ())
                        return;

                    auto tmp_prog_nodes = scope.top ().prog_;
                    cur_node = tmp_prog_nodes.top ();
                    tmp_prog_nodes.pop ();

                    if (cur_node->get_op () == op::WHILE) 
                        if (calculate_cond (cur_node->get_left ())) {
                            cur_node = cur_node->get_right ()->get_right ();
                            break;
                        }

                    prog_nodes_ = tmp_prog_nodes;

                    cur_node = cur_node->get_right ()->get_left ();        // var_list for update_htable ()
                    update_htable (cur_node, scope.top ().var_htable_);
                    scope.pop ();
                }
           
                if (prog_nodes_.empty ())
                    continue;
                cur_node = prog_nodes_.top ();
                prog_nodes_.pop ();
                break;
            }
            

        }

    }


    bool Interpreter::calculate_cond (const ast::IAST* cond) {
        assert (cond);

        if (cond->get_type () != OP) 
            return calculate_val (cond);

        auto l = calculate_val (cond->get_left ());
        auto r = calculate_val (cond->get_right ());

        assert (cond->get_type () == type::OP);
        auto oper = cond->get_op ();

        using namespace op;
        switch (oper) {
            default       : printf ("\nunknown comparison operator\n"); abort (); break;
            case MORE     : return l > r;
            case MOREOREQ : return l >= r;
            case LESS     : return l < r;
            case LESSOREQ : return l <= r;
            case EQUAL    : return l == r;
            case NOTEQUAL : return l != r;
        }
    }


    void build_expr_stack (const ast::IAST* node, std::stack<Node_info>& pref_notation) {
        assert (node);
    
        std::stack<Node_info> ().swap (pref_notation);
        
        while (node->get_type () == type::OP) {
            auto r = node->get_right ();
            auto l = node->get_left ();
            assert (l);
            assert (r);

            pref_notation.push ({node, false});
            if (r->get_type () == type::OP)
                pref_notation.push ({r, true});
            else if (r->get_type () == type::VAR || r->get_type () == type::VAL)
                pref_notation.push ({r, false});
            else 
                assert (0);

            node = l;
        }

        pref_notation.push ({node, false});
    }


    double Interpreter::get_value (const ast::IAST* node) {
        assert (node);
        double res = 0;
        if (node->get_type () == type::VAL)
            res = node->get_val ();
        else if (node->get_type () == type::VAR) {
            auto find_res = var_value_.find (node->get_var ());
            if (find_res == var_value_.end ()) {
                printf ("unknown var %s\n", node->get_var ());
                abort ();
            }
            res = find_res->second;
        }
        else 
            assert (0);

        return res;
    }


    double Interpreter::calculate_val (const ast::IAST* node) {
        assert (node);
        assert (node->get_type () != type::NAT);

        struct Expr {
            Expr (std::stack<Node_info> pref_notation, double value) : pref_notation_ (pref_notation), value_ (value) {}
            std::stack<Node_info> pref_notation_;  // see polish prefix notation
            double value_;              
        };
        std::stack<Expr> less_priority;     // need it when we calculate right subtree

        std::stack <Node_info> cur_pref_notation;
        build_expr_stack (node, cur_pref_notation);
        double right = 0;
        double left  = get_value (cur_pref_notation.top ().node_);
        cur_pref_notation.pop ();
        bool visit = false;

        for (;;) {
            assert (node);

            if (cur_pref_notation.empty ()) {
                if (less_priority.empty ())
                    break;
                
                right = left;
                cur_pref_notation = less_priority.top ().pref_notation_;
                left = less_priority.top ().value_;
                less_priority.pop ();
            }

            node  = cur_pref_notation.top ().node_;
            visit = cur_pref_notation.top ().need_to_visit_;
            cur_pref_notation.pop ();

            switch (node->get_type ()) {
                default: abort ();
                break;

                case type::OP:
                    if (!visit) {
                        using namespace op;
                        switch (node->get_op ()) {
                            default  : assert (0); break;
                            case ADD : left += right; break;
                            case SUB : left -= right; break;
                            case MUL : left *= right; break;
                            case DIV : left /= right; break;
                        }
                    }
                    else {
                        less_priority.push (Expr (cur_pref_notation, left));
                        build_expr_stack (node, cur_pref_notation);
                        left = get_value (cur_pref_notation.top().node_);
                        cur_pref_notation.pop ();
                    }
                break;

                case type::VAL:
                case type::VAR:
                    right = get_value (node);
                break;
            }
        }

        return left;
    }
    

    void Interpreter::update_htable (const ast::IAST* var_list, decltype (var_value_) old_htable) {
        assert (var_list);
        assert (var_list->get_type () == VAR);

        auto var_str = var_list->get_var ();
        assert (var_str);

        if (var_str[0] == '*') {
            for (const auto& elem : var_value_) {
                auto find_res = old_htable.find (elem.first);
                if (find_res != old_htable.end ())
                    find_res->second = elem.second;
            }
        }
        else {
            unsigned begin = 0;
            unsigned end   = 0;
            while (var_str[end] != '\0') {
                while (var_str[end] != ',' && var_str[end] != '\0')
                    end++;

                auto var = new char[end - begin + 1] ();
                std::copy (var_str + begin, var_str + end, var);
                end++;
                begin = end;

                auto find_res = var_value_.find (var);
                assert (find_res != var_value_.end ());
                auto find_res_old = old_htable.find(var);
                if (find_res_old != old_htable.end ())
                    find_res_old->second = find_res->second;
                delete[] var;
                if (var_str[end - 1] == '\0')
                    break;
            }
        }

        var_value_ = old_htable;
    }


    void Interpreter::create_htable (const ast::IAST* var_list) {
        assert (var_list);
        assert (var_list->get_type () == VAR);

        auto var_str = var_list->get_var ();
        assert (var_str);
        auto old_htable = var_value_;
        var_value_.clear ();

        unsigned begin = 0;
        unsigned end   = 0;
        while (var_str[end] != '\0') {
            if (var_str[begin] == '*') {
                var_value_ = old_htable;
                break;
            }
            while (var_str[end] != ',' && var_str[end] != '\0')
                end++;

            auto var = new char[end - begin + 1] ();
            std::copy (var_str + begin, var_str + end, var);
            end++;
            begin = end;

            auto find_res = old_htable.find (var);
            if (find_res == old_htable.end ()) {
                printf ("\nunknown var '%s' in capture block\n", var);
                abort ();
            }
            else
                var_value_.insert (std::make_pair (var, find_res->second));
            delete[] var;
            if (var_str[end - 1] == '\0')
                break;
        }

    }


    Interpreter& Interpreter::operator= (const Interpreter& that) {
        if (this == &that)
            return *this;

        delete root_;
        root_ = nullptr;
        Interpreter tmp (that);
        *this = std::move (tmp);

        return *this;
    }


    Interpreter& Interpreter::operator= (Interpreter&& that) {
        delete root_;
        root_ = that.root_;
        that.root_ = nullptr;

        prog_nodes_ = std::move (that.prog_nodes_);
        var_value_  = std::move (that.var_value_);

        return *this;
    }
}
