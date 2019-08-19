#include "runtime.h"

cminus::logic::exception::exception(const char *message, std::size_t line, std::size_t column)
	: base_type(message), line_(line), column_(column){}

std::size_t cminus::logic::exception::get_line() const{
	return line_;
}

std::size_t cminus::logic::exception::get_column() const{
	return column_;
}
