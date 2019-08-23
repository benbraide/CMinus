#include "../evaluator/evaluator_object.h"

cminus::logic::storage::global::global()
	: object("", nullptr){

}

cminus::logic::storage::global::~global() = default;

std::shared_ptr<cminus::type::object> cminus::logic::storage::global::get_primitve_type(type::primitive::id_type id) const{
	if (primitive_types_.empty())
		return nullptr;

	if (auto it = primitive_types_.find(id); it != primitive_types_.end())
		return it->second;

	return nullptr;
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
