#include "../logic/string_conversions.h"
#include "../evaluator/evaluator_id.h"
#include "../declaration/function_declaration_group.h"

#include "string_type.h"
#include "pointer_type.h"

cminus::type::primitive::primitive(id_type id)
	: named_object(convert_id_to_string(id), nullptr), id_(id){}

cminus::type::primitive::~primitive() = default;

bool cminus::type::primitive::is_auto() const{
	return (id_ == id_type::auto_);
}

void cminus::type::primitive::print_value(logic::runtime &runtime, std::shared_ptr<memory::reference> data) const{
	if (is_numeric() && data->has_attribute("#NaN#", true)){
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
	case id_type::int16_:
		runtime.writer.write_scalar(logic::to_string<__int16>::get(data->read_scalar<__int16>(runtime)) + "H");
		break;
	case id_type::uint16_:
		runtime.writer.write_scalar(logic::to_string<unsigned __int16>::get(data->read_scalar<unsigned __int16>(runtime)) + "UH");
		break;
	case id_type::int32_:
		runtime.writer.write_scalar(logic::to_string<__int32>::get(data->read_scalar<__int32>(runtime)));
		break;
	case id_type::uint32_:
		runtime.writer.write_scalar(logic::to_string<unsigned __int32>::get(data->read_scalar<unsigned __int32>(runtime)));
		break;
	case id_type::int64_:
		runtime.writer.write_scalar(logic::to_string<__int64>::get(data->read_scalar<__int64>(runtime)) + "L");
		break;
	case id_type::uint64_:
		runtime.writer.write_scalar(logic::to_string<unsigned __int64>::get(data->read_scalar<unsigned __int64>(runtime)) + "LL");
		break;
	case id_type::float_:
		runtime.writer.write_scalar(logic::to_string<float>::get(data->read_scalar<float>(runtime)) + "F");
		break;
	case id_type::double_:
		runtime.writer.write_scalar(logic::to_string<double>::get(data->read_scalar<double>(runtime)));
		break;
	case id_type::ldouble:
		runtime.writer.write_scalar(logic::to_string<long double>::get(data->read_scalar<long double>(runtime)) + "L");
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

bool cminus::type::primitive::is_exact(logic::runtime &runtime, const type::object &target) const{
	auto primitive_target = dynamic_cast<const primitive *>(&target);
	return (primitive_target != nullptr && primitive_target->id_ == id_);
}

cminus::type::object::score_result_type cminus::type::primitive::get_score(logic::runtime &runtime, const type::object &target, bool is_ref) const{
	if (converts_auto(target))
		return score_result_type::assignable;

	auto type_target = dynamic_cast<const primitive *>(&target);
	if (type_target == nullptr){//Check for string
		if (is_ref)
			return score_result_type::nil;

		if (id_ == id_type::nullptr_ && dynamic_cast<const pointer *>(&target) != nullptr)
			return score_result_type::assignable;

		if (is_numeric() && dynamic_cast<const string *>(&target) != nullptr)
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
		return runtime.global_storage->create_scalar(static_cast<std::byte>(0));
	case id_type::char_:
		return runtime.global_storage->create_scalar(static_cast<char>(0));
	case id_type::wchar_:
		return runtime.global_storage->create_scalar(static_cast<wchar_t>(0));
	case id_type::int16_:
		return runtime.global_storage->create_scalar(static_cast<__int16>(0));
	case id_type::uint16_:
		return runtime.global_storage->create_scalar(static_cast<unsigned __int16>(0));
	case id_type::int32_:
		return runtime.global_storage->create_scalar(static_cast<__int32>(0));
	case id_type::uint32_:
		return runtime.global_storage->create_scalar(static_cast<unsigned __int32>(0));
	case id_type::int64_:
		return runtime.global_storage->create_scalar(static_cast<__int64>(0));
	case id_type::uint64_:
		return runtime.global_storage->create_scalar(static_cast<unsigned __int64>(0));
	case id_type::float_:
		return runtime.global_storage->create_scalar(static_cast<float>(0));
	case id_type::double_:
		return runtime.global_storage->create_scalar(static_cast<double>(0));
	case id_type::ldouble:
		return runtime.global_storage->create_scalar(static_cast<long double>(0));
	case id_type::nan_:
		return runtime.global_storage->get_named_constant(node::named_constant::constant_type::nan_);
	default:
		break;
	}

	return nullptr;
}

std::shared_ptr<cminus::memory::reference> cminus::type::primitive::cast(logic::runtime &runtime, std::shared_ptr<memory::reference> data, std::shared_ptr<type::object> target_type, cast_type type) const{
	if (type != cast_type::static_ && type != cast_type::ref_static && type != cast_type::rval_static && type != cast_type::reinterpret)
		return nullptr;//Not supported

	auto primitive_target_type = dynamic_cast<primitive *>(target_type.get());
	if (primitive_target_type == nullptr){//Check for pointer or function
		auto pointer_target_type = dynamic_cast<pointer *>(target_type.get());
		if (id_ == id_type::nullptr_ && pointer_target_type != nullptr && (type == cast_type::static_ || type == cast_type::rval_static))
			return runtime.global_storage->create_scalar(data->read_scalar<unsigned __int64>(runtime));

		if (type == cast_type::reinterpret && pointer_target_type != nullptr && is_integral()){
			if (data->has_attribute("#NaN#", true))
				return runtime.global_storage->create_scalar(0ui64);
			return runtime.global_storage->create_scalar(cast_integral<unsigned __int64>(runtime, *data));
		}

		if ((type == cast_type::static_ || type == cast_type::rval_static) && dynamic_cast<string *>(target_type.get()) != nullptr){
			if (is_numeric() && data->has_attribute("#NaN#", true))
				return runtime.global_storage->create_string(runtime, 3u, "NaN");

			switch (id_){
			case id_type::nan_:
				return runtime.global_storage->create_string(runtime, 3u, "NaN");
			case id_type::int16_:
				return runtime.global_storage->create_string(runtime, logic::to_string<__int16>::get(data->read_scalar<__int16>(runtime)));
			case id_type::uint16_:
				return runtime.global_storage->create_string(runtime, logic::to_string<unsigned __int16>::get(data->read_scalar<unsigned __int16>(runtime)));
			case id_type::int32_:
				return runtime.global_storage->create_string(runtime, logic::to_string<__int32>::get(data->read_scalar<__int32>(runtime)));
			case id_type::uint32_:
				return runtime.global_storage->create_string(runtime, logic::to_string<unsigned __int32>::get(data->read_scalar<unsigned __int32>(runtime)));
			case id_type::int64_:
				return runtime.global_storage->create_string(runtime, logic::to_string<__int64>::get(data->read_scalar<__int64>(runtime)));
			case id_type::uint64_:
				return runtime.global_storage->create_string(runtime, logic::to_string<unsigned __int64>::get(data->read_scalar<unsigned __int64>(runtime)));
			case id_type::float_:
				return runtime.global_storage->create_string(runtime, logic::to_string<float>::get(data->read_scalar<float>(runtime)));
			case id_type::double_:
				return runtime.global_storage->create_string(runtime, logic::to_string<double>::get(data->read_scalar<double>(runtime)));
			case id_type::ldouble:
				return runtime.global_storage->create_string(runtime, logic::to_string<long double>::get(data->read_scalar<long double>(runtime)));
			default:
				break;
			}
		}

		return nullptr;
	}

	if (type == cast_type::reinterpret){
		if (id_ == id_type::function){
			switch (primitive_target_type->id_){
			case id_type::int16_:
				return runtime.global_storage->create_scalar(static_cast<__int16>(data->read_scalar<unsigned __int64>(runtime)));
			case id_type::uint16_:
				return runtime.global_storage->create_scalar(static_cast<unsigned __int16>(data->read_scalar<unsigned __int64>(runtime)));
			case id_type::int32_:
				return runtime.global_storage->create_scalar(static_cast<__int32>(data->read_scalar<unsigned __int64>(runtime)));
			case id_type::uint32_:
				return runtime.global_storage->create_scalar(static_cast<unsigned __int32>(data->read_scalar<unsigned __int64>(runtime)));
			case id_type::int64_:
				return runtime.global_storage->create_scalar(static_cast<__int64>(data->read_scalar<unsigned __int64>(runtime)));
			case id_type::uint64_:
				return runtime.global_storage->create_scalar(data->read_scalar<unsigned __int64>(runtime));
			default:
				break;
			}
			
			return nullptr;
		}

		if (is_integral() && primitive_target_type->id_ == id_type::function){
			if (data->has_attribute("#NaN#", true))
				return runtime.global_storage->create_scalar(0ui64);
			return runtime.global_storage->create_scalar(cast_integral<unsigned __int64>(runtime, *data));
		}

		return nullptr;
	}

	if (primitive_target_type->id_ == id_){//Same type
		if (type != cast_type::static_)
			return data;

		auto copy = std::make_shared<memory::data_reference>(target_type);
		if (copy != nullptr)//Copy bytes
			copy->write(runtime, *data, static_cast<std::size_t>(-1));

		return copy;
	}

	std::shared_ptr<memory::reference> value;
	try{
		switch (primitive_target_type->id_){
		case id_type::int16_:
			value = runtime.global_storage->create_scalar(cast_numeric<__int16>(runtime, *data));
			break;
		case id_type::uint16_:
			value = runtime.global_storage->create_scalar(cast_numeric<unsigned __int16>(runtime, *data));
			break;
		case id_type::int32_:
			value = runtime.global_storage->create_scalar(cast_numeric<__int32>(runtime, *data));
			break;
		case id_type::uint32_:
			value = runtime.global_storage->create_scalar(cast_numeric<unsigned __int32>(runtime, *data));
			break;
		case id_type::int64_:
			value = runtime.global_storage->create_scalar(cast_numeric<__int64>(runtime, *data));
			break;
		case id_type::uint64_:
			value = runtime.global_storage->create_scalar(cast_numeric<unsigned __int64>(runtime, *data));
			break;
		case id_type::float_:
			value = runtime.global_storage->create_scalar(cast_numeric<float>(runtime, *data));
			break;
		case id_type::double_:
			value = runtime.global_storage->create_scalar(cast_numeric<double>(runtime, *data));
			break;
		case id_type::ldouble:
			value = runtime.global_storage->create_scalar(cast_numeric<long double>(runtime, *data));
			break;
		default:
			break;
		}
	}
	catch (const memory::exception &e){
		if (e.get_code() == memory::error_code::incompatible_types)
			value = nullptr;
		else//Forward
			throw;
	}

	if (value == nullptr)
		return nullptr;

	if (id_ == id_type::nan_ || (is_numeric() && data->has_attribute("#NaN#", true)))
		value->add_attribute(runtime.global_storage->find_attribute("#NaN#", false));

	return value;
}

std::shared_ptr<cminus::evaluator::object> cminus::type::primitive::get_evaluator(logic::runtime &runtime) const{
	switch (id_){
	case id_type::bool_:
		return runtime.global_storage->get_evaluator(evaluator::id::boolean);
	case id_type::nan_:
		return nullptr;
	case id_type::byte_:
		return runtime.global_storage->get_evaluator(evaluator::id::byte);
	case id_type::char_:
	case id_type::wchar_:
		return runtime.global_storage->get_evaluator(evaluator::id::character);
	case id_type::int16_:
	case id_type::uint16_:
	case id_type::int32_:
	case id_type::uint32_:
	case id_type::int64_:
	case id_type::uint64_:
	case id_type::int128_:
	case id_type::uint128_:
		return runtime.global_storage->get_evaluator(evaluator::id::integral);
	case id_type::float_:
	case id_type::double_:
	case id_type::ldouble:
		return runtime.global_storage->get_evaluator(evaluator::id::floating_point);
	case id_type::nullptr_:
		return nullptr;
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
	return (id_type::int16_ <= id_ && id_ <= id_type::ldouble);
}

bool cminus::type::primitive::is_integral() const{
	return (id_type::int16_ <= id_ && id_ <= id_type::uint128_);
}

bool cminus::type::primitive::is_unsigned_integral() const{
	switch (id_){
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

	if (value == "float")
		return id_type::float_;

	if (value == "double")
		return id_type::double_;

	if (value == "float128_t")
		return id_type::ldouble;

	if (value == "nan_t")
		return id_type::nan_;

	if (value == "void")
		return id_type::void_;

	if (value == "function_t")
		return id_type::function;

	if (value == "auto")
		return id_type::auto_;

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
	case id_type::function:
		return "function_t";
	case id_type::auto_:
		return "auto";
	default:
		break;
	}

	return "";
}
