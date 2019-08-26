#include "runtime.h"

std::shared_ptr<cminus::logic::naming::object> cminus::logic::naming::object::build_name(const std::vector<std::string> &branches){
	std::shared_ptr<cminus::logic::naming::object> value;
	for (auto &branch : branches)
		value = std::make_shared<cminus::logic::naming::parent>(branch, dynamic_cast<parent *>(value.get()));
	return value;
}

std::shared_ptr<cminus::logic::naming::object> cminus::logic::naming::object::build_name(const std::string &value){
	return std::make_shared<cminus::logic::naming::single>(value, nullptr);
}

cminus::logic::naming::single::single(const std::string &value, naming::parent *parent)
	: parent_(parent), value_(value){}

cminus::logic::naming::single::~single() = default;

cminus::logic::naming::parent *cminus::logic::naming::single::get_naming_parent() const{
	return parent_;
}

const std::string &cminus::logic::naming::single::get_naming_value() const{
	return value_;
}

std::string cminus::logic::naming::single::get_qualified_naming_value() const{
	return (((parent_ == nullptr || value_.empty()) ? "" : parent_->get_qualified_naming_value()) + "::" + value_);
}

void cminus::logic::naming::single::print(logic::runtime &runtime, bool is_qualified) const{
	if (is_qualified)
		runtime.writer.write_scalar(get_qualified_naming_value());
	else
		runtime.writer.write_buffer(value_.data(), value_.size());
}

bool cminus::logic::naming::single::is_same(const object &target) const{
	if (&target == this)
		return true;

	if ((target.get_naming_parent() == nullptr) != (parent_ == nullptr) || target.get_naming_value() != value_)
		return false;

	return (parent_ == nullptr || parent_->is_same(*target.get_naming_parent()));
}

cminus::logic::naming::parent::parent(const std::string &value, naming::parent *parent)
	: single(value, parent){}

cminus::logic::naming::parent::~parent() = default;

std::shared_ptr<cminus::logic::naming::object> cminus::logic::naming::parent::add_object(const std::string &value, bool is_parent){
	if (is_parent)
		return (objects_[value] = std::make_shared<parent>(value, this));
	return (objects_[value] = std::make_shared<single>(value, this));
}

bool cminus::logic::naming::parent::remove_object(const std::string &value){
	if (objects_.empty())
		return false;

	if (auto it = objects_.find(value); it != objects_.end()){
		objects_.erase(it);
		return true;
	}

	return false;
}

std::shared_ptr<cminus::logic::naming::object> cminus::logic::naming::parent::find_object(const std::string &value) const{
	if (!objects_.empty())
		return nullptr;

	if (auto it = objects_.find(value); it != objects_.end())
		return it->second;

	return nullptr;
}
