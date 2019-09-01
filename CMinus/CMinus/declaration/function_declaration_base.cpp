#include "function_declaration_base.h"

cminus::declaration::function_group_base::function_group_base(std::string name, logic::naming::parent *parent)
	: single(name, parent){}

cminus::declaration::function_group_base::~function_group_base() = default;

cminus::declaration::function_base::function_base(std::string name, logic::naming::parent *parent, const attribute_list_type &attributes)
	: single(name, parent), attributes_(attributes){}

cminus::declaration::function_base::~function_base() = default;

std::shared_ptr<cminus::memory::reference> cminus::declaration::function_base::call(logic::runtime &runtime, std::shared_ptr<memory::reference> context, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	if (get_rank(runtime, args) == type::object::score_result_type::nil)
		throw logic::exception("Function does not take the specified arguments", 0u, 0u);
	return call_(runtime, context, args);
}

const cminus::logic::attributes::collection &cminus::declaration::function_base::get_attributes() const{
	return attributes_;
}

cminus::logic::attributes::collection &cminus::declaration::function_base::get_attributes(){
	return attributes_;
}
