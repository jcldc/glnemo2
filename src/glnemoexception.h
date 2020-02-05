//
// Created by kalterkrieg on 04/02/2020.
//

#ifndef GLNEMO2_GLNEMOEXCEPTION_H
#define GLNEMO2_GLNEMOEXCEPTION_H


#include <exception>
#include <string>

namespace glnemo {
class glnemoException : public std::exception {

public:
  explicit glnemoException(const char *msg) : _msg(msg) {
  }

  const char *what() const noexcept override {
    return _msg.c_str();
  }

private:
  std::string _msg;

};
}
#endif //GLNEMO2_GLNEMOEXCEPTION_H
