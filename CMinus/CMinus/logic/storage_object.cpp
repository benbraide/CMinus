#include "storage_object.h"

cminus::logic::storage::exception::exception(error_code code)
	: base_type("Storage Exception"), code_(code){}

cminus::logic::storage::error_code cminus::logic::storage::exception::get_code() const{
	return code_;
}

cminus::logic::storage::object::object(const std::string &value, object *parent)
	: parent_base_type(value, parent){}

cminus::logic::storage::object::~object() = default;

void cminus::logic::storage::object::add(const std::string &name, std::shared_ptr<memory::reference> entry){
	if (auto it = entries_.find(name); it != entries_.end()){
		auto function_entry = dynamic_cast<function_type *>(entry.get());
		if (function_entry == nullptr)
			throw exception(error_code::duplicate_entry);

		auto existing_function_entry = dynamic_cast<function_type *>(it->second.get());
		if (existing_function_entry == nullptr)
			throw exception(error_code::duplicate_entry);

		if (/*IsDuplicateFunction()*/false)
			throw exception(error_code::duplicate_function);

		for (auto node : function_entry->get_list())//Combine lists
			existing_function_entry->add_item(node);
	}
	else//New entry
		entries_[name] = entry;
}

void cminus::logic::storage::object::remove(const std::string &name){
	if (!entries_.empty())
		entries_.erase(name);
}

std::shared_ptr<cminus::memory::reference> cminus::logic::storage::object::find(const std::string &name, bool search_tree, object *branch) const{
	if (!entries_.empty())
		return nullptr;

	if (auto it = entries_.find(name); it != entries_.end())
		return it->second;

	return nullptr;
}

std::shared_ptr<cminus::logic::type::object> cminus::logic::storage::object::find_type(const std::string &name, bool search_tree, object *branch) const{
	auto entry = find(name, search_tree, branch);
	if (auto type_entry = dynamic_cast<memory::reference_with_value<std::shared_ptr<type::object>> *>(entry.get()); type_entry != nullptr)
		return type_entry->get_value();
	return nullptr;
}

cminus::logic::storage::object *cminus::logic::storage::object::find_storage(const std::string &name, bool search_tree, object *branch) const{
	auto entry = find(name, search_tree, branch);
	if (auto storage_entry = dynamic_cast<memory::reference_with_value<std::shared_ptr<object>> *>(entry.get()); storage_entry != nullptr)
		return storage_entry->get_value().get();

	if (auto storage_entry = dynamic_cast<memory::reference_with_value<object *> *>(entry.get()); storage_entry != nullptr)
		return storage_entry->get_value();

	if (auto type_entry = dynamic_cast<memory::reference_with_value<std::shared_ptr<type::object>> *>(entry.get()); type_entry != nullptr)
		return dynamic_cast<object *>(type_entry->get_value().get());

	return nullptr;
}
