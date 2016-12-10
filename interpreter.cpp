#include "interpreter.h"
#include <list>

namespace ipr {

    double Interpreter::run () {
        std::list<const ast::IAST*> nodes;

        const ast::IAST* cur = root_;
        while (cur->get_type () == type::OP && cur->get_op () == op::CODE) {
            nodes.push_back (cur);
            cur = cur->get_left ();
        }

        if (!nodes.empty ())
            calculate_assign (cur);
        else
            calculate_assign (root_);
        while (!nodes.empty ()) {
            cur = nodes.back ();
            nodes.pop_back ();

            calculate_assign (cur->get_right ());
        }

        auto find_res = htable_.find ("result");
        assert (find_res != htable_.end ());
        return find_res->second;
    }


    void Interpreter::calculate_assign (const ast::IAST* assign) {
        assert (assign);
        assert (assign->get_type () == type::OP);
        assert (assign->get_op () == op::ASSIGN);

        auto var = assign->get_left ();
        assert (var->get_type () == type::VAR);

        htable_.insert (std::make_pair (var->get_var (), calculate_val (assign->get_right ())));
    }


    double Interpreter::calculate_val (const ast::IAST* val_root) {
        assert (val_root);
        assert (val_root->get_type () != type::NAT);

        if (val_root->get_type () == type::VAL)
            return val_root->get_val ();
        if (val_root->get_type () == type::VAR) {
            auto find_res = htable_.find (val_root->get_var ());
            if (find_res == htable_.end ()) {
                printf ("unknown var %s\n", val_root->get_var ());
                abort ();
            }

            return find_res->second;
        }

        double left  = calculate_val (val_root->get_left  ());
        double right = calculate_val (val_root->get_right ());
        double res = 0;
        using namespace op;
        switch (val_root->get_op ()) {
            default  : assert (0); break;
            case ADD : res = left + right; break;
            case SUB : res = left - right; break;
            case MUL : res = left * right; break;
            case DIV : res = left / right; break;
        }

        return res;
    }





}