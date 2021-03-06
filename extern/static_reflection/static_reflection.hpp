#ifndef STATIC_REFLECTION_H
#define STATIC_REFLECTION_H

#include <variant>

#include <boost/spirit/home/x3.hpp>

#include <result.hpp>
#include <std_ast.hpp>
#include <std_helpers.hpp>

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

x3::rule<class reflexpr> const reflexpr = "reflexpr";
auto const reflexpr_def =
    optionaly_space >> "reflexpr" >> optionaly_space >> '(';

BOOST_SPIRIT_DEFINE(optionaly_space, scope_end, reflexpr);
}  // namespace rules

namespace helper = std_parser::rules::ast;

template <class Parent>
class StaticReflexParser {
  using Class = std_parser::rules::ast::Class;
  using Enumeration = std_parser::rules::ast::Enumeration;
  using Expression = std_parser::rules::ast::Expression;
  using RoundExpression = std_parser::rules::ast::RoundExpression;
  using var = std_parser::rules::ast::var;

  /**
   * Append pointer to data members  as &class_name<tmps>::member.name
   */
  void append_members(std::string& out, std::vector<var> data_members,
                      std::string_view class_name,
                      std::string_view class_templates) {
    for (auto& member : data_members) {
      out += '&';
      out += class_name;
      out += class_templates;
      out += "::";
      out += member.name;
      out += ',';
    }

    if (!data_members.empty()) {
      out.pop_back();
    }
  }

  /**
   * Append data member names  as "member.name"
   */
  void append_names(std::string& out, std::vector<var> data_members) {
    for (auto& member : data_members) {
      out += '\"';
      out += member.name;
      out += '\"';
      out += ',';
    }

    if (!data_members.empty()) {
      out.pop_back();
    }
  }

  /**
   * append types as decltype(std::declval<class_name<temps>().m.name)
   */
  void append_types(std::string& out, std::vector<var> data_members,
                    std::string_view class_name,
                    std::string_view class_templates) {
    for (auto& m : data_members) {
      out += "decltype(std::declval<";
      out += class_name;
      out += class_templates;
      out += ">().";
      out += m.name;
      out += ')';
      out += ',';
    }

    if (!data_members.empty()) {
      out.pop_back();
    }
  }

  // TODO: generate reflection for the current class
  // TODO: refactor this method extract to shorter ones
  auto generate_reflection(Class& c) {
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

    out +=
        "constexpr inline static auto public_data_members = std::make_tuple(";
    append_members(out, c.public_members, c.name, class_templates);
    out += ");\n";

    out +=
        "constexpr inline static auto public_data_member_names = "
        "std::make_tuple(";
    append_names(out, c.public_members);
    out += ");\n";

    out += "using public_data_member_types = std::tuple<";
    append_types(out, c.public_members, c.name, class_templates);
    out += ">;\n";

    out += "constexpr inline static auto data_members = std::make_tuple(";
    auto data_members = c.public_members;
    data_members.insert(data_members.end(), c.protected_members.begin(),
                        c.protected_members.end());
    data_members.insert(data_members.end(), c.private_members.begin(),
                        c.private_members.end());
    append_members(out, data_members, c.name, class_templates);
    out += ");\n";

    out += "constexpr inline static auto data_member_names = std::make_tuple(";
    append_names(out, data_members);
    out += ");\n";

    out += "using data_member_types = std::tuple<";
    append_types(out, data_members, c.name, class_templates);
    out += ">;\n";

    out += "using public_base_classes = std::tuple<";
    for (auto& type : c.public_bases) {
      out += helper::to_string(type);
      out += ',';
    }

    if (!c.public_bases.empty()) {
      out.pop_back();
    }
    out += ">;\n";
    auto base_classes = c.public_bases;
    base_classes.insert(base_classes.end(), c.protected_bases.begin(),
                        c.protected_bases.end());
    base_classes.insert(base_classes.end(), c.private_bases.begin(),
                        c.private_bases.end());
    out += "using base_classes = std::tuple<";
    for (auto& type : base_classes) {
      out += helper::to_string(type);
      out += ',';
    }

    if (!base_classes.empty()) {
      out.pop_back();
    }
    out += ">;\n";

    out += "constexpr static auto name = \"";
    out += c.name;
    out += "\";\n";

    out += "static constexpr auto object_type = ";
    switch (c.type) {
      case std_parser::rules::ast::class_type::CLASS:
        out += "reflect::ObjectType::CLASS;";
        break;
      case std_parser::rules::ast::class_type::STRUCT:
        out += "reflect::ObjectType::STRUCT;";
        break;
      default:
        break;
    }
    out += ";\n";

    out += "};";

    return out;
  }

  auto generate_reflection(Enumeration& c) {
    std::string out;
    out.reserve(300);
    out += "\n};\n template <> struct reflect::Reflect<";
    out += c.name;
    out += ">{\n";

    out += "constexpr static auto name = \"";
    out += c.name;
    out += "\";\n";

    out += "constexpr static auto enumerator_names = std::make_tuple(";
    for (auto& e : c.enumerators) {
      out += '\"';
      out += e;
      out += '\"';
      out += ',';
    }

    if (!c.enumerators.empty()) {
      out.pop_back();
    }

    out += ");\n";

    out += "constexpr static auto enumerator_constants = std::make_tuple(";
    for (auto& e : c.enumerators) {
      out += c.name;
      out += "::";
      out += e;
      out += ',';
    }

    if (!c.enumerators.empty()) {
      out.pop_back();
    }

    out += ");\n";

    out += "static constexpr auto object_type = reflect::ObjectType::ENUM;\n";

    out += "constexpr static bool is_scoped_enum = ";
    out += c.is_scoped() ? "true;\n" : "false;\n";

    out += "using underlying_type = ";
    for (auto& t : c.as) {
      out += t;
      out += "::";
    }
    if (!c.as.empty()) {
      out.pop_back();
      out.pop_back();
    }
    out += ';';
    out += '\n';

    out += "};";
    return out;
  }

  template <class Type, class V>
  bool is_(V v) {
    return std::holds_alternative<Type>(v);
  }

  template <class Type>
  auto generate_reflection() {
    auto& std_parser = parent.template get_parser<Parent::std_parser_id>();
    auto& current_type = std::get<Type>(std_parser.get_current_code_fragment());
    auto rez = generate_reflection(current_type);
    std_parser.template close_code_fragment<Type>();
    return rez;
  }

  // TODO: maybe use std::visit
  template <typename Iter>
  auto get_reflection(Iter begin) {
    auto& std_parser = parent.template get_parser<Parent::std_parser_id>();
    return is_<Class>(std_parser.get_current_code_fragment())
               ? std::optional{Result{begin, generate_reflection<Class>()}}
               : is_<Enumeration>(std_parser.get_current_code_fragment())
                     ? std::optional{Result{begin,
                                            generate_reflection<Enumeration>()}}
                     : std::nullopt;
  }

  template <typename Source>
  auto parse_end_of_scope(Source& source) {
    auto begin = source.begin();
    auto end = source.end();

    namespace x3 = boost::spirit::x3;
    bool parsed = x3::parse(begin, end, rules::scope_end);

    return parsed ? std::optional{begin} : std::nullopt;
  }

  template <typename Source>
  auto parse_reflexpr(Source& source) {
    auto begin = source.begin();
    auto end = source.end();

    namespace x3 = boost::spirit::x3;
    bool parsed = x3::parse(begin, end, rules::reflexpr);
    return parsed ? std::optional{begin} : std::nullopt;
  }

  template <typename Source>
  auto parse_end_reflexpr(Source& source) {
    auto begin = source.begin();
    auto end = source.end();

    namespace x3 = boost::spirit::x3;
    bool parsed = x3::parse(begin, end, rules::optionaly_space >> ')');
    return parsed ? std::optional{begin} : std::nullopt;
  }

  template <typename Iter>
  auto process_reflexpr(Iter it) {
    auto& std_parser = parent.template get_parser<Parent::std_parser_id>();
    auto& current =
        std::get<Expression>(std_parser.get_current_code_fragment());
    std_parser.open_new_code_fragment(RoundExpression{});
    in_reflexpr = true;
    return std::optional{Result{it, std::string{"reflexpr<"}}};
  }

  template <typename Iter>
  auto close_reflexpr(Iter it) {
    auto& std_parser = parent.template get_parser<Parent::std_parser_id>();
    std_parser.template close_code_fragment<RoundExpression>();
    in_reflexpr = false;
    return std::optional{Result{it, std::string{">"}}};
  }

  // DATA members
  Parent& parent;

  bool in_reflexpr = false;

 public:
  // TODO: when supported in std=c++2a change to fixed length string
  constexpr static int id = 5;

  StaticReflexParser(Parent& p) : parent{p} {}

  /**
   * A string to prepend to each file's start
   */
  std::string get_prepend() { return "#include<reflect.hpp>\n"; }

  template <class Source>
  using Out = std::optional<
      Result<decltype(std::declval<Source>().begin()), std::string>>;

  template <class Source>
  Out<Source> parse(Source& source) {
    auto& std_parser = parent.template get_parser<Parent::std_parser_id>();
    if (std_parser.template is_current_code_fragment<Class, Enumeration>()) {
      auto end_of_scope = parse_end_of_scope(source);
      return end_of_scope ? get_reflection(*end_of_scope) : std::nullopt;
    } else if (std_parser.template is_current_code_fragment<Expression>()) {
      auto begin_of_reflexpr = parse_reflexpr(source);
      return begin_of_reflexpr ? process_reflexpr(*begin_of_reflexpr)
                               : std::nullopt;
    } else if (in_reflexpr &&
               std_parser
                   .template is_current_code_fragment<RoundExpression>()) {
      auto end_of_reflexpr = parse_end_reflexpr(source);
      return end_of_reflexpr ? close_reflexpr(*end_of_reflexpr) : std::nullopt;
    }

    return std::nullopt;
  }
};
}  // namespace static_reflection

#endif  // STATIC_REFLECTION_H
