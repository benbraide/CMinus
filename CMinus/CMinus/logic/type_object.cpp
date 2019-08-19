#include "type_object.h"

cminus::logic::type::object::object(const std::string &value, naming::parent *parent)
	: single(value, parent){}

cminus::logic::type::object::~object() = default;
