#include "../node/list_node.h"
#include "../logic/specialized_storage.h"

#include "class_type.h"
#include "pointer_type.h"

cminus::type::class_::class_(logic::runtime &runtime, const std::string &name, logic::storage::object *parent)
	: with_storage(name, parent), size_(sizeof(void *)){
	memory::reference::attribute_list_type attributes{
		runtime.global_storage->find_attribute("Private", false),
		runtime.global_storage->find_attribute("ReadOnly", false)
	};

	entries_["this"] = std::make_shared<memory::placeholder_reference>(0u, std::make_shared<raw_pointer>(this), attributes);
}

cminus::type::class_::~class_() = default;

void cminus::type::class_::construct(logic::runtime &runtime, std::shared_ptr<memory::reference> target, std::shared_ptr<node::object> initialization) const{
	auto constructor = find(runtime, search_options{ this, target, get_naming_value(), false });
	auto callable = dynamic_cast<memory::function_reference *>(constructor->get_non_raw());

	if (callable == nullptr)
		throw logic::exception("Bad constructor definition", 0u, 0u);

	std::vector<std::shared_ptr<memory::reference>> args;
	if (initialization != nullptr){//Resolve initialization
		if (auto list_node = dynamic_cast<node::list *>(initialization.get()); list_node != nullptr){
			args.reserve(list_node->get_value().size());
			for (auto item : list_node->get_value())
				args.push_back(item->evaluate(runtime));
		}
		else//Single item
			args.push_back(initialization->evaluate(runtime));
	}

	callable->get_value()->call(runtime, callable->get_context(), args);
	target->remove_attribute("#Init#", true);
}

void cminus::type::class_::destruct(logic::runtime &runtime, std::shared_ptr<memory::reference> target) const{
	auto destructor = find(runtime, search_options{ this, target, ("~" + get_naming_value()), false });
	if (auto callable = dynamic_cast<memory::function_reference *>(destructor->get_non_raw()); callable != nullptr)
		callable->get_value()->call(runtime, callable->get_context(), std::vector<std::shared_ptr<memory::reference>>{});
	else
		throw logic::exception("Bad destructor definition", 0u, 0u);
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
	computed_base_type_info info{};
	get_computed_base_info_(target, info);
	return info.offset;
}

bool cminus::type::class_::is_exact(logic::runtime &runtime, const type::object &target) const{
	return (&target == this);
}

cminus::type::object::score_result_type cminus::type::class_::get_score(logic::runtime &runtime, const type::object &target, bool is_ref) const{
	if (&target == this)
		return score_result_type::exact;

	if (converts_auto(target))
		return score_result_type::assignable;

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

std::shared_ptr<cminus::evaluator::object> cminus::type::class_::get_evaluator(logic::runtime &runtime) const{
	return runtime.global_storage->get_evaluator(evaluator::id::class_);
}

std::shared_ptr<cminus::memory::reference> cminus::type::class_::find(logic::runtime &runtime, const search_options &options) const{
	const logic::storage::object *entry_parent = nullptr;
	auto entry = find_(runtime, search_options{ options.scope, options.context, options.name, options.search_tree, &entry_parent });
	if (options.branch != nullptr)//Update branch
		*options.branch = entry_parent;

	if (entry == nullptr || (options.context != nullptr && dynamic_cast<class_ *>(options.context->get_type().get()) != this))
		return entry;

	auto entry_class_parent = dynamic_cast<const class_ *>(entry_parent);
	if (entry_class_parent == nullptr)
		return entry;

	computed_base_type_info computed_info{};
	get_computed_base_info_(*entry_class_parent, computed_info);
	if (computed_info.access == access_type::nil)//Not related
		return entry;

	check_access_(dynamic_cast<const class_ *>(options.scope), computed_info.access, entry);
	if (options.context == nullptr || entry->has_attribute("Static", true, false))
		return entry;

	auto context = ((computed_info.offset == 0u || computed_info.offset == static_cast<std::size_t>(-1)) ? options.context : options.context->apply_offset(computed_info.offset));
	if (context == nullptr)//Error
		return entry;

	if (auto bound_entry = entry->bound_context(runtime, context, computed_info.offset); bound_entry != nullptr){
		if (options.name == "this"){//Update target attribute
			if (context->has_attribute("ReadOnly", true, true))
				bound_entry->add_attribute(runtime.global_storage->find_attribute("&ReadOnly", false));
			else//Remove read-only to pointed object
				bound_entry->remove_attribute("&ReadOnly", true);
		}

		return bound_entry;
	}

	return entry;
}

std::shared_ptr<cminus::memory::reference> cminus::type::class_::find_operator(logic::runtime &runtime, const type::object &target_type, bool is_ref, const namesless_search_options &options) const{
	if (auto primitive_type = dynamic_cast<const type::primitive *>(&target_type); primitive_type != nullptr)
		return find(runtime, search_options{ options.scope, options.context, type::primitive::convert_id_to_string(primitive_type->get_id()), options.search_tree, options.branch });

	if (auto storage_type = dynamic_cast<const with_storage *>(&target_type); storage_type != nullptr)
		return find(runtime, search_options{ options.scope, options.context, storage_type->get_qualified_naming_value(), options.search_tree, options.branch });

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

bool cminus::type::class_::add_declaration(logic::runtime &runtime, std::shared_ptr<declaration::variable> value){
	auto &name = value->get_name();
	if (name.empty())
		return false;

	if (entries_.find(name) != entries_.end())
		return false;

	logic::storage::runtime_storage_guard guard(runtime, std::make_shared<logic::storage::proxy>(*this));
	if (auto non_static_member = value->evaluate_class_member(runtime, size_); non_static_member != nullptr){//Declaration is non-static
		entries_[name] = non_static_member;
		non_static_entries_[name] = non_static_member;

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

void cminus::type::class_::traverse_non_static_entries(const std::function<void(const std::string &, std::shared_ptr<memory::reference>)> &callback) const{
	for (auto &entry : non_static_entries_)
		callback(entry.first, entry.second);
}

bool cminus::type::class_::validate_(const declaration::function_base &target) const{
	if (with_storage::validate_(target))
		return true;

	if (auto class_parent = dynamic_cast<class_ *>(target.get_naming_parent()); class_parent != nullptr && is_ancestor(*class_parent))
		return true;

	return false;
}

void cminus::type::class_::extend_function_group_(logic::runtime &runtime, declaration::function_group_base &group, std::shared_ptr<declaration::function_base> entry){
	if (auto existing_function = group.find(runtime, *entry); existing_function != nullptr){//Function previously declared or defined
		if (auto class_parent = dynamic_cast<class_ *>(existing_function->get_naming_parent()); class_parent != nullptr && is_ancestor(*class_parent))//Override
			group.replace(*existing_function, entry);
		else if (!existing_function->is_defined() && entry->is_defined())
			existing_function->define(entry->get_body());
		else//Duplicate declaration or definition
			throw logic::storage::exception(logic::storage::error_code::duplicate_entry);
	}
	else//New entry
		group.add(entry);
}

std::shared_ptr<cminus::memory::reference> cminus::type::class_::find_(logic::runtime &runtime, const search_options &options) const{
	if (base_types_.empty())
		return with_storage::find(runtime, options);

	search_options non_tree_search_options{ options.scope, options.context, options.name, false, options.branch };
	auto entry = with_storage::find(runtime, non_tree_search_options);

	if (entry != nullptr)//Entry found
		return nullptr;

	for (auto &base_type : base_types_){
		if (auto base_class = dynamic_cast<class_ *>(base_type.second.value.get()); base_class != nullptr && (entry = base_class->find(runtime, non_tree_search_options)) != nullptr)
			return entry;
	}

	if (!options.search_tree)
		return nullptr;

	if (auto storage_parent = dynamic_cast<logic::storage::object *>(parent_); storage_parent != nullptr)
		return storage_parent->find(runtime, options);

	return nullptr;
}

void cminus::type::class_::get_computed_base_info_(const type::object &target, computed_base_type_info &info) const{
	if (&target != this){
		if (!base_types_.empty()){
			info = computed_base_type_info{ access_type::public_, size_ };
			computed_base_type_info current_info{};

			for (auto base_type : base_types_){
				auto class_base_type = dynamic_cast<class_ *>(base_type.second.value.get());
				if (class_base_type == nullptr)
					continue;

				class_base_type->get_computed_base_info_(target, current_info);
				if (current_info.access != access_type::nil){//Found
					info.offset += current_info.offset;
					if (info.access < current_info.access)
						info.access = current_info.access;

					return;
				}

				info.offset += class_base_type->get_size();
			}

			info = computed_base_type_info{ access_type::nil, static_cast<std::size_t>(-1) };
		}
		else//Not related
			info = computed_base_type_info{ access_type::nil, static_cast<std::size_t>(-1) };
	}
	else//Same type
		info = computed_base_type_info{ access_type::private_, 0u };
}

void cminus::type::class_::check_access_(const class_ *scope, access_type access, std::shared_ptr<memory::reference> target) const{
	if (scope != nullptr && scope != this){
		switch (scope->compute_base_offset(*target->get_type())){
		case static_cast<std::size_t>(-1) ://Public access
			if (access < access_type::public_)
				access = access_type::public_;
			break;
		case 0u://Private access
			if (access < access_type::private_)
				access = access_type::private_;
			break;
		default://Protected access
			if (access < access_type::protected_)
				access = access_type::protected_;
			break;
		}
	}
	else if (access < access_type::public_)//Public access
		access = access_type::public_;

	if (target->has_attribute("Private", true, false)){
		if (access != access_type::private_)
			throw logic::exception("Private object is inaccessible", 0u, 0u);
	}
	else if (target->has_attribute("Protected", true, false)){
		if (access != access_type::private_ && access != access_type::protected_)
			throw logic::exception("Private object is inaccessible", 0u, 0u);
	}
}
