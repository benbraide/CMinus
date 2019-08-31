#include "../logic/runtime.h"

cminus::memory::reference::reference(std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context)
	: type_(type), context_(context){
	if (type_ != nullptr)
		size_ = type_->get_size();

	for (auto attribute : attributes)
		attributes_[attribute.get()] = attribute;
}

cminus::memory::reference::~reference() = default;

void cminus::memory::reference::set_type(std::shared_ptr<type::object> value){
	type_ = value;
	size_ = ((type_ == nullptr) ? 0u : type_->get_size());
}

std::shared_ptr<cminus::type::object> cminus::memory::reference::get_type() const{
	return type_;
}

bool cminus::memory::reference::is_nan() const{
	auto primitive_type = dynamic_cast<type::primitive *>(type_.get());
	if (primitive_type == nullptr)
		return false;

	if (primitive_type->get_id() == type::primitive::id_type::nan_)
		return true;

	return (primitive_type->is_numeric() && find_attribute("#NaN#", true, false) != nullptr);
}

void cminus::memory::reference::set_context(std::shared_ptr<reference> value){
	context_ = value;
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

const cminus::memory::reference::optimised_attribute_list_type cminus::memory::reference::get_attributes() const{
	return attributes_;
}

void cminus::memory::reference::traverse_attributes(logic::runtime &runtime, const std::function<void(std::shared_ptr<logic::attributes::object>)> &callback, logic::attributes::object::stage_type stage, bool include_context) const{
	for (auto &attribute : attributes_){
		if (stage == logic::attributes::object::stage_type::nil || attribute.first->handles_stage(runtime, stage))
			callback(attribute.second);
	}

	if (include_context && context_ != nullptr)
		context_->traverse_attributes(runtime, callback, stage, true);
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

cminus::memory::placeholder_reference::placeholder_reference(std::size_t relative_offset, std::shared_ptr<type::object> type, const attribute_list_type &attributes)
	: reference(type, attributes, nullptr), relative_offset_(relative_offset){}

cminus::memory::placeholder_reference::placeholder_reference(std::size_t relative_offset, std::shared_ptr<type::object> type)
	: placeholder_reference(relative_offset, type, attribute_list_type{}){}

cminus::memory::placeholder_reference::~placeholder_reference() = default;

std::shared_ptr<cminus::memory::reference> cminus::memory::placeholder_reference::apply_offset(std::size_t value) const{
	attribute_list_type attributes;
	if (!attributes_.empty()){
		attributes.reserve(attributes_.size());
		for (auto &attribute : attributes_)
			attributes.push_back(attribute.second);
	}

	return std::make_shared<placeholder_reference>((relative_offset_ + value), type_, attributes);
}

bool cminus::memory::placeholder_reference::is_lvalue() const{
	return true;
}

std::size_t cminus::memory::placeholder_reference::get_address() const{
	return 0u;
}

std::byte *cminus::memory::placeholder_reference::get_data(logic::runtime &runtime) const{
	return nullptr;
}

std::size_t cminus::memory::placeholder_reference::read(logic::runtime &runtime, std::byte *buffer, std::size_t size) const{
	return 0u;
}

std::size_t cminus::memory::placeholder_reference::read(logic::runtime &runtime, io::binary_writer &buffer, std::size_t size) const{
	return 0u;
}

std::size_t cminus::memory::placeholder_reference::read(logic::runtime &runtime, std::size_t buffer, std::size_t size) const{
	return 0u;
}

std::size_t cminus::memory::placeholder_reference::read(logic::runtime &runtime, reference &buffer, std::size_t size) const{
	return 0u;
}

std::size_t cminus::memory::placeholder_reference::write(logic::runtime &runtime, const std::byte *buffer, std::size_t size){
	return 0u;
}

std::size_t cminus::memory::placeholder_reference::write(logic::runtime &runtime, const io::binary_reader &buffer, std::size_t size){
	return 0u;
}

std::size_t cminus::memory::placeholder_reference::write(logic::runtime &runtime, std::size_t buffer, std::size_t size){
	return 0u;
}

std::size_t cminus::memory::placeholder_reference::write(logic::runtime &runtime, const reference &buffer, std::size_t size){
	return 0u;
}

std::size_t cminus::memory::placeholder_reference::write(logic::runtime &runtime, managed_object &object){
	return 0u;
}

std::size_t cminus::memory::placeholder_reference::set(logic::runtime &runtime, std::byte value, std::size_t size){
	return 0u;
}

void cminus::memory::placeholder_reference::set_relative_offset(std::size_t value){
	relative_offset_ = value;
}

std::size_t cminus::memory::placeholder_reference::get_relative_offset() const{
	return relative_offset_;
}

std::shared_ptr<cminus::memory::reference> cminus::memory::placeholder_reference::create(logic::runtime &runtime, std::shared_ptr<reference> context, std::size_t offset) const{
	attribute_list_type attributes;
	if (!attributes_.empty()){
		attributes.reserve(attributes_.size());
		for (auto &attribute : attributes_)
			attributes.push_back(attribute.second);
	}

	if (auto lval_context = dynamic_cast<const lval_reference *>(context.get()); lval_context != nullptr){
		if (find_attribute("Ref", true, false) == nullptr)
			return std::make_shared<lval_reference>((lval_context->get_address() + relative_offset_ + offset), type_, attributes, context);
		return std::make_shared<ref_reference>(type_, attributes, context);
	}

	if (auto rval_context = dynamic_cast<const rval_reference *>(context.get()); rval_context != nullptr)
		return std::make_shared<rval_reference>((rval_context->get_data(runtime) + (relative_offset_ + offset)), type_, attributes, context);

	return nullptr;
}

cminus::memory::lval_reference::lval_reference(logic::runtime &runtime, std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context)
	: reference(type, attributes, context){
	try{
		allocate_memory_(runtime, type_->get_size());
	}
	catch (...){
		address_ = 0u;
	}
}

cminus::memory::lval_reference::lval_reference(logic::runtime &runtime, std::shared_ptr<type::object> type, std::shared_ptr<reference> context)
	: lval_reference(runtime, type, attribute_list_type{}, context){}

cminus::memory::lval_reference::lval_reference(std::size_t address, std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context)
	: reference(type, attributes, context), address_(address){}

cminus::memory::lval_reference::lval_reference(std::size_t address, std::shared_ptr<type::object> type, std::shared_ptr<reference> context)
	: lval_reference(address, type, attribute_list_type{}, context){}

cminus::memory::lval_reference::~lval_reference(){
	try{
		if (deallocator_ != nullptr)
			deallocator_();
	}
	catch (...){}
}

std::shared_ptr<cminus::memory::reference> cminus::memory::lval_reference::apply_offset(std::size_t value) const{
	attribute_list_type attributes;
	if (!attributes_.empty()){
		attributes.reserve(attributes_.size());
		for (auto &attribute : attributes_)
			attributes.push_back(attribute.second);
	}

	return std::make_shared<lval_reference>((address_ + value), type_, attributes, context_);
}

bool cminus::memory::lval_reference::is_lvalue() const{
	return (true && (context_ == nullptr || context_->is_lvalue()));
}

std::size_t cminus::memory::lval_reference::get_address() const{
	return address_;
}

std::byte *cminus::memory::lval_reference::get_data(logic::runtime &runtime) const{
	auto block = runtime.memory_object.find_block(get_address());
	return ((block == nullptr) ? nullptr : block->get_data());
}

std::size_t cminus::memory::lval_reference::read(logic::runtime &runtime, std::byte *buffer, std::size_t size) const{
	return runtime.memory_object.read(get_address(), buffer, ((size == static_cast<std::size_t>(-1) || size_ < size) ? size_ : size));
}

std::size_t cminus::memory::lval_reference::read(logic::runtime &runtime, io::binary_writer &buffer, std::size_t size) const{
	return runtime.memory_object.read(get_address(), buffer, ((size == static_cast<std::size_t>(-1) || size_ < size) ? size_ : size));
}

std::size_t cminus::memory::lval_reference::read(logic::runtime &runtime, std::size_t buffer, std::size_t size) const{
	return runtime.memory_object.read(get_address(), buffer, ((size == static_cast<std::size_t>(-1) || size_ < size) ? size_ : size));
}

std::size_t cminus::memory::lval_reference::read(logic::runtime &runtime, reference &buffer, std::size_t size) const{
	return buffer.write(runtime, get_address(), ((size == static_cast<std::size_t>(-1) || size_ < size) ? size_ : size));
}

std::size_t cminus::memory::lval_reference::write(logic::runtime &runtime, const std::byte *buffer, std::size_t size){
	return runtime.memory_object.write(get_address(), buffer, ((size == static_cast<std::size_t>(-1) || size_ < size) ? size_ : size));
}

std::size_t cminus::memory::lval_reference::write(logic::runtime &runtime, const io::binary_reader &buffer, std::size_t size){
	return runtime.memory_object.write(get_address(), buffer, ((size == static_cast<std::size_t>(-1) || size_ < size) ? size_ : size));
}

std::size_t cminus::memory::lval_reference::write(logic::runtime &runtime, std::size_t buffer, std::size_t size){
	return runtime.memory_object.write(buffer, get_address(), ((size == static_cast<std::size_t>(-1) || size_ < size) ? size_ : size));
}

std::size_t cminus::memory::lval_reference::write(logic::runtime &runtime, const reference &buffer, std::size_t size){
	return buffer.read(runtime, get_address(), ((size == static_cast<std::size_t>(-1) || size_ < size) ? size_ : size));
}

std::size_t cminus::memory::lval_reference::write(logic::runtime &runtime, managed_object &object){
	return runtime.memory_object.write(get_address(), object);
}

std::size_t cminus::memory::lval_reference::set(logic::runtime &runtime, std::byte value, std::size_t size){
	return runtime.memory_object.set(get_address(), value, ((size == static_cast<std::size_t>(-1) || size_ < size) ? size_ : size));
}

void cminus::memory::lval_reference::allocate_memory_(logic::runtime &runtime, std::size_t size){
	auto block = runtime.memory_object.allocate_block(size, memory::block::attribute_none);
	if (block == nullptr)
		return;

	add_attribute(runtime.global_storage->find_attribute("#LVal", false));
	address_ = block->get_address();
	deallocator_ = [address = address_, &memory_object = runtime.memory_object](){
		if (address != 0u)
			memory_object.deallocate_block(address);
	};
}

cminus::memory::ref_reference::ref_reference(std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context)
	: lval_reference(0u, type, attributes, context){}

cminus::memory::ref_reference::ref_reference(std::shared_ptr<type::object> type, std::shared_ptr<reference> context)
	: ref_reference(type, attribute_list_type{}, context){}

cminus::memory::ref_reference::~ref_reference() = default;

void cminus::memory::ref_reference::write_address(std::size_t value){
	address_ = value;
}

cminus::memory::rval_reference::rval_reference(std::byte *data, std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context)
	: reference(type, attributes, context), data_(data){}

cminus::memory::rval_reference::rval_reference(std::byte *data, std::shared_ptr<type::object> type, std::shared_ptr<reference> context)
	: rval_reference(data, type, attribute_list_type{}, context){}

cminus::memory::rval_reference::~rval_reference() = default;

std::shared_ptr<cminus::memory::reference> cminus::memory::rval_reference::apply_offset(std::size_t value) const{
	attribute_list_type attributes;
	if (!attributes_.empty()){
		attributes.reserve(attributes_.size());
		for (auto &attribute : attributes_)
			attributes.push_back(attribute.second);
	}

	return std::make_shared<rval_reference>((data_ + value), type_, attributes, context_);
}

bool cminus::memory::rval_reference::is_lvalue() const{
	return false;
}

std::size_t cminus::memory::rval_reference::get_address() const{
	return 0u;
}

std::byte *cminus::memory::rval_reference::get_data(logic::runtime &runtime) const{
	return data_;
}

std::size_t cminus::memory::rval_reference::read(logic::runtime &runtime, std::byte *buffer, std::size_t size) const{
	if (size == static_cast<std::size_t>(-1) || size_ < size)//Restrict size
		size = size_;

	if (size == 0u)//Do nothing
		return 0u;

	if (0u < size && size <= size_)
		memcpy(buffer, data_, size);

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::rval_reference::read(logic::runtime &runtime, io::binary_writer &buffer, std::size_t size) const{
	if (size == static_cast<std::size_t>(-1) || size_ < size)//Restrict size
		size = size_;

	if (size == 0u)//Do nothing
		return 0u;

	if (0u < size && size <= size_)
		size = buffer.write(data_, size);

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::rval_reference::read(logic::runtime &runtime, std::size_t buffer, std::size_t size) const{
	if (size == static_cast<std::size_t>(-1) || size_ < size)//Restrict size
		size = size_;

	if (size == 0u)//Do nothing
		return 0u;

	if (0u < size && size <= size_)
		size = runtime.memory_object.write(buffer, data_, size);

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::rval_reference::read(logic::runtime &runtime, reference &buffer, std::size_t size) const{
	if (size == static_cast<std::size_t>(-1) || size_ < size)//Restrict size
		size = size_;

	if (size == 0u)//Do nothing
		return 0u;

	if (0u < size && size <= size_)
		size = buffer.write(runtime, data_, size);

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::rval_reference::write(logic::runtime &runtime, const std::byte *buffer, std::size_t size){
	if (size == static_cast<std::size_t>(-1) || size_ < size)//Restrict size
		size = size_;

	if (size == 0u)//Do nothing
		return 0u;

	if (0u < size && size <= size_)
		memcpy(data_, buffer, size);

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::rval_reference::write(logic::runtime &runtime, const io::binary_reader &buffer, std::size_t size){
	if (size == static_cast<std::size_t>(-1) || size_ < size)//Restrict size
		size = size_;

	if (size == 0u)//Do nothing
		return 0u;

	if (0u < size && size <= size_)
		size = buffer.read(data_, size);

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::rval_reference::write(logic::runtime &runtime, std::size_t buffer, std::size_t size){
	if (size == static_cast<std::size_t>(-1) || size_ < size)//Restrict size
		size = size_;

	if (size == 0u)//Do nothing
		return 0u;

	if (0u < size && size <= size_)
		size = runtime.memory_object.read(buffer, data_, size);

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::rval_reference::write(logic::runtime &runtime, const reference &buffer, std::size_t size){
	if (size == static_cast<std::size_t>(-1) || size_ < size)//Restrict size
		size = size_;

	if (size == 0u)//Do nothing
		return 0u;

	if (0u < size && size <= size_)
		size = buffer.read(runtime, data_, size);

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::rval_reference::write(logic::runtime &runtime, managed_object &object){
	return 0u;
}

std::size_t cminus::memory::rval_reference::set(logic::runtime &runtime, std::byte value, std::size_t size){
	if (size == static_cast<std::size_t>(-1) || size_ < size)//Restrict size
		size = size_;

	if (size == 0u)//Do nothing
		return 0u;

	if (0u < size && size <= size_)
		memset(data_, static_cast<int>(value), size);

	return ((size <= size_) ? size : 0u);
}

cminus::memory::data_reference::data_reference(std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context)
	: rval_reference(nullptr, type, attributes, context){
	if (0u < size_){
		value_ = std::make_unique<std::byte[]>(size_);
		data_ = value_.get();
	}
}

cminus::memory::data_reference::data_reference(std::shared_ptr<type::object> type, std::shared_ptr<reference> context)
	: data_reference(type, attribute_list_type{}, context){}

cminus::memory::data_reference::~data_reference() = default;
