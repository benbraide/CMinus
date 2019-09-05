#include "node_object.h"

cminus::node::object::object(object *parent)
	: parent_(parent){}

cminus::node::object::~object() = default;

cminus::node::object *cminus::node::object::get_parent() const{
	return parent_;
}

void cminus::node::object::substitute(const std::string &key, std::shared_ptr<object> value){}

void cminus::node::object::substitute(const object &target, std::shared_ptr<object> value){}

std::shared_ptr<cminus::memory::reference> cminus::node::object::lookup(logic::runtime &runtime, std::shared_ptr<memory::reference> context) const{
	return nullptr;
}

void cminus::node::object::set_parent_(std::shared_ptr<object> target){
	target->parent_ = this;
}

cminus::node::object_with_index::object_with_index(object *parent, const index_info &index)
	: object(parent), index_(index){}

cminus::node::object_with_index::~object_with_index() = default;

const cminus::node::object::index_info &cminus::node::object_with_index::get_index() const{
	return index_;
}
