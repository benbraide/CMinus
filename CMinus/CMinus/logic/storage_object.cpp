#include "function_group.h"

cminus::logic::storage::exception::exception(error_code code)
	: base_type("Storage Exception"), code_(code){}

cminus::logic::storage::error_code cminus::logic::storage::exception::get_code() const{
	return code_;
}

cminus::logic::storage::object::object(const std::string &value, object *parent)
	: parent_base_type(value, parent){}

cminus::logic::storage::object::~object() = default;

void cminus::logic::storage::object::add(logic::runtime &runtime, const std::string &name, std::shared_ptr<memory::reference> entry){
	if (!exists(name))
		entries_[name] = entry;
	else
		throw exception(error_code::duplicate_entry);
}

void cminus::logic::storage::object::add_function(logic::runtime &runtime, std::shared_ptr<logic::function_object> entry){
	if (!validate_(*entry))
		throw logic::exception("Cannot add function of unrelated storage", 0u, 0u);

	auto &name = entry->get_naming_value();
	if (name.empty())
		throw logic::exception("Cannot add an unnamed function", 0u, 0u);

	if (auto group_it = function_groups_.find(name); group_it == function_groups_.end()){//New entry
		auto block = runtime.memory_object.allocate_block(sizeof(void *), memory::block::attribute_executable);
		if (block == nullptr || block->get_address() == 0u)
			throw memory::exception(memory::error_code::allocation_failure, 0u);

		auto group = std::make_shared<logic::function_group>(name, this);
		if (group == nullptr)
			throw memory::exception(memory::error_code::allocation_failure, 0u);

		block->write_scalar<logic::function_group_base *>(0u, group.get());
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
	if (!entries_.empty())
		entries_.erase(name);
}

bool cminus::logic::storage::object::exists(const std::string &name) const{
	return (name.empty() || entries_.find(name) != entries_.end() || function_groups_.find(name) != function_groups_.end());
}

std::shared_ptr<cminus::memory::reference> cminus::logic::storage::object::find(logic::runtime &runtime, const std::string &name, bool search_tree, const object **branch) const{
	if (name.empty())
		return nullptr;

	if (auto it = entries_.find(name); it != entries_.end()){
		if (branch != nullptr)
			*branch = this;

		return it->second;
	}

	if (auto it = function_groups_.find(name); it != function_groups_.end()){
		if (branch != nullptr)
			*branch = this;

		return std::make_shared<memory::lval_reference>(
			it->second.address,
			runtime.global_storage->get_primitve_type(type::primitive::id_type::function),
			memory::reference::attribute_list_type{ runtime.global_storage->find_attribute("ReadOnly", false) },
			nullptr
		);
	}

	if (!search_tree)
		return nullptr;

	if (auto storage_parent = dynamic_cast<object *>(parent_); storage_parent != nullptr)
		return storage_parent->find(runtime, name, true, branch);

	return nullptr;
}

std::shared_ptr<cminus::logic::attributes::object> cminus::logic::storage::object::find_attribute(const std::string &name, bool search_tree, const object **branch) const{
	if (name.empty() || attributes_.empty())
		return nullptr;

	if (auto it = attributes_.find(name); it != attributes_.end())
		return it->second;

	return nullptr;
}

bool cminus::logic::storage::object::validate_(const logic::function_object &target) const{
	return (target.get_naming_parent() == this);
}

void cminus::logic::storage::object::extend_function_group_(logic::runtime &runtime, logic::function_group &group, std::shared_ptr<logic::function_object> entry){
	auto function_type = entry->get_computed_type();
	if (auto existing_function = group.find(runtime, *function_type); existing_function != nullptr){//Function previously declared or defined
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

void cminus::logic::storage::proxy::add_function(logic::runtime &runtime, std::shared_ptr<logic::function_object> entry){
	target_.add_function(runtime, entry);
}

void cminus::logic::storage::proxy::remove(const std::string &name){
	target_.remove(name);
}

bool cminus::logic::storage::proxy::exists(const std::string &name) const{
	return target_.exists(name);
}

std::shared_ptr<cminus::memory::reference> cminus::logic::storage::proxy::find(logic::runtime &runtime, const std::string &name, bool search_tree, const object **branch) const{
	return target_.find(runtime, name, search_tree, branch);
}

std::shared_ptr<cminus::logic::attributes::object> cminus::logic::storage::proxy::find_attribute(const std::string &name, bool search_tree, const object **branch) const{
	return target_.find_attribute(name, search_tree, branch);
}

cminus::logic::storage::runtime_storage_guard::runtime_storage_guard(logic::runtime &runtime, std::shared_ptr<object> current)
	: runtime_(runtime), old_(runtime.current_storage){
	runtime.current_storage = current;
}

cminus::logic::storage::runtime_storage_guard::~runtime_storage_guard(){
	runtime_.current_storage = old_;
}
