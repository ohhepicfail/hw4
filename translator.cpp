#include "translator.h"
#include <stack>
#include <numeric>

void Translator::run()
{
    auto node = prsr_.get_next_stmt();
    Frame frame;
    frame.emplace("result", -4);
    main_code_ << "\taddi sp, sp, -4\n";
    main_code_ << "\tli t0, 0\n";
    main_code_ << "\tsw t0, 0(sp)\n";
    int sp_offset = -4;
    std::list<const IAST*> code;
    while (node)
    {
        if (node->get_op() == op::FUNCTION)
        {
            translate_code(code, sp_offset, frame, main_code_);
            code.clear();
            translate_func_def(node);
        }
        else
            code.push_back(node);
        auto next = prsr_.get_next_stmt();
        if (next->get_right() == node)
            break;
        node = next->get_right();
    }
    translate_code(code, sp_offset, frame, main_code_);
    //load result to a0 if exists
    auto iter = frame.find("result");
    if (iter != frame.end())
        main_code_ << "\tlw a0, " << (*iter).second - sp_offset << "(sp)\n";
    //otherwise load 0 to a0
    else
        main_code_ << "\tli a0, 0\n";
    //clear stack
    main_code_ << "\taddi sp, sp, " << -sp_offset << "\n";
    //return
    main_code_ << "\tret\n";
}

void Translator::translate_eq(const IAST* node, int& sp_offset, 
                                  Frame& frame, std::stringstream& out)
{
    auto var = node->get_left();
    //in case of array access
    while (var->get_type() == OP)
        var = var->get_left();
    //check var exist
    bool var_exist = false;
    auto iter = frame.find(var->get_var());
    if (iter == frame.end())
    {
        //reserve space for new variable
        sp_offset -= 4;
        frame.emplace(var->get_var(), sp_offset);
        out << "\taddi sp, sp, -4\n";
    }
    else
        var_exist = true;
    translate_expr(node->get_right(), sp_offset, frame, out);
    if (var_exist)
    {
        auto lhs = node->get_left();
        //array
        if (lhs->get_type() == OP)
        {
            translate_indices(lhs, sp_offset, frame, out);
            out << "\tlw t0, 0(sp)\n";
            out << "\tsw t0, 0(t2)\n";
        }
        //var
        else
        {
            int var_offset = (*iter).second - sp_offset;
            //save res to var
            out << "\tlw t0, 0(sp)\n";
            out << "\tsw  t0, " << var_offset << "(sp)\n";
        }
    }
    else
    {
        //save res to new var
        out << "\tlw t0, 0(sp)\n";
        out << "\tsw t0, 4(sp)\n";
    }
    out << "\taddi sp, sp, 4\n";
    sp_offset += 4;
}

void Translator::translate_indices(const IAST* node, int& sp_offset,
                                const Frame& frame, std::stringstream& out)
{
    std::vector<const IAST*> indices_expr;
    //collect indices exprs
    while (node->get_type() == OP)
    {
        indices_expr.push_back(node->get_right());
        node = node->get_left();
    }
    std::vector<unsigned> offsets;
    auto& dims = node->get_dims();
    //calculate offset for each dim
    for (auto it = dims.rbegin(); it != dims.rend(); ++it)
    {
        offsets.push_back(std::accumulate(dims.rbegin(), it, 4u, [](auto lhs, auto rhs){ return lhs * rhs; }));
    }
    //translate indices exprs
    for (auto it = indices_expr.rbegin(); it != indices_expr.rend(); ++it)
        translate_expr(*it, sp_offset, frame, out);
    //calculate final offset
    out << "\tli t2, 0\n";
    for (unsigned i = 0; i < offsets.size(); ++i)
    {
        out << "\tlw t0, " << 4 * i << "(sp)\n";
        out << "\tli t1, " << offsets[i] << "\n";
        out << "\tmul t0, t0, t1\n";
        out << "\tadd t2, t2, t0\n";
    }
    auto iter = frame.find(node->get_var());
    auto arr_offset = (*iter).second - sp_offset;
    out << "\taddi t2, t2, " << arr_offset << "\n";
    out << "\tadd t2, t2, sp\n";
    sp_offset += 4 * offsets.size();
    out << "\taddi sp, sp, " << 4 * offsets.size() << "\n";
}

void Translator::translate_expr(const IAST* node, int& sp_offset,
                                const Frame& frame, std::stringstream& out)
{
    if (node == nullptr)
        return;
    auto type = node->get_type();
    if (type == VAR)
    {
        auto iter = frame.find(node->get_var());
        int var_offset = (*iter).second - sp_offset;
        //load var
        out << "\tlw t0, " << var_offset << "(sp)\n";
        //push var in stack for calculations
        out << "\taddi sp, sp, -4\n";
        out << "\tsw t0, 0(sp)\n";
        sp_offset -= 4;
        return;
    }
    if (type == VAL)
    {
        int imm = node->get_val();
        //load imm in reg and push to stack
        out << "\tli t0, " << imm << "\n";
        out << "\taddi sp, sp, -4\n";
        out << "\tsw t0, 0(sp)\n";
        sp_offset -= 4;
        return;
    }
    //special case for access
    if (node->get_op() == op::AACCESS)
    {
        //calculate total offset in t2
        translate_indices(node, sp_offset, frame, out);
        sp_offset -= 4;
        out << "\tlw t0, 0(t2)\n";
        out << "\taddi sp, sp, -4\n";
        out << "\tsw t0, 0(sp)\n";
        return;
    }
    //special case for call
    if (node->get_op() == op::CALL)
    {
        auto name = node->get_left();
        //save caller's ra
        sp_offset -= 4;
        out << "\taddi sp, sp, -4\n";
        out << "\tsw ra, 0(sp)\n";
        //push arguments backwards
        std::stack<const IAST*> args;
        node = node->get_right();
        for ( ; node; node = node->get_left())
            args.push(node);
        int args_num = args.size();
        while (!args.empty())
        {
            auto arg = args.top();
            args.pop();
            //load var
            if (arg->get_type() == VAR)
            {
                auto iter = frame.find(arg->get_var());
                out << "\tlw t0, " << (*iter).second - sp_offset << "(sp)\n";
            }
            //load imm 
            else
            {
                int imm = node->get_val();
                out << "\tli t0, " << imm << "\n";
            }
            //push arg in stack for calculations
            out << "\taddi sp, sp, -4\n";
            out << "\tsw t0, 0(sp)\n";
            sp_offset -= 4;
        }
        out << "\tjal ra, " << name->get_var() << "\n";
        //clear args
        out << "\taddi sp, sp, " << 4 * args_num << "\n";
        sp_offset += 4 * args_num;
        //load saved ra
        out << "\tlw ra, 0(sp)\n";
        //push return value
        out << "\tsw a0, 0(sp)\n";
        return;
    }
    //translate left operand
    translate_expr(node->get_left(), sp_offset, frame, out);
    //special case for ternar
    if (node->get_op() == op::TERN)
    {
        auto false_label = ++label_cnt_;
        auto exit_label = ++label_cnt_;
        sp_offset += 4;
        //load operand
        out << "\tlw t0, 0(sp)\n";
        out << "\taddi sp, sp ,4\n";
        //branch on false expr
        out << "\tbeqz t0, L_" << false_label << "\n";
        node = node->get_right();
        auto true_expr_node = node->get_left();
        auto false_expr_node = node->get_right();
        auto saved_offset = sp_offset;
        //translate true expr
        translate_expr(true_expr_node, sp_offset, frame, out);
        //jump to exit
        out << "\tj L_" << exit_label << "\n";
        //translate false expr
        out << "L_" << false_label << ":\n";
        translate_expr(false_expr_node, saved_offset, frame, out);
        out << "L_" << exit_label << ":\n";
        return;
    }
    //translate right operand
    translate_expr(node->get_right(), sp_offset, frame, out);
    //load operands
    out << "\tlw t0, 4(sp)\n";
    out << "\tlw t1, 0(sp)\n";
    //perform operation
    out << op::asm_string(node->get_op());
    //store res
    out << "\taddi sp, sp, 4\n";
    out << "\tsw t2, 0(sp)\n";
    sp_offset += 4;
}

void Translator::translate_def(const IAST* node, int& sp_offset,
                               Frame& frame, std::stringstream& out)
{
    node = node->get_left();
    int size = std::accumulate(node->get_dims().rbegin(),
                                node->get_dims().rend(),
                                4u,
                                [](auto lhs, auto rhs) { return lhs * rhs; });
    sp_offset -= size;
    frame.emplace(node->get_var(), sp_offset);
    out << "\taddi sp, sp, " << -size << "\n";
}

void Translator::translate_code(std::list<const IAST*>& code, int& sp_offset,
                                Frame& frame, std::stringstream& out)
{
    for (auto node : code)
    {
        switch (node->get_op())
        {
            case op::ASSIGN: translate_eq(node, sp_offset, frame, out);
                             break;
            case op::IF:     translate_if(node, sp_offset, frame, out);
                             break;
            case op::WHILE:  translate_while(node, sp_offset, frame, out);
                             break;
            case op::DEF:    translate_def(node, sp_offset, frame, out);
                             break;
            default: break;
        }
    }
}

void Translator::handle_capture(const IAST* node, const Frame& frame, Frame& cur_frame)
{
    auto& var_list = node->get_var();
    if (var_list.empty())
        return;
    //add vars from capture list
    if (var_list[0] == '*')
        cur_frame = frame;
    else
    {
        auto from_elem = var_list.begin();
        auto elem = from_elem + 1;
        for (; elem != var_list.end(); ++elem)
        {
            std::string cur_var;
            if (*elem == ',')
            {
                cur_var.assign(from_elem, elem);
                auto iter = frame.find(cur_var);
                cur_frame.insert(*iter);
                from_elem = elem + 1;
            }
        }
        std::string cur_var;
        cur_var.assign(from_elem, elem);
        auto iter = frame.find(cur_var);
        cur_frame.insert(*iter);
        cur_frame.insert(*frame.find("result"));
    }
}

void Translator::translate_if(const IAST* node, int& sp_offset, 
                              Translator::Frame& frame, std::stringstream& out)
{
    //translate expr
    auto expr = node->get_left();
    translate_expr(expr, sp_offset, frame, out);
    //translate if
    auto false_label = ++label_cnt_;
    sp_offset += 4;
    out << "\tlw t0, 0(sp)\n";
    out << "\taddi sp, sp, 4\n";
    out << "\tbeqz t0, L_" << false_label << "\n";
    node = node->get_right();
    //create frame
    Frame cur_frame;
    auto var = node->get_left();
    handle_capture(var, frame, cur_frame);
    //collect code to translate
    node = node->get_right();
    std::list<const IAST*> code;
    while (node->get_type() == OP && node->get_op() == op::CODE)
    {
        code.push_front(node->get_right());
        node = node->get_left();
    }
    code.push_front(node);
    int saved_sp_offset = sp_offset;
    //translate collected code
    translate_code(code, sp_offset, cur_frame, out);
    //clear stack
    out << "\taddi sp, sp, " << saved_sp_offset - sp_offset << "\n";
    sp_offset = saved_sp_offset;
    //false label
    out << "L_" << false_label << ":\n";
}

void Translator::translate_while(const IAST* node, int& sp_offset, 
                                 Translator::Frame& frame, std::stringstream& out)
{
    //check expr label
    auto check_label = ++label_cnt_;
    out << "L_" << check_label << ":\n";
    //translate expr
    auto expr = node->get_left();
    translate_expr(expr, sp_offset, frame, out);
    //translate while
    auto false_label = ++label_cnt_;
    sp_offset += 4;
    out << "\tlw t0, 0(sp)\n";
    out << "\taddi sp, sp, 4\n";
    out << "\tbeqz t0, L_" << false_label << "\n";
    node = node->get_right();
    //create frame
    Frame cur_frame;
    auto var = node->get_left();
    handle_capture(var, frame, cur_frame);
    //collect code to translate
    node = node->get_right();
    std::list<const IAST*> code;
    while (node->get_type() == OP && node->get_op() == op::CODE)
    {
        code.push_front(node->get_right());
        node = node->get_left();
    }
    code.push_front(node);
    int saved_sp_offset = sp_offset;
    //translate collected code
    translate_code(code, sp_offset, cur_frame, out);
    //clear stack
    out << "\taddi sp, sp, " << saved_sp_offset - sp_offset << "\n";
    sp_offset = saved_sp_offset;
    //jump to check expr
    out << "\tj L_" << check_label << "\n";
    //false label
    out << "L_" << false_label << ":\n";
}

void Translator::translate_func_def(const IAST* node)
{
    auto name = node->get_left();
    node = node->get_right();
    //set label
    func_defs_ << "\n.section .text\n.globl " << name->get_var() << "\n";
    func_defs_ << name->get_var() << ":\n";
    //push args to frame
    Frame frame;
    auto arg = node->get_left();
    int arg_cnt = 0;
    while (arg)
    {
        frame.emplace(arg->get_var(), arg_cnt * 4);
        ++arg_cnt;
        arg = arg->get_left();
    }
    int sp_offset = 0;
    frame.emplace("result", -4);
    sp_offset -= 4;
    func_defs_ << "\taddi sp, sp, -4\n";
    func_defs_ << "\tli t0, 0\n";
    func_defs_ << "\tsw t0, 0(sp)\n";
    node = node->get_right();
    //collect code to translate
    std::list<const IAST*> code;
    while (node->get_type() == OP && node->get_op() == op::CODE)
    {
        code.push_front(node->get_right());
        node = node->get_left();
    }
    code.push_front(node);
    //translate collected code
    translate_code(code, sp_offset, frame, func_defs_);
    //put return value to a0
    auto iter = frame.find("result");
    func_defs_ << "\tlw a0, " << (*iter).second - sp_offset << "(sp)\n";
    //clear stack and return
    func_defs_ << "\taddi sp, sp, " << -sp_offset << "\n";
    func_defs_ << "\tret\n";
}

void Translator::write_file(const std::string& file_name) const
{
    std::ofstream f;
    f.open(file_name);
    f << func_defs_.str() << "\n.section .text\n.globl main\nmain:\n" << main_code_.str();
    f.close();
}

