#include "assignment_evaluator.h"

cminus::evaluator::assignment::~assignment() = default;

std::shared_ptr<cminus::memory::reference> cminus::evaluator::assignment::evaluate_(logic::runtime &runtime, const object::operator_type &op, std::shared_ptr<memory::reference> left_value, const object::operand_type &right) const{
	if (left_value == nullptr || !std::holds_alternative<operator_id>(op) || std::get<operator_id>(op) != operator_id::assignment)
		return nullptr;

	if (!left_value->is_lvalue())
		throw logic::exception("Assignment requires an l-value destination", 0u, 0u);

	auto right_value = object::convert_operand_to_memory_reference(runtime, right);
	if (right_value == nullptr)
		throw logic::exception("Operator '=' does not take the specified operands", 0u, 0u);

	auto is_init = (left_value->find_attribute("#Init#", true, false) != nullptr);
	auto ref_left_value = dynamic_cast<memory::ref_reference *>(left_value.get());
	auto is_ref = (is_init && ref_left_value != nullptr);

	if (is_ref){//Copy reference
		if (right_value->is_lvalue())
			memory::reference::call_attributes(runtime, logic::attributes::object::stage_type::before_ref_assign, true, right_value, std::vector<std::shared_ptr<memory::reference>>{ left_value });
		else
			throw logic::exception("Ref assignment requires an l-value source", 0u, 0u);
	}

	auto left_type = left_value->get_type(), right_type = right_value->get_type();
	if (left_type == nullptr || right_type == nullptr)
		throw logic::exception("Operator '=' does not take the specified operands", 0u, 0u);

	std::shared_ptr<write_attribute_guard> write_guard;
	if (!is_init)//Check if write is allowed
		write_guard = std::make_shared<write_attribute_guard>(runtime, left_value, true);

	read_attribute_guard read_guard(runtime, right_value, true);
	if ((right_value = right_type->cast(runtime, right_value, left_type, (is_ref ? type::object::cast_type::ref_static : type::object::cast_type::rval_static))) == nullptr)
		throw logic::exception("Cannot assign object to destination type", 0u, 0u);

	if (is_ref)//Copy address
		ref_left_value->write_address(right_value->get_address());
	else if (!is_ref && (left_value = assign_(runtime, left_value, right_value)) == nullptr)
		throw logic::exception("Operator '=' does not take the specified operands", 0u, 0u);

	if (is_init)
		left_value->remove_attribute("#Init#", true);

	return left_value;
}

std::shared_ptr<cminus::memory::reference> cminus::evaluator::assignment::assign_(logic::runtime &runtime, std::shared_ptr<memory::reference> destination, std::shared_ptr<memory::reference> source) const{
	if (destination->write(runtime, *source, destination->get_type()->get_size()) != destination->get_type()->get_size())
		throw logic::exception("Assignment could not be completed", 0u, 0u);
	return destination;
}

cminus::evaluator::compound_assignment::~compound_assignment() = default;

std::shared_ptr<cminus::memory::reference> cminus::evaluator::compound_assignment::evaluate_(logic::runtime &runtime, const object::operator_type &op, std::shared_ptr<memory::reference> left_value, const object::operand_type &right) const{
	if (left_value == nullptr || !std::holds_alternative<operator_id>(op) || !object::operator_is_compound_assignment(std::get<operator_id>(op)))
		return nullptr;

	if (!left_value->is_lvalue())
		throw logic::exception("Assignment requires an l-value destination", 0u, 0u);

	auto non_compound_op = object::convert_compound_operator_to_non_compound(std::get<operator_id>(op));
	if (non_compound_op == operator_id::nil)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	auto left_type = left_value->get_type();
	if (left_type == nullptr)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	auto non_compound_evaluator = dynamic_cast<const object *>(this);
	if (non_compound_evaluator == nullptr && (non_compound_evaluator = left_type->get_evaluator(runtime).get()) == nullptr)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	if (auto result = non_compound_evaluator->evaluate_binary(runtime, non_compound_op, left_value, right); result != nullptr)
		return result;

	throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);
	return nullptr;
}
