#include "../type/pointer_type.h"

#include "pointer_evaluator.h"

cminus::evaluator::pointer::~pointer() = default;

std::shared_ptr<cminus::memory::reference> cminus::evaluator::pointer::evaluate_unary_left(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const{
	if (target == nullptr || !std::holds_alternative<operator_id>(op))
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);

	auto will_write = (std::get<operator_id>(op) == operator_id::increment || std::get<operator_id>(op) == operator_id::decrement);
	if (!will_write && std::get<operator_id>(op) != operator_id::times)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);

	if (will_write && !target->is_lvalue())
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' requires an l-value operand", 0u, 0u);

	auto base_type = dynamic_cast<type::pointer *>(target->get_type().get())->get_base_type();
	switch (std::get<operator_id>(op)){
	case operator_id::increment:
		target->write_scalar(runtime, (target->read_scalar<unsigned __int64>(runtime) + base_type->get_size()));
		return target;
	case operator_id::decrement:
		target->write_scalar(runtime, (target->read_scalar<unsigned __int64>(runtime) - base_type->get_size()));
		return target;
	default:
		break;
	}

	auto base = std::make_shared<memory::lval_reference>(runtime, target->read_scalar<unsigned __int64>(runtime), base_type, nullptr);
	if (base == nullptr)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);

	target->traverse_attributes(runtime, [&](std::shared_ptr<logic::attributes::object> attribute){//Copy applicable attributes
		if (auto base_attribute = attribute->get_pointer_target(runtime); base_attribute != nullptr)
			base->add_attribute(base_attribute);
	}, logic::attributes::object::stage_type::nil);

	return base;
}

std::shared_ptr<cminus::memory::reference> cminus::evaluator::pointer::evaluate_unary_right(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const{
	if (target == nullptr || !std::holds_alternative<operator_id>(op) || (std::get<operator_id>(op) != operator_id::increment && std::get<operator_id>(op) != operator_id::decrement))
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);

	if (!target->is_lvalue())
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' requires an l-value operand", 0u, 0u);

	unsigned __int64 computed_value;
	auto base_type = dynamic_cast<type::pointer *>(target->get_type().get())->get_base_type();

	switch (std::get<operator_id>(op)){
	case operator_id::increment:
		computed_value = (target->read_scalar<unsigned __int64>(runtime) + base_type->get_size());
		target->write_scalar(runtime, computed_value);
		return runtime.global_storage->create_scalar(computed_value, target->get_type());
	case operator_id::decrement:
		computed_value = (target->read_scalar<unsigned __int64>(runtime) - base_type->get_size());
		target->write_scalar(runtime, computed_value);
		return runtime.global_storage->create_scalar(computed_value, target->get_type());
	default:
		break;
	}

	return nullptr;
}

std::shared_ptr<cminus::memory::reference> cminus::evaluator::pointer::evaluate_binary(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> left_value, const operand_type &right) const{
	if (auto result = explicit_comparison::evaluate_(runtime, op, left_value, right); result != nullptr)//Handled
		return result;

	if (auto result = assignment::evaluate_(runtime, op, left_value, right); result != nullptr)//Handled
		return result;

	if (left_value == nullptr || !std::holds_alternative<operator_id>(op))
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	auto left_type = left_value->get_type();
	if (left_type == nullptr)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	if (std::get<operator_id>(op) == operator_id::member_pointer_access){
		auto base = std::make_shared<memory::lval_reference>(runtime, left_value->read_scalar<unsigned __int64>(runtime), dynamic_cast<type::pointer *>(left_type.get())->get_base_type(), nullptr);
		if (base == nullptr)
			throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);

		left_value->traverse_attributes(runtime, [&](std::shared_ptr<logic::attributes::object> attribute){//Copy applicable attributes
			if (auto base_attribute = attribute->get_pointer_target(runtime); base_attribute != nullptr)
				base->add_attribute(base_attribute);
		}, logic::attributes::object::stage_type::nil);

		return right->lookup(runtime, base);
	}

	auto requires_integral_right = (std::get<operator_id>(op) == operator_id::plus || std::get<operator_id>(op) == operator_id::minus || std::get<operator_id>(op) == operator_id::index);
	if (!requires_integral_right && !object::operator_is_relational(std::get<operator_id>(op)))
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	auto right_value = object::convert_operand_to_memory_reference(runtime, right);
	if (right_value == nullptr)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	auto right_type = right_value->get_type();
	if (right_type == nullptr)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	if (requires_integral_right){
		auto right_primitive_type = dynamic_cast<type::primitive *>(right_type.get());
		if (right_primitive_type == nullptr)
			throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

		unsigned __int64 offset = 0;
		switch (right_primitive_type->get_id()){
		case type::primitive::id_type::int16_:
			offset = (right_value->is_nan() ? 0u : static_cast<unsigned __int64>(right_value->read_scalar<__int16>(runtime)));
			break;
		case type::primitive::id_type::uint16_:
			offset = (right_value->is_nan() ? 0u : static_cast<unsigned __int64>(right_value->read_scalar<unsigned __int16>(runtime)));
			break;
		case type::primitive::id_type::int32_:
			offset = (right_value->is_nan() ? 0u : static_cast<unsigned __int64>(right_value->read_scalar<__int32>(runtime)));
			break;
		case type::primitive::id_type::uint32_:
			offset = (right_value->is_nan() ? 0u : static_cast<unsigned __int64>(right_value->read_scalar<unsigned __int32>(runtime)));
			break;
		case type::primitive::id_type::int64_:
			offset = (right_value->is_nan() ? 0u : static_cast<unsigned __int64>(right_value->read_scalar<__int64>(runtime)));
			break;
		case type::primitive::id_type::uint64_:
			offset = (right_value->is_nan() ? 0u : right_value->read_scalar<unsigned __int16>(runtime));
			break;
		default:
			throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);
			break;
		}

		unsigned __int64 computed_value;
		auto base_type = dynamic_cast<type::pointer *>(left_type.get())->get_base_type();

		switch (std::get<operator_id>(op)){
		case operator_id::increment:
			computed_value = (left_value->read_scalar<unsigned __int64>(runtime) + (base_type->get_size() * offset));
			left_value->write_scalar(runtime, computed_value);
			return runtime.global_storage->create_scalar(computed_value, left_type);
		case operator_id::decrement:
			computed_value = (left_value->read_scalar<unsigned __int64>(runtime) - (base_type->get_size() * offset));
			left_value->write_scalar(runtime, computed_value);
			return runtime.global_storage->create_scalar(computed_value, left_type);
		default:
			break;
		}

		auto base = std::make_shared<memory::lval_reference>(runtime, (left_value->read_scalar<unsigned __int64>(runtime) + (base_type->get_size() * offset)), base_type, nullptr);
		if (base == nullptr)
			throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

		left_value->traverse_attributes(runtime, [&](std::shared_ptr<logic::attributes::object> attribute){//Copy applicable attributes
			if (auto base_attribute = attribute->get_pointer_target(runtime); base_attribute != nullptr)
				base->add_attribute(base_attribute);
		}, logic::attributes::object::stage_type::nil);

		return base;
	}

	std::shared_ptr<memory::reference> cast_left_value, cast_right_value;
	if ((cast_right_value = right_type->cast(runtime, right_value, left_type, type::object::cast_type::rval_static)) == nullptr){
		if ((cast_left_value = left_type->cast(runtime, left_value, right_type, type::object::cast_type::rval_static)) != nullptr)
			cast_right_value = right_value;
		else//Both casts failed
			throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);
	}
	else//Cast successful
		cast_left_value = left_value;

	auto result = boolean::evaluate_<unsigned __int64>(runtime, std::get<operator_id>(op), cast_left_value, cast_right_value);
	if (result == nullptr)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	return result;
}
