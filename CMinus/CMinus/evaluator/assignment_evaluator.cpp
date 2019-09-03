#include "../node/memory_reference_node.h"

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
			right_value->call_attributes(runtime, logic::attributes::object::stage_type::before_ref_assign, true, std::vector<std::shared_ptr<memory::reference>>{ left_value });
		else
			throw logic::exception("Ref assignment requires an l-value source", 0u, 0u);
	}

	auto left_type = left_value->get_type(), right_type = right_value->get_type();
	if (left_type == nullptr || right_type == nullptr)
		throw logic::exception("Operator '=' does not take the specified operands", 0u, 0u);

	if ((right_value = right_type->cast(runtime, right_value, left_type, (is_ref ? type::object::cast_type::ref_static : type::object::cast_type::rval_static))) == nullptr)
		throw logic::exception("Cannot assign object to destination type", 0u, 0u);

	if (!is_ref){//Copy value
		if (is_init){
			logic::attributes::read_guard guard(runtime, right_value.get(), true);
			runtime.memory_object.write(right_value->get_address(), left_value->get_address(), left_type->get_size());
		}
		else if (left_value->write(runtime, *right_value, left_type->get_size()) != left_type->get_size())
			throw logic::exception("Assignment could not be completed", 0u, 0u);

		after_value_copy_(runtime, left_value, right_value);
	}
	else//Copy address
		ref_left_value->write_address(right_value->get_address());

	return left_value;
}

void cminus::evaluator::assignment::after_value_copy_(logic::runtime &runtime, std::shared_ptr<memory::reference> left_value, std::shared_ptr<memory::reference> right_value) const{}

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

	auto evaluator = dynamic_cast<const object *>(this);
	if (evaluator == nullptr && (evaluator = left_type->get_evaluator(runtime).get()) == nullptr)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	if (auto result = evaluator->evaluate_binary(runtime, non_compound_op, left_value, right); result != nullptr)
		return evaluator->evaluate_binary(runtime, operator_id::assignment, left_value, std::make_shared<node::memory_reference>(nullptr, result));

	throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);
	return nullptr;
}
