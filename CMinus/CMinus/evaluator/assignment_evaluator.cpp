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
	auto is_ref = (is_init && left_value->find_attribute("Ref", true, false) != nullptr);

	if (is_ref){//Copy reference
		if (!right_value->is_lvalue())
			throw logic::exception("Ref assignment requires an l-value source", 0u, 0u);

		right_value->traverse_attributes(runtime, [&](std::shared_ptr<logic::attributes::object> attribute){
			if (attribute->is_required_on_ref_destination(runtime) && left_value->find_attribute(attribute, true) == nullptr)
				throw logic::exception("'" + attribute->get_qualified_naming_value() + "' is required on ref destination", 0u, 0u);
		}, logic::attributes::object::stage_type::nil, true);
	}

	auto left_type = left_value->get_type(), right_type = right_value->get_type();
	if (left_type == nullptr || right_type == nullptr)
		throw logic::exception("Operator '=' does not take the specified operands", 0u, 0u);

	std::shared_ptr<write_attribute_guard> write_guard;
	if (!is_init)//Check if write is allowed
		write_guard = std::make_shared<write_attribute_guard>(runtime, left_value, true);

	read_attribute_guard read_guard(runtime, right_value, true);
	switch (left_type->get_score(runtime, *right_type, is_ref)){
	case type::object::score_result_type::exact:
	case type::object::score_result_type::assignable:
		break;
	case type::object::score_result_type::ancestor:
		right_value = std::make_shared<memory::hard_reference>((right_value->get_address() + right_type->compute_base_offset(*left_type)), left_type, left_value->get_context());
		break;
	case type::object::score_result_type::widened:
	case type::object::score_result_type::too_widened:
	case type::object::score_result_type::shortened:
	case type::object::score_result_type::too_shortened:
	case type::object::score_result_type::compatible:
	case type::object::score_result_type::class_compatible:
		if ((right_value = right_type->cast(runtime, right_value, left_type, (is_ref ? type::object::cast_type::ref_static : type::object::cast_type::static_))) == nullptr)
			throw logic::exception("Cannot assign object to destination type", 0u, 0u);
		break;
	default:
		throw logic::exception("Cannot assign object to destination type", 0u, 0u);
		break;
	}

	if (right_value == nullptr)
		throw logic::exception("Operator '=' does not take the specified operands", 0u, 0u);

	if (is_ref && (left_value = assign_address_(runtime, left_value, right_value)) == nullptr)//Copy address
		throw logic::exception("Operator '=' does not take the specified operands", 0u, 0u);
	
	if (!is_ref && (left_value = assign_(runtime, left_value, right_value)) == nullptr)
		throw logic::exception("Operator '=' does not take the specified operands", 0u, 0u);

	if (is_init)
		left_value->remove_attribute("#Init#", true);

	return left_value;
}

std::shared_ptr<cminus::memory::reference> cminus::evaluator::assignment::assign_address_(logic::runtime &runtime, std::shared_ptr<memory::reference> destination, std::shared_ptr<memory::reference> source) const{
	destination->set_address(source->get_address());
	return destination;
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
