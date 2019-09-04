#include "byte_evaluator.h"

cminus::evaluator::byte::~byte() = default;

std::shared_ptr<cminus::memory::reference> cminus::evaluator::byte::evaluate_unary_left(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const{
	if (target == nullptr || !std::holds_alternative<operator_id>(op) || std::get<operator_id>(op) != operator_id::bitwise_inverse)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);

	return evaluate_unary_left_<std::byte>(runtime, std::get<operator_id>(op), target);
}

std::shared_ptr<cminus::memory::reference> cminus::evaluator::byte::evaluate_unary_right(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const{
	throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);
	return nullptr;
}

std::shared_ptr<cminus::memory::reference> cminus::evaluator::byte::evaluate_binary(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> left_value, const operand_type &right) const{
	if (auto result = explicit_comparison::evaluate_(runtime, op, left_value, right); result != nullptr)//Handled
		return result;

	if (auto result = assignment::evaluate_(runtime, op, left_value, right); result != nullptr)//Handled
		return result;

	if (auto result = compound_assignment::evaluate_(runtime, op, left_value, right); result != nullptr)//Handled
		return result;

	if (left_value == nullptr || !std::holds_alternative<operator_id>(op) || (!object::operator_is_integral(std::get<operator_id>(op)) && !object::operator_is_relational(std::get<operator_id>(op))))
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	auto right_value = object::convert_operand_to_memory_reference(runtime, right);
	if (right_value == nullptr)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	if (auto result = evaluate_shift_<std::byte>(runtime, std::get<operator_id>(op), left_value, right_value); result != nullptr)//Handled
		return result;

	auto left_type = left_value->get_type(), right_type = right_value->get_type();
	if (left_type == nullptr || right_type == nullptr || !left_type->is_exact(runtime, *right_type))
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	auto result = evaluate_binary_<std::byte>(runtime, std::get<operator_id>(op), left_value, right_value);
	if (result == nullptr && (result = boolean::evaluate_<std::byte>(runtime, std::get<operator_id>(op), left_value, right_value)) == nullptr)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	return result;
}
