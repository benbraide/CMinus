#include "type_with_storage.h"

cminus::type::with_storage::with_storage(const std::string &name, logic::storage::object *parent)
	: storage_base_type(name, parent){}

cminus::type::with_storage::~with_storage() = default;

void cminus::type::with_storage::print(logic::runtime &runtime, bool is_qualified) const{
	storage_base_type::print(runtime, is_qualified);
}
