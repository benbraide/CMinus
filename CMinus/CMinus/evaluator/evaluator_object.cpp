#include "evaluator_object.h"

cminus::evaluator::object::~object() = default;

std::string cminus::evaluator::object::convert_operator_to_string(const operator_type &value){
	if (std::holds_alternative<std::string>(value))
		return std::get<std::string>(value);

	switch (std::get<operator_id>(value)){
	case operator_id::compound_left_shift:
		return "<<=";
	case operator_id::compound_right_shift:
		return ">>=";
	case operator_id::compound_times:
		return "*=";
	case operator_id::compound_divide:
		return "/=";
	case operator_id::compound_modulus:
		return "%=";
	case operator_id::compound_plus:
		return "+=";
	case operator_id::compound_minus:
		return "-=";
	case operator_id::compound_bitwise_and:
		return "&=";
	case operator_id::compound_bitwise_xor:
		return "^=";
	case operator_id::compound_bitwise_or:
		return "|=";
	case operator_id::explicit_equality:
		return "===";
	case operator_id::explicit_inverse_equality:
		return "!==";
	case operator_id::equality:
		return "==";
	case operator_id::inverse_equality:
		return "!=";
	case operator_id::relational_and:
		return "&&";
	case operator_id::relational_or:
		return "||";
	case operator_id::member_pointer_access:
		return "->";
	case operator_id::left_shift:
		return "<<";
	case operator_id::right_shift:
		return ">>";
	case operator_id::less_or_equal:
		return "<=";
	case operator_id::more_or_equal:
		return ">=";
	case operator_id::increment:
		return "++";
	case operator_id::decrement:
		return "--";
	case operator_id::assignment:
		return "=";
	case operator_id::member_access:
		return ".";
	case operator_id::times:
		return "*";
	case operator_id::divide:
		return "/";
	case operator_id::modulus:
		return "%";
	case operator_id::plus:
		return "+";
	case operator_id::minus:
		return "-";
	case operator_id::less:
		return "<";
	case operator_id::more:
		return ">";
	case operator_id::bitwise_and:
		return "&";
	case operator_id::bitwise_xor:
		return "^";
	case operator_id::bitwise_or:
		return "|";
	case operator_id::bitwise_inverse:
		return "~";
	case operator_id::relational_not:
		return "!";
	case operator_id::comma:
		return ",";
	case operator_id::new_:
		return "new";
	case operator_id::delete_:
		return "delete";
	case operator_id::sizeof_:
		return "sizeof";
	case operator_id::typeof:
		return "typeof";
	default:
		break;
	}

	return "";
}

cminus::evaluator::operator_id cminus::evaluator::object::convert_compound_operator_to_non_compound(operator_id id){
	switch (id){
	case operator_id::compound_plus:
		return operator_id::plus;
	case operator_id::compound_minus:
		return operator_id::minus;
	case operator_id::compound_times:
		return operator_id::times;
	case operator_id::compound_divide:
		return operator_id::divide;
	case operator_id::compound_modulus:
		return operator_id::modulus;
	case operator_id::compound_bitwise_or:
		return operator_id::bitwise_or;
	case operator_id::compound_bitwise_xor:
		return operator_id::bitwise_xor;
	case operator_id::compound_bitwise_and:
		return operator_id::bitwise_and;
	case operator_id::compound_left_shift:
		return operator_id::left_shift;
	case operator_id::compound_right_shift:
		return operator_id::right_shift;
	default:
		break;
	}

	return operator_id::nil;
}

bool cminus::evaluator::object::operator_is_compound_assignment(operator_id id){
	switch (id){
	case operator_id::compound_plus:
	case operator_id::compound_minus:
	case operator_id::compound_times:
	case operator_id::compound_divide:
	case operator_id::compound_modulus:
	case operator_id::compound_bitwise_or:
	case operator_id::compound_bitwise_xor:
	case operator_id::compound_bitwise_and:
	case operator_id::compound_left_shift:
	case operator_id::compound_right_shift:
		return true;
	default:
		break;
	}

	return false;
}

bool cminus::evaluator::object::operator_is_arithmetic(operator_id id){
	switch (id){
	case operator_id::plus:
	case operator_id::minus:
	case operator_id::times:
	case operator_id::divide:
		return true;
	default:
		break;
	}

	return false;
}

bool cminus::evaluator::object::operator_is_integral_arithmetic(operator_id id){
	switch (id){
	case operator_id::modulus:
	case operator_id::bitwise_or:
	case operator_id::bitwise_xor:
	case operator_id::bitwise_and:
	case operator_id::left_shift:
	case operator_id::right_shift:
		return true;
	default:
		break;
	}

	return false;
}

bool cminus::evaluator::object::operator_is_relational(operator_id id){
	switch (id){
	case operator_id::less:
	case operator_id::less_or_equal:
	case operator_id::equality:
	case operator_id::explicit_equality:
	case operator_id::explicit_inverse_equality:
	case operator_id::inverse_equality:
	case operator_id::more_or_equal:
	case operator_id::more:
		return true;
	default:
		break;
	}

	return false;
}

bool cminus::evaluator::object::operator_is_shift(operator_id id, bool include_index){
	switch (id){
	case operator_id::index:
		return include_index;
	case operator_id::left_shift:
	case operator_id::right_shift:
		return true;
	default:
		break;
	}

	return false;
}

bool cminus::evaluator::object::operator_is_floating_point(operator_id id){
	return (operator_is_arithmetic(id) || operator_is_relational(id));
}

bool cminus::evaluator::object::operator_is_integral(operator_id id){
	return (operator_is_arithmetic(id) || operator_is_integral_arithmetic(id) || operator_is_relational(id) || operator_is_shift(id, true));
}

std::shared_ptr<cminus::memory::reference> cminus::evaluator::object::convert_operand_to_memory_reference(logic::runtime &runtime, const operand_type &value){
	return value->evaluate(runtime);
}

cminus::evaluator::write_attribute_guard::write_attribute_guard(logic::runtime &runtime, std::shared_ptr<memory::reference> target, bool include_context){
	memory::reference::call_attributes(runtime, logic::attributes::object::stage_type::before_write, include_context, target);
	callback_ = [&runtime, target, include_context]{
		memory::reference::call_attributes(runtime, logic::attributes::object::stage_type::after_write, include_context, target);
	};
}

cminus::evaluator::write_attribute_guard::~write_attribute_guard(){
	try{
		if (callback_ != nullptr)
			callback_();
	}
	catch (...){}
}

cminus::evaluator::read_attribute_guard::read_attribute_guard(logic::runtime &runtime, std::shared_ptr<memory::reference> target, bool include_context){
	memory::reference::call_attributes(runtime, logic::attributes::object::stage_type::before_read, include_context, target);
	callback_ = [&runtime, target, include_context]{
		memory::reference::call_attributes(runtime, logic::attributes::object::stage_type::after_read, include_context, target);
	};
}

cminus::evaluator::read_attribute_guard::~read_attribute_guard(){
	try{
		if (callback_ != nullptr)
			callback_();
	}
	catch (...){}
}
