#include "../logic/runtime.h"

cminus::type::primitive::primitive(id_type id)
	: named_object(convert_id_to_string(id), nullptr), id_(id){}

cminus::type::primitive::~primitive() = default;

std::size_t cminus::type::primitive::get_size() const{
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

std::size_t cminus::type::primitive::compute_base_offset(const type::object &target) const{
	return 0u;
}

cminus::type::object::score_result_type cminus::type::primitive::get_score(const type::object &target, bool is_ref) const{
	auto type_target = dynamic_cast<const primitive *>(&target);
	if (type_target == nullptr){//Check for pointer
		if (type_target == nullptr)
			return score_result_type::nil;
	}

	if (type_target->id_ == id_)
		return score_result_type::exact;

	if (is_ref)
		return score_result_type::nil;

	switch (id_){
	case id_type::bool_:
	case id_type::byte_:
	case id_type::char_:
	case id_type::wchar_:
		return score_result_type::nil;
	case id_type::nan_:
		return score_result_type::assignable;
	default:
		break;
	};

	if (is_integral()){
		if (type_target->is_integral())
			return ((id_ < type_target->id_) ? score_result_type::shortened : score_result_type::widened);
		return (type_target->is_floating_point() ? score_result_type::too_shortened : score_result_type::nil);
	}

	if (is_floating_point()){
		if (type_target->is_floating_point())
			return ((id_ < type_target->id_) ? score_result_type::shortened : score_result_type::widened);
		return (type_target->is_integral() ? score_result_type::too_widened : score_result_type::nil);
	}

	return score_result_type::nil;
}

std::shared_ptr<cminus::memory::reference> cminus::type::primitive::convert_value(logic::runtime &runtime, std::shared_ptr<memory::reference> data, std::shared_ptr<type::object> target_type, bool is_ref) const{
	if (is_ref)
		return nullptr;

	if (data->get_address() == 0u)//Reference with value
		return convert_value(runtime, data->get_data(runtime), target_type);

	auto primitive_target_type = dynamic_cast<const primitive *>(target_type.get());
	if (primitive_target_type == nullptr)
		return nullptr;

	switch (primitive_target_type->id_){
	case id_type::int8_:
		return std::make_shared<memory::reference_with_value<__int8>>(target_type, nullptr, convert_source_<__int8>(runtime.memory_object, data->get_address(), *primitive_target_type));
	case id_type::uint8_:
		return std::make_shared<memory::reference_with_value<unsigned __int8>>(target_type, nullptr, convert_source_<unsigned __int8>(runtime.memory_object, data->get_address(), *primitive_target_type));
	case id_type::int16_:
		return std::make_shared<memory::reference_with_value<__int16>>(target_type, nullptr, convert_source_<__int16>(runtime.memory_object, data->get_address(), *primitive_target_type));
	case id_type::uint16_:
		return std::make_shared<memory::reference_with_value<unsigned __int16>>(target_type, nullptr, convert_source_<unsigned __int16>(runtime.memory_object, data->get_address(), *primitive_target_type));
	case id_type::int32_:
		return std::make_shared<memory::reference_with_value<__int32>>(target_type, nullptr, convert_source_<__int32>(runtime.memory_object, data->get_address(), *primitive_target_type));
	case id_type::uint32_:
		return std::make_shared<memory::reference_with_value<unsigned __int32>>(target_type, nullptr, convert_source_<unsigned __int32>(runtime.memory_object, data->get_address(), *primitive_target_type));
	case id_type::int64_:
		return std::make_shared<memory::reference_with_value<__int64>>(target_type, nullptr, convert_source_<__int64>(runtime.memory_object, data->get_address(), *primitive_target_type));
	case id_type::uint64_:
		return std::make_shared<memory::reference_with_value<unsigned __int64>>(target_type, nullptr, convert_source_<unsigned __int64>(runtime.memory_object, data->get_address(), *primitive_target_type));
	case id_type::float_:
		return std::make_shared<memory::reference_with_value<float>>(target_type, nullptr, convert_source_<float>(runtime.memory_object, data->get_address(), *primitive_target_type));
	case id_type::double_:
		return std::make_shared<memory::reference_with_value<double>>(target_type, nullptr, convert_source_<double>(runtime.memory_object, data->get_address(), *primitive_target_type));
	case id_type::ldouble:
		return std::make_shared<memory::reference_with_value<long double>>(target_type, nullptr, convert_source_<long double>(runtime.memory_object, data->get_address(), *primitive_target_type));
	default:
		break;
	}

	return nullptr;
}

std::shared_ptr<cminus::memory::reference> cminus::type::primitive::convert_value(logic::runtime &runtime, const std::byte *data, std::shared_ptr<type::object> target_type) const{
	if (data == nullptr)//Data required
		return nullptr;

	auto primitive_target_type = dynamic_cast<const primitive *>(target_type.get());
	if (primitive_target_type == nullptr)
		return nullptr;

	switch (primitive_target_type->id_){
	case id_type::int8_:
		return std::make_shared<memory::reference_with_value<__int8>>(target_type, nullptr, convert_source_<__int8>(runtime.memory_object, data, *primitive_target_type));
	case id_type::uint8_:
		return std::make_shared<memory::reference_with_value<unsigned __int8>>(target_type, nullptr, convert_source_<unsigned __int8>(runtime.memory_object, data, *primitive_target_type));
	case id_type::int16_:
		return std::make_shared<memory::reference_with_value<__int16>>(target_type, nullptr, convert_source_<__int16>(runtime.memory_object, data, *primitive_target_type));
	case id_type::uint16_:
		return std::make_shared<memory::reference_with_value<unsigned __int16>>(target_type, nullptr, convert_source_<unsigned __int16>(runtime.memory_object, data, *primitive_target_type));
	case id_type::int32_:
		return std::make_shared<memory::reference_with_value<__int32>>(target_type, nullptr, convert_source_<__int32>(runtime.memory_object, data, *primitive_target_type));
	case id_type::uint32_:
		return std::make_shared<memory::reference_with_value<unsigned __int32>>(target_type, nullptr, convert_source_<unsigned __int32>(runtime.memory_object, data, *primitive_target_type));
	case id_type::int64_:
		return std::make_shared<memory::reference_with_value<__int64>>(target_type, nullptr, convert_source_<__int64>(runtime.memory_object, data, *primitive_target_type));
	case id_type::uint64_:
		return std::make_shared<memory::reference_with_value<unsigned __int64>>(target_type, nullptr, convert_source_<unsigned __int64>(runtime.memory_object, data, *primitive_target_type));
	case id_type::float_:
		return std::make_shared<memory::reference_with_value<float>>(target_type, nullptr, convert_source_<float>(runtime.memory_object, data, *primitive_target_type));
	case id_type::double_:
		return std::make_shared<memory::reference_with_value<double>>(target_type, nullptr, convert_source_<double>(runtime.memory_object, data, *primitive_target_type));
	case id_type::ldouble:
		return std::make_shared<memory::reference_with_value<long double>>(target_type, nullptr, convert_source_<long double>(runtime.memory_object, data, *primitive_target_type));
	default:
		break;
	}

	return nullptr;
}

bool cminus::type::primitive::is_same(const logic::naming::object &target) const{
	auto type_target = dynamic_cast<const primitive *>(&target);
	return (type_target != nullptr && type_target->id_ == id_);
}

cminus::type::primitive::id_type cminus::type::primitive::get_id() const{
	return id_;
}

bool cminus::type::primitive::is_numeric() const{
	return (id_type::int8_ <= id_ && id_ <= id_type::ldouble);
}

bool cminus::type::primitive::is_integral() const{
	return (id_type::int8_ <= id_ && id_ <= id_type::uint128_);
}

bool cminus::type::primitive::is_unsigned_integral() const{
	switch (id_){
	case id_type::uint8_:
	case id_type::uint16_:
	case id_type::uint32_:
	case id_type::uint64_:
	case id_type::uint128_:
		return true;
	default:
		break;
	}

	return false;
}

bool cminus::type::primitive::is_floating_point() const{
	return (id_type::float_ <= id_ && id_ <= id_type::ldouble);
}

cminus::type::primitive::id_type cminus::type::primitive::convert_string_to_id(const std::string &value){
	if (value == "nullptr_t")
		return id_type::nullptr_;

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

	if (value == "nan_t")
		return id_type::nan_;

	return id_type::nil;
}

std::string cminus::type::primitive::convert_id_to_string(id_type value){
	switch (value){
	case id_type::nullptr_:
		return "nullptr_t";
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
	case id_type::nan_:
		return "nan_t";
	default:
		break;
	}

	return "";
}
