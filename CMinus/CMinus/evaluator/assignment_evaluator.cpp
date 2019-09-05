#include "../type/pointer_type.h"
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

	auto is_init = left_value->has_attribute("#Init#", true);
	auto ref_left_value = dynamic_cast<memory::ref_reference *>(left_value->get_non_raw());

	auto is_ref = (is_init && ref_left_value != nullptr);
	auto is_val = (!is_ref && left_value->has_attribute("Val", true));

	auto left_type = left_value->get_type(), right_type = right_value->get_type();
	if (left_type == nullptr || right_type == nullptr)
		throw logic::exception("Operator '=' does not take the specified operands", 0u, 0u);

	if (is_ref){//Copy reference
		if (right_value->is_lvalue()){
			right_value->traverse_attributes([&](std::shared_ptr<logic::attributes::object> attribute){
				if (attribute->is_required_on_ref_assignment(runtime)){
					if (!left_value->has_attribute(*attribute))
						throw logic::exception("'" + attribute->get_qualified_naming_value() + "' attribute is required on a ref assignment!", 0u, 0u);
				}
			});
		}
		else if (!(is_val = left_value->has_attribute("ReadOnly", true)))
			throw logic::exception("'Ref' assignment requires an l-value source", 0u, 0u);
	}
	else if (is_val && right_value->is_lvalue())
		throw logic::exception("'Val' assignment requires an r-value source", 0u, 0u);

	if (dynamic_cast<type::pointer *>(right_value.get()) != nullptr){//Pointer assignment
		right_value->traverse_attributes([&](std::shared_ptr<logic::attributes::object> attribute){
			if (attribute->is_required_on_pointer_assignment(runtime)){
				if (!left_value->has_attribute(*attribute))
					throw logic::exception("'" + attribute->get_qualified_naming_value() + "' attribute is required on a pointer assignment!", 0u, 0u);
			}
		});
	}

	if (right_type->converts_auto(*left_type))//Update auto type
		left_value->set_type(left_type = right_type);

	type::object::cast_type cast_type;
	if (is_ref)
		cast_type = (is_val ? type::object::cast_type::static_ : type::object::cast_type::ref_static);
	else//Not a reference assignment
		cast_type = type::object::cast_type::rval_static;

	if ((right_value = right_type->cast(runtime, right_value, left_type, cast_type)) == nullptr)
		throw logic::exception("Cannot assign object to destination type", 0u, 0u);

	if (!is_ref){//Copy value
		if (is_init){
			logic::attributes::read_guard guard(runtime, right_value.get());
			if (auto right_address = right_value->get_address(); right_address == 0u)
				runtime.memory_object.write(left_value->get_address(), right_value->get_data(runtime), left_type->get_size());
			else//Read from address
				runtime.memory_object.write(right_address, left_value->get_address(), left_type->get_size());
		}
		else if (left_value->write(runtime, *right_value, left_type->get_size()) != left_type->get_size())
			throw logic::exception("Assignment could not be completed", 0u, 0u);

		after_value_copy_(runtime, left_value, right_value);
	}
	else if (is_val)//R-value reference
		left_value = std::make_shared<memory::rval_ref_reference>(right_value);
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
