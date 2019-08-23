#include "integral_evaluator.h"

cminus::evaluator::integral::~integral() = default;

std::shared_ptr<cminus::memory::reference> cminus::evaluator::integral::evaluate_unary_left(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const{
	if (target == nullptr || !std::holds_alternative<operator_id>(op))
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);

	auto type = target->get_type();
	auto primitive_type = dynamic_cast<logic::type::primitive *>(type.get());

	if (primitive_type == nullptr || !primitive_type->is_integral())
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);

	auto will_write = (std::get<operator_id>(op) == operator_id::increment || std::get<operator_id>(op) == operator_id::decrement);
	if (!will_write && std::get<operator_id>(op) != operator_id::bitwise_inverse && std::get<operator_id>(op) != operator_id::plus){
		if (std::get<operator_id>(op) != operator_id::minus || primitive_type->is_unsigned_integral())
			throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);
	}

	std::shared_ptr<write_attribute_guard> write_guard;
	if (will_write)
		write_guard = std::make_shared<write_attribute_guard>(runtime, target, true);

	read_attribute_guard read_guard(runtime, target, true);
	if (target->is_nan()){
		switch (std::get<operator_id>(op)){
		case operator_id::increment:
		case operator_id::decrement:
			return target;
		case operator_id::bitwise_inverse:
			return runtime.global_storage->get_named_constant(node::named_constant::constant_type::nan_);
		default:
			break;
		}

		return arithmetic::evaluate_nan_unary_left_(runtime, std::get<operator_id>(op));
	}

	switch (primitive_type->get_id()){
	case logic::type::primitive::id_type::int8_:
		return arithmetic::evaluate_unary_left_<__int8>(runtime, std::get<operator_id>(op), target);
	case logic::type::primitive::id_type::uint8_:
		return arithmetic::evaluate_unsigned_unary_left_<unsigned __int8>(runtime, std::get<operator_id>(op), target);
	case logic::type::primitive::id_type::int16_:
		return arithmetic::evaluate_unary_left_<__int16>(runtime, std::get<operator_id>(op), target);
	case logic::type::primitive::id_type::uint16_:
		return arithmetic::evaluate_unsigned_unary_left_<unsigned __int16>(runtime, std::get<operator_id>(op), target);
	case logic::type::primitive::id_type::int32_:
		return arithmetic::evaluate_unary_left_<__int32>(runtime, std::get<operator_id>(op), target);
	case logic::type::primitive::id_type::uint32_:
		return arithmetic::evaluate_unsigned_unary_left_<unsigned __int32>(runtime, std::get<operator_id>(op), target);
	case logic::type::primitive::id_type::int64_:
		return arithmetic::evaluate_unary_left_<__int64>(runtime, std::get<operator_id>(op), target);
	case logic::type::primitive::id_type::uint64_:
		return arithmetic::evaluate_unsigned_unary_left_<unsigned __int64>(runtime, std::get<operator_id>(op), target);
	default:
		break;
	}

	throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);
	return nullptr;
}

std::shared_ptr<cminus::memory::reference> cminus::evaluator::integral::evaluate_unary_right(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const{
	if (target == nullptr || !std::holds_alternative<operator_id>(op))
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);

	auto type = target->get_type();
	auto primitive_type = dynamic_cast<logic::type::primitive *>(type.get());

	if (primitive_type == nullptr || !primitive_type->is_integral())
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);

	auto will_write = (std::get<operator_id>(op) == operator_id::increment || std::get<operator_id>(op) == operator_id::decrement);
	if (!will_write && std::get<operator_id>(op) != operator_id::bitwise_inverse && std::get<operator_id>(op) != operator_id::plus){
		if (std::get<operator_id>(op) != operator_id::minus || primitive_type->is_unsigned_integral())
			throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);
	}

	std::shared_ptr<write_attribute_guard> write_guard;
	if (will_write)
		write_guard = std::make_shared<write_attribute_guard>(runtime, target, true);

	read_attribute_guard read_guard(runtime, target, true);
	if (target->is_nan()){
		switch (std::get<operator_id>(op)){
		case operator_id::increment:
		case operator_id::decrement:
			return target;
		case operator_id::bitwise_inverse:
			return runtime.global_storage->get_named_constant(node::named_constant::constant_type::nan_);
		default:
			break;
		}

		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);
		return nullptr;
	}

	switch (primitive_type->get_id()){
	case logic::type::primitive::id_type::int8_:
		return arithmetic::evaluate_unsigned_unary_right_<__int8>(runtime, std::get<operator_id>(op), target);
	case logic::type::primitive::id_type::uint8_:
		return arithmetic::evaluate_unsigned_unary_right_<unsigned __int8>(runtime, std::get<operator_id>(op), target);
	case logic::type::primitive::id_type::int16_:
		return arithmetic::evaluate_unsigned_unary_right_<__int16>(runtime, std::get<operator_id>(op), target);
	case logic::type::primitive::id_type::uint16_:
		return arithmetic::evaluate_unsigned_unary_right_<unsigned __int16>(runtime, std::get<operator_id>(op), target);
	case logic::type::primitive::id_type::int32_:
		return arithmetic::evaluate_unsigned_unary_right_<__int32>(runtime, std::get<operator_id>(op), target);
	case logic::type::primitive::id_type::uint32_:
		return arithmetic::evaluate_unsigned_unary_right_<unsigned __int32>(runtime, std::get<operator_id>(op), target);
	case logic::type::primitive::id_type::int64_:
		return arithmetic::evaluate_unsigned_unary_right_<__int64>(runtime, std::get<operator_id>(op), target);
	case logic::type::primitive::id_type::uint64_:
		return arithmetic::evaluate_unsigned_unary_right_<unsigned __int64>(runtime, std::get<operator_id>(op), target);
	default:
		break;
	}

	throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);
	return nullptr;
}

std::shared_ptr<cminus::memory::reference> cminus::evaluator::integral::evaluate_binary(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> left_value, const operand_type &right) const{
	if (auto result = explicit_comparison::evaluate_(runtime, op, left_value, right); result != nullptr)//Handled
		return result;

	if (auto result = assignment::evaluate_(runtime, op, left_value, right); result != nullptr)//Handled
		return result;

	if (auto result = compound_assignment::evaluate_(runtime, op, left_value, right); result != nullptr)//Handled
		return result;

	if (left_value == nullptr || !std::holds_alternative<operator_id>(op) || (!object::operator_is_arithmetic(std::get<operator_id>(op)) && !object::operator_is_integral(std::get<operator_id>(op))))
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	auto right_value = object::convert_operand_to_memory_reference(runtime, right);
	if (right_value == nullptr)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	auto left_type = left_value->get_type(), right_type = right_value->get_type();
	if (left_type == nullptr || right_type == nullptr)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	if (left_type->get_score(*right_type, false) != logic::type::object::score_result_type::exact){
		if (left_value->is_nan() || right_value->is_nan()){
			if (auto result = arithmetic::evaluate_nan_(runtime, std::get<operator_id>(op)); result != nullptr)//Handled
				return result;

			if (auto result = arithmetic::compare_nan_(runtime, std::get<operator_id>(op), left_value); result != nullptr)//Handled
				return result;
		}

		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);
	}

	auto left_primitive_type = dynamic_cast<logic::type::primitive *>(left_type.get());
	if (left_primitive_type == nullptr)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	read_attribute_guard left_read_guard(runtime, left_value, true);
	read_attribute_guard right_read_guard(runtime, right_value, true);

	switch (left_primitive_type->get_id()){
	case logic::type::primitive::id_type::int8_:
		return evaluate_binary_<__int8>(runtime, std::get<operator_id>(op), left_value, right_value);
	case logic::type::primitive::id_type::uint8_:
		return evaluate_binary_<unsigned __int8>(runtime, std::get<operator_id>(op), left_value, right_value);
	case logic::type::primitive::id_type::int16_:
		return evaluate_binary_<__int16>(runtime, std::get<operator_id>(op), left_value, right_value);
	case logic::type::primitive::id_type::uint16_:
		return evaluate_binary_<unsigned __int16>(runtime, std::get<operator_id>(op), left_value, right_value);
	case logic::type::primitive::id_type::int32_:
		return evaluate_binary_<__int32>(runtime, std::get<operator_id>(op), left_value, right_value);
	case logic::type::primitive::id_type::uint32_:
		return evaluate_binary_<unsigned __int32>(runtime, std::get<operator_id>(op), left_value, right_value);
	case logic::type::primitive::id_type::int64_:
		return evaluate_binary_<__int64>(runtime, std::get<operator_id>(op), left_value, right_value);
	case logic::type::primitive::id_type::uint64_:
		return evaluate_binary_<unsigned __int64>(runtime, std::get<operator_id>(op), left_value, right_value);
	default:
		break;
	}

	throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);
	return nullptr;
}

std::shared_ptr<cminus::memory::reference> cminus::evaluator::integral::assign_(logic::runtime &runtime, std::shared_ptr<memory::reference> destination, std::shared_ptr<memory::reference> source) const{
	if (source->is_nan()){//Set 'NaN attribute
		if (!destination->is_nan())
			destination->add_attribute(runtime.global_storage->find_attribute("#NaN#", false));
	}
	else if ((destination = assignment::assign_(runtime, destination, source)) != nullptr && destination->is_nan())
		destination->remove_attribute(runtime.global_storage->find_attribute("#NaN#", false));

	return destination;
}
