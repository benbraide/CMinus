#include "../type/class_type.h"

#include "function_declaration_group.h"

cminus::declaration::function_group::function_group(std::string name, logic::naming::parent *parent)
	: function_group_base(name, parent){}

cminus::declaration::function_group::~function_group() = default;

const cminus::logic::attributes::collection &cminus::declaration::function_group::get_attributes() const{
	return (list_.empty() ? attributes_ : list_.begin()->first->get_attributes());
}

void cminus::declaration::function_group::add(std::shared_ptr<function_base> value){
	list_[value.get()] = value;
}

void cminus::declaration::function_group::replace(function_base &existing_entry, std::shared_ptr<function_base> new_entry){
	if (auto it = list_.find(&existing_entry); it != list_.end()){
		list_.erase(it);
		list_[new_entry.get()] = new_entry;
	}
}

cminus::declaration::function_base *cminus::declaration::function_group::find(logic::runtime &runtime, const function_base &tmpl) const{
	if (list_.empty())
		return nullptr;

	for (auto &item : list_){
		if (item.first->is_exact(runtime, tmpl))
			return item.second.get();
	}

	return nullptr;
}

cminus::declaration::function_base *cminus::declaration::function_group::get_highest_ranked(logic::runtime &runtime, std::shared_ptr<memory::reference> context, const std::vector<std::shared_ptr<memory::reference>> &args, std::size_t *count) const{
	if (list_.empty())
		return nullptr;

	auto is_const = (context != nullptr && context->has_attribute("ReadOnly", true)), is_const_ctx = false;
	int highest_rank_score = type::object::get_score_value(type::object::score_result_type::nil), current_rank_score;

	std::size_t match_count = 0u;
	std::shared_ptr<function_base> highest_ranked;
	for (auto &item : list_){
		if (item.first->is_operator()){
			if (auto class_parent = dynamic_cast<type::class_ *>(item.first->get_naming_parent()); class_parent != nullptr && !item.first->get_attributes().has("Static", true)){//Member operator
				if (args.empty() || class_parent->compute_base_offset(*args[0]->get_type()) == static_cast<std::size_t>(-1))
					continue;//First argument must be related
			}
			else//Non-member or static
				is_const_ctx = is_const;
		}
		else if (dynamic_cast<type::class_ *>(item.first->get_naming_parent()) == nullptr || item.first->get_attributes().has("Static", true))
			is_const_ctx = is_const;
		else//Member function
			is_const_ctx = item.first->get_attributes().has("ReadOnlyContext", true);

		if (is_const && !is_const_ctx)
			continue;

		if (highest_rank_score < (current_rank_score = (type::object::get_score_value(item.first->get_rank(runtime, args)) - ((is_const_ctx == is_const) ? 0 : 1)))){
			highest_rank_score = current_rank_score;
			highest_ranked = item.second;
			match_count = 1u;
		}
		else if (highest_rank_score == current_rank_score)
			++match_count;
	}

	if (count != nullptr)
		*count = match_count;

	return highest_ranked.get();
}

std::shared_ptr<cminus::memory::reference> cminus::declaration::function_group::call(logic::runtime &runtime, std::shared_ptr<memory::reference> context, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	std::size_t match_count = 0u;
	auto highest_ranked = get_highest_ranked(runtime, context, args, &match_count);
	if (highest_ranked == nullptr)
		throw logic::exception("Function does not take the specified arguments", 0u, 0u);

	if (1u < match_count)
		throw logic::exception("Function call is ambiguous", 0u, 0u);

	if (args.empty())
		return highest_ranked->call_(runtime, context, args);

	auto class_parent = dynamic_cast<type::class_ *>(highest_ranked->get_naming_parent());
	if (class_parent == nullptr)
		return highest_ranked->call_(runtime, context, args);

	if (auto offset = class_parent->compute_base_offset(*args[0]->get_type()); 0u < offset && offset != static_cast<std::size_t>(-1))
		context = context->apply_offset(runtime, offset);//Adjust context

	if (!highest_ranked->is_operator() || highest_ranked->get_attributes().has("Static", true))
		return highest_ranked->call_(runtime, context, args);

	std::vector<std::shared_ptr<memory::reference>> mem_args;
	if (1u < args.size()){//Ignore first argument
		mem_args.reserve(args.size() - 1u);
		for (auto arg : args)
			mem_args.push_back(arg);
	}

	return highest_ranked->call_(runtime, context, args);
}
