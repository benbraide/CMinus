#include "function_object.h"

cminus::logic::function_object::function_object(std::string name, naming::parent *parent, const std::vector<std::shared_ptr<attributes::object>> &attributes, std::shared_ptr<declaration> return_declaration, const std::vector<std::shared_ptr<declaration>> &params, std::shared_ptr<node::object> body)
	: single(name, parent), attributes_(attributes), return_declaration_(return_declaration), params_(params), body_(body){
	compute_values_();
}

cminus::logic::function_object::function_object(std::string name, naming::parent *parent, std::vector<std::shared_ptr<attributes::object>> &&attributes, std::shared_ptr<declaration> return_declaration, std::vector<std::shared_ptr<declaration>> &&params, std::shared_ptr<node::object> body)
	: single(name, parent), attributes_(std::move(attributes)), return_declaration_(return_declaration), params_(std::move(params)), body_(body){
	compute_values_();
}

cminus::logic::function_object::~function_object() = default;

void cminus::logic::function_object::add(std::shared_ptr<function_object> value){
	throw logic::exception("Cannot add function to a non-group object", 0u, 0u);
}

void cminus::logic::function_object::replace(function_object &existing_entry, std::shared_ptr<function_object> new_entry){}

std::shared_ptr<cminus::logic::function_object> cminus::logic::function_object::find(logic::runtime &runtime, const type::object &type) const{
	return (computed_type_->is_exact(runtime, type) ? const_cast<function_object *>(this)->shared_from_this() : nullptr);
}

std::shared_ptr<cminus::logic::function_object> cminus::logic::function_object::get_highest_ranked(logic::runtime &runtime, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	return ((get_rank(runtime, args) == type::object::score_result_type::nil) ? nullptr : const_cast<function_object *>(this)->shared_from_this());
}

std::shared_ptr<cminus::memory::reference> cminus::logic::function_object::call(logic::runtime &runtime, std::shared_ptr<memory::reference> context, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	if (get_rank(runtime, args) == type::object::score_result_type::nil)
		throw exception("Function does not take the specified arguments", 0u, 0u);
	return call_(runtime, context, args);
}

cminus::type::object::score_result_type cminus::logic::function_object::get_rank(logic::runtime &runtime, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	if (args.size() < min_arg_count_ || max_arg_count_ < args.size())
		return type::object::score_result_type::nil;

	if (args.empty())
		return type::object::score_result_type::exact;

	type::object::score_result_type lowest_rank = type::object::score_result_type::exact, current_rank;
	int lowest_rank_score = type::object::get_score_value(lowest_rank), current_rank_score;

	auto arg_it = args.begin();
	auto param_it = params_.begin();

	auto param_type_has_ref = false, attributes_mismatch = false;
	std::shared_ptr<type::object> variadic_base_type, param_type;

	for (; arg_it != args.end(); ++arg_it){
		if (variadic_base_type == nullptr){
			if (param_it == params_.end())
				break;

			param_type = (*param_it)->get_type();
			if (auto variadic_type = dynamic_cast<type::variadic *>(param_type.get()); variadic_type != nullptr)
				param_type = variadic_base_type = variadic_type->get_base_type();

			param_type_has_ref = type::function::has_attribute((*param_it)->get_attributes(), "Ref");
			++param_it;
		}
		else
			param_type = variadic_base_type;

		if (param_type_has_ref){//Check attributes
			if ((current_rank = param_type->get_score(runtime, *(*arg_it)->get_type(), true)) == type::object::score_result_type::nil)
				break;

			attributes_mismatch = false;
			(*arg_it)->traverse_attributes(runtime, [&](std::shared_ptr<attributes::object> attribute){
				if (!attributes_mismatch && attribute->is_required_on_ref_destination(runtime) && !type::function::has_attribute((*param_it)->get_attributes(), attribute))
					attributes_mismatch = true;
			}, attributes::object::stage_type::nil, false);

			if (attributes_mismatch)
				break;
		}
		else if ((current_rank = param_type->get_score(runtime, *(*arg_it)->get_type(), false)) == type::object::score_result_type::nil)
			return type::object::score_result_type::nil;

		if ((current_rank_score = type::object::get_score_value(current_rank)) < lowest_rank_score)
			lowest_rank = current_rank;//Update rank
	}

	if (arg_it != args.end())
		return type::object::score_result_type::nil;

	return lowest_rank;
}

void cminus::logic::function_object::print(logic::runtime &runtime) const{
	print_attributes_(runtime);
	return_declaration_->print(runtime);

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

const std::vector<std::shared_ptr<cminus::logic::attributes::object>> &cminus::logic::function_object::get_attributes() const{
	return attributes_;
}

std::shared_ptr<cminus::type::object> cminus::logic::function_object::get_return_type() const{
	return return_declaration_->get_type();
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
	std::vector<type::function::type_info> params_types;
	params_types.reserve(params_.size());

	for (auto param : params_){
		if (max_arg_count_ == static_cast<std::size_t>(-1))
			throw exception("Bad function parameter list", 0u, 0u);

		params_types.push_back(type::function::type_info{
			param->get_type(),
			param->get_attributes()
		});

		if (dynamic_cast<type::variadic *>(param->get_type().get()) == nullptr){\
			if (param->get_initialization() == nullptr){
				if (max_arg_count_ == min_arg_count_)
					++min_arg_count_;
				else
					throw exception("Bad function parameter list", 0u, 0u);
			}

			++max_arg_count_;
		}
		else//Variadic type
			max_arg_count_ = static_cast<std::size_t>(-1);
	}

	computed_type_ = std::make_shared<type::function>(type::function::type_info{ return_declaration_->get_type(), return_declaration_->get_attributes() }, std::move(params_types));
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

void cminus::logic::function_object::print_name_(logic::runtime &runtime) const{
	auto name = get_qualified_naming_value();
	if (name.empty())
		runtime.writer.write_buffer("function", 8u);
	else
		runtime.writer.write_buffer(name.data(), name.size());
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

std::shared_ptr<cminus::memory::reference> cminus::logic::function_object::call_(logic::runtime &runtime, std::shared_ptr<memory::reference> context, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	if (body_ == nullptr)
		throw exception("Undefined function called", 0u, 0u);

	if (context != nullptr && dynamic_cast<type::object *>(context->get_type().get()) != dynamic_cast<type::object *>(parent_))
		throw exception("Function context is invalid", 0u, 0u);

	if (type::function::has_attribute(attributes_, "Static"))
		context = nullptr;//Ignore context
	else if (context == nullptr)
		throw exception("A member function requires a context", 0u, 0u);

	std::shared_ptr<memory::reference> return_value;
	storage::runtime_storage_guard guard(runtime, std::make_shared<storage::function>(*this, context, dynamic_cast<storage::object *>(parent_)));

	try{
		copy_args_(runtime, args);
		copy_return_value_(runtime, nullptr);
	}
	catch (storage::specialized::interrupt_type e){//Check for value return
		if (e == storage::specialized::interrupt_type::return_)
			return_value = copy_return_value_(runtime, dynamic_cast<storage::function *>(runtime.current_storage.get())->get_raised_interrupt_value());
		else//Forward exception
			throw;
	}
	catch (...){
		throw;//Forward exception
	}

	return return_value;
}

void cminus::logic::function_object::copy_args_(logic::runtime &runtime, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	auto arg_it = args.begin();
	auto param_it = params_.begin();
	auto variadic_it = params_.end();

	std::vector<std::shared_ptr<memory::reference>> variadic_references;
	std::shared_ptr<declaration> variadic_declaration, current_declaration;

	for (; arg_it != args.end(); ++arg_it){
		if (variadic_it == params_.end()){
			if (param_it == params_.end())
				break;

			auto param_type = (*param_it)->get_type();
			if (auto variadic_type = dynamic_cast<type::variadic *>(param_type.get()); variadic_type != nullptr){
				variadic_declaration = std::make_shared<declaration>((*param_it)->get_attributes(), variadic_type->get_base_type(), "", nullptr);
				current_declaration = variadic_declaration;
				variadic_it = param_it;
			}
			else
				current_declaration = *param_it;
			++param_it;
		}
		else
			current_declaration = variadic_declaration;

		if (current_declaration->get_name().empty()){
			auto reference = current_declaration->allocate_memory(runtime);
			if (reference == nullptr || reference->get_address() == 0u)
				throw memory::exception(memory::error_code::allocation_failure, 0u);

			current_declaration->initialize_memory(runtime, reference, *arg_it);
			dynamic_cast<storage::function *>(runtime.current_storage.get())->add_unnamed(reference);

			if (current_declaration == variadic_declaration){
				reference->add_attribute(runtime.global_storage->find_attribute("#LVal#", false));
				variadic_references.push_back(reference);
			}
		}
		else//Named
			current_declaration->evaluate(runtime, *arg_it);
	}

	for (; param_it != params_.end(); ++param_it){//Check for parameters with default arguments
		if (variadic_declaration != nullptr)
			break;

		auto param_type = (*param_it)->get_type();
		if (auto variadic_type = dynamic_cast<type::variadic *>(param_type.get()); variadic_type != nullptr){
			variadic_it = param_it;
			break;
		}
		else
			current_declaration = *param_it;

		if (current_declaration->get_name().empty()){
			auto reference = current_declaration->allocate_memory(runtime);
			if (reference == nullptr || reference->get_address() == 0u)
				throw memory::exception(memory::error_code::allocation_failure, 0u);

			current_declaration->initialize_memory(runtime, reference, nullptr);
			dynamic_cast<storage::function *>(runtime.current_storage.get())->add_unnamed(reference);
		}
		else//Named
			current_declaration->evaluate(runtime, nullptr);
	}

	if (variadic_it != params_.end()){//Add variadic entry
		variadic_declaration = std::make_shared<declaration>((*param_it)->get_attributes(), (*variadic_it)->get_type(), (*variadic_it)->get_name(), nullptr);
	}
}

std::shared_ptr<cminus::memory::reference> cminus::logic::function_object::copy_return_value_(logic::runtime &runtime, std::shared_ptr<memory::reference> value) const{
	auto is_void_return = (return_declaration_ == nullptr);
	if (!is_void_return){
		auto primitive_type = dynamic_cast<type::primitive *>(return_declaration_->get_type().get());
		is_void_return = (primitive_type != nullptr && primitive_type->get_id() == type::primitive::id_type::void_);
	}

	if (is_void_return){
		if (value != nullptr)
			throw exception("A void function cannot return a value", 0u, 0u);
		return nullptr;
	}

	if (value == nullptr)
		throw exception("A void function must return a value", 0u, 0u);

	auto reference = return_declaration_->allocate_memory(runtime);
	if (reference == nullptr || reference->get_address() == 0u)
		throw memory::exception(memory::error_code::allocation_failure, 0u);

	return_declaration_->initialize_memory(runtime, reference, value);
	dynamic_cast<storage::function *>(runtime.current_storage.get())->add_unnamed(reference);

	return reference;
}
