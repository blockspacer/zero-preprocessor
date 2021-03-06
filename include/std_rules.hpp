#ifndef STD_RULES_H
#define STD_RULES_H

#include <string>

#include <std_ast.hpp>

#include <boost/spirit/home/x3.hpp>

namespace std_parser::rules {
namespace x3 = boost::spirit::x3;

using x3::alpha;
using x3::char_;
using x3::digit;
using x3::eol;
using x3::lit;

static struct class_type_ : x3::symbols<ast::class_type> {
  class_type_() {
    add("class", ast::class_type::CLASS)("struct", ast::class_type::STRUCT);
  }
} class_type;

static struct access_modifier_ : x3::symbols<ast::access_modifier> {
  access_modifier_() {
    add("public", ast::access_modifier::PUBLIC)(
        "protected", ast::access_modifier::PROTECTED)(
        "private", ast::access_modifier::PRIVATE);
  }
} access_modifier;

static struct is_constructor_ : x3::symbols<ast::Constructor> {
  is_constructor_() { add("~", ast::Constructor::DESTRUCTOR); }
} is_constructor;

static struct enum_class_ : x3::symbols<ast::EnumType> {
  enum_class_() { add("class", ast::EnumType::ENUM_CLASS); }
} enum_class;

static struct type_qualifier_ : x3::symbols<ast::TypeQualifier> {
  type_qualifier_() {
    add("constexpr", ast::TypeQualifier::Constexpr)(
        "const", ast::TypeQualifier::Const)("&&", ast::TypeQualifier::R_Ref)(
        "&", ast::TypeQualifier::L_Ref)("*", ast::TypeQualifier::Pointer);
  }
} type_qualifier;

static struct method_qualifier_ : x3::symbols<ast::MethodQualifier> {
  method_qualifier_() {
    add("&&", ast::MethodQualifier::R_REF)("&", ast::MethodQualifier::L_REF);
  }
} method_qualifier;

static struct binary_operator_ : x3::symbols<ast::Operator> {
  binary_operator_() {
    add("+=", ast::Operator::PlusEq)("+", ast::Operator::Plus)(
        "-=", ast::Operator::MinusEq)("->*", ast::Operator::ArrowDeref)(
        "->", ast::Operator::Arrow)("-", ast::Operator::Minus)(
        ".*", ast::Operator::DotDeref)(".", ast::Operator::Dot)(
        "*=", ast::Operator::MultiplyEq)("*", ast::Operator::Multiply)(
        "/=", ast::Operator::DivideEq)("/", ast::Operator::Divide)(
        "%=", ast::Operator::ModuleEq)("%", ast::Operator::Module)(
        ">>=", ast::Operator::RShiftEq)(">>", ast::Operator::RShift)(
        ">=", ast::Operator::GtEq)(">", ast::Operator::Gt)(
        "<<=", ast::Operator::LShiftEq)("<<", ast::Operator::LShift)(
        "<=", ast::Operator::LtEq)("<", ast::Operator::Lt)(
        "&&", ast::Operator::And)("&=", ast::Operator::BitAndEq)(
        "&", ast::Operator::BitAnd)("||", ast::Operator::Or)(
        "|=", ast::Operator::BitOrEq)("|", ast::Operator::BitOr)(
        "~=", ast::Operator::TildeEq)("~", ast::Operator::Tilde)(
        "^=", ast::Operator::BitXorEq)("^", ast::Operator::BitXor)(
        "!=", ast::Operator::NotEq)("!", ast::Operator::Not)(
        "==", ast::Operator::EqEq)("=", ast::Operator::Eq);
  }
} binary_operator;

// TODO: replace with x3::matches[p]
static auto bool_attr = [](auto p) {
  return (x3::omit[p] >> x3::attr(true)) | x3::attr(false);
};

x3::rule<class some_space> const some_space = "some_space";
auto const some_space_def = +(eol | ' ' | '\t');

x3::rule<class optionaly_space> const optionaly_space = "optionaly_space";
auto const optionaly_space_def = *(eol | ' ' | '\t');

x3::rule<class include, std::string> const include = "include";
auto const include_def = optionaly_space >> '#' >> *(lit(' ') | '\t') >>
                         lit("include") >> optionaly_space >>
                         (('<' >> *(char_ - '>') >> '>') |
                          ('"' >> *(char_ - '"') >> '"'));

x3::rule<class skip_line> const skip_line = "skip_line";
auto const skip_line_def = *(char_ - eol);

x3::rule<class comment> const comment = "comment";
auto const comment_def =
    optionaly_space >>
    (("//" >> *(char_ - eol)) |
     ("/*" >> x3::repeat[+(char_ - '*') | (lit('*') >> (char_ - '/'))] >>
      "*/"));

x3::rule<class class_access_modifier, ast::access_modifier> const
    class_access_modifier = "class_access_modifier";
auto const class_access_modifier_def =
    access_modifier >> optionaly_space >> ':';

x3::rule<class arg_separator> const arg_separator = "arg_separator";
auto const arg_separator_def = optionaly_space >> ',' >> optionaly_space;

// TODO: emit an enum
x3::rule<class prefix_operator> const prefix_operator = "prefix_operator";
auto const prefix_operator_def = lit("++") | "--" | '*' | '&' | '!';

x3::rule<class sufix_operator> const sufix_operator = "sufix_operator";
auto const sufix_operator_def = lit("++") | "--";

x3::rule<class call_operator> const call_operator = "call_operator";
auto const call_operator_def =
    (lit('(') >> optionaly_space >> ')') | (lit('[') >> optionaly_space >> ']');

x3::rule<class all_overloadable_operators> const all_overloadable_operators =
    "all_overloadable_operators";
auto const all_overloadable_operators_def =
    lit("+=") | "++" | '+' | "-=" | "->*" | "->" | "--" | '-' | "*=" | '*' |
    "/=" | '/' | "%=" | '%' | ">>=" | ">>" | ">=" | '>' | "<<=" | "<<" | "<=" |
    '<' | "&&" | "&=" | '&' | "||" | "|=" | '|' | "~=" | '~' | "^=" | '^' |
    "!=" | '!' | "==" | '=' | call_operator;

x3::rule<class operator_sep_old> const operator_sep_old = "operator_sep_old";
auto const operator_sep_old_def = optionaly_space >>
                                  // TODO: this makes 1.*foo() or a->23 valid
                                  x3::omit[binary_operator] >> optionaly_space;

x3::rule<class operator_sep, ast::Operator> const operator_sep = "operator_sep";
auto const operator_sep_def = optionaly_space >>
                              (binary_operator |
                               (lit(',') >> x3::attr(ast::Operator::Comma))) >>
                              optionaly_space;

x3::rule<class scope_begin> const scope_begin = "scope_begin";
auto const scope_begin_def = optionaly_space >> '{';

x3::rule<class scope_end> const scope_end = "scope_end";
auto const scope_end_def = optionaly_space >> '}' >> optionaly_space >>
                           -lit(';');

x3::rule<class statement_end> const statement_end = "statement_end";
auto const statement_end_def = optionaly_space >> ';';

x3::rule<class name, std::string> const name = "name";
auto const name_def = ((alpha | char_('_')) >> *(alpha | digit | char_('_')));

// TODO: add nested namespaces c++17
x3::rule<class namespace_begin, std::string> const namespace_begin =
    "namespace_begin";
auto const namespace_begin_def =
    lit("namespace") >> some_space >> name >> scope_begin;

// TODO: move the - sign in expressions not in digits
x3::rule<class digits> const digits = "digits";
auto const digits_def = -lit('-') >> optionaly_space >> +digit >>
                        *(lit('\'') >> x3::omit[+digit]);

x3::rule<class integral, ast::UnqulifiedType> const integral = "integral";
auto const integral_def =
    digits >>
    (((lit("LLU") | "LLu" | "llU" | "llu") >>
      x3::attr(ast::UnqulifiedType{{"unsigned long long int"}, {}})) |
     ((lit("LU") | "lU" | "Lu" | "lu") >>
      x3::attr(ast::UnqulifiedType{{"unsigned long int"}, {}})) |
     ((lit("LL") | "ll") >>
      x3::attr(ast::UnqulifiedType{{"long long int"}, {}})) |
     ((lit('U') | 'u') >> x3::attr(ast::UnqulifiedType{{"unsigned int"}, {}})) |
     ((lit('L') | 'l') >> x3::attr(ast::UnqulifiedType{{"long int"}, {}})) |
     x3::attr(ast::UnqulifiedType{{"int"}, {}}));

x3::rule<class floating, ast::UnqulifiedType> const floating = "floating";
auto const floating_def =
    ((digits >> '.' >> -digits) | (lit('.') >> digits)) >>
    (((lit('f') | 'F') >> x3::attr(ast::UnqulifiedType{{"float"}, {}})) |
     ((lit('l') | 'L') >> x3::attr(ast::UnqulifiedType{{"long double"}, {}})) |
     // if not specified default to double
     x3::attr(ast::UnqulifiedType{{"double"}, {}}));

x3::rule<class number, ast::UnqulifiedType> const number = "number";
auto const number_def = floating | integral;

// TODO: add support for escaped /" inside string
x3::rule<class quoted_string> const quoted_string = "quoted_string";
auto const quoted_string_def = lit('"') >> *(char_ - '"') >> '"';

x3::rule<class string_literal, ast::UnqulifiedType> const string_literal =
    "string_literal";
auto const string_literal_def = x3::omit[quoted_string % optionaly_space] >>
                                // TODO: need to emit the string value
                                x3::attr(ast::UnqulifiedType{{"char[]"}, {}});

// TODO: is this ok?
x3::rule<class char_literal, ast::UnqulifiedType> const char_literal =
    "char_literal";
auto const char_literal_def = x3::omit[lit('\'') >> (char_ - '\'') >> '\''] >>
                              x3::attr(ast::UnqulifiedType{{"char"}, {}});

x3::rule<class type_, ast::Type_> const type_ = "type_";
auto const type__def = name >> *(lit("::") >> name);

x3::rule<class type_qualifiers, std::vector<ast::TypeQualifier>> const
    type_qualifiers = "type_qualifiers";
// NOTE: !name is to not parse const from start of a name e.g. int const_name
auto const type_qualifiers_def = type_qualifier % optionaly_space >> !name;

x3::rule<class var_type, ast::UnqulifiedType> const var_type = "var_type";
x3::rule<class type, ast::Type> const type = "type";
x3::rule<class template_values, ast::TemplateTypes> const template_values =
    "template_values";

auto const var_type_def = (type_) >> -(optionaly_space >> template_values);

auto const template_values_def = '<' >> optionaly_space >>
                                 ((type | digits) % arg_separator) >> '>';

auto const type_def = -(type_qualifiers >> some_space) >> var_type >>
                      -(optionaly_space >> type_qualifiers);

// TODO: delete most of this with the new expression refactor
x3::rule<class argument> const argument = "argument";
x3::rule<class optionaly_arguments> const optionaly_arguments =
    "optionaly_arguments";
x3::rule<class function_call> const function_call = "function_call";
x3::rule<class expression_old> const expression_old = "expression_old";
x3::rule<class paren_expression_old> const paren_expression_old =
    "paren_expression_old";
x3::rule<class optionaly_paren_expression_old> const
    optionaly_paren_expression_old = "optionaly_paren_expression_old";
x3::rule<class init_list> const init_list = "init_list";
x3::rule<class arg_init_list> const arg_init_list = "arg_init_list";

// TODO: type here denotes a variable name
// change it to variable_type that also covers ::var
auto const argument_def = arg_init_list | function_call | var_type |
                          x3::omit[number] | x3::omit[char_literal] |
                          x3::omit[string_literal] | paren_expression_old;

auto const optionaly_arguments_def =
    -((expression_old | init_list) % arg_separator);

auto const function_call_def = type >> optionaly_space >> '(' >>
                               optionaly_space >> optionaly_arguments >>
                               optionaly_space >> ')';
// TODO: this allows prefix ++ on rvalues
auto const expression_old_def =
    (-(prefix_operator >> optionaly_space) >> argument >>
     -(optionaly_space >> sufix_operator)) %
    operator_sep_old;
auto const paren_expression_old_def =
    '(' >> optionaly_space >> expression_old >> optionaly_space >> ')';
auto const optionaly_paren_expression_old_def =
    '(' >> optionaly_space >> -expression_old >> optionaly_space >> ')';
auto const init_list_def =
    '{' >> optionaly_space >> optionaly_arguments >> optionaly_space >> '}';

auto const arg_init_list_def = type >> optionaly_space >> init_list;

x3::rule<class param, ast::var> const param = "param";
auto const param_def = type >> some_space >> name;

x3::rule<class optional_param, ast::var> const optional_param =
    "optional_param";
auto const optional_param_def = type >> -(some_space >> name);

x3::rule<class param_optionaly_default, ast::var> const
    param_optionaly_default = "param_optionaly_default";
auto const param_optionaly_default_def = optional_param >>
                                         -(optionaly_space >> '=' >>
                                           optionaly_space >>
                                           (expression_old | init_list));

x3::rule<class optionaly_params, ast::params> const optionaly_params =
    "optionaly_params";
auto const optionaly_params_def = -(param_optionaly_default % arg_separator);

x3::rule<class is_noexcept, bool> const is_noexcept = "is_noexcept";
auto const is_noexcept_def = bool_attr(
    optionaly_space >> "noexcept" >> optionaly_space >>
    -('(' >> optionaly_space >> expression_old >> optionaly_space >> ')'));

// new expressions
// TODO: remove this
x3::rule<class type_or_name, ast::UnqulifiedType> const type_or_name =
    "type_or_name";
auto const type_or_name_def = var_type;

// TODO: add a literal to the Expression
x3::rule<class literal, ast::LiteralExpression> const literal = "literal";
auto const literal_def = number | char_literal | string_literal;

x3::rule<class parenthesis_begin, ast::RoundExpression> const
    parenthesis_begin = "parenthesis_begin";
auto const parenthesis_begin_def = lit('(') >> x3::attr(ast::RoundExpression{});

x3::rule<class parenthesis_expr_begin, ast::RoundExpression> const
    parenthesis_expr_begin = "parenthesis_expr_begin";
auto const parenthesis_expr_begin_def =
    -(prefix_operator >> optionaly_space) >> parenthesis_begin;

x3::rule<class parenthesis_end> const parenthesis_end = "parenthesis_end";
auto const parenthesis_end_def = lit(')');

x3::rule<class curly_begin, ast::CurlyExpression> const curly_begin =
    "curly_begin";
auto const curly_begin_def = lit('{') >> x3::attr(ast::CurlyExpression{});

x3::rule<class curly_end> const curly_end = "curly_end";
auto const curly_end_def = lit('}');

x3::rule<class capture_default> const capture_default = "capture_default";
auto const capture_default_def = lit('=') |
                                 '&' >> &(optionaly_space >> (lit(',') | ']'));

x3::rule<class capture_params> const capture_params = "capture_params";
auto const capture_params_def = (-lit('&') >> name) % arg_separator;

x3::rule<class lambda_capture> const lambda_capture = "lambda_capture";
auto const lambda_capture_def = lit('[') >> optionaly_space >>
                                (']' | (((capture_default >> optionaly_space >>
                                          -(',' >> optionaly_space >>
                                            capture_params)) |
                                         capture_params) >>
                                        optionaly_space >> ']'));

x3::rule<class lambda_specifiers> const lambda_specifiers = "lambda_specifiers";
auto const lambda_specifiers_def = optionaly_space >> lit("mutable") |
                                   "constexpr";

x3::rule<class lambda, ast::Lambda> const lambda = "lambda";
auto const lambda_def = x3::omit[lambda_capture] >> optionaly_space >>
                        x3::omit[-('(' >> optionaly_space >> optionaly_params >>
                                   ')' >> -lambda_specifiers >> is_noexcept)] >>
                        optionaly_space >> '{' >>
                        x3::attr(ast::Lambda{ast::LambdaState::Body, {}, {}});

x3::rule<class variable_expression, ast::UnqulifiedType> const
    variable_expression = "variable_expression";
auto const variable_expression_def = -(prefix_operator >> optionaly_space) >>
                                     type_or_name >>
                                     -(optionaly_space >> sufix_operator);

x3::rule<class fce_expression, ast::ValueExpression> const fce_expression =
    "fce_expression";
auto const fce_expression_def = variable_expression >> optionaly_space >>
                                (parenthesis_begin | curly_begin |
                                 x3::attr(ast::VariableExpression{}));

x3::rule<class expression,
         std::variant<ast::VariableExpression, ast::LiteralExpression,
                      ast::RoundExpression, ast::CurlyExpression,
                      ast::Lambda>> const expression = "expression";
auto const expression_def =
    fce_expression | literal | lambda | parenthesis_expr_begin;

x3::rule<class return_statement,
         std::variant<ast::VariableExpression, ast::LiteralExpression,
                      ast::RoundExpression, ast::CurlyExpression,
                      ast::Lambda>> const return_statement = "return_statement";
auto const return_statement_def = "return" >> some_space >> expression;

// ============================================

x3::rule<class constructor_init> const constructor_init = "constructor_init";
auto const constructor_init_def = name >> optionaly_space >>
                                  (init_list | optionaly_paren_expression_old);

// TODO: support for new / delete ?
x3::rule<class var_old, ast::var> const var_old = "var_old";
auto const var_old_def = param >> optionaly_space >>
                         -(('=' >> optionaly_space >> expression_old) |
                           (-('=' >> optionaly_space) >> init_list)) >>
                         optionaly_space;

x3::rule<class var_with_init, ast::var> const var_with_init = "var_with_init";
auto const var_with_init_def = param >> optionaly_space >>
                               &(lit('=') | '{' | ';');

// TODO: support full for loop expressions?
x3::rule<class for_loop> const for_loop = "for_loop";
auto const for_loop_def =
    lit("for") >> optionaly_space >> '(' >> optionaly_space >>
    (((-var_old >> ';') >> optionaly_space >> -expression_old >>
      optionaly_space >> ';' >> optionaly_space >> -expression_old) |
     (param >> optionaly_space >> ':' >> optionaly_space >> expression_old)) >>
    optionaly_space >> ')';

x3::rule<class while_loop> const while_loop = "while_loop";
auto const while_loop_def =
    lit("while") >> optionaly_space >> '(' >> optionaly_space >>
    ((var_old >> &(optionaly_space >> ')')) | expression_old) >> optionaly_space
    >> ')';

x3::rule<class if_expression, ast::IfExpression> const if_expression =
    "if_expression_old";
auto const if_expression_def = lit("if") >> optionaly_space >>
                               -lit("constexpr") >> optionaly_space
                               >> x3::attr(ast::IfExpression{});

x3::rule<class else_expression> const else_expression = "else_expression";
auto const else_expression_def = lit("else");

// TODO: need variadic templates
x3::rule<class template_parameter, ast::TemplateParameter> const
    template_parameter = "template_parameter";

// TODO: default template parameters are omited
auto const template_parameter_def = type_ >> some_space >> name >>
                                    -(optionaly_space >> '=' >>
                                      optionaly_space >>
                                      x3::omit[(type | number)]);

x3::rule<class template_parameters, ast::TemplateParameters> const
    template_parameters = "template_parameters";
auto const template_parameters_def =
    lit("template") >> optionaly_space >> '<' >> optionaly_space >>
    template_parameter % arg_separator >> optionaly_space >> '>';

x3::rule<class function_signiture, ast::function_signiture> const
    function_signiture = "function_signiture";
auto const function_signiture_def =
    -(template_parameters >> optionaly_space) >>
    bool_attr(lit("constexpr") >> some_space) >> type >> some_space >> name
    >> optionaly_space >> '(' >> optionaly_space >> optionaly_params >>
    ')' >> is_noexcept;

x3::rule<class function_start, ast::function_signiture> const function_start =
    "function_start";
auto const function_start_def = function_signiture >> scope_begin;

x3::rule<class is_pure_virtual, bool> const is_pure_virtual = "is_pure_virtual";
auto const is_pure_virtual_def =
    bool_attr(optionaly_space >> '=' >> optionaly_space >> '0');

x3::rule<class method_signiture, ast::method_signiture> const method_signiture =
    "method_signiture";
// TODO: this allows for both template and virtual, but the real compiler will
// handle it; maybe separate into two definitions and do an | on them
auto const method_signiture_def =
    -(template_parameters >> optionaly_space) >>
    bool_attr(lit("constexpr") >> some_space) >>
    bool_attr(lit("virtual") >> some_space) >> type >> some_space >> name
    >> optionaly_space >> '(' >> optionaly_space >> optionaly_params >>
    ')' >> optionaly_space >> bool_attr(lit("const")) >> optionaly_space >>
    -(method_qualifier >> optionaly_space) >> is_noexcept >>
    bool_attr(lit("override")) >> is_pure_virtual;

// TODO: const operators and &&
x3::rule<class operator_signiture, ast::operator_signiture> const
    operator_signiture = "operator_signiture";
auto const operator_signiture_def = -(template_parameters >> optionaly_space) >>
                                    bool_attr(lit("constexpr") >> some_space) >>
                                    bool_attr(lit("virtual")) >> optionaly_space
                                    >> type >> some_space >>
                                    "operator" >> optionaly_space
                                    >> all_overloadable_operators
                                    >> optionaly_space >> '(' >> optionaly_space
                                    >> optionaly_params >> ')' >> is_noexcept
                                    >> is_pure_virtual;

// TODO: can be both virtual and have : initialization
x3::rule<class constructor, ast::constructor> const constructor = "constructor";
auto const constructor_def = -(template_parameters >> optionaly_space) >>
                             bool_attr(lit("constexpr") >> some_space) >>
                             bool_attr(lit("virtual") >> some_space) >>
                             -(is_constructor >> optionaly_space) >> name
                             >> optionaly_space >> '(' >> optionaly_space
                             >> optionaly_params >> ')' >> is_noexcept
                             >> is_pure_virtual >>
                             -(optionaly_space >> ':' >> optionaly_space >>
                               constructor_init % arg_separator);

x3::rule<class class_inheritance, ast::class_inheritance> const
    class_inheritance = "class_inheritance";
auto const class_inheritance_def = -(access_modifier >> some_space) >> var_type;

x3::rule<class class_inheritances, ast::class_bases> const class_inheritances =
    "class_inheritances";
auto const class_inheritances_def =
    ':' >> optionaly_space >> class_inheritance % arg_separator;

// TODO: name of class can be namespace::name for forward declaration
x3::rule<class class_or_struct, ast::class_or_struct> const class_or_struct =
    "class_or_struct";
auto const class_or_struct_def = -(template_parameters >> optionaly_space) >>
                                 class_type >> some_space >> name >>
                                 -(optionaly_space >> class_inheritances);

x3::rule<class enumeration, ast::enum_> const enumeration = "enumeration";
auto const enumeration_def = lit("enum") >>
                             -(optionaly_space >> enum_class) >> some_space
                             >> name >> -(optionaly_space >> ':' >>
                                          optionaly_space >> type_);

x3::rule<class enumerators, std::vector<std::string>> const enumerators =
    "enumerators";
auto const enumerators_def = name % arg_separator;

BOOST_SPIRIT_DEFINE(
    some_space, optionaly_space, include, skip_line, comment, arg_separator,
    class_access_modifier, prefix_operator, sufix_operator,
    all_overloadable_operators, operator_sep_old, operator_sep, call_operator,
    scope_begin, scope_end, namespace_begin, statement_end, name, type_,
    type_qualifiers, type, var_type, template_values, digits, integral,
    floating, number, quoted_string, string_literal, char_literal, argument,
    optionaly_arguments, function_call, expression_old, paren_expression_old,
    optionaly_paren_expression_old, init_list, arg_init_list, optionaly_params,
    return_statement, type_or_name, literal, parenthesis_begin,
    parenthesis_expr_begin, parenthesis_end, curly_begin, curly_end, expression,
    capture_default, capture_params, lambda_capture, lambda_specifiers, lambda,
    param, optional_param, param_optionaly_default, var_old, var_with_init,
    constructor_init, for_loop, while_loop, if_expression, else_expression);

BOOST_SPIRIT_DEFINE(template_parameter, template_parameters, is_noexcept,
                    function_signiture, function_start, is_pure_virtual,
                    method_signiture, operator_signiture, constructor,
                    class_inheritance, class_inheritances, class_or_struct,
                    enumeration, enumerators, variable_expression,
                    fce_expression);
}  // namespace std_parser::rules

#endif  //! STD_RULES_H
