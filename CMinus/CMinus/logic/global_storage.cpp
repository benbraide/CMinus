#include "../type/string_type.h"
#include "../evaluator/evaluator_object.h"

cminus::logic::storage::global::global(logic::runtime &runtime)
	: object("", nullptr){
	primitive_types_[type::primitive::id_type::string] = std::make_shared<type::string>(runtime);
	for (auto id = type::primitive::id_type::nullptr_; id < type::primitive::id_type::string; id = static_cast<type::primitive::id_type>(static_cast<int>(id) + 1))
		primitive_types_[id] = std::make_shared<type::primitive>(id);

	for (auto id = node::named_constant::constant_type::false_; id <= node::named_constant::constant_type::true_; id = static_cast<node::named_constant::constant_type>(static_cast<int>(id) + 1)){
		named_constants_[id] = std::make_shared<memory::scalar_reference<node::named_constant::constant_type>>(
			primitive_types_[type::primitive::id_type::bool_],
			nullptr,
			id
		);
	}

	named_constants_[node::named_constant::constant_type::nullptr_] = std::make_shared<memory::scalar_reference<unsigned __int64>>(
		primitive_types_[type::primitive::id_type::nullptr_],
		nullptr,
		0ui64
	);

	named_constants_[node::named_constant::constant_type::nan_] = std::make_shared<memory::scalar_reference<node::named_constant::constant_type>>(
		primitive_types_[type::primitive::id_type::nan_],
		nullptr,
		node::named_constant::constant_type::nan_
	);
}

cminus::logic::storage::global::~global() = default;

std::shared_ptr<cminus::type::object> cminus::logic::storage::global::get_primitve_type(type::primitive::id_type id) const{
	if (primitive_types_.empty())
		return nullptr;

	if (auto it = primitive_types_.find(id); it != primitive_types_.end())
		return it->second;

	return nullptr;
}

std::shared_ptr<cminus::type::object> cminus::logic::storage::global::get_string_type() const{
	return get_primitve_type(type::primitive::id_type::string);
}

std::shared_ptr<cminus::memory::reference> cminus::logic::storage::global::get_named_constant(node::named_constant::constant_type type) const{
	if (named_constants_.empty())
		return nullptr;

	if (auto it = named_constants_.find(type); it != named_constants_.end())
		return it->second;

	return nullptr;
}

std::shared_ptr<cminus::evaluator::object> cminus::logic::storage::global::get_evaluator(evaluator::id id) const{
	if (evaluators_.empty())
		return nullptr;

	if (auto it = evaluators_.find(id); it != evaluators_.end())
		return it->second;

	return nullptr;
}

const char *cminus::logic::storage::global::get_string_data(logic::runtime &runtime, std::shared_ptr<evaluator::object> object) const{
	return nullptr;
}
