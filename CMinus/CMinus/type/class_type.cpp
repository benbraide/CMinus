#include "class_type.h"

cminus::type::class_::class_(const std::string &name, logic::storage::object *parent)
	: with_storage(name, parent){}

cminus::type::class_::~class_() = default;

void cminus::type::class_::construct_default(logic::runtime &runtime, std::shared_ptr<memory::reference> target) const{

}

void cminus::type::class_::construct(logic::runtime &runtime, std::shared_ptr<memory::reference> target, std::shared_ptr<node::object> initialization) const{

}

void cminus::type::class_::print_value(logic::runtime &runtime, std::shared_ptr<memory::reference> data) const{

}

std::size_t cminus::type::class_::get_size() const{
	std::size_t ancestors_size = 0u;
	for (auto base_type : base_types_)
		ancestors_size += base_type.second.value->get_size();

	return (size_ + ancestors_size);
}

std::size_t cminus::type::class_::compute_base_offset(const type::object &target) const{
	if (&target == this)
		return 0u;

	if (base_types_.empty())
		return static_cast<std::size_t>(-1);

	std::size_t offset = size_;
	for (auto base_type : base_types_){
		if (auto base_offset = base_type.second.value->compute_base_offset(target); base_offset != static_cast<std::size_t>(-1))
			return (offset + base_offset);

		offset += base_type.second.value->get_size();
	}

	return static_cast<std::size_t>(-1);
}

cminus::type::object::score_result_type cminus::type::class_::get_score(logic::runtime &runtime, const type::object &target, bool is_ref) const{
	if (&target == this)
		return score_result_type::exact;



	return score_result_type::nil;
}

std::shared_ptr<cminus::memory::reference> cminus::type::class_::get_default_value(logic::runtime &runtime) const{
	return nullptr;
}

std::shared_ptr<cminus::memory::reference> cminus::type::class_::cast(logic::runtime &runtime, std::shared_ptr<memory::reference> data, std::shared_ptr<type::object> target_type, cast_type type) const{
	return nullptr;
}

std::shared_ptr<cminus::memory::reference> cminus::type::class_::find(const std::string &name, bool search_tree, const storage_base_type **branch) const{
	return nullptr;
}

bool cminus::type::class_::add_base(access_type access, std::shared_ptr<type::object> value){
	return false;
}

bool cminus::type::class_::add_function(access_type access, std::shared_ptr<logic::function_object> value){
	return false;
}

bool cminus::type::class_::add_declaration(access_type access, std::shared_ptr<logic::declaration> value){
	if (entries_.find(value->get_name()) != entries_.end())
		return false;

	//auto entry = std::make_shared<memory::hard_reference>(size_);
	//entries_[value->get_name()] = entry;

	return true;
}

cminus::type::class_::relationship_type cminus::type::class_::get_relationship(const type::object &target) const{
	return relationship_type::nil;
}
