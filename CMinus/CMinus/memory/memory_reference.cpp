#include "../logic/runtime.h"

cminus::memory::reference::reference(std::shared_ptr<logic::type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context)
	: type_(type), context_(context){
	for (auto attribute : attributes)
		attributes_[attribute.get()] = attribute;
}

cminus::memory::reference::~reference() = default;

std::shared_ptr<cminus::memory::reference> cminus::memory::reference::clone(const attribute_list_type &attributes, bool inherit_attributes) const{
	if (!inherit_attributes)
		return clone_(attributes);

	attribute_list_type combined_attributes;
	combined_attributes.reserve(attributes_.size() + attributes.size());

	for (auto &attribute : attributes_)
		combined_attributes.push_back(attribute.second);

	for (auto &attribute : attributes)
		combined_attributes.push_back(attribute);

	return clone_(combined_attributes);
}

std::shared_ptr<cminus::logic::type::object> cminus::memory::reference::get_type() const{
	return type_;
}

std::shared_ptr<cminus::memory::reference> cminus::memory::reference::get_context() const{
	return context_;
}

void cminus::memory::reference::add_attribute(std::shared_ptr<logic::attributes::object> value){
	attributes_[value.get()] = value;
}

void cminus::memory::reference::remove_attribute(const std::string &name, bool global_only){
	if (attributes_.empty())
		return;

	for (auto it = attributes_.begin(); it != attributes_.end(); ++it){
		if ((it->first->get_naming_parent() == nullptr) == global_only && it->first->get_naming_value() == name){
			attributes_.erase(it);
			break;
		}
	}
}

void cminus::memory::reference::remove_attribute(std::shared_ptr<logic::naming::object> name){
	if (attributes_.empty())
		return;

	for (auto it = attributes_.begin(); it != attributes_.end(); ++it){
		if (it->first->is_same(*name)){
			attributes_.erase(it);
			break;
		}
	}
}

std::shared_ptr<cminus::logic::attributes::object> cminus::memory::reference::find_attribute(const std::string &name, bool global_only, bool include_context) const{
	if (attributes_.empty())
		return nullptr;

	for (auto &attribute : attributes_){
		if ((attribute.first->get_naming_parent() == nullptr) == global_only && attribute.first->get_naming_value() == name)
			return attribute.second;
	}

	return ((include_context && context_ != nullptr) ? context_->find_attribute(name, global_only, true) : nullptr);
}

std::shared_ptr<cminus::logic::attributes::object> cminus::memory::reference::find_attribute(std::shared_ptr<logic::naming::object> name, bool include_context) const{
	if (attributes_.empty())
		return nullptr;

	for (auto &attribute : attributes_){
		if (attribute.first->is_same(*name))
			return attribute.second;
	}

	return ((include_context && context_ != nullptr) ? context_->find_attribute(name, true) : nullptr);
}

void cminus::memory::reference::traverse_attributes(const std::function<void(std::shared_ptr<logic::attributes::object>)> &callback, logic::attributes::object::stage_type stage, bool include_context) const{
	for (auto &attribute : attributes_){
		if (stage == logic::attributes::object::stage_type::nil || attribute.first->handles_stage(stage))
			callback(attribute.second);
	}

	if (include_context && context_ != nullptr)
		context_->traverse_attributes(callback, stage, true);
}

bool cminus::memory::reference::is_lvalue() const{
	return (get_address() != 0u);
}

std::size_t cminus::memory::reference::get_address() const{
	return 0u;
}

const std::byte *cminus::memory::reference::get_data(logic::runtime &runtime) const{
	return nullptr;
}

std::size_t cminus::memory::reference::read(logic::runtime &runtime, std::byte *buffer, std::size_t size) const{
	return 0u;
}

std::size_t cminus::memory::reference::read(logic::runtime &runtime, io::binary_writer &buffer, std::size_t size) const{
	return 0u;
}

std::size_t cminus::memory::reference::read(logic::runtime &runtime, std::size_t buffer, std::size_t size) const{
	return 0u;
}

std::size_t cminus::memory::reference::read(logic::runtime &runtime, reference &buffer, std::size_t size) const{
	return 0u;
}

std::size_t cminus::memory::reference::write(logic::runtime &runtime, const std::byte *buffer, std::size_t size){
	return 0u;
}

std::size_t cminus::memory::reference::write(logic::runtime &runtime, const io::binary_reader &buffer, std::size_t size){
	return 0u;
}

std::size_t cminus::memory::reference::write(logic::runtime &runtime, std::size_t buffer, std::size_t size){
	return 0u;
}

std::size_t cminus::memory::reference::write(logic::runtime &runtime, const reference &buffer, std::size_t size){
	return 0u;
}

std::size_t cminus::memory::reference::write(logic::runtime &runtime, managed_object &object){
	return 0u;
}

std::size_t cminus::memory::reference::set(logic::runtime &runtime, std::byte value, std::size_t size){
	return 0u;
}

cminus::memory::hard_reference::hard_reference(logic::runtime &runtime, std::shared_ptr<logic::type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context)
	: reference(type, attributes, context){
	try{
		if (auto block = runtime.memory_object.allocate_block(type->get_size(), memory::block::attribute_none); block != nullptr){
			address_ = block->get_address();
			block->set(0u, static_cast<std::byte>(0), block->get_size());
			deallocator_ = [this, &memory_object = runtime.memory_object](){
				if (address_ != 0u)
					memory_object.deallocate_block(address_);
			};
		}
	}
	catch (...){
		address_ = 0u;
	}
}

cminus::memory::hard_reference::hard_reference(logic::runtime &runtime, std::shared_ptr<logic::type::object> type, std::shared_ptr<reference> context)
	: hard_reference(runtime, type, attribute_list_type{}, context){}

cminus::memory::hard_reference::hard_reference(std::size_t address, std::shared_ptr<logic::type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context)
	: reference(type, attributes, context), address_(address){}

cminus::memory::hard_reference::hard_reference(std::size_t address, std::shared_ptr<logic::type::object> type, std::shared_ptr<reference> context)
	: hard_reference(address, type, attribute_list_type{}, context){}

cminus::memory::hard_reference::~hard_reference(){
	try{
		if (deallocator_ != nullptr)
			deallocator_();
	}
	catch (...){}
}

std::size_t cminus::memory::hard_reference::get_address() const{
	return address_;
}

const std::byte *cminus::memory::hard_reference::get_data(logic::runtime &runtime) const{
	auto block = runtime.memory_object.find_block(address_);
	return ((block == nullptr) ? nullptr : block->get_data());
}

std::size_t cminus::memory::hard_reference::read(logic::runtime &runtime, std::byte *buffer, std::size_t size) const{
	return runtime.memory_object.read(address_, buffer, size);
}

std::size_t cminus::memory::hard_reference::read(logic::runtime &runtime, io::binary_writer &buffer, std::size_t size) const{
	return runtime.memory_object.read(address_, buffer, size);
}

std::size_t cminus::memory::hard_reference::read(logic::runtime &runtime, std::size_t buffer, std::size_t size) const{
	return runtime.memory_object.read(address_, buffer, size);
}

std::size_t cminus::memory::hard_reference::read(logic::runtime &runtime, reference &buffer, std::size_t size) const{
	if (auto address = buffer.get_address(); address != 0u)
		return runtime.memory_object.read(address_, address, size);
	return 0u;
}

std::size_t cminus::memory::hard_reference::write(logic::runtime &runtime, const std::byte *buffer, std::size_t size){
	return runtime.memory_object.write(address_, buffer, size);
}

std::size_t cminus::memory::hard_reference::write(logic::runtime &runtime, const io::binary_reader &buffer, std::size_t size){
	return runtime.memory_object.write(address_, buffer, size);
}

std::size_t cminus::memory::hard_reference::write(logic::runtime &runtime, std::size_t buffer, std::size_t size){
	return runtime.memory_object.write(buffer, address_, size);
}

std::size_t cminus::memory::hard_reference::write(logic::runtime &runtime, const reference &buffer, std::size_t size){
	if (auto address = buffer.get_address(); address != 0u)
		return runtime.memory_object.write(address, address_, size);

	if (auto data = buffer.get_data(runtime); data != nullptr)
		return runtime.memory_object.write(address_, data, size);

	return 0u;
}

std::size_t cminus::memory::hard_reference::write(logic::runtime &runtime, managed_object &object){
	return runtime.memory_object.write(address_, object);
}

std::size_t cminus::memory::hard_reference::set(logic::runtime &runtime, std::byte value, std::size_t size){
	return runtime.memory_object.set(address_, value, size);
}

std::shared_ptr<cminus::memory::reference> cminus::memory::hard_reference::clone_(const attribute_list_type &attributes) const{
	return std::make_shared<hard_reference>(address_, type_, attributes, context_);
}

/*
cminus::memory::proxy_reference::proxy_reference(std::shared_ptr<reference> target, unsigned int attributes)
	: reference(target->get_type(), attributes), target_(target){}

cminus::memory::proxy_reference::~proxy_reference() = default;

void cminus::memory::proxy_reference::write(logic::runtime &runtime, std::shared_ptr<reference> source){
	target_->write(runtime, source);
}

void cminus::memory::proxy_reference::write(logic::runtime &runtime, const std::byte *source, std::shared_ptr<logic::type::object> type){
	target_->write(runtime, source, type);
}

unsigned int cminus::memory::proxy_reference::get_attributes() const{
	return (target_->get_attributes() | attributes_);
}

std::size_t cminus::memory::proxy_reference::get_address() const{
	return target_->get_address();
}

const std::byte *cminus::memory::proxy_reference::get_data() const{
	return target_->get_data();
}

std::shared_ptr<cminus::memory::reference> cminus::memory::proxy_reference::get_target() const{
	return target_;
}*/
