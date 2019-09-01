#include "../type/function_type.h"
#include "../evaluator/evaluator_object.h"

#include "runtime.h"
#include "declaration.h"

cminus::logic::declaration::declaration(const std::vector<std::shared_ptr<attributes::object>> &attributes, std::shared_ptr<type::object> type, std::string name, std::shared_ptr<node::object> initialization)
	: attributes_(attributes), type_(type), name_(name), initialization_(initialization){}

cminus::logic::declaration::~declaration() = default;

void cminus::logic::declaration::evaluate(logic::runtime &runtime, std::shared_ptr<memory::reference> value) const{
	//[Deprecated("Message")] --> Registered as 'after look-up'
	//void print();

	//[ReadOnly] --> Registered as 'before write'
	//unsigned int32_t state_none = 0;

	//[NotNull] --> Registered as 'before write' and 'after uninitialized declaration'
	//const void *ptr = &state_none;

	//[LogWrite("Written to 'store' with value " + self)] --> Registered as 'after write'
	//string store;

	//After looking up a variable, call all registered 'after look-up' attributes

	//Before reading from a variable, call all registered 'before read' attributes
	//After reading from a variable, call all registered 'after read' attributes

	//Before writing from a variable, call all registered 'before write' attributes
	//After writing from a variable, call all registered 'after write' attributes

	if (name_.empty())
		throw exception("Cannot evaluate an unnamed declaration", 0u, 0u);

	auto reference = static_value_;
	if (reference == nullptr){//No static entry found
		if ((reference = allocate_memory(runtime)) != nullptr && reference->get_address() != 0u)
			initialize_memory(runtime, reference, value);
		else
			throw memory::exception(memory::error_code::allocation_failure, 0u);
	}

	//reference->add_attribute(runtime.global_storage->find_attribute("#LVal#", false));

	runtime.current_storage->add(runtime, name_, reference);
	if (reference->find_attribute("Static", true, false) != nullptr)
		static_value_ = reference;
}

std::shared_ptr<cminus::memory::reference> cminus::logic::declaration::evaluate_class_member(logic::runtime &runtime, std::size_t relative_offset) const{
	if (name_.empty())
		throw exception("Cannot evaluate an unnamed declaration", 0u, 0u);

	if (static_value_ != nullptr)//Static entry found
		return nullptr;

	if (type::function::has_attribute(attributes_, "Static")){
		evaluate(runtime, nullptr);
		return nullptr;
	}

	return std::make_shared<memory::placeholder_reference>(relative_offset, type_, attributes_);
}

void cminus::logic::declaration::print(logic::runtime &runtime) const{
	print_attributes_(runtime);
	if (type_ != nullptr)
		print_type_(runtime);

	print_name_(runtime);
	if (initialization_ != nullptr)
		print_initialization_(runtime);
}

const std::vector<std::shared_ptr<cminus::logic::attributes::object>> &cminus::logic::declaration::get_attributes() const{
	return attributes_;
}

std::shared_ptr<cminus::type::object> cminus::logic::declaration::get_type() const{
	return type_;
}

const std::string &cminus::logic::declaration::get_name() const{
	return name_;
}

std::shared_ptr<cminus::node::object> cminus::logic::declaration::get_initialization() const{
	return initialization_;
}

std::shared_ptr<cminus::memory::reference> cminus::logic::declaration::allocate_memory(logic::runtime &runtime) const{
	std::shared_ptr<memory::reference> reference;
	auto is_ref = type::function::has_attribute(attributes_, "Ref");

	if (is_ref)
		reference = std::make_shared<memory::ref_reference>(type_, attributes_, nullptr);
	else//Not a reference
		reference = std::make_shared<memory::lval_reference>(runtime, type_, attributes_, nullptr);

	if (reference == nullptr || reference->get_address() == 0u)
		throw memory::exception(memory::error_code::allocation_failure, 0u);

	if (!is_ref)//Default construction
		reference->get_type()->construct_default(runtime, reference);

	return reference;
}

void cminus::logic::declaration::initialize_memory(logic::runtime &runtime, std::shared_ptr<memory::reference> target, std::shared_ptr<memory::reference> value) const{
	if (value == nullptr && initialization_ != nullptr && (value = initialization_->evaluate(runtime)) == nullptr)
		throw logic::exception("Failed to evaluate initialization", 0u, 0u);

	if (value != nullptr){
		target->add_attribute(runtime.global_storage->find_attribute("#Init#", false));
		type_->get_evaluator(runtime)->evaluate_binary(runtime, evaluator::operator_id::assignment, target, value);
	}
	else if (target->find_attribute("Ref", true, false) == nullptr)
		target->call_attributes(runtime, attributes::object::stage_type::after_uninitialized_declaration, false, target);
	else
		throw logic::exception("Reference declaration requires initialization", 0u, 0u);
}

void cminus::logic::declaration::print_attributes_(logic::runtime &runtime) const{
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

void cminus::logic::declaration::print_type_(logic::runtime &runtime) const{
	type_->print(runtime, true);
}

void cminus::logic::declaration::print_name_(logic::runtime &runtime) const{
	if (!name_.empty())
		runtime.writer.write_buffer(name_.data(), name_.size());
}

void cminus::logic::declaration::print_initialization_(logic::runtime &runtime) const{
	runtime.writer.write_buffer(" = ", 3u);
	initialization_->print(runtime);
}

cminus::logic::contructed_declaration::contructed_declaration(const std::vector<std::shared_ptr<attributes::object>> &attributes, std::shared_ptr<type::object> type, std::string name, std::shared_ptr<node::object> initialization)
	: declaration(attributes, type, name, initialization){}

cminus::logic::contructed_declaration::~contructed_declaration() = default;

std::shared_ptr<cminus::memory::reference> cminus::logic::contructed_declaration::allocate_memory(logic::runtime &runtime) const{
	auto reference = std::make_shared<memory::lval_reference>(runtime, type_, attributes_, nullptr);
	if (reference == nullptr || reference->get_address() == 0u)
		throw memory::exception(memory::error_code::allocation_failure, 0u);

	return reference;
}

void cminus::logic::contructed_declaration::initialize_memory(logic::runtime &runtime, std::shared_ptr<memory::reference> target, std::shared_ptr<memory::reference> value) const{
	if (initialization_ == nullptr)
		target->get_type()->construct_default(runtime, target);
	else
		target->get_type()->construct(runtime, target, initialization_);
}

void cminus::logic::contructed_declaration::print_initialization_(logic::runtime &runtime) const{
	runtime.writer.write_scalar('(');
	initialization_->print(runtime);
	runtime.writer.write_scalar(')');
}

cminus::logic::uniform_contructed_declaration::uniform_contructed_declaration(const std::vector<std::shared_ptr<attributes::object>> &attributes, std::shared_ptr<type::object> type, std::string name, std::shared_ptr<node::object> initialization)
	: contructed_declaration(attributes, type, name, initialization){}

cminus::logic::uniform_contructed_declaration::~uniform_contructed_declaration() = default;

void cminus::logic::uniform_contructed_declaration::print_initialization_(logic::runtime &runtime) const{
	runtime.writer.write_scalar('{');
	initialization_->print(runtime);
	runtime.writer.write_scalar('}');
}
