#include "runtime.h"

cminus::logic::exception::exception(const std::string &message, std::size_t line, std::size_t column)
	: base_type(nullptr), message_(message), line_(line), column_(column){}

cminus::logic::exception::~exception() = default;

char const *cminus::logic::exception::what() const{
	return message_.data();
}

std::size_t cminus::logic::exception::get_line() const{
	return line_;
}

std::size_t cminus::logic::exception::get_column() const{
	return column_;
}
