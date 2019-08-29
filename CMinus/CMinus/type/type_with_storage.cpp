#include "type_with_storage.h"

cminus::type::with_storage::with_storage(const std::string &name, logic::storage::object *parent)
	: storage_base_type(name, parent){}

cminus::type::with_storage::~with_storage() = default;
