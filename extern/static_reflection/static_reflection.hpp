#ifndef STATIC_REFLECTION_H
#define STATIC_REFLECTION_H

#include <variant>

#include <boost/spirit/home/x3.hpp>

#include <result.hpp>
#include <std_ast.hpp>

namespace static_reflection {
namespace rules {
namespace x3 = boost::spirit::x3;

using x3::eol;
using x3::lit;

x3::rule<class optionaly_space> const optionaly_space = "optionaly_space";
auto const optionaly_space_def = *(eol | ' ' | '\t');

x3::rule<class scope_end> const scope_end = "scope_end";
auto const scope_end_def = optionaly_space >> '}' >> optionaly_space >>
                           lit(';');

BOOST_SPIRIT_DEFINE(optionaly_space, scope_end);
}  // namespace rules

template <class Parent>
class StaticReflexParser {
  using Class = std_parser::rules::ast::Class;
  // TODO: generate reflection for the current class
  // TODO: refactor this method extract to shorter ones
  auto generate_class_reflection(Class& c) {
    std::string out;
    out.reserve(300);
    out += "\nfriend reflect::Reflect<";
    out += c.name;
    out += ">;\n};\n";

    if (c.is_templated()) {
      out += "template <";
      for (auto& tmp : c.template_parameters.template_parameters) {
        for (auto& s : tmp.type) {
          out += s;
          out += "::";
        }
        out.pop_back();
        out.pop_back();
        out += ' ';
        out += tmp.name;
        out += ',';
      }
      out.pop_back();
      out += '>';
      out += " struct reflect::Reflect<";
    } else {
      out += "template <> struct reflect::Reflect<";
    }
    out += c.name;
    std::string class_templates;
    if (c.is_templated()) {
      class_templates.reserve(50);
      class_templates += "<";
      for (auto& tmp : c.template_parameters.template_parameters) {
        class_templates += tmp.name;
        class_templates += ',';
      }
      class_templates.pop_back();
      class_templates += '>';
      out += class_templates;
    }
    out += "> {\n";

    out += "constexpr inline static std::tuple public_data_members = {";
    for (auto& kv : c.public_members) {
      out += '&';
      out += c.name;
      if (c.is_templated()) {
        out += class_templates;
      }
      out += "::";
      out += kv.name;
      out += ',';
    }

    if (!c.public_members.empty()) {
      out.pop_back();
    }
    out += "};\n";

    out += "constexpr inline static std::tuple public_data_member_names = {";
    for (auto& kv : c.public_members) {
      out += '\"';
      out += kv.name;
      out += '\"';
      out += ',';
    }

    if (!c.public_members.empty()) {
      out.pop_back();
    }
    out += "};\n";

    out += "using public_data_member_types = std::tuple<";
    for (auto& m : c.public_members) {
      out += "decltype(std::declval<";
      out += c.name;
      if (c.is_templated()) {
        out += class_templates;
      }
      out += ">().";
      out += m.name;
      out += ')';
      out += ',';
    }

    if (!c.public_members.empty()) {
      out.pop_back();
    }
    out += ">;\n";

    out += "constexpr inline static std::tuple data_members = {";
    auto data_members = c.public_members;
    data_members.insert(data_members.end(), c.protected_members.begin(),
                        c.protected_members.end());
    data_members.insert(data_members.end(), c.private_members.begin(),
                        c.private_members.end());
    for (auto& kv : data_members) {
      out += '&';
      out += c.name;
      if (c.is_templated()) {
        out += class_templates;
      }
      out += "::";
      out += kv.name;
      out += ',';
    }

    if (!data_members.empty()) {
      out.pop_back();
    }
    out += "};\n";

    out += "constexpr inline static std::tuple data_member_names = {";
    for (auto& kv : data_members) {
      out += '\"';
      out += kv.name;
      out += '\"';
      out += ',';
    }

    if (!data_members.empty()) {
      out.pop_back();
    }
    out += "};\n";

    out += "using data_member_types = std::tuple<";
    for (auto& m : data_members) {
      out += "decltype(std::declval<";
      out += c.name;
      if (c.is_templated()) {
        out += class_templates;
      }
      out += ">().";
      out += m.name;
      out += ')';
      out += ',';
    }

    if (!data_members.empty()) {
      out.pop_back();
    }
    out += ">;\n";

    out += "using public_base_classes = std::tuple<";
    for (auto& type : c.public_bases) {
      out += type.to_string();
      out += ',';
    }

    if (!c.public_bases.empty()) {
      out.pop_back();
    }
    out += ">;\n";

    out += "constexpr static auto name = \"";
    out += c.name;
    out += "\";\n";

    out += "static constexpr auto object_type = \"";
    switch (c.type) {
      case std_parser::rules::ast::class_type::CLASS:
        out += "reflect::ObjectType::CLASS;";
        break;
      case std_parser::rules::ast::class_type::STRUCT:
        out += "reflect::ObjectType::STRUCT;";
        break;
    }
    out += "\";\n";

    out += "};";

    return out;
  }

  template <class V>
  bool is_class(V v) {
    return std::holds_alternative<Class>(v);
  }

  auto generate_reflection() {
    auto& std_parser = parent.template get_parser<Parent::std_parser_id>();
    auto& current_class = std::get<Class>(std_parser.get_current_nesting());
    auto rez = generate_class_reflection(current_class);
    std_parser.close_current_class();
    return rez;
  }

  template <typename Source>
  auto parse_end_of_class(Source& source) {
    auto begin = source.begin();
    auto end = source.end();

    namespace x3 = boost::spirit::x3;
    bool parsed = x3::parse(begin, end, rules::scope_end);

    return parsed ? std::optional{Result{begin, generate_reflection()}}
                  : std::nullopt;
  }

  Parent& parent;

 public:
  // TODO: when supported in std=c++2a change to fixed length string
  constexpr static int id = 5;

  StaticReflexParser(Parent& p) : parent{p} {}

  /**
   * A string to prepend to each file's start
   */
  std::string get_prepend() {
    return std::string(
        "namespace reflect { template<class T> struct Reflect;}\n");
  }

  template <class Source>
  auto parse(Source& source) {
    auto& std_parser = parent.template get_parser<Parent::std_parser_id>();

    // TODO: parse $reflexpr
    return is_class(std_parser.get_current_nesting())
               ? parse_end_of_class(source)
               : std::nullopt;
  }
};
}  // namespace static_reflection

#endif  // STATIC_REFLECTION_H