#include "../logic/runtime.h"

cminus::type::object::~object() = default;

int cminus::type::object::get_score_value(score_result_type score){
	switch (score){
	case score_result_type::exact:
		return 100;
	case score_result_type::ancestor:
	case score_result_type::widened:
		return 80;
	case score_result_type::too_widened:
		return 70;
	case score_result_type::shortened:
		return 60;
	case score_result_type::too_shortened:
		return 50;
	case score_result_type::compatible:
		return 30;
	case score_result_type::class_compatible:
		return 20;
	default:
		break;
	}

	return 0;
}

cminus::type::named_object::named_object(const std::string &value, logic::naming::parent *parent)
	: single(value, parent){}

cminus::type::named_object::~named_object() = default;

void cminus::type::named_object::print(logic::runtime &runtime, bool is_qualified) const{
	if (parent_ == nullptr)
		runtime.writer.write_buffer(value_.data(), value_.size());
	else//Default
		single::print(runtime, is_qualified);
}

std::string cminus::type::named_object::get_qualified_naming_value() const{
	return ((parent_ == nullptr) ? value_ : single::get_qualified_naming_value());
}
