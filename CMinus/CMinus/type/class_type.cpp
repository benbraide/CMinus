#include "../logic/specialized_storage.h"

#include "class_type.h"
#include "pointer_type.h"

cminus::type::class_::class_(logic::runtime &runtime, const std::string &name, logic::storage::object *parent)
	: with_storage(name, parent), size_(sizeof(void *)){
	memory::reference::attribute_list_type attributes{
		runtime.global_storage->find_attribute("ReadOnly", false)
	};

	entries_["this"] = std::make_shared<memory::placeholder_reference>(0u, std::make_shared<raw_pointer>(this), attributes);
	this_storage_ = std::make_shared<logic::storage::proxy>(*this);
}

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

bool cminus::type::class_::is_exact(logic::runtime &runtime, const type::object &target) const{
	return (&target == this);
}

cminus::type::object::score_result_type cminus::type::class_::get_score(logic::runtime &runtime, const type::object &target, bool is_ref) const{
	if (&target == this)
		return score_result_type::exact;

	if (is_ancestor(target))
		return score_result_type::ancestor;

	return score_result_type::nil;
}

std::shared_ptr<cminus::memory::reference> cminus::type::class_::get_default_value(logic::runtime &runtime) const{
	return nullptr;
}

std::shared_ptr<cminus::memory::reference> cminus::type::class_::cast(logic::runtime &runtime, std::shared_ptr<memory::reference> data, std::shared_ptr<type::object> target_type, cast_type type) const{
	if (type != cast_type::static_ && type != cast_type::ref_static && type != cast_type::rval_static)
		return nullptr;

	if (target_type.get() == this){//Same type
		if (type != cast_type::static_)
			return data;

		auto copy = std::make_shared<memory::data_reference>(target_type, nullptr);
		if (copy != nullptr)//Copy bytes
			copy->write(runtime, *data, static_cast<std::size_t>(-1));

		return copy;
	}

	auto base_offset = compute_base_offset(*target_type);
	if (base_offset != static_cast<std::size_t>(-1)){//Target type is an ancestor
		auto adjusted_data = data->apply_offset(base_offset);
		if (adjusted_data != nullptr)
			adjusted_data->set_type(target_type);

		if (type != cast_type::static_)
			return adjusted_data;

		auto copy = std::make_shared<memory::data_reference>(target_type, nullptr);
		if (copy != nullptr)//Copy bytes
			copy->write(runtime, *adjusted_data, static_cast<std::size_t>(-1));

		return copy;
	}

	return nullptr;
}

std::shared_ptr<cminus::memory::reference> cminus::type::class_::find(logic::runtime &runtime, const std::string &name, bool search_tree, const storage_base_type **branch) const{
	if (base_types_.empty())
		return with_storage::find(runtime, name, search_tree, branch);

	auto entry = with_storage::find(runtime, name, false, branch);
	if (entry != nullptr)//Entry found
		return nullptr;

	for (auto &base_type : base_types_){
		if (auto base_class = dynamic_cast<class_ *>(base_type.second.value.get()); base_class != nullptr && (entry = base_class->find(runtime, name, false, branch)) != nullptr)
			return entry;
	}

	if (!search_tree)
		return nullptr;

	if (auto storage_parent = dynamic_cast<logic::storage::object *>(parent_); storage_parent != nullptr)
		return storage_parent->find(runtime, name, true, branch);

	return nullptr;
}

bool cminus::type::class_::add_base(logic::runtime &runtime, access_type access, std::shared_ptr<type::object> value){
	if (value.get() == this || is_ancestor(*value))
		return false;

	auto class_target = dynamic_cast<class_ *>(value.get());
	if (class_target == nullptr)
		return false;

	base_types_[class_target->get_naming_value()] = base_type_info{ access, value };
	return true;
}

bool cminus::type::class_::add_function(logic::runtime &runtime, access_type access, std::shared_ptr<logic::function_object> value){
	return false;
}

bool cminus::type::class_::add_declaration(logic::runtime &runtime, access_type access, std::shared_ptr<logic::declaration> value){
	if (entries_.find(value->get_name()) != entries_.end())
		return false;

	logic::storage::runtime_storage_guard guard(runtime, this_storage_);
	if (auto non_static_member = value->evaluate_class_member(runtime, size_); non_static_member != nullptr){//Declaration is non-static
		entries_[value->get_name()] = non_static_member;
		if (non_static_member->find_attribute("Ref", true, false) == nullptr)
			size_ += non_static_member->get_type()->get_size();
	}

	return true;
}

cminus::type::class_::relationship_type cminus::type::class_::get_relationship(const type::object &target) const{
	if (&target == this)
		return relationship_type::self;

	if (is_base(target))
		return relationship_type::base;

	if (is_ancestor(target))
		return relationship_type::ancestor;

	if (auto class_target = dynamic_cast<const class_ *>(&target); class_target != nullptr && class_target->is_ancestor(*this))
		return relationship_type::inherited;

	return relationship_type::nil;
}

bool cminus::type::class_::is_ancestor(const type::object &target) const{
	if (base_types_.empty() || dynamic_cast<const class_ *>(&target) == nullptr)
		return false;

	for (auto &base_type : base_types_){
		if (base_type.second.value.get() == &target)
			return true;

		if (auto base_class_type = dynamic_cast<class_ *>(base_type.second.value.get()); base_class_type != nullptr && base_class_type->is_ancestor(target))
			return true;
	}

	return false;
}

bool cminus::type::class_::is_base(const type::object &target) const{
	if (base_types_.empty() || dynamic_cast<const class_ *>(&target) == nullptr)
		return false;

	for (auto &base_type : base_types_){
		if (base_type.second.value.get() == &target)
			return true;
	}

	return false;
}
