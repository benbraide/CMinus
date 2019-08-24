#include "../evaluator/evaluator_object.h"

#include "runtime.h"
#include "declaration.h"

cminus::logic::declaration::declaration(const std::vector<std::shared_ptr<attributes::object>> &attributes, std::shared_ptr<type::object> type, std::string name, std::shared_ptr<node::object> initialization)
	: attributes_(attributes), type_(type), name_(name), initialization_(initialization){}

cminus::logic::declaration::~declaration() = default;

void cminus::logic::declaration::evaluate(logic::runtime &runtime, std::shared_ptr<memory::reference> value) const{
	//#TODO: Check for static entry in storage
	//if (runtime.current_storage_->UseStaticEntry(name_))
		//return;

	//[Deprecated("Message")] --> Registered as 'after look-up'
	//void print();

	//[ReadOnly] --> Registered as 'before write'
	//unsigned int32_t state_none = 0;

	//[NotNull] --> Registered as 'before write'
	//const void *ptr = &state_none;

	//[LogWrite("Written to 'store' with value " + self)] --> Registered as 'after write'
	//string store;

	//After looking up a variable, call all registered 'after look-up' attributes

	//Before reading from a variable, call all registered 'before read' attributes
	//After reading from a variable, call all registered 'after read' attributes

	//Before writing from a variable, call all registered 'before write' attributes
	//After writing from a variable, call all registered 'after write' attributes

	auto reference = allocate_memory(runtime);
	if (reference != nullptr && reference->get_address() != 0u)
		initialize_memory(runtime, reference, value);
	else
		throw memory::exception(memory::error_code::allocation_failure, 0u);

	runtime.current_storage->add(name_, reference);
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
	return std::make_shared<memory::hard_reference>(runtime, type_, attributes_, nullptr);
}

void cminus::logic::declaration::initialize_memory(logic::runtime &runtime, std::shared_ptr<memory::reference> target, std::shared_ptr<memory::reference> value) const{
	auto is_ref = (target->find_attribute("Ref", true, false) != nullptr);
	if (value == nullptr){
		if (initialization_ == nullptr){
			if (!is_ref)
				value = type_->get_default_value(runtime);
			else
				throw logic::exception("Reference declaration requires initialization", 0u, 0u);
		}
		else if ((value = initialization_->evaluate(runtime)) == nullptr)
			throw logic::exception("Failed to evaluate initialization", 0u, 0u);
	}

	if (value != nullptr){
		target->add_attribute(runtime.global_storage->find_attribute("#Init#", false));
		type_->get_evaluator(runtime)->evaluate_binary(runtime, evaluator::operator_id::assignment, target, value);
	}
	else if (is_ref)
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