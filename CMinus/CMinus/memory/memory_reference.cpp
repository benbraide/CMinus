#include "../logic/runtime.h"

cminus::memory::reference::reference(std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context)
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

std::shared_ptr<cminus::type::object> cminus::memory::reference::get_type() const{
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

void cminus::memory::reference::traverse_attributes(logic::runtime &runtime, const std::function<void(std::shared_ptr<logic::attributes::object>)> &callback, logic::attributes::object::stage_type stage, bool include_context) const{
	for (auto &attribute : attributes_){
		if (stage == logic::attributes::object::stage_type::nil || attribute.first->handles_stage(runtime, stage))
			callback(attribute.second);
	}

	if (include_context && context_ != nullptr)
		context_->traverse_attributes(runtime, callback, stage, true);
}

bool cminus::memory::reference::is_lvalue() const{
	return (find_attribute("#LVal#", true, false) != nullptr && (context_ == nullptr || context_->is_lvalue()));
}

bool cminus::memory::reference::is_ref() const{
	return false;
}

bool cminus::memory::reference::is_nan() const{
	if (auto primitive_type = dynamic_cast<type::primitive *>(type_.get()); primitive_type != nullptr && primitive_type->get_id() == type::primitive::primitive::id_type::nan_)
		return true;
	return (find_attribute("#NaN#", true, false) != nullptr);
}

void cminus::memory::reference::set_address(std::size_t value){}

std::size_t cminus::memory::reference::get_address() const{
	return 0u;
}

const std::byte *cminus::memory::reference::get_data(logic::runtime &runtime) const{
	return nullptr;
}

int cminus::memory::reference::compare(logic::runtime &runtime, const std::byte *buffer, std::size_t size) const{
	if (buffer == nullptr || type_->get_size() != size)
		return std::numeric_limits<int>::min();

	auto result = 0;
	if (auto address = get_address(); address != 0u){
		if (runtime.memory_object.compare(result, address, buffer, size) != size)
			return std::numeric_limits<int>::min();
	}
	else if (auto data = get_data(runtime); data != nullptr)
		result = memcmp(data, buffer, size);
	else
		result = std::numeric_limits<int>::min();

	return result;
}

int cminus::memory::reference::compare(logic::runtime &runtime, std::size_t buffer, std::size_t size) const{
	if (buffer == 0u || type_->get_size() != size)
		return std::numeric_limits<int>::min();

	auto result = 0;
	if (auto address = get_address(); address != 0u){
		if (runtime.memory_object.compare(result, address, buffer, size) != size)
			return std::numeric_limits<int>::min();
	}
	else if (auto data = get_data(runtime); data != nullptr){
		if (runtime.memory_object.compare(result, buffer, data, size) != size)
			return std::numeric_limits<int>::min();

		switch (result){
		case -1:
		case 1:
			return -result;
		default:
			break;
		}

		return result;
	}

	return result;
}

int cminus::memory::reference::compare(logic::runtime &runtime, const reference &buffer, std::size_t size) const{
	if (auto address = buffer.get_address(); address != 0u)
		return compare(runtime, address, size);
	return compare(runtime, buffer.get_data(runtime), size);
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

cminus::memory::hard_reference::hard_reference(logic::runtime &runtime, std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context)
	: reference(type, attributes, context){
	try{
		if (find_attribute("Ref", true, false) == nullptr){//Destination address will be copied
			if (auto block = runtime.memory_object.allocate_block(type->get_size(), memory::block::attribute_none); block != nullptr){
				add_attribute(runtime.global_storage->find_attribute("#LVal", false));
				address_ = block->get_address();
				deallocator_ = [address = address_, &memory_object = runtime.memory_object](){
					if (address != 0u)
						memory_object.deallocate_block(address);
				};
			}
		}
	}
	catch (...){
		address_ = 0u;
	}
}

cminus::memory::hard_reference::hard_reference(logic::runtime &runtime, std::shared_ptr<type::object> type, std::shared_ptr<reference> context)
	: hard_reference(runtime, type, attribute_list_type{}, context){}

cminus::memory::hard_reference::hard_reference(std::size_t address, std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context)
	: reference(type, attributes, context), address_(address){}

cminus::memory::hard_reference::hard_reference(std::size_t address, std::shared_ptr<type::object> type, std::shared_ptr<reference> context)
	: hard_reference(address, type, attribute_list_type{}, context){}

cminus::memory::hard_reference::~hard_reference(){
	try{
		if (deallocator_ != nullptr)
			deallocator_();
	}
	catch (...){}
}

bool cminus::memory::hard_reference::is_ref() const{
	return (address_ != 0u && deallocator_ == nullptr);
}

void cminus::memory::hard_reference::set_address(std::size_t value){
	if (find_attribute("#Init#", true, false) != nullptr && find_attribute("Ref", true, false) != nullptr)
		address_ = value;
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

void cminus::memory::reference::call_attributes(logic::runtime &runtime, logic::attributes::object::stage_type stage, bool include_context, std::shared_ptr<memory::reference> target, const std::vector<std::shared_ptr<memory::reference>> &args){
	target->traverse_attributes(runtime, [&](std::shared_ptr<logic::attributes::object> attr){
		attr->call(runtime, stage, target, args);
	}, stage, include_context);
}

void cminus::memory::reference::call_attributes(logic::runtime &runtime, logic::attributes::object::stage_type stage, bool include_context, std::shared_ptr<memory::reference> target){
	target->traverse_attributes(runtime, [&](std::shared_ptr<logic::attributes::object> attr){
		attr->call(runtime, stage, target);
	}, stage, include_context);
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

void cminus::memory::proxy_reference::write(logic::runtime &runtime, const std::byte *source, std::shared_ptr<type::object> type){
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
