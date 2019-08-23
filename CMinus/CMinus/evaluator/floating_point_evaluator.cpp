#include "floating_point_evaluator.h"

cminus::evaluator::floating_point::~floating_point() = default;

std::shared_ptr<cminus::memory::reference> cminus::evaluator::floating_point::evaluate_unary_left(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const{
	if (target == nullptr || !std::holds_alternative<operator_id>(op))
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);

	if (std::get<operator_id>(op) != operator_id::plus && std::get<operator_id>(op) != operator_id::minus)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);

	auto type = target->get_type();
	auto primitive_type = dynamic_cast<type::primitive *>(type.get());

	if (primitive_type == nullptr || !primitive_type->is_floating_point())
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);

	read_attribute_guard left_read_guard(runtime, target, true);
	if (target->is_nan())
		return arithmetic::evaluate_nan_unary_left_(runtime, std::get<operator_id>(op));

	switch (primitive_type->get_id()){
	case type::primitive::id_type::float_:
		return arithmetic::evaluate_unary_left_<float>(runtime, std::get<operator_id>(op), target);
	case type::primitive::id_type::double_:
		return arithmetic::evaluate_unary_left_<double>(runtime, std::get<operator_id>(op), target);
	case type::primitive::id_type::ldouble:
		return arithmetic::evaluate_unary_left_<long double>(runtime, std::get<operator_id>(op), target);
	default:
		break;
	}

	throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);
	return nullptr;
}

std::shared_ptr<cminus::memory::reference> cminus::evaluator::floating_point::evaluate_binary(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> left_value, const operand_type &right) const{
	if (auto result = explicit_comparison::evaluate_(runtime, op, left_value, right); result != nullptr)//Handled
		return result;

	if (auto result = assignment::evaluate_(runtime, op, left_value, right); result != nullptr)//Handled
		return result;

	if (auto result = compound_assignment::evaluate_(runtime, op, left_value, right); result != nullptr)//Handled
		return result;

	if (left_value == nullptr || !std::holds_alternative<operator_id>(op) || !object::operator_is_arithmetic(std::get<operator_id>(op)))
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	auto right_value = object::convert_operand_to_memory_reference(runtime, right);
	if (right_value == nullptr)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	auto left_type = left_value->get_type(), right_type = right_value->get_type();
	if (left_type == nullptr || right_type == nullptr)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	if (left_type->get_score(*right_type, false) != type::object::score_result_type::exact){
		if (left_value->is_nan() || right_value->is_nan()){
			read_attribute_guard left_read_guard(runtime, left_value, true);
			read_attribute_guard right_read_guard(runtime, right_value, true);

			if (auto result = arithmetic::evaluate_nan_(runtime, std::get<operator_id>(op)); result != nullptr)//Handled
				return result;

			if (auto result = arithmetic::compare_nan_(runtime, std::get<operator_id>(op), left_value); result != nullptr)//Handled
				return result;
		}

		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);
	}

	auto left_primitive_type = dynamic_cast<type::primitive *>(left_type.get());
	if (left_primitive_type == nullptr)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	read_attribute_guard left_read_guard(runtime, left_value, true);
	read_attribute_guard right_read_guard(runtime, right_value, true);

	switch (left_primitive_type->get_id()){
	case type::primitive::id_type::float_:
		return evaluate_binary_<float>(runtime, std::get<operator_id>(op), left_value, right_value);
	case type::primitive::id_type::double_:
		return evaluate_binary_<double>(runtime, std::get<operator_id>(op), left_value, right_value);
	case type::primitive::id_type::ldouble:
		return evaluate_binary_<long double>(runtime, std::get<operator_id>(op), left_value, right_value);
	default:
		break;
	}

	throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);
	return nullptr;
}

std::shared_ptr<cminus::memory::reference> cminus::evaluator::floating_point::assign_(logic::runtime &runtime, std::shared_ptr<memory::reference> destination, std::shared_ptr<memory::reference> source) const{
	if (source->is_nan()){//Set 'NaN attribute
		if (!destination->is_nan())
			destination->add_attribute(runtime.global_storage->find_attribute("#NaN#", false));
	}
	else if ((destination = assignment::assign_(runtime, destination, source)) != nullptr && destination->is_nan())
		destination->remove_attribute(runtime.global_storage->find_attribute("#NaN#", false));

	return destination;
}
