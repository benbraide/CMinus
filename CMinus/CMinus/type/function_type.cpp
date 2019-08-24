#include "function_type.h"

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

std::size_t cminus::type::function::get_size() const{
	return sizeof(void *);
}

std::size_t cminus::type::function::compute_base_offset(const object &target) const{
	return 0u;
}

cminus::type::object::score_result_type cminus::type::function::get_score(logic::runtime &runtime, const object &target, bool is_ref) const{
	auto type_target = dynamic_cast<const function *>(&target);
	if (type_target == nullptr){//Check for pointer
		if (is_ref)
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

std::shared_ptr<cminus::memory::reference> cminus::type::function::convert_value(logic::runtime &runtime, std::shared_ptr<memory::reference> data, std::shared_ptr<object> target_type, bool is_ref) const{
	return nullptr;
}

std::shared_ptr<cminus::memory::reference> cminus::type::function::convert_value(logic::runtime &runtime, const std::byte *data, std::shared_ptr<object> target_type) const{
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

bool cminus::type::function::has_attribute(const std::vector<std::shared_ptr<logic::attributes::object>> &attributes, const std::string &name){
	if (attributes.empty())
		return false;

	for (auto attribute : attributes){
		if (attribute->get_naming_parent() == nullptr && attribute->get_naming_value() == name)
			return true;
	}

	return false;
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

	return ((left.value->get_score(runtime, *right.value, false) == score_result_type::exact) ? score_result_type::exact : score_result_type::nil);
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
