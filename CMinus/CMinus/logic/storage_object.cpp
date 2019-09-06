#include "../evaluator/evaluator_object.h"
#include "../declaration/function_declaration_group.h"

cminus::logic::storage::exception::exception(error_code code)
	: base_type("Storage Exception"), code_(code){}

cminus::logic::storage::error_code cminus::logic::storage::exception::get_code() const{
	return code_;
}

cminus::logic::storage::object::object(const std::string &value, object *parent)
	: parent_base_type(value, parent){}

cminus::logic::storage::object::~object(){
	destroy_entries_();
}

void cminus::logic::storage::object::raise_interrupt(interrupt_type type, std::shared_ptr<memory::reference> value){
	if (auto object_parent = dynamic_cast<object *>(parent_); object_parent != nullptr)
		object_parent->raise_interrupt(type, value);
	else
		invalid_interrupt_(type, value);
}

std::shared_ptr<cminus::memory::reference> cminus::logic::storage::object::get_raised_interrupt_value() const{
	return nullptr;
}

std::shared_ptr<cminus::memory::reference> cminus::logic::storage::object::get_context() const{
	if (auto object_parent = dynamic_cast<object *>(parent_); object_parent != nullptr)
		return object_parent->get_context();
	return nullptr;
}

void cminus::logic::storage::object::add(logic::runtime &runtime, const std::string &name, std::shared_ptr<memory::reference> entry){
	if (!exists(name)){
		entries_[name] = entry;
		entries_order_.push_back(entry.get());
	}
	else
		throw exception(error_code::duplicate_entry);
}

void cminus::logic::storage::object::add_function(logic::runtime &runtime, std::shared_ptr<declaration::function_base> entry, std::size_t group_address){
	if (!validate_(*entry))
		throw logic::exception("Cannot add function of unrelated storage", 0u, 0u);

	auto &name = entry->get_naming_value();
	if (name.empty())
		throw logic::exception("Cannot add an unnamed function", 0u, 0u);

	if (auto group_it = function_groups_.find(name); group_it == function_groups_.end()){//New entry
		std::shared_ptr<memory::block> block;
		if (group_address == 0u)
			block = runtime.memory_object.allocate_block(sizeof(void *), memory::block::attribute_executable);
		else
			block = runtime.memory_object.find_block(group_address);

		if (block == nullptr || block->get_address() == 0u)
			throw memory::exception(memory::error_code::allocation_failure, 0u);

		auto group = std::make_shared<declaration::function_group>(name, this);
		if (group == nullptr)
			throw memory::exception(memory::error_code::allocation_failure, 0u);

		block->write_scalar<declaration::function_group_base *>(0u, group.get());
		if (group_address == 0u)
			block->set_attributes(block->get_attributes() | memory::block::attribute_write_protected);

		group->add(entry);
		function_groups_[name] = function_group_info{ block->get_address(), group };
	}
	else if (entries_.find(name) == entries_.end())//Extend group
		extend_function_group_(runtime, *group_it->second.value, entry);
	else
		throw exception(error_code::duplicate_entry);
}

void cminus::logic::storage::object::remove(const std::string &name){
	if (!entries_.empty()){
		if (auto entry = entries_.find(name); entry != entries_.end()){
			entries_.erase(entry);
			if (auto it = std::find(entries_order_.begin(), entries_order_.end(), entry->second.get()); it != entries_order_.end())
				entries_order_.erase(it);
		}
	}
}

bool cminus::logic::storage::object::exists(const std::string &name) const{
	return (name.empty() || entries_.find(name) != entries_.end() || function_groups_.find(name) != function_groups_.end());
}

std::shared_ptr<cminus::memory::reference> cminus::logic::storage::object::find(logic::runtime &runtime, const search_options &options) const{
	if (options.name.empty())
		return nullptr;

	if (auto it = entries_.find(options.name); it != entries_.end()){
		it->second->call_attributes(runtime, logic::attributes::object::stage_type::after_lookup);
		if (options.branch != nullptr)
			*options.branch = this;

		return it->second;
	}

	if (auto it = function_groups_.find(options.name); it != function_groups_.end()){
		auto entry = std::make_shared<memory::function_reference>(runtime, it->second.address, it->second.value.get());
		it->second.value->get_attributes().call(runtime, logic::attributes::object::stage_type::after_lookup, entry);

		if (options.branch != nullptr)
			*options.branch = this;

		return entry;
	}

	if (!options.search_tree)
		return nullptr;

	if (auto storage_parent = dynamic_cast<object *>(parent_); storage_parent != nullptr)
		return storage_parent->find(runtime, options);

	return nullptr;
}

std::shared_ptr<cminus::memory::reference> cminus::logic::storage::object::find(logic::runtime &runtime, const std::string &name, bool search_tree) const{
	return find(runtime, search_options{ nullptr, nullptr, name, search_tree, nullptr });
}

std::shared_ptr<cminus::memory::reference> cminus::logic::storage::object::find_operator(logic::runtime &runtime, evaluator::operator_id id, const namesless_search_options &options) const{
	return find(runtime, search_options{ options.scope, options.context, evaluator::object::convert_operator_to_string(id), options.search_tree, options.branch });
}

std::shared_ptr<cminus::logic::attributes::object> cminus::logic::storage::object::find_attribute(const std::string &name, bool search_tree, const object **branch) const{
	if (name.empty() || attributes_.empty())
		return nullptr;

	if (name[0] == '&'){//Pointer
		auto value = find_attribute(name.substr(1), search_tree, branch);
		return ((value == nullptr) ? nullptr : std::make_shared<logic::attributes::pointer_object>(value));
	}

	if (auto it = attributes_.find(name); it != attributes_.end())
		return it->second;

	return nullptr;
}

void cminus::logic::storage::object::destroy_entries_(){
	if (entries_order_.empty())
		return;

	for (auto it = entries_order_.rbegin(); it != entries_order_.rend(); ++it)
		destroy_entry_(*it);

	entries_order_.clear();
}

void cminus::logic::storage::object::destroy_entry_(memory::reference *entry){
	for (auto it = entries_.begin(); it != entries_.end(); ++it){
		if (it->second.get() == entry){
			entries_.erase(it);
			break;
		}
	}
}

void cminus::logic::storage::object::invalid_interrupt_(interrupt_type type, std::shared_ptr<memory::reference> value){
	switch (type){
	case interrupt_type::return_:
		throw logic::exception("A 'return' statement requires a function scope", 0u, 0u);
		break;
	case interrupt_type::break_:
		throw logic::exception("A 'return' statement requires a loop or switch scope", 0u, 0u);
		break;
	case interrupt_type::continue_:
		throw logic::exception("A 'return' statement requires a loop scope", 0u, 0u);
		break;
	default:
		break;
	}
}

bool cminus::logic::storage::object::validate_(const declaration::function_base &target) const{
	return (target.get_naming_parent() == this);
}

void cminus::logic::storage::object::extend_function_group_(logic::runtime &runtime, declaration::function_group_base &group, std::shared_ptr<declaration::function_base> entry){
	if (auto existing_function = group.find(runtime, *entry); existing_function != nullptr){//Function previously declared or defined
		if (!existing_function->is_defined() && entry->is_defined())
			existing_function->define(entry->get_body());
		else//Duplicate declaration or definition
			throw exception(error_code::duplicate_entry);
	}
	else//New entry
		group.add(entry);
}

cminus::logic::storage::proxy::proxy(object &target)
	: object(target.get_naming_value(), dynamic_cast<object *>(target.get_naming_parent())), target_(target){}

cminus::logic::storage::proxy::~proxy() = default;

void cminus::logic::storage::proxy::print(logic::runtime &runtime, bool is_qualified) const{
	target_.print(runtime, is_qualified);
}

bool cminus::logic::storage::proxy::is_same(const naming::object &target) const{
	return target_.is_same(target);
}

void cminus::logic::storage::proxy::add(logic::runtime &runtime, const std::string &name, std::shared_ptr<memory::reference> entry){
	target_.add(runtime, name, entry);
}

void cminus::logic::storage::proxy::add_function(logic::runtime &runtime, std::shared_ptr<declaration::function_base> entry, std::size_t group_address){
	target_.add_function(runtime, entry, group_address);
}

void cminus::logic::storage::proxy::remove(const std::string &name){
	target_.remove(name);
}

bool cminus::logic::storage::proxy::exists(const std::string &name) const{
	return target_.exists(name);
}

std::shared_ptr<cminus::memory::reference> cminus::logic::storage::proxy::find(logic::runtime &runtime, const search_options &options) const{
	return target_.find(runtime, options);
}

std::shared_ptr<cminus::memory::reference> cminus::logic::storage::proxy::find(logic::runtime &runtime, const std::string &name, bool search_tree) const{
	return target_.find(runtime, name, search_tree);
}

std::shared_ptr<cminus::logic::attributes::object> cminus::logic::storage::proxy::find_attribute(const std::string &name, bool search_tree, const object **branch) const{
	return target_.find_attribute(name, search_tree, branch);
}

cminus::logic::storage::runtime_storage_guard::runtime_storage_guard(logic::runtime &runtime, std::shared_ptr<object> current)
	: runtime_(runtime), old_(runtime.current_storage), new_(current){
	runtime.current_storage = current.get();
}

cminus::logic::storage::runtime_storage_guard::~runtime_storage_guard(){
	runtime_.current_storage = old_;
}
