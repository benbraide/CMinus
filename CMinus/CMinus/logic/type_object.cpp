#include "runtime.h"

cminus::logic::type::object::~object() = default;

cminus::logic::type::named_object::named_object(const std::string &value, naming::parent *parent)
	: single(value, parent){}

cminus::logic::type::named_object::~named_object() = default;

void cminus::logic::type::named_object::print(logic::runtime &runtime, bool is_qualified) const{
	if (parent_ == nullptr)
		runtime.writer.write_buffer(value_.data(), value_.size());
	else//Default
		single::print(runtime, is_qualified);
}

std::string cminus::logic::type::named_object::get_qualified_naming_value() const{
	return ((parent_ == nullptr) ? value_ : single::get_qualified_naming_value());
}
