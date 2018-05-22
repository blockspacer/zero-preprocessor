#ifndef GEN_UTILS_H
#define GEN_UTILS_H

#include <algorithm>
#include <iostream>
#include <iterator>

#include <std_ast.h>

#include <meta_classes_rules.h>
#include <meta_process.h>

namespace meta_classes {
// TODO: replace with a range when we have them
template <class Iter>
auto gen_target_output(rules::ast::Target& t, Iter begin, Iter end) {
  std::string out;
  out.reserve(100);
  out += t.target;

  auto& outputs = t.output;
  outputs.erase(std::remove_if(outputs.begin(), outputs.end(),
                               [](auto& s) { return s.empty(); }),
                outputs.end());

  // move begin to the closing bracket of ->(target)
  begin = std::find(begin, end, ')');

  if (auto start_bracket = std::find(begin, end, '{'); start_bracket != end) {
    begin = start_bracket + 1;
    for (auto& s : outputs) {
      auto it = std::search(begin, end, s.begin(), s.end());
      out += " << \"";
      // TODO: maybe we need to escape " characters or use raw string
      out.append(begin, it);

      out += "\" << ";
      out += s;
      begin = std::find(it + s.size(), end, '$') + 1;
    }

    auto rbeg = std::make_reverse_iterator(begin);
    auto rend = std::make_reverse_iterator(end);
    auto it = std::find(rend, rbeg, '}');
    auto dist = std::distance(rend, it) + 1;
    out += " << \"";
    // TODO: maybe we need to escape " characters or use raw string
    out.append(begin, end - dist);
    out += "\";";
  } else {
    out += " << ";
    if (outputs.size() != 1) {
       throw std::runtime_error("error in meta class parser");
    }
    out += outputs.front();
    out += ';';
  }

  return out;
}

template <class Container>
std::string gen_main(Container& meta_classes) {
  std::string out;
  out.reserve(100);
  out +=
      "static const std::unordered_map<std::string, void(*)(meta::type,const "
      "meta::type)> funs {\n";
  for (auto& meta_class : meta_classes) {
    out += "{\"";
    out += meta_class;
    out += "\", &";
    out += meta_class;
    out += "},";
  }

  out.pop_back();
  out += "};";

  out += R"main(
int main(int argc, char* argv[]) {
  int mode;
  while (true) {
    std::cin >> mode;
    switch (mode) {
      case 1: {
        std::cout << funs.size();
        for (auto& kv : funs) {
          std::cout << kv.first << '\n';
        }
        break;
      }
      case 2: {
        std::string fun;
        std::cin >> fun;
        auto type = meta::read_type();
        auto const t2 = type;
        auto f = funs.at(fun);
        f(type, t2);
        std::string output = type.get_representation();
        std::cout << output.size() << std::endl;
        std::cout << output << std::endl;
        break;
      }
      case 3:
        return 0;
      default:
        std::cout << "unknown mode " << mode;
    }
  }

  return 0;
}
)main";
  return out;
}

template <typename Writer>
void write_type(std_parser::rules::ast::Type const& type, Writer& writer) {
  std::string type_out;
  type_out.reserve(30);
  // TODO: add templates
  for (auto& t : type.name) {
    type_out += t;
    type_out += "::";
  }
  type_out.pop_back();
  type_out.pop_back();

  writer << type_out << std::endl;
}

template <typename Writer>
void write_methods(std::vector<std_parser::rules::ast::Function> const& methods,
                   Writer& writer) {
  for (auto& m : methods) {
    std::cout << m.name << '\n';
    auto& type = m.return_type;
    write_type(type, writer);

    writer << m.name << std::endl;
    auto& params = m.parameters.parameters;
    writer << params.size() << std::endl;

    for (auto& p : params) {
      write_type(p.type, writer);
      writer << p.name << std::endl;
    }
  }
}

std::string gen_meta_class(MetaProcess& process,
                           const std::string_view meta_class,
                           std_parser::rules::ast::Class& cls) {
  std::cout << "getting output from meta process\n";
  process.output << 2 << std::endl;
  process.output << meta_class << std::endl;

  process.output << cls.name << std::endl;
  process.output << (cls.public_methods.size() + cls.private_methods.size() +
                     cls.protected_methods.size())
                 << std::endl;

  std::cout << "num methods "
            << (cls.public_methods.size() + cls.private_methods.size() +
                cls.protected_methods.size())
            << "\n";
  std::cout << "public methods\n";
  write_methods(cls.public_methods, process.output);

  std::cout << "private methods\n";
  write_methods(cls.private_methods, process.output);

  std::cout << "protected methods\n";
  write_methods(cls.protected_methods, process.output);

  std::cout << "num data members "
            << (cls.public_members.size() + cls.private_members.size() +
                cls.protected_members.size())
            << "\n";

  std::cout << "public methods\n";
  for (auto& m : cls.public_members) {
    std::cout << m.name << '\n';
  }

  std::cout << "public methods\n";
  for (auto& m : cls.private_members) {
    std::cout << m.name << '\n';
  }

  std::string output, line;
  std::size_t output_size;
  std::cout << "reading output...";
  process.input >> output_size;
  std::cout << "output lenght should be: " << output_size << '\n';
  while (output.size() < output_size) {
    std::cout << "reading output...";
    std::getline(process.input, line);
    output += line;
    output += '\n';
    std::cout << "read line: " << line;
  }
  std::cout << "result for meta class:\n" << output;

  return output;
}
}  // namespace meta_classes

#endif  //! GEN_UTILS_H
