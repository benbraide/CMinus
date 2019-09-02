#include "../node/memory_reference_node.h"
#include "../evaluator/evaluator_object.h"

#include "variable_declaration.h"

cminus::declaration::variable::variable(const attribute_list_type &attributes, std::shared_ptr<type::object> type, std::string name, std::shared_ptr<node::object> initialization)
	: attributes_(attributes), type_(type), name_(name), initialization_(initialization){}

cminus::declaration::variable::variable(const optimised_attribute_list_type &attributes, std::shared_ptr<type::object> type, std::string name, std::shared_ptr<node::object> initialization)
	: attributes_(attributes), type_(type), name_(name), initialization_(initialization){}

cminus::declaration::variable::variable(const logic::attributes::collection &attributes, std::shared_ptr<type::object> type, std::string name, std::shared_ptr<node::object> initialization)
	: attributes_(attributes), type_(type), name_(name), initialization_(initialization){}

cminus::declaration::variable::~variable() = default;

void cminus::declaration::variable::evaluate(logic::runtime &runtime, std::shared_ptr<memory::reference> value) const{
	//[Deprecated("Message")] --> Registered as 'after look-up'
	//void print();

	//[ReadOnly] --> Registered as 'before write'
	//unsigned int32_t state_none = 0;

	//[NotNull] --> Registered as 'before write' and 'after uninitialized variable'
	//const void *ptr = &state_none;

	//[LogWrite("Written to 'store' with value " + self)] --> Registered as 'after write'
	//string store;

	//After looking up a variable, call all registered 'after look-up' attributes

	//Before reading from a variable, call all registered 'before read' attributes
	//After reading from a variable, call all registered 'after read' attributes

	//Before writing from a variable, call all registered 'before write' attributes
	//After writing from a variable, call all registered 'after write' attributes

	if (name_.empty())
		throw logic::exception("Cannot evaluate an unnamed variable", 0u, 0u);

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

std::shared_ptr<cminus::memory::reference> cminus::declaration::variable::evaluate_class_member(logic::runtime &runtime, std::size_t relative_offset) const{
	if (name_.empty())
		throw logic::exception("Cannot evaluate an unnamed variable", 0u, 0u);

	if (static_value_ != nullptr)//Static entry found
		return nullptr;

	if (attributes_.has("Static", true)){
		evaluate(runtime, nullptr);
		return nullptr;
	}

	return std::make_shared<memory::placeholder_reference>(relative_offset, type_, attributes_);
}

void cminus::declaration::variable::print(logic::runtime &runtime) const{
	print_attributes_(runtime);
	if (type_ != nullptr)
		print_type_(runtime);

	print_name_(runtime);
	if (initialization_ != nullptr)
		print_initialization_(runtime);
}

const cminus::logic::attributes::collection &cminus::declaration::variable::get_attributes() const{
	return attributes_;
}

cminus::logic::attributes::collection &cminus::declaration::variable::get_attributes(){
	return attributes_;
}

std::shared_ptr<cminus::type::object> cminus::declaration::variable::get_type() const{
	return type_;
}

const std::string &cminus::declaration::variable::get_name() const{
	return name_;
}

std::shared_ptr<cminus::node::object> cminus::declaration::variable::get_initialization() const{
	return initialization_;
}

std::shared_ptr<cminus::memory::reference> cminus::declaration::variable::allocate_memory(logic::runtime &runtime) const{
	std::shared_ptr<memory::reference> reference;
	auto is_ref = attributes_.has("Ref", true);

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

void cminus::declaration::variable::initialize_memory(logic::runtime &runtime, std::shared_ptr<memory::reference> target, std::shared_ptr<memory::reference> value) const{
	if (value == nullptr && initialization_ != nullptr && (value = initialization_->evaluate(runtime)) == nullptr)
		throw logic::exception("Failed to evaluate initialization", 0u, 0u);

	if (value != nullptr){
		target->add_attribute(runtime.global_storage->find_attribute("#Init#", false));
		type_->get_evaluator(runtime)->evaluate_binary(runtime, evaluator::operator_id::assignment, target, std::make_shared<node::memory_reference>(nullptr, value));
	}
	else if (target->find_attribute("Ref", true, false) == nullptr)
		target->get_attributes().call(runtime, logic::attributes::object::stage_type::after_uninitialized_declaration, target);
	else
		throw logic::exception("Reference variable requires initialization", 0u, 0u);
}

void cminus::declaration::variable::print_attributes_(logic::runtime &runtime) const{
	attributes_.print(runtime);
}

void cminus::declaration::variable::print_type_(logic::runtime &runtime) const{
	type_->print(runtime, true);
}

void cminus::declaration::variable::print_name_(logic::runtime &runtime) const{
	if (!name_.empty())
		runtime.writer.write_buffer(name_.data(), name_.size());
}

void cminus::declaration::variable::print_initialization_(logic::runtime &runtime) const{
	runtime.writer.write_buffer(" = ", 3u);
	initialization_->print(runtime);
}

cminus::declaration::contructed_variable::contructed_variable(const std::vector<std::shared_ptr<logic::attributes::object>> &attributes, std::shared_ptr<type::object> type, std::string name, std::shared_ptr<node::object> initialization)
	: variable(attributes, type, name, initialization){}

cminus::declaration::contructed_variable::~contructed_variable() = default;

std::shared_ptr<cminus::memory::reference> cminus::declaration::contructed_variable::allocate_memory(logic::runtime &runtime) const{
	auto reference = std::make_shared<memory::lval_reference>(runtime, type_, attributes_, nullptr);
	if (reference == nullptr || reference->get_address() == 0u)
		throw memory::exception(memory::error_code::allocation_failure, 0u);

	return reference;
}

void cminus::declaration::contructed_variable::initialize_memory(logic::runtime &runtime, std::shared_ptr<memory::reference> target, std::shared_ptr<memory::reference> value) const{
	if (initialization_ == nullptr)
		target->get_type()->construct_default(runtime, target);
	else
		target->get_type()->construct(runtime, target, initialization_);
}

void cminus::declaration::contructed_variable::print_initialization_(logic::runtime &runtime) const{
	runtime.writer.write_scalar('(');
	initialization_->print(runtime);
	runtime.writer.write_scalar(')');
}

cminus::declaration::uniform_contructed_variable::uniform_contructed_variable(const std::vector<std::shared_ptr<logic::attributes::object>> &attributes, std::shared_ptr<type::object> type, std::string name, std::shared_ptr<node::object> initialization)
	: contructed_variable(attributes, type, name, initialization){}

cminus::declaration::uniform_contructed_variable::~uniform_contructed_variable() = default;

void cminus::declaration::uniform_contructed_variable::print_initialization_(logic::runtime &runtime) const{
	runtime.writer.write_scalar('{');
	initialization_->print(runtime);
	runtime.writer.write_scalar('}');
}
