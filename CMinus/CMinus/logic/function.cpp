#include "function.h"

cminus::logic::function_object::function_object(std::shared_ptr<type::object> owner_type, const std::vector<std::shared_ptr<attributes::object>> &attributes, std::shared_ptr<type::object> return_type, std::string name, const std::vector<std::shared_ptr<declaration>> &params, std::shared_ptr<node::object> body)
	: owner_type_(owner_type), attributes_(attributes), return_type_(return_type), name_(name), params_(params), body_(body){
	compute_values_();
}

cminus::logic::function_object::function_object(std::shared_ptr<type::object> owner_type, const std::vector<std::shared_ptr<attributes::object>> &attributes, std::shared_ptr<type::object> return_type, std::string name, std::vector<std::shared_ptr<declaration>> &&params, std::shared_ptr<node::object> body)
	: owner_type_(owner_type), attributes_(attributes), return_type_(return_type), name_(name), params_(std::move(params)), body_(body){
	compute_values_();
}

cminus::logic::function_object::~function_object() = default;

void cminus::logic::function_object::call(logic::runtime &runtime, std::shared_ptr<memory::reference> value, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	if (body_ == nullptr)
		throw exception("Undefined function called", 0u, 0u);
}

void cminus::logic::function_object::print(logic::runtime &runtime) const{
	print_attributes_(runtime);
	if (return_type_ != nullptr)
		print_return_type_(runtime);

	print_name_(runtime);
	print_params_(runtime);

	if (body_ != nullptr)
		print_body_(runtime);
}

void cminus::logic::function_object::define(std::shared_ptr<node::object> body){
	if (body_ == nullptr)
		body_ = body;
	else
		throw exception("Function redefinition", 0u, 0u);
}

bool cminus::logic::function_object::is_defined() const{
	return (body_ != nullptr);
}

std::shared_ptr<cminus::type::object> cminus::logic::function_object::get_computed_type() const{
	return computed_type_;
}

std::shared_ptr<cminus::type::object> cminus::logic::function_object::get_owner_type() const{
	return owner_type_;
}

const std::vector<std::shared_ptr<cminus::logic::attributes::object>> &cminus::logic::function_object::get_attributes() const{
	return attributes_;
}

std::shared_ptr<cminus::type::object> cminus::logic::function_object::get_return_type() const{
	return return_type_;
}

const std::string &cminus::logic::function_object::get_name() const{
	return name_;
}

const std::vector<std::shared_ptr<cminus::logic::declaration>> &cminus::logic::function_object::get_params() const{
	return params_;
}

std::shared_ptr<cminus::node::object> cminus::logic::function_object::get_body() const{
	return body_;
}

std::size_t cminus::logic::function_object::get_min_arg_count() const{
	return min_arg_count_;
}

std::size_t cminus::logic::function_object::get_max_arg_count() const{
	return max_arg_count_;
}

void cminus::logic::function_object::compute_values_(){
	std::vector<std::shared_ptr<type::object>> params_types;
	params_types.reserve(params_.size());

	for (auto param : params_){
		params_types.push_back(param->get_type());
		if (param->get_initialization() == nullptr)
			++min_arg_count_;
		
		if (false/*param->get_type()->IsVariadiic()*/)
			max_arg_count_ = static_cast<std::size_t>(-1);
		else if (++max_arg_count_ != min_arg_count_)
			throw exception("Bad function parameter list", 0u, 0u);
	}
}

void cminus::logic::function_object::print_attributes_(logic::runtime &runtime) const{
	if (attributes_.empty())
		return;

	runtime.writer.write_scalar('[');
	auto is_first = true;

	for (auto attribute : attributes_){
		if (!is_first)
			runtime.writer.write_scalar(', ');
		else
			is_first = false;

		attribute->print(runtime, true);
	}

	runtime.writer.write_scalar(']');
}

void cminus::logic::function_object::print_return_type_(logic::runtime &runtime) const{
	return_type_->print(runtime, true);
}

void cminus::logic::function_object::print_name_(logic::runtime &runtime) const{
	if (!name_.empty())
		runtime.writer.write_buffer(name_.data(), name_.size());
}

void cminus::logic::function_object::print_params_(logic::runtime &runtime) const{
	if (params_.empty()){
		runtime.writer.write_buffer("()", 2u);
		return;
	}

	runtime.writer.write_scalar('(');
	auto is_first = true;

	for (auto param : params_){
		if (!is_first)
			runtime.writer.write_scalar(', ');
		else
			is_first = false;

		param->print(runtime);
	}

	runtime.writer.write_scalar(')');
}

void cminus::logic::function_object::print_body_(logic::runtime &runtime) const{
	body_->print(runtime);
}
