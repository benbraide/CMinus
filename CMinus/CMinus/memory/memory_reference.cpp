#include "../declaration/function_declaration_group.h"

cminus::memory::reference::reference(std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context)
	: type_(type), attributes_(attributes), context_(context){
	if (type_ != nullptr)
		size_ = type_->get_size();
}

cminus::memory::reference::reference(std::shared_ptr<type::object> type, const optimised_attribute_list_type &attributes, std::shared_ptr<reference> context)
	: type_(type), attributes_(attributes), context_(context){
	if (type_ != nullptr)
		size_ = type_->get_size();
}

cminus::memory::reference::reference(std::shared_ptr<type::object> type, const logic::attributes::collection &attributes, std::shared_ptr<reference> context)
	: type_(type), attributes_(attributes), context_(context){
	if (type_ != nullptr)
		size_ = type_->get_size();
}

cminus::memory::reference::~reference(){
	try{
		if (deallocator_ != nullptr){
			deallocator_();
			deallocator_ = nullptr;
		}
	}
	catch (...){}
}

cminus::memory::reference *cminus::memory::reference::get_non_raw() const{
	return const_cast<reference *>(this);
}

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

std::shared_ptr<cminus::memory::reference> cminus::memory::reference::bound_context(logic::runtime &runtime, std::shared_ptr<reference> value, std::size_t offset) const{
	return nullptr;
}

std::shared_ptr<cminus::memory::reference> cminus::memory::reference::get_context() const{
	return context_;
}

void cminus::memory::reference::add_attribute(std::shared_ptr<logic::attributes::object> value){
	attributes_.add(value);
}

void cminus::memory::reference::remove_attribute(const std::string &name, bool global_only){
	attributes_.remove(name, global_only);
}

void cminus::memory::reference::remove_attribute(const logic::naming::object &name){
	attributes_.remove(name);
}

std::shared_ptr<cminus::logic::attributes::object> cminus::memory::reference::find_attribute(const std::string &name, bool global_only, bool include_context) const{
	if (auto entry = attributes_.find(name, global_only); entry != nullptr)
		return entry;

	return ((include_context && context_ != nullptr) ? context_->find_attribute(name, global_only, true) : nullptr);
}

std::shared_ptr<cminus::logic::attributes::object> cminus::memory::reference::find_attribute(const logic::naming::object &name, bool include_context) const{
	if (auto entry = attributes_.find(name); entry != nullptr)
		return entry;

	return ((include_context && context_ != nullptr) ? context_->find_attribute(name, true) : nullptr);
}

bool cminus::memory::reference::has_attribute(const std::string &name, bool global_only, bool include_context) const{
	return (find_attribute(name, global_only, include_context) != nullptr);
}

bool cminus::memory::reference::has_attribute(const logic::naming::object &name, bool include_context) const{
	return (find_attribute(name, include_context) != nullptr);
}

const cminus::logic::attributes::collection &cminus::memory::reference::get_attributes() const{
	return attributes_;
}

void cminus::memory::reference::traverse_attributes(logic::runtime &runtime, const std::function<void(std::shared_ptr<logic::attributes::object>)> &callback, logic::attributes::object::stage_type stage, bool include_context) const{
	attributes_.traverse(runtime, callback, stage);
	if (include_context && context_ != nullptr)
		context_->traverse_attributes(runtime, callback, stage, true);
}

void cminus::memory::reference::call_attributes(logic::runtime &runtime, logic::attributes::object::stage_type stage, bool include_context, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	attributes_.call(runtime, stage, std::make_shared<raw_reference>(const_cast<reference *>(this)), args);
	if (include_context && context_ != nullptr)
		context_->call_attributes(runtime, stage, true, args);
}

void cminus::memory::reference::call_attributes(logic::runtime &runtime, logic::attributes::object::stage_type stage, bool include_context) const{
	call_attributes(runtime, stage, include_context, std::vector<std::shared_ptr<memory::reference>>{});
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

cminus::memory::placeholder_reference::placeholder_reference(std::size_t relative_offset, std::shared_ptr<type::object> type, const attribute_list_type &attributes)
	: reference(type, attributes, nullptr), relative_offset_(relative_offset){}

cminus::memory::placeholder_reference::placeholder_reference(std::size_t relative_offset, std::shared_ptr<type::object> type, const optimised_attribute_list_type &attributes)
	: reference(type, attributes, nullptr), relative_offset_(relative_offset){}

cminus::memory::placeholder_reference::placeholder_reference(std::size_t relative_offset, std::shared_ptr<type::object> type, const logic::attributes::collection &attributes)
	: reference(type, attributes, nullptr), relative_offset_(relative_offset){}

cminus::memory::placeholder_reference::placeholder_reference(std::size_t relative_offset, std::shared_ptr<type::object> type)
	: placeholder_reference(relative_offset, type, attribute_list_type{}){}

cminus::memory::placeholder_reference::~placeholder_reference() = default;

std::shared_ptr<cminus::memory::reference> cminus::memory::placeholder_reference::bound_context(logic::runtime &runtime, std::shared_ptr<reference> value, std::size_t offset) const{
	if (auto lval_context = dynamic_cast<const lval_reference *>(value->get_non_raw()); lval_context != nullptr){
		if (find_attribute("Ref", true, false) == nullptr)
			return std::make_shared<lval_reference>((lval_context->get_address() + relative_offset_ + offset), type_, attributes_, value);
		return std::make_shared<ref_reference>(type_, attributes_, value);
	}

	if (auto rval_context = dynamic_cast<const rval_reference *>(value->get_non_raw()); rval_context != nullptr)
		return std::make_shared<rval_reference>((rval_context->get_data(runtime) + (relative_offset_ + offset)), type_, attributes_, value);

	return nullptr;
}

std::shared_ptr<cminus::memory::reference> cminus::memory::placeholder_reference::apply_offset(std::size_t value) const{
	return std::make_shared<placeholder_reference>((relative_offset_ + value), type_, attributes_);
}

bool cminus::memory::placeholder_reference::is_lvalue() const{
	return true;
}

std::size_t cminus::memory::placeholder_reference::get_address() const{
	throw logic::exception("A non-static member requires an object context", 0u, 0u);
	return 0u;
}

std::byte *cminus::memory::placeholder_reference::get_data(logic::runtime &runtime) const{
	throw logic::exception("A non-static member requires an object context", 0u, 0u);
	return nullptr;
}

std::size_t cminus::memory::placeholder_reference::read(logic::runtime &runtime, std::byte *buffer, std::size_t size) const{
	throw logic::exception("A non-static member requires an object context", 0u, 0u);
	return 0u;
}

std::size_t cminus::memory::placeholder_reference::read(logic::runtime &runtime, io::binary_writer &buffer, std::size_t size) const{
	throw logic::exception("A non-static member requires an object context", 0u, 0u);
	return 0u;
}

std::size_t cminus::memory::placeholder_reference::read(logic::runtime &runtime, std::size_t buffer, std::size_t size) const{
	throw logic::exception("A non-static member requires an object context", 0u, 0u);
	return 0u;
}

std::size_t cminus::memory::placeholder_reference::read(logic::runtime &runtime, reference &buffer, std::size_t size) const{
	throw logic::exception("A non-static member requires an object context", 0u, 0u);
	return 0u;
}

std::size_t cminus::memory::placeholder_reference::write(logic::runtime &runtime, const std::byte *buffer, std::size_t size){
	throw logic::exception("A non-static member requires an object context", 0u, 0u);
	return 0u;
}

std::size_t cminus::memory::placeholder_reference::write(logic::runtime &runtime, const io::binary_reader &buffer, std::size_t size){
	throw logic::exception("A non-static member requires an object context", 0u, 0u);
	return 0u;
}

std::size_t cminus::memory::placeholder_reference::write(logic::runtime &runtime, std::size_t buffer, std::size_t size){
	throw logic::exception("A non-static member requires an object context", 0u, 0u);
	return 0u;
}

std::size_t cminus::memory::placeholder_reference::write(logic::runtime &runtime, const reference &buffer, std::size_t size){
	throw logic::exception("A non-static member requires an object context", 0u, 0u);
	return 0u;
}

std::size_t cminus::memory::placeholder_reference::write(logic::runtime &runtime, managed_object &object){
	throw logic::exception("A non-static member requires an object context", 0u, 0u);
	return 0u;
}

std::size_t cminus::memory::placeholder_reference::set(logic::runtime &runtime, std::byte value, std::size_t size){
	throw logic::exception("A non-static member requires an object context", 0u, 0u);
	return 0u;
}

void cminus::memory::placeholder_reference::set_relative_offset(std::size_t value){
	relative_offset_ = value;
}

std::size_t cminus::memory::placeholder_reference::get_relative_offset() const{
	return relative_offset_;
}

cminus::memory::raw_reference::raw_reference(reference *target)
	: reference(target->get_type(), attribute_list_type{}, nullptr), target_(target){}

cminus::memory::raw_reference::~raw_reference() = default;

cminus::memory::reference *cminus::memory::raw_reference::get_non_raw() const{
	return target_->get_non_raw();
}

void cminus::memory::raw_reference::set_type(std::shared_ptr<type::object> value){
	target_->set_type(value);
}

std::shared_ptr<cminus::type::object> cminus::memory::raw_reference::get_type() const{
	return target_->get_type();
}

bool cminus::memory::raw_reference::is_nan() const{
	return target_->is_nan();
}

void cminus::memory::raw_reference::set_context(std::shared_ptr<reference> value){
	target_->set_context(value);
}

std::shared_ptr<cminus::memory::reference> cminus::memory::raw_reference::bound_context(logic::runtime &runtime, std::shared_ptr<reference> value, std::size_t offset) const{
	return target_->bound_context(runtime, value, offset);
}

std::shared_ptr<cminus::memory::reference> cminus::memory::raw_reference::get_context() const{
	return target_->get_context();
}

void cminus::memory::raw_reference::add_attribute(std::shared_ptr<logic::attributes::object> value){
	target_->add_attribute(value);
}

void cminus::memory::raw_reference::remove_attribute(const std::string &name, bool global_only){
	target_->remove_attribute(name, global_only);
}

void cminus::memory::raw_reference::remove_attribute(const logic::naming::object &name){
	target_->remove_attribute(name);
}

std::shared_ptr<cminus::logic::attributes::object> cminus::memory::raw_reference::find_attribute(const std::string &name, bool global_only, bool include_context) const{
	return target_->find_attribute(name, global_only, include_context);
}

std::shared_ptr<cminus::logic::attributes::object> cminus::memory::raw_reference::find_attribute(const logic::naming::object &name, bool include_context) const{
	return target_->find_attribute(name, include_context);
}

bool cminus::memory::raw_reference::has_attribute(const std::string &name, bool global_only, bool include_context) const{
	return target_->has_attribute(name, global_only, include_context);
}

bool cminus::memory::raw_reference::has_attribute(const logic::naming::object &name, bool include_context) const{
	return target_->has_attribute(name, include_context);
}

const cminus::logic::attributes::collection &cminus::memory::raw_reference::get_attributes() const{
	return target_->get_attributes();
}

void cminus::memory::raw_reference::traverse_attributes(logic::runtime &runtime, const std::function<void(std::shared_ptr<logic::attributes::object>)> &callback, logic::attributes::object::stage_type stage, bool include_context) const{
	target_->traverse_attributes(runtime, callback, stage, include_context);
}

void cminus::memory::raw_reference::call_attributes(logic::runtime &runtime, logic::attributes::object::stage_type stage, bool include_context, const std::vector<std::shared_ptr<memory::reference>> &args) const{
	target_->call_attributes(runtime, stage, include_context, args);
}

void cminus::memory::raw_reference::call_attributes(logic::runtime &runtime, logic::attributes::object::stage_type stage, bool include_context) const{
	target_->call_attributes(runtime, stage, include_context);
}

std::shared_ptr<cminus::memory::reference> cminus::memory::raw_reference::apply_offset(std::size_t value) const{
	return target_->apply_offset(value);
}

bool cminus::memory::raw_reference::is_lvalue() const{
	return target_->is_lvalue();
}

std::size_t cminus::memory::raw_reference::get_address() const{
	return target_->get_address();
}

std::byte *cminus::memory::raw_reference::get_data(logic::runtime &runtime) const{
	return target_->get_data(runtime);
}

std::size_t cminus::memory::raw_reference::read(logic::runtime &runtime, std::byte *buffer, std::size_t size) const{
	return target_->read(runtime, buffer, size);
}

std::size_t cminus::memory::raw_reference::read(logic::runtime &runtime, io::binary_writer &buffer, std::size_t size) const{
	return target_->read(runtime, buffer, size);
}

std::size_t cminus::memory::raw_reference::read(logic::runtime &runtime, std::size_t buffer, std::size_t size) const{
	return target_->read(runtime, buffer, size);
}

std::size_t cminus::memory::raw_reference::read(logic::runtime &runtime, reference &buffer, std::size_t size) const{
	return target_->read(runtime, buffer, size);
}

std::size_t cminus::memory::raw_reference::write(logic::runtime &runtime, const std::byte *buffer, std::size_t size){
	return target_->write(runtime, buffer, size);
}

std::size_t cminus::memory::raw_reference::write(logic::runtime &runtime, const io::binary_reader &buffer, std::size_t size){
	return target_->write(runtime, buffer, size);
}

std::size_t cminus::memory::raw_reference::write(logic::runtime &runtime, std::size_t buffer, std::size_t size){
	return target_->write(runtime, buffer, size);
}

std::size_t cminus::memory::raw_reference::write(logic::runtime &runtime, const reference &buffer, std::size_t size){
	return target_->write(runtime, buffer, size);
}

std::size_t cminus::memory::raw_reference::write(logic::runtime &runtime, managed_object &object){
	return target_->write(runtime, object);
}

std::size_t cminus::memory::raw_reference::set(logic::runtime &runtime, std::byte value, std::size_t size){
	return target_->set(runtime, value, size);
}

int cminus::memory::raw_reference::compare(logic::runtime &runtime, const std::byte *buffer, std::size_t size) const{
	return target_->compare(runtime, buffer, size);
}

int cminus::memory::raw_reference::compare(logic::runtime &runtime, std::size_t buffer, std::size_t size) const{
	return target_->compare(runtime, buffer, size);
}

int cminus::memory::raw_reference::compare(logic::runtime &runtime, const reference &buffer, std::size_t size) const{
	return target_->compare(runtime, buffer, size);
}

cminus::memory::reference *cminus::memory::raw_reference::get_target() const{
	return target_;
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

cminus::memory::lval_reference::lval_reference(logic::runtime &runtime, std::shared_ptr<type::object> type, const optimised_attribute_list_type &attributes, std::shared_ptr<reference> context)
	: reference(type, attributes, context){
	try{
		allocate_memory_(runtime, type_->get_size());
	}
	catch (...){
		address_ = 0u;
	}
}

cminus::memory::lval_reference::lval_reference(logic::runtime &runtime, std::shared_ptr<type::object> type, const logic::attributes::collection &attributes, std::shared_ptr<reference> context)
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

cminus::memory::lval_reference::lval_reference(std::size_t address, std::shared_ptr<type::object> type, const optimised_attribute_list_type &attributes, std::shared_ptr<reference> context)
	: reference(type, attributes, context), address_(address){}

cminus::memory::lval_reference::lval_reference(std::size_t address, std::shared_ptr<type::object> type, const logic::attributes::collection &attributes, std::shared_ptr<reference> context)
	: reference(type, attributes, context), address_(address){}

cminus::memory::lval_reference::lval_reference(std::size_t address, std::shared_ptr<type::object> type, std::shared_ptr<reference> context)
	: lval_reference(address, type, attribute_list_type{}, context){}

cminus::memory::lval_reference::~lval_reference(){
	try{
		if (deallocator_ != nullptr){
			deallocator_();
			deallocator_ = nullptr;
		}
	}
	catch (...){}
}

std::shared_ptr<cminus::memory::reference> cminus::memory::lval_reference::apply_offset(std::size_t value) const{
	return std::make_shared<lval_reference>((address_ + value), type_, attributes_, context_);
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
	logic::attributes::read_guard guard(runtime, const_cast<lval_reference *>(this), true);
	return runtime.memory_object.read(get_address(), buffer, ((size == static_cast<std::size_t>(-1) || size_ < size) ? size_ : size));
}

std::size_t cminus::memory::lval_reference::read(logic::runtime &runtime, io::binary_writer &buffer, std::size_t size) const{
	logic::attributes::read_guard guard(runtime, const_cast<lval_reference *>(this), true);
	return runtime.memory_object.read(get_address(), buffer, ((size == static_cast<std::size_t>(-1) || size_ < size) ? size_ : size));
}

std::size_t cminus::memory::lval_reference::read(logic::runtime &runtime, std::size_t buffer, std::size_t size) const{
	logic::attributes::read_guard guard(runtime, const_cast<lval_reference *>(this), true);
	return runtime.memory_object.read(get_address(), buffer, ((size == static_cast<std::size_t>(-1) || size_ < size) ? size_ : size));
}

std::size_t cminus::memory::lval_reference::read(logic::runtime &runtime, reference &buffer, std::size_t size) const{
	logic::attributes::read_guard guard(runtime, const_cast<lval_reference *>(this), true);
	return buffer.write(runtime, get_address(), ((size == static_cast<std::size_t>(-1) || size_ < size) ? size_ : size));
}

std::size_t cminus::memory::lval_reference::write(logic::runtime &runtime, const std::byte *buffer, std::size_t size){
	logic::attributes::write_guard guard(runtime, const_cast<lval_reference *>(this), true);
	return runtime.memory_object.write(get_address(), buffer, ((size == static_cast<std::size_t>(-1) || size_ < size) ? size_ : size));
}

std::size_t cminus::memory::lval_reference::write(logic::runtime &runtime, const io::binary_reader &buffer, std::size_t size){
	logic::attributes::write_guard guard(runtime, const_cast<lval_reference *>(this), true);
	return runtime.memory_object.write(get_address(), buffer, ((size == static_cast<std::size_t>(-1) || size_ < size) ? size_ : size));
}

std::size_t cminus::memory::lval_reference::write(logic::runtime &runtime, std::size_t buffer, std::size_t size){
	logic::attributes::write_guard guard(runtime, const_cast<lval_reference *>(this), true);
	return runtime.memory_object.write(buffer, get_address(), ((size == static_cast<std::size_t>(-1) || size_ < size) ? size_ : size));
}

std::size_t cminus::memory::lval_reference::write(logic::runtime &runtime, const reference &buffer, std::size_t size){
	logic::attributes::write_guard guard(runtime, const_cast<lval_reference *>(this), true);
	return buffer.read(runtime, get_address(), ((size == static_cast<std::size_t>(-1) || size_ < size) ? size_ : size));
}

std::size_t cminus::memory::lval_reference::write(logic::runtime &runtime, managed_object &object){
	logic::attributes::write_guard guard(runtime, const_cast<lval_reference *>(this), true);
	return runtime.memory_object.write(get_address(), object);
}

std::size_t cminus::memory::lval_reference::set(logic::runtime &runtime, std::byte value, std::size_t size){
	logic::attributes::write_guard guard(runtime, const_cast<lval_reference *>(this), true);
	return runtime.memory_object.set(get_address(), value, ((size == static_cast<std::size_t>(-1) || size_ < size) ? size_ : size));
}

void cminus::memory::lval_reference::allocate_memory_(logic::runtime &runtime, std::size_t size){
	auto block = runtime.memory_object.allocate_block(size, memory::block::attribute_none);
	if (block != nullptr){
		address_ = block->get_address();
		deallocator_ = [&](){
			type_->destruct(runtime, std::make_shared<raw_reference>(this));
			if (address_ != 0u)
				runtime.memory_object.deallocate_block(address_);
		};
	}
}

cminus::memory::ref_reference::ref_reference(std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context)
	: lval_reference(0u, type, attributes, context){}

cminus::memory::ref_reference::ref_reference(std::shared_ptr<type::object> type, const optimised_attribute_list_type &attributes, std::shared_ptr<reference> context)
	: lval_reference(0u, type, attributes, context){}

cminus::memory::ref_reference::ref_reference(std::shared_ptr<type::object> type, const logic::attributes::collection &attributes, std::shared_ptr<reference> context)
	: lval_reference(0u, type, attributes, context){}

cminus::memory::ref_reference::ref_reference(std::shared_ptr<type::object> type, std::shared_ptr<reference> context)
	: ref_reference(type, attribute_list_type{}, context){}

cminus::memory::ref_reference::~ref_reference() = default;

void cminus::memory::ref_reference::write_address(std::size_t value){
	address_ = value;
}

cminus::memory::rval_ref_reference::rval_ref_reference(std::shared_ptr<reference> target)
	: raw_reference(target.get()), rval_target_(target){}

cminus::memory::rval_ref_reference::~rval_ref_reference() = default;

std::shared_ptr<cminus::memory::reference> cminus::memory::rval_ref_reference::get_rval_target() const{
	return rval_target_;
}

cminus::memory::function_reference::function_reference(logic::runtime &runtime, std::size_t address, declaration::function_group_base *value)
	: function_reference(address, runtime.global_storage->get_primitve_type(type::primitive::id_type::function), value, attribute_list_type{ runtime.global_storage->find_attribute("ReadOnly", false) }, nullptr){}

cminus::memory::function_reference::function_reference(std::size_t address, std::shared_ptr<type::object> type, declaration::function_group_base *value, const attribute_list_type &attributes, std::shared_ptr<reference> context)
	: lval_reference(address, type, attributes, context), value_(value){}

cminus::memory::function_reference::function_reference(std::size_t address, std::shared_ptr<type::object> type, declaration::function_group_base *value, const optimised_attribute_list_type &attributes, std::shared_ptr<reference> context)
	: lval_reference(address, type, attributes, context), value_(value){}

cminus::memory::function_reference::function_reference(std::size_t address, std::shared_ptr<type::object> type, declaration::function_group_base *value, const logic::attributes::collection &attributes, std::shared_ptr<reference> context)
	: lval_reference(address, type, attributes, context), value_(value){}

cminus::memory::function_reference::~function_reference() = default;

std::shared_ptr<cminus::memory::reference> cminus::memory::function_reference::bound_context(logic::runtime &runtime, std::shared_ptr<reference> value, std::size_t offset) const{
	return std::make_shared<function_reference>(address_, type_, value_, attributes_, value);
}

std::shared_ptr<cminus::memory::reference> cminus::memory::function_reference::apply_offset(std::size_t value) const{
	return ((value == 0u) ? std::make_shared<function_reference>(address_, type_, value_, attributes_, context_) : nullptr);
}

cminus::declaration::function_group_base *cminus::memory::function_reference::get_value() const{
	return value_;
}

cminus::memory::rval_reference::rval_reference(std::byte *data, std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context)
	: reference(type, attributes, context), data_(data){}

cminus::memory::rval_reference::rval_reference(std::byte *data, std::shared_ptr<type::object> type, const optimised_attribute_list_type &attributes, std::shared_ptr<reference> context)
	: reference(type, attributes, context), data_(data){}

cminus::memory::rval_reference::rval_reference(std::byte *data, std::shared_ptr<type::object> type, const logic::attributes::collection &attributes, std::shared_ptr<reference> context)
	: reference(type, attributes, context), data_(data){}

cminus::memory::rval_reference::rval_reference(std::byte *data, std::shared_ptr<type::object> type, std::shared_ptr<reference> context)
	: rval_reference(data, type, attribute_list_type{}, context){}

cminus::memory::rval_reference::~rval_reference() = default;

std::shared_ptr<cminus::memory::reference> cminus::memory::rval_reference::apply_offset(std::size_t value) const{
	return std::make_shared<rval_reference>((data_ + value), type_, attributes_, context_);
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

cminus::memory::data_reference::data_reference(logic::runtime &runtime, std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context)
	: rval_reference(nullptr, type, attributes, context){
	if (0u < size_){
		value_ = std::make_unique<std::byte[]>(size_);
		data_ = value_.get();
		deallocator_ = [&](){
			type_->destruct(runtime, std::make_shared<raw_reference>(this));
		};
	}
}

cminus::memory::data_reference::data_reference(logic::runtime &runtime, std::shared_ptr<type::object> type, const optimised_attribute_list_type &attributes, std::shared_ptr<reference> context)
	: rval_reference(nullptr, type, attributes, context){
	if (0u < size_){
		value_ = std::make_unique<std::byte[]>(size_);
		data_ = value_.get();
		deallocator_ = [&](){
			type_->destruct(runtime, std::make_shared<raw_reference>(this));
		};
	}
}

cminus::memory::data_reference::data_reference(logic::runtime &runtime, std::shared_ptr<type::object> type, const logic::attributes::collection &attributes, std::shared_ptr<reference> context)
	: rval_reference(nullptr, type, attributes, context){
	if (0u < size_){
		value_ = std::make_unique<std::byte[]>(size_);
		data_ = value_.get();
		deallocator_ = [&](){
			type_->destruct(runtime, std::make_shared<raw_reference>(this));
		};
	}
}

cminus::memory::data_reference::data_reference(logic::runtime &runtime, std::shared_ptr<type::object> type, std::shared_ptr<reference> context)
	: data_reference(runtime, type, attribute_list_type{}, context){}

cminus::memory::data_reference::data_reference(std::shared_ptr<type::object> type, const attribute_list_type &attributes, std::shared_ptr<reference> context)
	: rval_reference(nullptr, type, attributes, context){
	if (0u < size_){
		value_ = std::make_unique<std::byte[]>(size_);
		data_ = value_.get();
	}
}

cminus::memory::data_reference::data_reference(std::shared_ptr<type::object> type, const optimised_attribute_list_type &attributes, std::shared_ptr<reference> context)
	: rval_reference(nullptr, type, attributes, context){
	if (0u < size_){
		value_ = std::make_unique<std::byte[]>(size_);
		data_ = value_.get();
	}
}

cminus::memory::data_reference::data_reference(std::shared_ptr<type::object> type, const logic::attributes::collection &attributes, std::shared_ptr<reference> context)
	: rval_reference(nullptr, type, attributes, context){
	if (0u < size_){
		value_ = std::make_unique<std::byte[]>(size_);
		data_ = value_.get();
	}
}

cminus::memory::data_reference::data_reference(std::shared_ptr<type::object> type, std::shared_ptr<reference> context)
	: data_reference(type, attribute_list_type{}, context){}

cminus::memory::data_reference::~data_reference(){
	try{
		if (deallocator_ != nullptr){
			deallocator_();
			deallocator_ = nullptr;
		}
	}
	catch (...){}
}

std::shared_ptr<cminus::memory::reference> cminus::memory::data_reference::apply_offset(std::size_t value) const{
	if (value != 0u)
		return rval_reference::apply_offset(value);

	if (auto clone = std::make_shared<data_reference>(type_, attributes_, context_); clone != nullptr && clone->data_ != nullptr){
		memcpy(clone->data_, data_, size_);
		return clone;
	}

	return nullptr;
}
