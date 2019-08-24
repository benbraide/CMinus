#include "../node/named_constant_node.h"

#include "comparison_evaluator.h"

cminus::evaluator::explicit_comparison::~explicit_comparison() = default;

std::shared_ptr<cminus::memory::reference> cminus::evaluator::explicit_comparison::evaluate_(logic::runtime &runtime, const object::operator_type &op, std::shared_ptr<memory::reference> left_value, const object::operand_type &right) const{
	if (left_value == nullptr || !std::holds_alternative<operator_id>(op) || (std::get<operator_id>(op) != operator_id::explicit_equality && std::get<operator_id>(op) != operator_id::explicit_inverse_equality))
		return nullptr;

	auto right_value = object::convert_operand_to_memory_reference(runtime, right);
	if (right_value == nullptr)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	auto left_type = left_value->get_type(), right_type = right_value->get_type();
	if (left_type == nullptr || right_type == nullptr)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	auto is_equal = (std::get<operator_id>(op) == operator_id::explicit_equality);
	if (is_equal != (left_type->get_score(runtime, *right_type, false) == type::object::score_result_type::exact))
		return runtime.global_storage->get_named_constant(node::named_constant::constant_type::false_);

	auto comparison_evaluator = dynamic_cast<const object *>(this);
	if (comparison_evaluator == nullptr && (comparison_evaluator = left_type->get_evaluator(runtime).get()) == nullptr)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	if (auto result = comparison_evaluator->evaluate_binary(runtime, (is_equal ? operator_id::equality : operator_id::inverse_equality), left_value, right_value); result != nullptr)
		return result;

	throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);
	return nullptr;
}

cminus::evaluator::equality_comparison::~equality_comparison() = default;

std::shared_ptr<cminus::memory::reference> cminus::evaluator::equality_comparison::evaluate_(logic::runtime &runtime, const object::operator_type &op, std::shared_ptr<memory::reference> left_value, const object::operand_type &right) const{
	if (left_value == nullptr || !std::holds_alternative<operator_id>(op) || (std::get<operator_id>(op) != operator_id::equality && std::get<operator_id>(op) != operator_id::inverse_equality))
		return nullptr;

	auto right_value = object::convert_operand_to_memory_reference(runtime, right);
	if (right_value == nullptr)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	auto left_type = left_value->get_type(), right_type = right_value->get_type();
	if (left_type == nullptr || right_type == nullptr || left_type->get_score(runtime, *right_type, false) != type::object::score_result_type::exact)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	read_attribute_guard left_read_guard(runtime, left_value, true);
	read_attribute_guard right_read_guard(runtime, right_value, true);

	auto result = left_value->compare(runtime, *right_value, left_type->get_size());
	if (result == std::numeric_limits<int>::min())
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	if ((result == 0) == (std::get<operator_id>(op) == operator_id::equality))
		return runtime.global_storage->get_named_constant(node::named_constant::constant_type::true_);

	return runtime.global_storage->get_named_constant(node::named_constant::constant_type::false_);
}
