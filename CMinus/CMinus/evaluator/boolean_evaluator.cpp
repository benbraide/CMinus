#include "boolean_evaluator.h"

cminus::evaluator::boolean::~boolean() = default;

std::shared_ptr<cminus::memory::reference> cminus::evaluator::boolean::evaluate_unary_left(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const{
	if (target == nullptr || !std::holds_alternative<operator_id>(op) || std::get<operator_id>(op) != operator_id::relational_not)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);

	auto type = target->get_type();
	auto primitive_type = dynamic_cast<logic::type::primitive *>(type.get());

	if (primitive_type == nullptr || primitive_type->get_id() != logic::type::primitive::id_type::bool_)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);

	read_attribute_guard read_guard(runtime, target, true);
	if (target->read_scalar<constant_type>(runtime) != constant_type::true_)
		return runtime.global_storage->get_named_constant(constant_type::true_);

	return runtime.global_storage->get_named_constant(constant_type::false_);
}

std::shared_ptr<cminus::memory::reference> cminus::evaluator::boolean::evaluate_unary_right(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const{
	throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);
	return nullptr;
}

std::shared_ptr<cminus::memory::reference> cminus::evaluator::boolean::evaluate_binary(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> left_value, const operand_type &right) const{
	if (auto result = explicit_comparison::evaluate_(runtime, op, left_value, right); result != nullptr)//Handled
		return result;

	if (auto result = assignment::evaluate_(runtime, op, left_value, right); result != nullptr)//Handled
		return result;

	if (auto result = equality_comparison::evaluate_(runtime, op, left_value, right); result != nullptr)
		return result;

	throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);
	return nullptr;
}
