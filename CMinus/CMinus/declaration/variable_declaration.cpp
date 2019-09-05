#include "../node/memory_reference_node.h"
#include "../evaluator/evaluator_object.h"

#include "variable_declaration.h"

cminus::declaration::variable::variable(const attribute_list_type &attributes, std::shared_ptr<type::object> type, std::string name, std::shared_ptr<node::object> initialization)
	: attributes_(attributes), type_(type), name_(name), initialization_(initialization){}

cminus::declaration::variable::variable(const optimised_attribute_list_type &attributes, std::shared_ptr<type::object> type, std::string name, std::shared_ptr<node::object> initialization)
	: attributes_(attributes), type_(type), name_(name), initialization_(initialization){}

cminus::declaration::variable::variable(const logic::attributes::collection &attributes, std::shared_ptr<type::object> type, std::string name, std::shared_ptr<node::object> initialization)
	: attributes_(attributes), type_(type), name_(name), initialization_(initialization){}

cminus::declaration::variable::variable(const attribute_list_type &attributes, std::shared_ptr<type::object> type, std::string name, std::shared_ptr<memory::reference> initialization)
	: attributes_(attributes), type_(type), name_(name), initialization_(std::make_shared<node::memory_reference>(nullptr, initialization)){}

cminus::declaration::variable::variable(const optimised_attribute_list_type &attributes, std::shared_ptr<type::object> type, std::string name, std::shared_ptr<memory::reference> initialization)
	: attributes_(attributes), type_(type), name_(name), initialization_(std::make_shared<node::memory_reference>(nullptr, initialization)){}

cminus::declaration::variable::variable(const logic::attributes::collection &attributes, std::shared_ptr<type::object> type, std::string name, std::shared_ptr<memory::reference> initialization)
	: attributes_(attributes), type_(type), name_(name), initialization_(std::make_shared<node::memory_reference>(nullptr, initialization)){}

cminus::declaration::variable::variable(const attribute_list_type &attributes, std::shared_ptr<type::object> type, std::string name, nullptr_t)
	: attributes_(attributes), type_(type), name_(name), initialization_(nullptr){}

cminus::declaration::variable::variable(const optimised_attribute_list_type &attributes, std::shared_ptr<type::object> type, std::string name, nullptr_t)
	: attributes_(attributes), type_(type), name_(name), initialization_(nullptr){}

cminus::declaration::variable::variable(const logic::attributes::collection &attributes, std::shared_ptr<type::object> type, std::string name, nullptr_t)
	: attributes_(attributes), type_(type), name_(name), initialization_(nullptr){}

cminus::declaration::variable::~variable() = default;

void cminus::declaration::variable::evaluate(logic::runtime &runtime, std::shared_ptr<node::object> initialization) const{
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
			initialize_memory(runtime, reference, initialization);
		else
			throw memory::exception(memory::error_code::allocation_failure, 0u);
	}

	runtime.current_storage->add(runtime, name_, reference);
	if (reference->find_attribute("Static", true) != nullptr)
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
	if (attributes_.has("Ref", true))
		reference = std::make_shared<memory::ref_reference>(runtime, type_, attributes_, nullptr);
	else//Not a reference
		reference = std::make_shared<memory::lval_reference>(runtime, type_, attributes_, nullptr);

	if (reference == nullptr || reference->get_address() == 0u)
		throw memory::exception(memory::error_code::allocation_failure, 0u);

	return reference;
}

void cminus::declaration::variable::initialize_memory(logic::runtime &runtime, std::shared_ptr<memory::reference> target, std::shared_ptr<node::object> value) const{
	auto initialization = ((value == nullptr) ? initialization_ : value);
	if (initialization == nullptr)
		target->call_attributes(runtime, logic::attributes::object::stage_type::after_uninitialized_declaration);

	target->add_attribute(runtime.global_storage->find_attribute("#Init#", false));
	target->get_type()->construct(runtime, target, initialization);
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

cminus::declaration::contructed_variable::~contructed_variable() = default;

std::shared_ptr<cminus::memory::reference> cminus::declaration::contructed_variable::allocate_memory(logic::runtime &runtime) const{
	auto reference = std::make_shared<memory::lval_reference>(runtime, type_, attributes_, nullptr);
	if (reference == nullptr || reference->get_address() == 0u)
		throw memory::exception(memory::error_code::allocation_failure, 0u);

	return reference;
}

void cminus::declaration::contructed_variable::initialize_memory(logic::runtime &runtime, std::shared_ptr<memory::reference> target, std::shared_ptr<node::object> value) const{
	target->add_attribute(runtime.global_storage->find_attribute("#Init#", false));
	target->get_type()->construct(runtime, target, ((value == nullptr) ? initialization_ : value));
}

void cminus::declaration::contructed_variable::print_initialization_(logic::runtime &runtime) const{
	runtime.writer.write_scalar('(');
	initialization_->print(runtime);
	runtime.writer.write_scalar(')');
}

cminus::declaration::uniform_contructed_variable::~uniform_contructed_variable() = default;

void cminus::declaration::uniform_contructed_variable::print_initialization_(logic::runtime &runtime) const{
	runtime.writer.write_scalar('{');
	initialization_->print(runtime);
	runtime.writer.write_scalar('}');
}
