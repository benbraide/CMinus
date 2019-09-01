#include "../logic/function_group.h"

cminus::type::function::function(const type_info &return_type, const std::vector<type_info> &parameter_types)
	: return_type_(return_type), parameter_types_(parameter_types){}

cminus::type::function::function(const type_info &return_type, std::vector<type_info> &&parameter_types)
	: return_type_(return_type), parameter_types_(std::move(parameter_types)){}

cminus::type::function::~function() = default;

void cminus::type::function::print(logic::runtime &runtime, bool is_qualified) const{
	print_(runtime, return_type_);
	runtime.writer.write_scalar('(');

	if (!parameter_types_.empty()){//Print parameters
		auto is_first = true;
		for (auto &parameter_type : parameter_types_){
			if (!is_first)
				runtime.writer.write_scalar(', ');
			else
				is_first = false;

			print_(runtime, parameter_type);
		}
	}

	runtime.writer.write_scalar(')');
}

void cminus::type::function::print_value(logic::runtime &runtime, std::shared_ptr<memory::reference> data) const{

}

std::size_t cminus::type::function::get_size() const{
	return sizeof(void *);
}

cminus::type::object::score_result_type cminus::type::function::get_score(logic::runtime &runtime, const object &target, bool is_ref) const{
	auto type_target = dynamic_cast<const function *>(&target);
	if (type_target == nullptr){//Check for pointer
		if (auto primitive_target_type = dynamic_cast<const primitive *>(&target); primitive_target_type != nullptr && primitive_target_type->get_id() == primitive::id_type::function)
			return score_result_type::assignable;
		return score_result_type::nil;
	}

	if ((return_type_.value == nullptr) != (type_target->return_type_.value == nullptr))
		return score_result_type::nil;

	if (parameter_types_.size() != type_target->parameter_types_.size())
		return score_result_type::nil;

	if (return_type_.value != nullptr && get_exact_score_(runtime, return_type_, type_target->return_type_) != score_result_type::exact)
		return score_result_type::nil;

	for (auto it = parameter_types_.begin(), target_it = type_target->parameter_types_.begin(); it != parameter_types_.end(); ++it, ++target_it){
		if (it->value == nullptr || target_it->value == nullptr || get_exact_score_(runtime, *it, *target_it) != score_result_type::exact)
			return score_result_type::nil;
	}

	return score_result_type::exact;
}

std::shared_ptr<cminus::memory::reference> cminus::type::function::get_default_value(logic::runtime &runtime) const{
	return nullptr;
}

std::shared_ptr<cminus::memory::reference> cminus::type::function::cast(logic::runtime &runtime, std::shared_ptr<memory::reference> data, std::shared_ptr<object> target_type, cast_type type) const{
	if (type != cast_type::static_ && type != cast_type::ref_static && type != cast_type::rval_static && type != cast_type::reinterpret)
		return nullptr;//Not supported

	auto function_target_type = dynamic_cast<function *>(target_type.get());
	if (function_target_type != nullptr){
		if (type == cast_type::reinterpret || !is_exact(runtime, *target_type))
			return nullptr;

		if (type == cast_type::static_)
			return std::make_shared<memory::scalar_reference<unsigned __int64>>(target_type, nullptr, data->read_scalar<unsigned __int64>(runtime));

		return data;
	}

	auto primitive_target_type = dynamic_cast<const primitive *>(target_type.get());
	if (primitive_target_type == nullptr)
		return nullptr;

	if (primitive_target_type->get_id() == primitive::id_type::function){
		if (type == cast_type::reinterpret)
			return nullptr;

		if (type == cast_type::static_)
			return std::make_shared<memory::scalar_reference<unsigned __int64>>(target_type, nullptr, data->read_scalar<unsigned __int64>(runtime));

		if ((data = data->apply_offset(0u)) != nullptr)
			data->set_type(target_type);

		return data;
	}

	if (type != cast_type::reinterpret)
		return nullptr;

	switch (primitive_target_type->get_id()){
	case primitive::id_type::int8_:
		return std::make_shared<cminus::memory::scalar_reference<__int8>>(target_type, nullptr, static_cast<__int8>(data->read_scalar<unsigned __int64>(runtime)));
	case primitive::id_type::uint8_:
		return std::make_shared<cminus::memory::scalar_reference<unsigned __int8>>(target_type, nullptr, static_cast<unsigned __int8>(data->read_scalar<unsigned __int64>(runtime)));
	case primitive::id_type::int16_:
		return std::make_shared<cminus::memory::scalar_reference<__int16>>(target_type, nullptr, static_cast<__int16>(data->read_scalar<unsigned __int64>(runtime)));
	case primitive::id_type::uint16_:
		return std::make_shared<cminus::memory::scalar_reference<unsigned __int16>>(target_type, nullptr, static_cast<unsigned __int16>(data->read_scalar<unsigned __int64>(runtime)));
	case primitive::id_type::int32_:
		return std::make_shared<cminus::memory::scalar_reference<__int32>>(target_type, nullptr, static_cast<__int32>(data->read_scalar<unsigned __int64>(runtime)));
	case primitive::id_type::uint32_:
		return std::make_shared<cminus::memory::scalar_reference<unsigned __int32>>(target_type, nullptr, static_cast<unsigned __int32>(data->read_scalar<unsigned __int64>(runtime)));
	case primitive::id_type::int64_:
		return std::make_shared<cminus::memory::scalar_reference<__int64>>(target_type, nullptr, static_cast<__int64>(data->read_scalar<unsigned __int64>(runtime)));
	case primitive::id_type::uint64_:
		return std::make_shared<cminus::memory::scalar_reference<unsigned __int64>>(target_type, nullptr, data->read_scalar<unsigned __int64>(runtime));
	default:
		break;
	}

	return nullptr;
}

std::shared_ptr<cminus::evaluator::object> cminus::type::function::get_evaluator(logic::runtime &runtime) const{
	return nullptr;
}

cminus::type::object::score_result_type cminus::type::function::get_params_score(logic::runtime &runtime, const object &target, bool is_ref) const{
	auto type_target = dynamic_cast<const function *>(&target);
	if (type_target == nullptr)
		return score_result_type::nil;

	if (parameter_types_.size() != type_target->parameter_types_.size())
		return score_result_type::nil;

	for (auto it = parameter_types_.begin(), target_it = type_target->parameter_types_.begin(); it != parameter_types_.end(); ++it, ++target_it){
		if (it->value == nullptr || target_it->value == nullptr || get_exact_score_(runtime, *it, *target_it) != score_result_type::exact)
			return score_result_type::nil;
	}

	return score_result_type::exact;
}

const cminus::type::function::type_info &cminus::type::function::get_return_type() const{
	return return_type_;
}

const std::vector<cminus::type::function::type_info> &cminus::type::function::get_parameter_types() const{
	return parameter_types_;
}

void cminus::type::function::traverse_parameter_types(const std::function<void(const type_info &)> &callback) const{
	for (auto &parameter_type : parameter_types_)
		callback(parameter_type);
}

std::shared_ptr<cminus::logic::attributes::object> cminus::type::function::find_attribute(const std::vector<std::shared_ptr<logic::attributes::object>> &attributes, const std::string &name){
	if (attributes.empty())
		return nullptr;

	for (auto attribute : attributes){
		if (attribute->get_naming_parent() == nullptr && attribute->get_naming_value() == name)
			return attribute;
	}

	return nullptr;
}

bool cminus::type::function::has_attribute(const std::vector<std::shared_ptr<logic::attributes::object>> &attributes, const std::string &name){
	return (find_attribute(attributes, name) != nullptr);
}

bool cminus::type::function::has_attribute(const std::vector<std::shared_ptr<logic::attributes::object>> &attributes, std::shared_ptr<logic::naming::object> name){
	if (attributes.empty())
		return false;

	for (auto attribute : attributes){
		if (attribute->is_same(*name))
			return true;
	}

	return false;
}

cminus::type::object::score_result_type cminus::type::function::get_exact_score_(logic::runtime &runtime, const type_info &left, const type_info &right) const{
	for (auto attribute : left.attributes){
		if (attribute->is_included_in_comparison(runtime) && !has_attribute(right.attributes, attribute))
			return score_result_type::nil;
	}

	return (left.value->is_exact(runtime, *right.value) ? score_result_type::exact : score_result_type::nil);
}

void cminus::type::function::print_(logic::runtime &runtime, const type_info &info) const{
	if (!info.attributes.empty()){//Print type attributes
		runtime.writer.write_scalar('[');
		auto is_first = true;

		for (auto attribute : info.attributes){
			if (!is_first)
				runtime.writer.write_scalar(', ');
			else
				is_first = false;

			attribute->print(runtime, true);
		}

		runtime.writer.write_scalar(']');
	}

	if (info.value != nullptr)
		info.value->print(runtime, true);
}
