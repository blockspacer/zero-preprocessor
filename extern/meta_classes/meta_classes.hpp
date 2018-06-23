#ifndef META_CLASSES_H
#define META_CLASSES_H

#include <algorithm>
#include <iostream>
#include <iterator>
#include <unordered_set>
#include <variant>

#include <overloaded.hpp>
#include <result.hpp>
#include <source_loader.hpp>
#include <std_ast.hpp>
#include <string_utils.hpp>

#include <gen_utils.hpp>
#include <meta_classes_rules.hpp>
#include <meta_process.hpp>

#include <boost/process.hpp>

namespace bp = boost::process;

namespace meta_classes {
template <typename Parent>
class MetaClassParser {
  template <class T>
  auto make_result(T out) {
    return out ? std::optional{Result{out->processed_to, std::string()}}
               : std::nullopt;
  }

  template <class Source, class Writer>
  auto parse_meta_calss_function(Source& source, Writer& writer) {
    auto& std_parser = parent.template get_parser<Parent::std_parser_id>();

    const auto out = std_parser.parse(source);
    std::string res;
    if (out) {
      // if inside a constexpr function add it as a meta function
      const auto& current_nesting = std_parser.get_current_nesting();
      using Fun = std_parser::rules::ast::Function;
      if (std::holds_alternative<Fun>(current_nesting)) {
        const Fun& fun = std::get<Fun>(current_nesting);
        meta_classes.emplace(fun.name);
        inside_meta_class_function = true;
        res = {out->result.begin(), out->result.end()};
        const std::string c = "constexpr";
        auto it = std::search(res.begin(), res.end(), c.begin(), c.end());
        res.erase(it, it + c.size());

        writer(res);
      }
    }

    return out ? std::optional{Result{out->processed_to, std::string()}}
               : std::nullopt;
  }

  template <class Source, class Writer>
  auto parse_constexpr_function(Source& source, Writer& writer) {
    auto begin = source.begin();
    auto end = source.end();

    auto& std_parser = parent.template get_parser<Parent::std_parser_id>();
    auto out = std_parser.try_parse_function(begin, end);

    if (out) {
      auto res = out->result;
      // TODO: check for params to be meta::type
      bool is_constexpr_function = res.is_constexpr;

      return is_constexpr_function ? parse_meta_calss_function(source, writer)
                                   : std::nullopt;
    }

    return make_result(out);
  }

  template <class Source>
  auto parse_meta_class(Source& source) {
    bool is_parsed = false;
    auto begin = source.begin();
    auto end = source.end();
    auto& std_parser = parent.template get_parser<Parent::std_parser_id>();

    auto tmpls = std_parser.try_parse_templates_params(begin, end);
    begin = tmpls ? tmpls->processed_to : begin;
    auto meta_name = std_parser.try_parse_name(begin, end);

    bool is_known_meta_class =
        meta_name && meta_classes.count(meta_name->result);
    // NOTE: while used just to use break instead of goto
    while (is_known_meta_class) {
      begin = meta_name->processed_to;

      auto class_name = std_parser.try_parse_name(begin, end);
      if (!class_name) {
        break;
      }
      begin = class_name->processed_to;
      auto class_bases = std_parser.try_parse_class_bases(begin, end);
      if (class_bases) {
        begin = class_bases->processed_to;
      }

      auto scope_begin = std_parser.try_parse_scope_begin(begin, end);
      if (!scope_begin) {
        break;
      }
      begin = scope_begin->processed_to;

      using Class = std_parser::rules::ast::class_or_struct;
      Class cls;
      if (tmpls) {
        cls.template_parameters = std::move(tmpls->result);
      }
      cls.name = class_name->result;
      cls.type = std_parser::rules::ast::class_type::META_CLASS;
      if (class_bases) {
        cls.bases = std::move(class_bases->result);
      }
      std_parser.open_new_nesting(std::move(cls));
      is_parsed = true;
      current_meta_class = std::move(meta_name->result);
      current_meta_class_name = std::move(class_name->result);
      break;
    }

    return is_parsed ? std::optional{Result{begin, std::string{}}}
                     : std::nullopt;
  }

  template <class Source, class Writer>
  auto parse_include(Source& source, Writer& writer) {
    auto begin = source.begin();
    auto end = source.end();

    auto& std_parser = parent.template get_parser<Parent::std_parser_id>();
    auto out = std_parser.parse_include(source);

    if (out) {
      std::string inc(source.begin(), out->processed_to);
      writer(inc);
    }

    return out ? std::optional{Result{out->processed_to,
                                      std::string{out->result}}}
               : std::nullopt;
  }

  template <class Source, class Writer>
  auto parse_meta(Source& source, Writer& writer) {
    auto out = parse_constexpr_function(source, writer);
    if (out) {
      return out;
    }
    out = parse_meta_class(source);
    if (out) {
      return out;
    }

    return parse_include(source, writer);
  }

  template <class Source, class Writer>
  auto parse_target(Source& source, Writer& writer) {
    auto begin = source.begin();
    auto end = source.end();

    rules::ast::Target t;
    namespace x3 = boost::spirit::x3;
    bool parsed = x3::parse(begin, end,
                            // begin rules
                            rules::target,
                            // end rules
                            t);

    std::string out;
    if (parsed) {
      out = gen_target_output(t, source.begin(), begin);
      writer(out);
    }

    return parsed ? std::optional{Result{begin, std::string()}} : std::nullopt;
  }

  template <class Source, class Writer>
  auto parse_inside_constexpr_function(Source& source, Writer& writer) {
    auto& std_parser = parent.template get_parser<Parent::std_parser_id>();
    auto out = std_parser.parse(source);
    if (out) {
      writer(out->result);

      // if end of function inside_meta_class_function = false;
      auto& current_nesting = std_parser.get_current_nesting();
      using Fun = std_parser::rules::ast::Function;
      using Scope = std_parser::rules::ast::Scope;
      if (!std::holds_alternative<Fun>(current_nesting) &&
          !std::holds_alternative<Scope>(current_nesting)) {
        inside_meta_class_function = false;
      }
    }

    return out ? make_result(out) : parse_target(source, writer);
  }

  /**
   * Traverse the current nestings and try to find a class with our
   * current_meta_class_name
   */
  bool is_still_inside_meta_class() {
    auto& std_parser = parent.template get_parser<Parent::std_parser_id>();
    auto& nestings = std_parser.get_all_nestings();

    using Class = std_parser::rules::ast::Class;

    return std::any_of(nestings.begin(), nestings.end(), [this](auto& nesting) {
      if (!std::holds_alternative<Class>(nesting)) {
        return false;
      }
      auto class_nesting = std::get<Class>(nesting);
      return class_nesting.name == current_meta_class_name;
    });
  }

  template <class Source>
  auto parse_inside_meta_class(Source& source) {
    auto& std_parser = parent.template get_parser<Parent::std_parser_id>();
    auto begin = source.begin();
    auto end = source.end();

    namespace x3 = boost::spirit::x3;
    bool parsed = x3::parse(begin, end, rules::scope_end);

    std::string output;
    using Class = std_parser::rules::ast::Class;
    if (parsed && meta_process.ok()) {
      auto& current_nesting = std_parser.get_current_nesting();
      auto& cls = std::get<Class>(current_nesting);
      output =
          gen_meta_class(meta_process, current_meta_class, cls, std_parser);
    }

    auto out = std_parser.parse(source);
    if (out) {
      if (!is_still_inside_meta_class()) {
        current_meta_class.clear();
        current_meta_class_name.clear();
        return std::optional{Result{out->processed_to, output}};
      }
    }

    // TODO: if not parsed throw error ?
    return make_result(out);
  }

  Parent& parent;
  std::string meta_exe;
  std::unordered_set<std::string> meta_classes;
  bool inside_meta_class_function = false;
  std::string current_meta_class;
  std::string current_meta_class_name;
  std::ofstream out_file;
  source::SourceLoader source_loader;
  MetaProcess meta_process;
  bool is_source = false;

 public:
  // TODO: when supported in std=c++2a change to fixed length string
  constexpr static int id = 7;

  MetaClassParser(Parent& p, std::string_view meta_exe,
                  std::string_view meta_out)
      : parent{p}, meta_exe{meta_exe}, source_loader{{}, meta_out} {
    if (!this->meta_exe.empty()) {
      meta_process = MetaProcess(this->meta_exe);
      bp::opstream& p1 = meta_process.output;
      bp::ipstream& p2 = meta_process.input;
      p1 << '1' << std::endl;
      std::string out;
      int n;
      p2 >> n;
      while (n-- && p2 >> out) {
        meta_classes.emplace(std::move(out));
      }
    }
  }

  MetaClassParser(MetaClassParser&&) = default;

  ~MetaClassParser() noexcept {
    if (!this->meta_exe.empty()) {
      meta_process.output << 3 << std::endl;
      meta_process.wait();
    }
  }

  void start_preprocess(std::string_view source_name) {
    std::cout << "preprocess " << source_name << std::endl;
    out_file = source_loader.open_source(source_name);
    out_file << "#include <meta.hpp>" << std::endl;
    is_source = source::is_source(source_name);
  }

  /**
   * Parse constexpr meta class function
   * and write them
   */
  template <class Source>
  auto preprocess(Source& source) {
    auto writer = [this](auto& src) {
      for (auto& elem : src) {
        out_file << elem;
      }
      out_file << '\n';
    };

    // TODO: fix this

    if (inside_meta_class_function) {
      return parse_inside_constexpr_function(source, writer);
    }
    if (!current_meta_class.empty()) {
      return parse_inside_meta_class(source);
    }

    return parse_meta(source, writer);
  }

  void finish_preprocess() {
    if (is_source) {
      out_file << gen_main(meta_classes);
    }
    out_file.close();
  }

  /**
   * Parse a Meta class
   */
  template <class Source>
  auto parse(Source& source) {
    // TODO: fix this
    auto writer = [](auto&) {};
    if (inside_meta_class_function) {
      return parse_inside_constexpr_function(source, writer);
    }
    if (!current_meta_class.empty()) {
      return parse_inside_meta_class(source);
    }

    return parse_meta(source, writer);
  }
};
}  // namespace meta_classes

#endif  // META_CLASSES_H
