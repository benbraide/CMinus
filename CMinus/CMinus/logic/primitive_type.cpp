#include "primitive_type.h"

cminus::logic::type::primitive::primitive(id_type id)
	: naming_base_type(convert_id_to_string(id)), id_(id){}

cminus::logic::type::primitive::~primitive() = default;

std::size_t cminus::logic::type::primitive::get_size() const{
	switch (id_){
	case id_type::bool_:
	case id_type::byte_:
	case id_type::char_:
		return sizeof(std::byte);
	case id_type::wchar_:
		return sizeof(wchar_t);
	case id_type::int8_:
	case id_type::uint8_:
		return sizeof(__int8);
	case id_type::int16_:
	case id_type::uint16_:
		return sizeof(__int16);
	case id_type::int32_:
	case id_type::uint32_:
		return sizeof(__int32);
	case id_type::int64_:
	case id_type::uint64_:
		return sizeof(__int64);
	case id_type::int128_:
	case id_type::uint128_:
		return 16u;
	case id_type::float_:
		return sizeof(float);
	case id_type::double_:
		return sizeof(double);
	case id_type::ldouble:
		return sizeof(long double);
	default:
		break;
	}

	return 0u;
}

cminus::logic::type::object::score_result_type cminus::logic::type::primitive::get_score(const type::object &target) const{
	auto type_target = dynamic_cast<const primitive *>(&target);
	if (type_target == nullptr){//Check for composite
		if (type_target == nullptr)
			return score_result_type::nil;
	}

	if (type_target->id_ == id_)
		return score_result_type::exact;

	switch (id_){
	case id_type::bool_:
	case id_type::byte_:
	case id_type::char_:
	case id_type::wchar_:
		return score_result_type::nil;
	default:
		break;
	};

	if (is_integer()){
		if (type_target->is_integer())
			return ((id_ < type_target->id_) ? score_result_type::shortened : score_result_type::widened);
		return (type_target->is_floating_point() ? score_result_type::too_shortened : score_result_type::nil);
	}

	if (is_floating_point()){
		if (type_target->is_floating_point())
			return ((id_ < type_target->id_) ? score_result_type::shortened : score_result_type::widened);
		return (type_target->is_integer() ? score_result_type::too_widened : score_result_type::nil);
	}

	return score_result_type::nil;
}

std::shared_ptr<cminus::memory::reference> cminus::logic::type::primitive::convert_value(memory::object &memory_object, std::size_t address, std::shared_ptr<type::object> target_type) const{
	auto primitive_target_type = dynamic_cast<const primitive *>(target_type.get());
	if (primitive_target_type == nullptr)
		throw memory::exception(memory::error_code::incompatible_types, address);

	auto block = memory_object.find_block(address);
	if (block == nullptr)
		throw memory::exception(memory::error_code::block_not_found, address);

	auto new_ref = std::make_shared<memory::reference>(memory_object, target_type, memory::reference::attribute_constant);
	if (new_ref == nullptr || new_ref->get_address() == 0u)
		throw memory::exception(memory::error_code::allocation_failure, 0u);

	auto new_block = memory_object.find_block(new_ref->get_address());
	if (new_block == nullptr)//Shouldn't execute this block
		throw memory::exception(memory::error_code::block_not_found, new_ref->get_address());

	switch (primitive_target_type->id_){
	case id_type::int8_:
		new_block->write_scalar(0u, convert_source_<__int8>(*block, *primitive_target_type));
		break;
	case id_type::uint8_:
		new_block->write_scalar(0u, convert_source_<unsigned __int8>(*block, *this));
		break;
	case id_type::int16_:
		new_block->write_scalar(0u, convert_source_<__int16>(*block, *this));
		break;
	case id_type::uint16_:
		new_block->write_scalar(0u, convert_source_<unsigned __int16>(*block, *this));
		break;
	case id_type::int32_:
		new_block->write_scalar(0u, convert_source_<__int32>(*block, *this));
		break;
	case id_type::uint32_:
		new_block->write_scalar(0u, convert_source_<unsigned __int32>(*block, *this));
		break;
	case id_type::int64_:
		new_block->write_scalar(0u, convert_source_<__int64>(*block, *this));
		break;
	case id_type::uint64_:
		new_block->write_scalar(0u, convert_source_<unsigned __int64>(*block, *this));
		break;
	case id_type::float_:
		new_block->write_scalar(0u, convert_source_<float>(*block, *this));
		break;
	case id_type::double_:
		new_block->write_scalar(0u, convert_source_<double>(*block, *this));
		break;
	case id_type::ldouble:
		new_block->write_scalar(0u, convert_source_<long double>(*block, *this));
		break;
	default:
		throw memory::exception(memory::error_code::incompatible_types, address);
		break;
	}

	new_block->set_attributes(new_block->get_attributes() | memory::block::attribute_write_protected);//Prevent writes
	return new_ref;
}

std::string cminus::logic::type::primitive::get_qualified_naming_value() const{
	return value_;
}

void cminus::logic::type::primitive::print(io::writer &writer, bool is_qualified) const{
	writer.write_buffer(value_.data(), value_.size());
}

bool cminus::logic::type::primitive::is_same(const naming::object &target) const{
	auto type_target = dynamic_cast<const primitive *>(&target);
	return (type_target != nullptr && type_target->id_ == id_);
}

cminus::logic::type::primitive::id_type cminus::logic::type::primitive::get_id() const{
	return id_;
}

bool cminus::logic::type::primitive::is_integer() const{
	return (id_type::int8_ <= id_ && id_ <= id_type::uint128_);
}

bool cminus::logic::type::primitive::is_floating_point() const{
	return (id_type::float_ <= id_ && id_ <= id_type::ldouble);
}

cminus::logic::type::primitive::id_type cminus::logic::type::primitive::convert_string_to_id(const std::string &value){
	if (value == "bool")
		return id_type::bool_;

	if (value == "byte")
		return id_type::byte_;

	if (value == "char")
		return id_type::char_;

	if (value == "wchar")
		return id_type::wchar_;

	if (value == "int8_t")
		return id_type::int8_;

	if (value == "uint8_t")
		return id_type::uint8_;

	if (value == "int16_t")
		return id_type::int16_;

	if (value == "uint16_t")
		return id_type::uint16_;

	if (value == "int32_t")
		return id_type::int32_;

	if (value == "uint32_t")
		return id_type::uint32_;

	if (value == "int64_t")
		return id_type::int64_;

	if (value == "uint64_t")
		return id_type::uint64_;

	if (value == "int128_t")
		return id_type::int128_;

	if (value == "uint128_t")
		return id_type::uint128_;

	if (value == "float32_t")
		return id_type::float_;

	if (value == "float64_t")
		return id_type::double_;

	if (value == "float128_t")
		return id_type::ldouble;

	return id_type::nil;
}

std::string cminus::logic::type::primitive::convert_id_to_string(id_type value){
	switch (value){
	case id_type::bool_:
		return "bool";
	case id_type::byte_:
		return "byte";
	case id_type::char_:
		return "char";
	case id_type::wchar_:
		return "wchar";
	case id_type::int8_:
		return "int8_t";
	case id_type::uint8_:
		return "uint8_t";
	case id_type::int16_:
		return "int16_t";
	case id_type::uint16_:
		return "uint16_t";
	case id_type::int32_:
		return "int32_t";
	case id_type::uint32_:
		return "uint32_t";
	case id_type::int64_:
		return "int64_t";
	case id_type::uint64_:
		return "uint64_t";
	case id_type::int128_:
		return "int128_t";
	case id_type::uint128_:
		return "uint128_t";
	case id_type::float_:
		return "float32_t";
	case id_type::double_:
		return "float64_t";
	case id_type::ldouble:
		return "float128_t";
	default:
		break;
	}

	return "";
}
