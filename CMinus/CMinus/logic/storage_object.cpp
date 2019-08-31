#include "runtime.h"

cminus::logic::storage::exception::exception(error_code code)
	: base_type("Storage Exception"), code_(code){}

cminus::logic::storage::error_code cminus::logic::storage::exception::get_code() const{
	return code_;
}

cminus::logic::storage::object::object(const std::string &value, object *parent)
	: parent_base_type(value, parent){}

cminus::logic::storage::object::~object() = default;

void cminus::logic::storage::object::add(const std::string &name, std::shared_ptr<memory::reference> entry){
	if (auto existing_entry = find_existing(name); existing_entry != nullptr){//Check if entry is a function collection
		/*auto function_entry = dynamic_cast<function_type *>(entry.get());
		if (function_entry == nullptr)
			throw exception(error_code::duplicate_entry);

		auto existing_function_entry = dynamic_cast<function_type *>(it->second.get());
		if (existing_function_entry == nullptr)
			throw exception(error_code::duplicate_entry);

		if (/ *IsDuplicateFunction()* /false)
			throw exception(error_code::duplicate_function);

		for (auto node : function_entry->get_list())//Combine lists
			existing_function_entry->add_item(node);*/
	}
	else if (!name.empty())//New entry
		entries_[name] = entry;
}

void cminus::logic::storage::object::remove(const std::string &name){
	if (!entries_.empty())
		entries_.erase(name);
}

std::shared_ptr<cminus::memory::reference> cminus::logic::storage::object::find(logic::runtime &runtime, const std::string &name, bool search_tree, const object **branch) const{
	if (name.empty() || entries_.empty())
		return nullptr;

	if (auto it = entries_.find(name); it != entries_.end()){
		if (branch != nullptr)
			*branch = this;
		return it->second;
	}

	if (!search_tree)
		return nullptr;

	if (auto storage_parent = dynamic_cast<object *>(parent_); storage_parent != nullptr)
		return storage_parent->find(runtime, name, true, branch);

	return nullptr;
}

std::shared_ptr<cminus::memory::reference> cminus::logic::storage::object::find_existing(const std::string &name) const{
	if (name.empty() || entries_.empty())
		return nullptr;

	if (auto it = entries_.find(name); it != entries_.end())
		return it->second;

	return nullptr;
}

std::shared_ptr<cminus::logic::attributes::object> cminus::logic::storage::object::find_attribute(const std::string &name, bool search_tree, const object **branch) const{
	if (name.empty() || attributes_.empty())
		return nullptr;

	if (auto it = attributes_.find(name); it != attributes_.end())
		return it->second;

	return nullptr;
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

void cminus::logic::storage::proxy::add(const std::string &name, std::shared_ptr<memory::reference> entry){
	target_.add(name, entry);
}

void cminus::logic::storage::proxy::remove(const std::string &name){
	target_.remove(name);
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
