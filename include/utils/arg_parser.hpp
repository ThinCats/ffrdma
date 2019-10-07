#ifndef ARGPARSER_HPP
#define ARGPARSER_HPP
#include "assert.h"
#include "thlib.hpp"
#include <iostream>
#include <map>
#include <string>
#include <vector>

THLIB_NAMESPACE_BEGIN
namespace thlib {
enum class ArgType { T_FLOAT, T_BOOL, T_INT, T_STRING };

class ArgData {

public:
  ArgData(ArgType t) : type(t) {}
  ArgData(const std::string &str) {
    type = ArgType::T_STRING;
    base_data.string_p = new std::string(str);
    isRaw = false;
  }

  ArgData(int num) {
    type = ArgType::T_INT;
    base_data.num_i = num;
    isRaw = false;
  }

  ArgData(double num) {
    type = ArgType::T_FLOAT;
    base_data.num_d = num;
    isRaw = false;
  }

  ArgData(bool e) {
    type = ArgType::T_BOOL;
    base_data.bool_e = e;
    isRaw = false;
  }

  ArgData(ArgData &&data) { *this = std::move(data); }
  ArgData(const ArgData &data) { *this = data; }

  ~ArgData() { clear(); }

  ArgData &operator=(ArgData &&data) {
    if (this != &data) {
      clear();
      isRaw = data.isRaw;
      type = data.type;
      base_data = data.base_data;
      data.base_data.string_p = nullptr;
    }
    return *this;
  }

  ArgData &operator=(const ArgData &data) {
    clear();
    isRaw = data.isRaw;
    type = data.type;
    if (data.type == ArgType::T_STRING && data.base_data.string_p != nullptr) {
      base_data.string_p = new std::string(*data.base_data.string_p);
    } else {
      base_data = data.base_data;
    }
    return *this;
  }

  void clear() {
    switch (type) {
    case ArgType::T_STRING:
      if (base_data.string_p != nullptr) {
        delete base_data.string_p;
        base_data.string_p = nullptr;
      }
      break;
    default:
      break;
    }
    isRaw = true;
  }

  bool isDefined() const { return !isRaw; }
  ArgType getType() const { return type; }

  const std::string &getStr() const {
    assert(type == ArgType::T_STRING && base_data.string_p != nullptr &&
           !isRaw);
    return *base_data.string_p;
  }

  int getInt() const {
    assert(type == ArgType::T_INT && !isRaw);
    return base_data.num_i;
  }

  double getDouble() const {
    assert(type == ArgType::T_FLOAT && !isRaw);
    return base_data.num_d;
  }

  bool getBool() const {
    assert(type == ArgType::T_BOOL && !isRaw);
    return base_data.bool_e;
  }

  void setStr(const std::string &str) {
    clear();
    type = ArgType::T_STRING;
    base_data.string_p = new std::string(str);
    isRaw = false;
  }

  void setInt(int num) {
    clear();
    type = ArgType::T_INT;
    base_data.num_i = num;
    isRaw = false;
  }

  void setDouble(double num) {
    clear();
    type = ArgType::T_FLOAT;
    base_data.num_d = num;
    isRaw = false;
  }

  void setBool(bool e) {
    clear();
    type = ArgType::T_BOOL;
    base_data.bool_e = e;
    isRaw = false;
  }

  // set base_data to default one;
  void setDefault() {
    assert(isRaw);
    switch (type) {
    case ArgType::T_BOOL:
      base_data.bool_e = false;
      break;
    case ArgType::T_STRING:
      base_data.string_p = new std::string("");
      break;
    case ArgType::T_FLOAT:
      base_data.num_d = 0;
      break;
    case ArgType::T_INT:
      base_data.num_i = 0;
      break;
    }
    isRaw = false;
  }

private:
  union {
    std::string *string_p = nullptr;
    int num_i;
    double num_d;
    bool bool_e;
  } base_data;
  ArgType type = ArgType::T_INT;
  // whether has been initialized
  bool isRaw = true;
};

// Created by parse_args()
// contains parsed arg value
class ArgObj {
public:
  ArgObj() {}
  ~ArgObj() {}

  /**
   * @brief Get the Last Parsing Index object
   *  the lastParsingIndex is the index parser stops
   *  argv[lastParsingIndex:] is not parsed
   * @return int
   */
  int getLastParsingIndex() const { return m_last_index; }
  void setLastParsingIndex(int index) { m_last_index = index; }

  const ArgData &getArgData(const std::string &arg) const {
    try {
      return arg_items.at(arg);
    } catch (std::out_of_range &) {
      std::cerr << "No Argument Item in ArgObj: " << arg << std::endl;
      std::exit(-1);
    }
  }

  // return whther the option is opened
  bool isOptionOn(const std::string &arg) const {
    return getArgData(arg).getBool();
  }

  // Get argument's string
  const std::string &getValStr(const std::string &arg) {
    return getArgData(arg).getStr();
  }

  int getValInt(const std::string &arg) { return getArgData(arg).getInt(); }

  double getValDouble(const std::string &arg) {
    return getArgData(arg).getDouble();
  }

  void emplace(const std::string &arg_name, const ArgData &arg_data) {
    arg_items.emplace(arg_name, arg_data);
  }

private:
  std::map<std::string, ArgData> arg_items;
  int m_last_index = 0;
};

class ArgParser;
// An argument instance for argparser to save
struct Argument {
public:
  friend class ArgParser;
  Argument(const std::string &l_arg, ArgType type, bool is_mst,
           const std::string &defa, const std::string &s_arg,
           const std::string &help)
      : longArg(l_arg), argValue(type), isMust(is_mst), defaultVal(defa),
        shortArg(s_arg), helpMessage(help) {}
  ~Argument() {}

  bool isMustArg() const { return isMust; }

  // Check whether arg is defined
  bool isDefined() const { return argValue.isDefined(); }

  ArgType getType() const { return argValue.getType(); }

private:
  // long args like --lexer, --test
  std::string longArg = "";
  ArgData argValue;
  bool isMust;
  std::string defaultVal = "";
  // short args like "-l, -t..."
  std::string shortArg = "";
  // for help print
  std::string helpMessage = "";
};

class ArgParser {
public:
  ArgParser(const std::string &des = "", const std::string &helpCmd = "")
      : description(des) {
    // Add help node
    if (helpCmd == "") {
      this->add_argument("help", ArgType::T_BOOL, false, "h", "Print Help");
      this->m_helpCmd = "help";
    } else {
      // add user spcified help command
      this->add_argument(helpCmd, ArgType::T_BOOL, false, "", "Print Help");
      this->m_helpCmd = helpCmd;
    }
    // help is set false by defaut
    argumentContainer[argsMap.at(m_helpCmd)].argValue.setBool(false);
  }
  ~ArgParser() {}

  // no default value and short
  void add_argument(const std::string &long_arg, ArgType type, bool isMust,
                    const std::string &help_msg) {
    add_argument(long_arg, type, isMust, "", "", help_msg);
  }

  // no default value
  void add_argument(const std::string &long_arg, ArgType type, bool isMust,
                    const std::string &short_arg, const std::string &help_msg) {
    add_argument(long_arg, type, isMust, "", short_arg, help_msg);
  }

  // --lexer --out-file a.c -l ""
  void add_argument(const std::string &long_arg, ArgType type, bool isMust,
                    const std::string &defaultValue,
                    const std::string &short_arg, const std::string &help_msg) {
    // Add it to args Map
    argumentContainer.emplace_back(long_arg, type, isMust, defaultValue,
                                   short_arg, help_msg);

    auto pair_l = argsMap.emplace(long_arg, argumentContainer.size() - 1);
    if (pair_l.second == false) {
      // Duplicated argument
      exitProgram("Duplicated Arguments");
    }

    // If short exists
    if (short_arg != "") {
      auto pair_s = argsMap.emplace(short_arg, argumentContainer.size() - 1);
      if (pair_s.second == false) {
        // Duplicated argument
        exitProgram("Duplicated Arguments");
      }
    }
  }

  // Will create one object each call
  ArgObj parse_args(int argc, char **args) {
    // program name
    programName = std::string(*args);
    // Save the previous arg, if not option
    size_t prev_arg = std::string::npos;
    char *arg;
    int last_index;
    // "--lexer --option"
    // if encounter --, stop parsing (return last_index)
    try {
      for (last_index = 1; last_index < argc; ++last_index) {
        arg = args[last_index];
        if (*arg == '-') {
          arg++;
          // long argument
          if (*arg == '-') {
            arg++;
            // just --, end parsing
            if (*(arg) == '\0' || isspace(*(arg))) {
              last_index++;
              break;
            }
            // --lexer
            prev_arg = argsMap.at(std::string(arg));
            if (argumentContainer[prev_arg].getType() == ArgType::T_BOOL) {
              argumentContainer[prev_arg].argValue.setBool(true);
            }
          }
          // short argument
          else {
            prev_arg = argsMap.at(std::string(arg));
            if (argumentContainer[prev_arg].getType() == ArgType::T_BOOL) {
              argumentContainer[prev_arg].argValue.setBool(true);
            }
          }
          // set default, if option is open and isn't must
          auto &argument = argumentContainer[prev_arg];
          if (!argument.isMustArg() && argument.defaultVal != "")
            strToArgData(std::string(argument.defaultVal), argument.argValue);

        } else {
          if (prev_arg == std::string::npos)
            exitProgram("Error Position of argument value");
          // the value, like --out-file a.c
          auto &argValue = argumentContainer[prev_arg].argValue;
          strToArgData(std::string(arg), argValue);
          prev_arg = std::string::npos;
        }
      }
    } catch (std::out_of_range &t) {
      exitProgram("Unknown Parsed Arguments " + std::string(t.what()));
    } catch (std::invalid_argument &t) {
      exitProgram("Unknown Parsed Arguments t" + std::string(t.what()));
    }

    // Args not valid
    if (!checkArgsValid()) {
      std::exit(-1);
    }

    ArgObj argObj;
    argObj.setLastParsingIndex(last_index);
    // Create arg obj
    for (auto &arg : argumentContainer) {
      if (!arg.isDefined()) {
        // havn't have value, set to default
        arg.argValue.setDefault();
      }
      argObj.emplace(arg.longArg, arg.argValue);
    }
    return argObj;
  }

  // parse string, and set value in arg data cores to type
  void strToArgData(const std::string &value, ArgData &data) {
    switch (data.getType()) {
    case ArgType::T_BOOL:
      // ignore, always false
      data.setBool(false);
      break;
    case ArgType::T_STRING:
      data.setStr(std::string(value));
      break;
    case ArgType::T_FLOAT:
      data.setDouble(std::stod(value));
      break;
    case ArgType::T_INT:
      data.setInt(std::stoi(value));
      break;
    }
  }

  bool checkArgsValid() const {
    // help is set, print and exit
    if (argumentContainer[argsMap.at(this->m_helpCmd)].argValue.getBool() ==
        true) {
      printHelp(std::cout);
      return false;
    }
    return checkMustArg();
  }

  // Check all must argument is defined
  bool checkMustArg() const {
    for (auto &arg : argumentContainer) {
      if (arg.isMustArg() && !arg.isDefined()) {
        exitProgram("Must Define All Non-optional Argument");
        return false;
      }
    }
    return true;
  }

  void printBriefUsage(std::ostream &out) const {
    out << std::endl;
    out << "Usage:" << std::endl;
    out << programName << " ";
    // print arguments
    // --debug --out-file <string> --epoch <int>
    for (auto &arg : argumentContainer) {
      if (arg.isMustArg()) {
        out << "< ";
      } else {
        // optional
        out << "[ ";
      }
      // print --lexer | -l
      out << "--" << arg.longArg;
      // has short argument
      if (arg.shortArg != "") {
        out << " | " << '-' << arg.shortArg;
      }
      out << " ";

      switch (arg.getType()) {
      case ArgType::T_FLOAT:
        out << "<float> ";
        break;
      case ArgType::T_INT:
        out << "<int> ";
        break;
      case ArgType::T_STRING:
        out << "<string> ";
        break;
      default:
        break;
      }
      if (arg.isMustArg()) {
        out << ">";
      } else {
        out << "]";
      }
      out << " ";
    }
    out << std::endl;
  }

  void printHelp(std::ostream &out) const {
    out << std::endl;
    // print program name
    out << "Program: " << programName << std::endl;
    // print description
    if (description != "")
      out << "Description: " << description << std::endl;
    printBriefUsage(out);
    out << std::endl;
    out << "Help Message:" << std::endl;
    // print help message:
    // --lexer <help message if exists>
    for (auto &arg : argumentContainer) {
      if (arg.helpMessage != "") {
        // indent
        out << "  ";
        // --lexer | -l   <help-message>
        out << "--" << arg.longArg;
        if (arg.shortArg != "")
          out << " | "
              << "-" << arg.shortArg;
        out << "   " << arg.helpMessage << std::endl;
      }
    }
  }

private:
  size_t getArgPos(const std::string &arg) const {
    try {
      return argsMap.at(arg);
    } catch (std::out_of_range &) {
      exitProgram("Argument Not Exist");
    }
    return std::string::npos;
  }

  void exitProgram(const std::string &errorMsg) const {
    std::cerr << "\n"
              << "[Error] " << errorMsg << std::endl;
    printBriefUsage(std::cout);
    std::exit(-1);
  }

private:
  std::string m_helpCmd;

private:
  std::string programName;
  std::string description;
  // For saving argument, key is the long argument, value is the
  // postion of container of argument
  std::map<std::string, size_t> argsMap;
  std::vector<Argument> argumentContainer;
};
} // namespace thlib
THLIB_NAMESPACE_END
#endif // ARGPARSER_HPP