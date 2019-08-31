#include "../logic/runtime.h"

#include "pointer_type.h"
#include "function_type.h"

cminus::type::primitive::primitive(id_type id)
	: named_object(convert_id_to_string(id), nullptr), id_(id){}

cminus::type::primitive::~primitive() = default;

void cminus::type::primitive::print_value(logic::runtime &runtime, std::shared_ptr<memory::reference> data) const{
	if ((is_integral() || is_floating_point()) && data->find_attribute("#NaN#", true, false) != nullptr){
		runtime.writer.write_buffer("NaN", 3u);
		return;
	}

	switch (id_){
	case id_type::byte_:
		break;
	case id_type::char_:
		runtime.writer.write_scalar(data->read_scalar<char>(runtime));
		break;
	case id_type::wchar_:
		runtime.writer.write_scalar(data->read_scalar<wchar_t>(runtime));
		break;
	case id_type::int8_:
		runtime.writer.write_scalar(std::to_string(data->read_scalar<__int8>(runtime)) + "i8");
		break;
	case id_type::uint8_:
		runtime.writer.write_scalar(std::to_string(data->read_scalar<unsigned __int8>(runtime)) + "ui8");
		break;
	case id_type::int16_:
		runtime.writer.write_scalar(std::to_string(data->read_scalar<__int16>(runtime)) + "i16");
		break;
	case id_type::uint16_:
		runtime.writer.write_scalar(std::to_string(data->read_scalar<unsigned __int16>(runtime)) + "ui16");
		break;
	case id_type::int32_:
		runtime.writer.write_scalar(std::to_string(data->read_scalar<__int32>(runtime)) + "i32");
		break;
	case id_type::uint32_:
		runtime.writer.write_scalar(std::to_string(data->read_scalar<unsigned __int32>(runtime)) + "ui32");
		break;
	case id_type::int64_:
		runtime.writer.write_scalar(std::to_string(data->read_scalar<__int64>(runtime)) + "i64");
		break;
	case id_type::uint64_:
		runtime.writer.write_scalar(std::to_string(data->read_scalar<unsigned __int64>(runtime)) + "ui64");
		break;
	case id_type::float_:
		runtime.writer.write_scalar(std::to_string(data->read_scalar<float>(runtime)) + "f32");
		break;
	case id_type::double_:
		runtime.writer.write_scalar(std::to_string(data->read_scalar<double>(runtime)) + "f64");
		break;
	case id_type::ldouble:
		runtime.writer.write_scalar(std::to_string(data->read_scalar<long double>(runtime)) + "f128");
		break;
	case id_type::nan_:
		runtime.writer.write_buffer("NaN", 3u);
		break;
	case id_type::nullptr_:
		runtime.writer.write_buffer("nullptr", 7u);
		break;
	default:
		break;
	}

	if (id_ == id_type::bool_){
		switch (data->read_scalar<node::named_constant::constant_type>(runtime)){
		case node::named_constant::constant_type::indeterminate:
			runtime.writer.write_buffer("indeterminate", 13u);
			break;
		case node::named_constant::constant_type::false_:
			runtime.writer.write_buffer("false", 5u);
			break;
		case node::named_constant::constant_type::true_:
			runtime.writer.write_buffer("true", 4u);
			break;
		default:
			break;
		}
	}
}

std::size_t cminus::type::primitive::get_size() const{
	switch (id_){
	case id_type::bool_:
	case id_type::nan_:
		return sizeof(node::named_constant::constant_type);
	case id_type::byte_:
		return sizeof(std::byte);
	case id_type::char_:
		return sizeof(char);
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
	case id_type::nullptr_:
		return sizeof(nullptr);
	default:
		break;
	}

	return 0u;
}

cminus::type::object::score_result_type cminus::type::primitive::get_score(logic::runtime &runtime, const type::object &target, bool is_ref) const{
	auto type_target = dynamic_cast<const primitive *>(&target);
	if (type_target == nullptr){//Check for string
		if (is_ref)
			return score_result_type::nil;

		if (id_ == id_type::nullptr_ && dynamic_cast<const pointer *>(&target) != nullptr)
			return score_result_type::assignable;
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
	case id_type::nullptr_:
	case id_type::void_:
		return score_result_type::nil;
	case id_type::nan_:
		return (type_target->is_numeric() ? score_result_type::assignable : score_result_type::nil);
	default:
		break;
	};

	if (is_integral()){
		if (type_target->is_integral())
			return ((id_ < type_target->id_) ? score_result_type::widened : score_result_type::shortened);
		return (type_target->is_floating_point() ? score_result_type::too_widened : score_result_type::nil);
	}

	if (is_floating_point()){
		if (type_target->is_floating_point())
			return ((id_ < type_target->id_) ? score_result_type::widened : score_result_type::shortened);
		return (type_target->is_integral() ? score_result_type::too_shortened : score_result_type::nil);
	}

	return score_result_type::nil;
}

std::shared_ptr<cminus::memory::reference> cminus::type::primitive::get_default_value(logic::runtime &runtime) const{
	switch (id_){
	case id_type::nullptr_:
		return runtime.global_storage->get_named_constant(node::named_constant::constant_type::nullptr_);
	case id_type::bool_:
		return runtime.global_storage->get_named_constant(node::named_constant::constant_type::indeterminate);
	case id_type::byte_:
		return std::make_shared<memory::scalar_reference<std::byte>>(runtime.global_storage->get_primitve_type(type::primitive::id_type::byte_), nullptr, static_cast<std::byte>(0));
	case id_type::char_:
		return std::make_shared<memory::scalar_reference<char>>(runtime.global_storage->get_primitve_type(type::primitive::id_type::char_), nullptr, static_cast<char>(0));
	case id_type::wchar_:
		return std::make_shared<memory::scalar_reference<wchar_t>>(runtime.global_storage->get_primitve_type(type::primitive::id_type::wchar_), nullptr, static_cast<wchar_t>(0));
	case id_type::int8_:
		return std::make_shared<memory::scalar_reference<__int8>>(runtime.global_storage->get_primitve_type(type::primitive::id_type::int8_), nullptr, static_cast<__int8>(0));
	case id_type::uint8_:
		return std::make_shared<memory::scalar_reference<unsigned __int8>>(runtime.global_storage->get_primitve_type(type::primitive::id_type::uint8_), nullptr, static_cast<unsigned __int8>(0));
	case id_type::int16_:
		return std::make_shared<memory::scalar_reference<__int16>>(runtime.global_storage->get_primitve_type(type::primitive::id_type::int16_), nullptr, static_cast<__int16>(0));
	case id_type::uint16_:
		return std::make_shared<memory::scalar_reference<unsigned __int16>>(runtime.global_storage->get_primitve_type(type::primitive::id_type::uint16_), nullptr, static_cast<unsigned __int16>(0));
	case id_type::int32_:
		return std::make_shared<memory::scalar_reference<__int32>>(runtime.global_storage->get_primitve_type(type::primitive::id_type::int32_), nullptr, static_cast<__int32>(0));
	case id_type::uint32_:
		return std::make_shared<memory::scalar_reference<unsigned __int32>>(runtime.global_storage->get_primitve_type(type::primitive::id_type::uint32_), nullptr, static_cast<unsigned __int32>(0));
	case id_type::int64_:
		return std::make_shared<memory::scalar_reference<__int64>>(runtime.global_storage->get_primitve_type(type::primitive::id_type::int64_), nullptr, static_cast<__int64>(0));
	case id_type::uint64_:
		return std::make_shared<memory::scalar_reference<unsigned __int64>>(runtime.global_storage->get_primitve_type(type::primitive::id_type::uint64_), nullptr, static_cast<unsigned __int64>(0));
	case id_type::float_:
		return std::make_shared<memory::scalar_reference<float>>(runtime.global_storage->get_primitve_type(type::primitive::id_type::float_), nullptr, static_cast<float>(0));
	case id_type::double_:
		return std::make_shared<memory::scalar_reference<double>>(runtime.global_storage->get_primitve_type(type::primitive::id_type::double_), nullptr, static_cast<double>(0));
	case id_type::ldouble:
		return std::make_shared<memory::scalar_reference<long double>>(runtime.global_storage->get_primitve_type(type::primitive::id_type::ldouble), nullptr, static_cast<long double>(0));
	case id_type::nan_:
		return runtime.global_storage->get_named_constant(node::named_constant::constant_type::nan_);
	default:
		break;
	}

	return nullptr;
}

std::shared_ptr<cminus::memory::reference> cminus::type::primitive::cast(logic::runtime &runtime, std::shared_ptr<memory::reference> data, std::shared_ptr<type::object> target_type, cast_type type) const{
	if (type != cast_type::static_ && type != cast_type::reinterpret)
		return nullptr;//Not supported

	auto primitive_target_type = dynamic_cast<primitive *>(target_type.get());
	if (primitive_target_type == nullptr){//Check for pointer or function
		auto pointer_target_type = dynamic_cast<pointer *>(target_type.get());
		if (id_ == id_type::nullptr_ && type == cast_type::static_ && pointer_target_type != nullptr)
			return std::make_shared<memory::scalar_reference<unsigned __int64>>(target_type, nullptr, data->read_scalar<unsigned __int64>(runtime));

		if (is_integral() && type == cast_type::reinterpret && (pointer_target_type != nullptr || dynamic_cast<function *>(target_type.get()) != nullptr)){
			if (data->find_attribute("#NaN#", true, false) != nullptr)
				return std::make_shared<memory::scalar_reference<unsigned __int64>>(target_type, nullptr, 0ui64);
			return std::make_shared<memory::scalar_reference<unsigned __int64>>(target_type, nullptr, cast_integral<unsigned __int64>(runtime, *data));
		}

		return nullptr;
	}

	std::shared_ptr<memory::reference> value;
	switch (primitive_target_type->id_){
	case id_type::int8_:
		value = std::make_shared<memory::scalar_reference<__int8>>(target_type, nullptr, cast_numeric<__int8>(runtime, *data));
		break;
	case id_type::uint8_:
		value = std::make_shared<memory::scalar_reference<unsigned __int8>>(target_type, nullptr, cast_numeric<unsigned __int8>(runtime, *data));
		break;
	case id_type::int16_:
		value = std::make_shared<memory::scalar_reference<__int16>>(target_type, nullptr, cast_numeric<__int16>(runtime, *data));
		break;
	case id_type::uint16_:
		value = std::make_shared<memory::scalar_reference<unsigned __int16>>(target_type, nullptr, cast_numeric<unsigned __int16>(runtime, *data));
		break;
	case id_type::int32_:
		value = std::make_shared<memory::scalar_reference<__int32>>(target_type, nullptr, cast_numeric<__int32>(runtime, *data));
		break;
	case id_type::uint32_:
		value = std::make_shared<memory::scalar_reference<unsigned __int32>>(target_type, nullptr, cast_numeric<unsigned __int32>(runtime, *data));
		break;
	case id_type::int64_:
		value = std::make_shared<memory::scalar_reference<__int64>>(target_type, nullptr, cast_numeric<__int64>(runtime, *data));
		break;
	case id_type::uint64_:
		value = std::make_shared<memory::scalar_reference<unsigned __int64>>(target_type, nullptr, cast_numeric<unsigned __int64>(runtime, *data));
		break;
	case id_type::float_:
		value = std::make_shared<memory::scalar_reference<float>>(target_type, nullptr, cast_numeric<float>(runtime, *data));
		break;
	case id_type::double_:
		value = std::make_shared<memory::scalar_reference<double>>(target_type, nullptr, cast_numeric<double>(runtime, *data));
		break;
	case id_type::ldouble:
		value = std::make_shared<memory::scalar_reference<long double>>(target_type, nullptr, cast_numeric<long double>(runtime, *data));
		break;
	default:
		break;
	}

	if (value == nullptr)
		return nullptr;

	if (id_ == id_type::nan_)
		value->add_attribute(runtime.global_storage->find_attribute("#NaN#", false));
	else if (auto nan_attr = data->find_attribute("#NaN#", true, false); nan_attr != nullptr)
		value->add_attribute(nan_attr);

	return value;
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

	if (value == "void")
		return id_type::void_;

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
	case id_type::void_:
		return "void";
	default:
		break;
	}

	return "";
}
