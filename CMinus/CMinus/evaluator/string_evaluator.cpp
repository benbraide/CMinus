#include "../type/string_type.h"

#include "string_evaluator.h"

cminus::evaluator::string::~string() = default;

std::shared_ptr<cminus::memory::reference> cminus::evaluator::string::evaluate_unary_left(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const{
	throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);
	return nullptr;
}

std::shared_ptr<cminus::memory::reference> cminus::evaluator::string::evaluate_unary_right(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> target) const{
	throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operand", 0u, 0u);
	return nullptr;
}

std::shared_ptr<cminus::memory::reference> cminus::evaluator::string::evaluate_binary(logic::runtime &runtime, const operator_type &op, std::shared_ptr<memory::reference> left_value, const operand_type &right) const{
	if (auto result = explicit_comparison::evaluate_(runtime, op, left_value, right); result != nullptr)//Handled
		return result;

	if (auto result = compound_assignment::evaluate_(runtime, op, left_value, right); result != nullptr)//Handled
		return result;

	if (left_value == nullptr || !std::holds_alternative<operator_id>(op))
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	auto requires_integral_right = (std::get<operator_id>(op) == operator_id::index);
	if (!requires_integral_right && std::get<operator_id>(op) != operator_id::assignment && std::get<operator_id>(op) != operator_id::plus && !operator_is_relational(std::get<operator_id>(op)))
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	auto right_value = object::convert_operand_to_memory_reference(runtime, right);
	if (right_value == nullptr)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	auto left_type = left_value->get_type(), right_type = right_value->get_type();
	if (left_type == nullptr || right_type == nullptr)
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

		logic::attributes::collection::list_type attributes;
		if (left_value->has_attribute("ReadOnly", true))
			attributes.push_back(runtime.global_storage->find_attribute("ReadOnly", false));

		return std::make_shared<memory::lval_reference>(
			runtime,
			(runtime.global_storage->get_member_reference(runtime, left_value, "data_")->read_scalar<unsigned __int64>(runtime) + offset),
			runtime.global_storage->get_primitve_type(type::primitive::id_type::char_),
			attributes,
			nullptr
		);
	}

	if ((right_value = right_type->cast(runtime, right_value, left_type, type::object::cast_type::rval_static)) == nullptr)
		throw logic::exception("Cannot assign object to destination type", 0u, 0u);

	if (std::get<operator_id>(op) == operator_id::assignment){//Copy right string's content
		auto data = runtime.global_storage->get_member_reference(runtime, left_value, "data_");
		auto size = runtime.global_storage->get_member_reference(runtime, left_value, "size_");

		auto right_data_value = runtime.global_storage->get_string_data(runtime, right_value);
		auto right_size_value = strlen(right_data_value);

		std::size_t new_data_address;
		if (right_size_value != size->read_scalar<std::size_t>(runtime)){//Reallocate buffer
			auto new_data_block = runtime.memory_object.allocate_block((right_size_value + 1u), 0u);
			if (new_data_block == nullptr)
				throw memory::exception(memory::error_code::allocation_failure, 0u);

			if ((new_data_address = new_data_block->get_address()) == 0u)
				throw memory::exception(memory::error_code::allocation_failure, 0u);

			if (auto data_address = data->read_scalar<unsigned __int64>(runtime); data_address != 0u)//Free
				runtime.memory_object.deallocate_block(data_address);

			data->write_scalar(runtime, new_data_address);//Copy new address
			size->write_scalar(runtime, right_size_value);//Update size
		}
		else//Reuse buffer
			new_data_address = data->read_scalar<unsigned __int64>(runtime);

		runtime.memory_object.write(new_data_address, reinterpret_cast<const std::byte *>(right_data_value), right_size_value);
		runtime.memory_object.write_scalar((new_data_address + right_size_value), '\0');

		return left_value;
	}

	auto left_data_value = runtime.global_storage->get_string_data(runtime, left_value);
	auto right_data_value = runtime.global_storage->get_string_data(runtime, right_value);

	if (std::get<operator_id>(op) == operator_id::plus)//Concatenate two strings
		return runtime.global_storage->create_string(runtime, (std::string(left_data_value) + right_data_value));

	auto left_size_value = strlen(left_data_value);
	auto right_size_value = strlen(right_data_value);

	constant_type result;
	switch (std::get<operator_id>(op)){
	case operator_id::less:
		result = ((compare_strings_(left_data_value, right_data_value, left_size_value, right_size_value) < 0) ? constant_type::true_ : constant_type::false_);
		break;
	case operator_id::less_or_equal:
		result = ((compare_strings_(left_data_value, right_data_value, left_size_value, right_size_value) <= 0) ? constant_type::true_ : constant_type::false_);
		break;
	case operator_id::equality:
		result = ((compare_strings_(left_data_value, right_data_value, left_size_value, right_size_value) == 0) ? constant_type::true_ : constant_type::false_);
		break;
	case operator_id::inverse_equality:
		result = ((compare_strings_(left_data_value, right_data_value, left_size_value, right_size_value) != 0) ? constant_type::true_ : constant_type::false_);
		break;
	case operator_id::more_or_equal:
		result = ((compare_strings_(left_data_value, right_data_value, left_size_value, right_size_value) >= 0) ? constant_type::true_ : constant_type::false_);
		break;
	case operator_id::more:
		result = ((compare_strings_(left_data_value, right_data_value, left_size_value, right_size_value) > 0) ? constant_type::true_ : constant_type::false_);
		break;
	default:
		result = constant_type::nil;
		break;
	}

	if (result == constant_type::nil)
		throw logic::exception("Operator '" + object::convert_operator_to_string(op) + "' does not take the specified operands", 0u, 0u);

	return runtime.global_storage->get_named_constant(result);
}

int cminus::evaluator::string::compare_strings_(const char *left, const char *right, std::size_t left_size, std::size_t right_size) const{
	if (left_size == right_size && left_size == 0u)
		return 0;//Equal

	if (left_size == 0u)
		return -1;//Less

	if (right_size == 0u)
		return 1;//Greater

	auto result = strncmp(left, right, ((left_size < right_size) ? left_size : right_size));
	switch (result){
	case 0:
		if (left_size == right_size)
			return 0;//Equal characters and sizes
		return ((left_size < right_size) ? -1 : 1);//Equal characters, different sizes
	default:
		break;
	}

	return result;
}
