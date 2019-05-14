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
        struct var_info {
            int offset_;
            bool is_ref_;
            var_info() = delete;
            var_info(int offset, bool is_ref = false):
                offset_(offset), is_ref_(is_ref)
            {}
            operator int() const { return offset_; }
        };
        using Frame = std::unordered_map<std::string, var_info>;
        unsigned label_cnt_ = 0;
        parser::Parser prsr_;
        std::stringstream func_defs_,
                          main_code_;
    private:
        void translate_indices(const IAST* node, int& sp_offset,
                               const Frame& frame, std::stringstream& out);
        void handle_capture(const IAST* node, const Frame& frame, Frame& cur_frame);

        void translate_def(const IAST* node, int& sp_offset,
                           Frame& frame, std::stringstream& out);
        void translate_code(std::list<const IAST*>& node,  int& sp_offset,
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
