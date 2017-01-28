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
        if (find_res == htable_.end ()) {
            printf ("No variable 'result'\n");
            abort ();
        }
        return find_res->second;
    }


    void Interpreter::calculate_assign (const ast::IAST* assign) {
        assert (assign);
        assert (assign->get_type () == type::OP);
        assert (assign->get_op () == op::ASSIGN);

        auto var = assign->get_left ();
        assert (var->get_type () == type::VAR);

        auto res = calculate_tern (assign->get_right ());
        auto find_res = htable_.find (var->get_var ());
        if (find_res == htable_.end ())
            htable_.insert (std::make_pair (var->get_var (), res));
        else
            find_res->second = res;
    }


    double Interpreter::calculate_tern (const ast::IAST* tern) {
        assert (tern);

        if (tern->get_type () == type::OP && tern->get_op () == op::TERN) {
            bool cond = calculate_cond (tern->get_left ());
            auto right = tern->get_right ();
            assert (right);
            if (cond)
                return calculate_val (right->get_left ());
            else
                return calculate_val (right->get_right ());

        }
        else
            return calculate_val (tern);
    }


    bool Interpreter::calculate_cond (const ast::IAST* cond) {
        assert (cond);

        auto l = calculate_val (cond->get_left ());
        auto r = calculate_val (cond->get_right ());

        assert (cond->get_type () == type::OP);
        auto oper = cond->get_op ();

        using namespace op;
        switch (oper) {
            default       : printf ("\nunknown a comparison operator\n"); abort (); break;
            case MORE     : return l > r;
            case MOREOREQ : return l >= r;
            case LESS     : return l < r;
            case LESSOREQ : return l <= r;
            case EQUAL    : return l == r;
            case NOTEQUAL : return l != r;
        }
    }


    double Interpreter::calculate_val (const ast::IAST* val_root) {
        assert (val_root);
        assert (val_root->get_type () != type::NAT);

        if (val_root->get_type () == type::VAL)
            return val_root->get_val ();
        if (val_root->get_type () == type::OP
            && val_root->get_op () == op::SUB
            && val_root->get_left () == nullptr)
            return -calculate_val (val_root->get_right ());
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

        htable_ = std::move (that.htable_);

        return *this;
    }
}
