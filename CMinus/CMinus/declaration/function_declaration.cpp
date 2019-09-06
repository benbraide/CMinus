#include "../type/class_type.h"
#include "../type/pointer_type.h"
#include "../node/memory_reference_node.h"

#include "function_declaration.h"

cminus::declaration::function::function(logic::runtime &runtime, std::string name, logic::naming::parent *parent, const attribute_list_type &attributes, std::shared_ptr<variable> return_declaration, const std::vector<std::shared_ptr<variable>> &params, std::shared_ptr<node::object> body)
	: function_base(name, parent, attributes), return_declaration_(return_declaration), params_(params), body_(body){
	compute_values_();
	if (auto return_type = ((return_declaration_ == nullptr) ? nullptr : return_declaration_->get_type()); return_type != nullptr){
		attributes_.traverse([&](std::shared_ptr<logic::attributes::object> value){
			if (value->applies_to_type(runtime, *return_type))
				return_declaration_->get_attributes().add(value);
		});
	}
}

cminus::declaration::function::function(std::string name, logic::naming::parent *parent)
	: function_base(name, parent, attribute_list_type{}){}

cminus::declaration::function::~function() = default;

bool cminus::declaration::function::is_exact(logic::runtime &runtime, const function_base &tmpl) const{
	auto function_tmpl = dynamic_cast<const function *>(&tmpl);
	if (function_tmpl == nullptr)
		return false;

	if (min_arg_count_ != function_tmpl->min_arg_count_ || max_arg_count_ != function_tmpl->max_arg_count_ || params_.size() != function_tmpl->params_.size())
		return false;

	if ((return_declaration_ == nullptr) != (function_tmpl->return_declaration_ == nullptr))
		return false;

	if (attributes_.get_list().size() != tmpl.get_attributes().get_list().size())
		return false;

	auto mismatch = false;
	attributes_.traverse(runtime, [&](std::shared_ptr<logic::attributes::object> value){
		if (!mismatch)//Check for attribute
			mismatch = !tmpl.get_attributes().has(*value);
	}, logic::attributes::object::stage_type::nil);

	if (mismatch)
		return false;

	if (return_declaration_ != nullptr && !is_exact_(runtime, return_declaration_, function_tmpl->return_declaration_))
		return false;

	for (auto param_it = params_.begin(), other_param_it = function_tmpl->params_.begin(); param_it != params_.end(); ++param_it, ++other_param_it){
		if (!is_exact_(runtime, *param_it, *other_param_it))
			return false;
	}

	return true;
}

cminus::type::object::score_result_type cminus::declaration::function::get_rank(logic::runtime &runtime, const std::vector<std::shared_ptr<memory::reference>> &args) const{
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

			param_type_has_ref = (*param_it)->get_attributes().has("Ref", true);
			++param_it;
		}
		else
			param_type = variadic_base_type;

		if (param_type_has_ref){//Check attributes
			if (!(*arg_it)->is_lvalue() || (current_rank = param_type->get_score(runtime, *(*arg_it)->get_type(), true)) == type::object::score_result_type::nil)
				return type::object::score_result_type::nil;

			attributes_mismatch = false;
			(*arg_it)->traverse_attributes(runtime, [&](std::shared_ptr<logic::attributes::object> attribute){
				if (!attributes_mismatch && attribute->is_required_on_ref_assignment(runtime) && !(*param_it)->get_attributes().has(*attribute))
					attributes_mismatch = true;
			}, logic::attributes::object::stage_type::nil);

			if (attributes_mismatch)
				return type::object::score_result_type::nil;
		}
		else if ((current_rank = param_type->get_score(runtime, *(*arg_it)->get_type(), false)) == type::object::score_result_type::nil)
			return type::object::score_result_type::nil;

		if ((current_rank_score = type::object::get_score_value(current_rank)) < lowest_rank_score){
			if ((lowest_rank = current_rank) == type::object::score_result_type::nil)//Update rank
				return type::object::score_result_type::nil;
		}
	}

	return ((arg_it == args.end()) ? lowest_rank : type::object::score_result_type::nil);
}

void cminus::declaration::function::print(logic::runtime &runtime) const{
	print_attributes_(runtime);
	print_return_(runtime);

	print_name_(runtime);
	print_params_(runtime);

	if (body_ != nullptr)
		print_body_(runtime);
}

void cminus::declaration::function::define(std::shared_ptr<node::object> body){
	if (body_ == nullptr)
		body_ = body;
	else
		throw logic::exception("Function redefinition", 0u, 0u);
}

bool cminus::declaration::function::is_defined() const{
	return (body_ != nullptr);
}

bool cminus::declaration::function::is_operator() const{
	return false;
}

bool cminus::declaration::function::supports_return_statement() const{
	return true;
}

void cminus::declaration::function::traverse_params(const std::function<void(std::shared_ptr<variable>)> &callback) const{
	for (auto param : params_)
		callback(param);
}

std::shared_ptr<cminus::type::object> cminus::declaration::function::get_return_type() const{
	return return_declaration_->get_type();
}

std::shared_ptr<cminus::node::object> cminus::declaration::function::get_body() const{
	return body_;
}

std::size_t cminus::declaration::function::get_min_arg_count() const{
	return min_arg_count_;
}

std::size_t cminus::declaration::function::get_max_arg_count() const{
	return max_arg_count_;
}

void cminus::declaration::function::compute_values_(){
	for (auto param : params_){
		if (max_arg_count_ == static_cast<std::size_t>(-1))
			throw logic::exception("Bad function parameter list", 0u, 0u);

		if (dynamic_cast<type::variadic *>(param->get_type().get()) == nullptr){
			if (param->get_initialization() == nullptr){
				if (max_arg_count_ == min_arg_count_)
					++min_arg_count_;
				else
					throw logic::exception("Bad function parameter list", 0u, 0u);
			}

			++max_arg_count_;
		}
		else//Variadic type
			max_arg_count_ = static_cast<std::size_t>(-1);
	}
}

bool cminus::declaration::function::is_exact_(logic::runtime &runtime, std::shared_ptr<variable> first, std::shared_ptr<variable> second) const{
	if (!first->get_type()->is_exact(runtime, *second->get_type()))
		return false;

	if (first->get_attributes().get_list().size() != second->get_attributes().get_list().size())
		return false;

	auto mismatch = false;
	first->get_attributes().traverse(runtime, [&](std::shared_ptr<logic::attributes::object> value){
		if (!mismatch)//Check for attribute
			mismatch = !second->get_attributes().has(*value);
	}, logic::attributes::object::stage_type::nil);

	return !mismatch;
}

void cminus::declaration::function::print_attributes_(logic::runtime &runtime) const{
	attributes_.print(runtime);
}

void cminus::declaration::function::print_return_(logic::runtime &runtime) const{
	if (return_declaration_ != nullptr){
		return_declaration_->get_type()->print(runtime, true);
		if (dynamic_cast<type::pointer *>(return_declaration_->get_type().get()) == nullptr)
			runtime.writer.write_scalar(' ');
	}
	else
		runtime.writer.write_buffer("void ", 5u);
}

void cminus::declaration::function::print_name_(logic::runtime &runtime) const{
	auto name = get_qualified_naming_value();
	if (name.empty())
		runtime.writer.write_buffer("function", 8u);
	else
		runtime.writer.write_buffer(name.data(), name.size());
}

void cminus::declaration::function::print_params_(logic::runtime &runtime) const{
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

void cminus::declaration::function::print_body_(logic::runtime &runtime) const{
	body_->print(runtime);
}

std::shared_ptr<cminus::memory::reference> cminus::declaration::function::call_(logic::runtime &runtime, std::shared_ptr<memory::reference> context, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	if (!is_defined())
		throw logic::exception("Undefined function called", 0u, 0u);

	if (attributes_.has("Static", true))
		context = nullptr;//Ignore context
	else if (context != nullptr){
		auto type_parent = dynamic_cast<type::object *>(parent_);
		if (type_parent == nullptr || context->get_type()->compute_base_offset(*type_parent) == static_cast<std::size_t>(-1))
			throw logic::exception("A member function requires a context of related type", 0u, 0u);
	}
	else if (dynamic_cast<type::class_ *>(parent_) != nullptr)
		throw logic::exception("A member function requires a context", 0u, 0u);

	std::shared_ptr<memory::reference> return_value;
	logic::storage::runtime_storage_guard guard(runtime, std::make_shared<logic::storage::function>(*this, context, dynamic_cast<logic::storage::object *>(parent_)));

	try{
		copy_args_(runtime, args);
		evaluate_body_(runtime);
		copy_return_value_(runtime, nullptr);
	}
	catch (logic::storage::specialized::interrupt_type e){//Check for value return
		if (e == logic::storage::specialized::interrupt_type::return_)
			return_value = copy_return_value_(runtime, runtime.current_storage->get_raised_interrupt_value());
		else//Forward exception
			throw;
	}
	catch (...){
		throw;//Forward exception
	}

	return return_value;
}

void cminus::declaration::function::copy_args_(logic::runtime &runtime, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	auto arg_it = args.begin();
	auto param_it = params_.begin();
	auto variadic_it = params_.end();

	std::vector<std::shared_ptr<memory::reference>> variadic_references;
	std::shared_ptr<variable> variadic_declaration, current_declaration;

	for (; arg_it != args.end(); ++arg_it){
		if (variadic_it == params_.end()){
			if (param_it == params_.end())
				break;

			auto param_type = (*param_it)->get_type();
			if (auto variadic_type = dynamic_cast<type::variadic *>(param_type.get()); variadic_type != nullptr){
				variadic_declaration = std::make_shared<variable>((*param_it)->get_attributes(), variadic_type->get_base_type(), "", nullptr);
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
			if (reference == nullptr)
				throw memory::exception(memory::error_code::allocation_failure, 0u);

			current_declaration->initialize_memory(runtime, reference, std::make_shared<node::memory_reference>(nullptr, *arg_it));
			dynamic_cast<logic::storage::function *>(runtime.current_storage)->add_unnamed(reference);

			if (current_declaration == variadic_declaration){
				reference->add_attribute(runtime.global_storage->find_attribute("#LVal#", false));
				variadic_references.push_back(reference);
			}
		}
		else//Named
			current_declaration->evaluate(runtime, std::make_shared<node::memory_reference>(nullptr, *arg_it));
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
			if (reference == nullptr)
				throw memory::exception(memory::error_code::allocation_failure, 0u);

			current_declaration->initialize_memory(runtime, reference, nullptr);
			dynamic_cast<logic::storage::function *>(runtime.current_storage)->add_unnamed(reference);
		}
		else//Named
			current_declaration->evaluate(runtime, nullptr);
	}

	if (variadic_it != params_.end()){//Add variadic entry
		variadic_declaration = std::make_shared<variable>((*param_it)->get_attributes(), (*variadic_it)->get_type(), (*variadic_it)->get_name(), nullptr);
	}
}

void cminus::declaration::function::evaluate_body_(logic::runtime &runtime) const{
	if (body_ != nullptr)
		body_->evaluate(runtime);
}

std::shared_ptr<cminus::memory::reference> cminus::declaration::function::copy_return_value_(logic::runtime &runtime, std::shared_ptr<memory::reference> value) const{
	auto is_void_return = (return_declaration_ == nullptr);
	if (!is_void_return){
		auto primitive_type = dynamic_cast<type::primitive *>(return_declaration_->get_type().get());
		is_void_return = (primitive_type != nullptr && primitive_type->get_id() == type::primitive::id_type::void_);
	}

	if (is_void_return){
		if (value != nullptr)
			throw logic::exception("A void function cannot return a value", 0u, 0u);
		return nullptr;
	}

	if (value == nullptr)
		throw logic::exception("A void function must return a value", 0u, 0u);

	auto reference = return_declaration_->allocate_memory(runtime);
	if (reference == nullptr)
		throw memory::exception(memory::error_code::allocation_failure, 0u);

	return_declaration_->initialize_memory(runtime, reference, std::make_shared<node::memory_reference>(nullptr, value));
	dynamic_cast<logic::storage::function *>(runtime.current_storage)->add_unnamed(reference);

	return reference;
}
