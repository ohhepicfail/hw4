#ifndef TRAN_H
#define TRAN_H

#include <unordered_map>
#include <list>
#include <string>
#include <fstream>
#include <sstream>
#include "parser.h"


class Translator final {
    private:
        using Frame = std::unordered_map<std::string, int>;
        unsigned label_cnt_ = 0;
        parser::Parser prsr_;
        std::stringstream func_defs_,
                          main_code_;
    private:
        void translate_code(std::list<const IAST*> node,  int& sp_offset, 
                            Frame& frame, std::stringstream& out);
        void translate_eq(const IAST* node, int& sp_offset, 
                          Frame& frame, std::stringstream& out);
        void translate_if(const IAST* node,  int& sp_offset, 
                          Frame& frame, std::stringstream& out);
        void translate_while(const IAST* node,  int& sp_offset, 
                             Frame& frame, std::stringstream& out);
        void translate_expr(const IAST* node, int& sp_offset, 
                            const Frame& frame, std::stringstream& out);
        void translate_func_def(const IAST* node);
    public:
        Translator() = delete;
        explicit Translator(const std::string& file_name): 
            prsr_(file_name.c_str(), parser::TRANSLATOR)
        {}
        Translator(const Translator& that) = delete;
        Translator(Translator&& that) = delete;
        Translator& operator=(const Translator& that) = delete;
        Translator& operator=(Translator&& that) = delete;
        void run();
        void write_file(const std::string& file_name) const;
};

#endif
