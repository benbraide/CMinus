#include "memory_block.h"

cminus::memory::exception::exception(error_code code, std::size_t address)
	: base_type("Memory Exception"), code_(code), address_(address){}

cminus::memory::error_code cminus::memory::exception::get_code() const{
	return code_;
}

std::size_t cminus::memory::exception::get_address() const{
	return address_;
}

cminus::memory::block::block() = default;

cminus::memory::block::block(std::size_t address, std::size_t size, unsigned int attributes)
	: address_(address), size_(size), attributes_(attributes){}

cminus::memory::block::~block() = default;

std::size_t cminus::memory::block::get_address() const{
	return address_;
}

std::size_t cminus::memory::block::get_size() const{
	return size_;
}

void cminus::memory::block::set_attributes(unsigned int value){
	attributes_ = value;
}

unsigned int cminus::memory::block::get_attributes() const{
	return attributes_;
}

std::size_t cminus::memory::block::convert_address_to_offset(std::size_t value) const{
	return (((address_ <= value) && (value < (address_ + size_))) ? (value - address_) : static_cast<std::size_t>(-1));
}

std::size_t cminus::memory::block::convert_offset_to_address(std::size_t value) const{
	return ((value < size_) ? (value + address_) : 0u);
}

bool cminus::memory::block::hit_test(std::size_t address) const{
	return ((address_ <= address) && (address < (address_ + size_)));
}

bool cminus::memory::block::hit_test(std::size_t offset, std::size_t size) const{
	return ((offset + size) <= size_);
}

bool cminus::memory::block::has_attributes(unsigned int value, bool all) const{
	return (all ? ((get_attributes() & value) == value) : ((get_attributes() & value) != 0u));
}

bool cminus::memory::block::is_access_protected() const{
	return has_attributes(attribute_access_protected, true);
}

bool cminus::memory::block::is_write_protected() const{
	return has_attributes(attribute_write_protected, true);
}

bool cminus::memory::block::is_executable() const{
	return has_attributes(attribute_executable, true);
}

cminus::memory::protected_block::protected_block(){
	attributes_ = (attribute_access_protected | attribute_write_protected);
}

cminus::memory::protected_block::protected_block(std::size_t address, std::size_t size)
	: block(address, size, (attribute_access_protected | attribute_write_protected)){}

cminus::memory::protected_block::~protected_block() = default;

std::byte *cminus::memory::protected_block::get_data(std::size_t offset) const{
	return nullptr;
}

std::size_t cminus::memory::protected_block::compare(int &result, std::size_t offset, const std::byte *buffer, std::size_t size) const{
	if (size != 0u)
		throw exception(error_code::access_protected, (address_ + offset));
	return 0u;
}

std::size_t cminus::memory::protected_block::compare(int &result, std::size_t offset, const block &buffer, std::size_t size, std::size_t buffer_offset) const{
	if (size != 0u)
		throw exception(error_code::access_protected, (address_ + offset));
	return 0u;
}

std::size_t cminus::memory::protected_block::read(std::size_t offset, std::byte *buffer, std::size_t size) const{
	if (size != 0u)
		throw exception(error_code::access_protected, (address_ + offset));
	return 0u;
}

std::size_t cminus::memory::protected_block::read(std::size_t offset, io::binary_writer &buffer, std::size_t size) const{
	if (size != 0u)
		throw exception(error_code::access_protected, (address_ + offset));
	return 0u;
}

std::size_t cminus::memory::protected_block::read(std::size_t offset, block &buffer, std::size_t size, std::size_t buffer_offset) const{
	if (size != 0u)
		throw exception(error_code::access_protected, (address_ + offset));
	return 0u;
}

std::size_t cminus::memory::protected_block::write(std::size_t offset, const std::byte *buffer, std::size_t size){
	if (size != 0u)
		throw exception(error_code::write_protected, (address_ + offset));
	return 0u;
}

std::size_t cminus::memory::protected_block::write(std::size_t offset, const io::binary_reader &buffer, std::size_t size){
	if (size != 0u)
		throw exception(error_code::write_protected, (address_ + offset));
	return 0u;
}

std::size_t cminus::memory::protected_block::write(std::size_t offset, const block &buffer, std::size_t size, std::size_t buffer_offset){
	if (size != 0u)
		throw exception(error_code::write_protected, (address_ + offset));
	return 0u;
}

std::size_t cminus::memory::protected_block::write(managed_object &object){
	throw exception(error_code::write_protected, address_);
	return 0u;
}

std::size_t cminus::memory::protected_block::set(std::size_t offset, std::byte value, std::size_t size){
	if (size != 0u)
		throw exception(error_code::write_protected, (address_ + offset));
	return 0u;
}

cminus::memory::free_block::free_block() = default;

cminus::memory::free_block::free_block(std::size_t address, std::size_t size)
	: protected_block(address, size){}

cminus::memory::free_block::~free_block() = default;

cminus::memory::offset_block::offset_block(std::shared_ptr<block> target, std::size_t offset)
	: block((target->get_address() + offset), (target->get_size() - offset), target->get_attributes()), target_(target), offset_(offset){}

cminus::memory::offset_block::~offset_block() = default;

std::byte *cminus::memory::offset_block::get_data(std::size_t offset) const{
	return target_->get_data(offset_ + offset);
}

unsigned int cminus::memory::offset_block::get_attributes() const{
	return (target_->get_attributes() & ~attribute_has_managed_object);
}

std::size_t cminus::memory::offset_block::compare(int &result, std::size_t offset, const std::byte *buffer, std::size_t size) const{
	return target_->compare(result, (offset_ + offset), buffer, size);
}

std::size_t cminus::memory::offset_block::compare(int &result, std::size_t offset, const block &buffer, std::size_t size, std::size_t buffer_offset) const{
	return target_->compare(result, (offset_ + offset), buffer, size, buffer_offset);
}

std::size_t cminus::memory::offset_block::read(std::size_t offset, std::byte *buffer, std::size_t size) const{
	return target_->read((offset_ + offset), buffer, size);
}

std::size_t cminus::memory::offset_block::read(std::size_t offset, io::binary_writer &buffer, std::size_t size) const{
	return target_->read((offset_ + offset), buffer, size);
}

std::size_t cminus::memory::offset_block::read(std::size_t offset, block &buffer, std::size_t size, std::size_t buffer_offset) const{
	return target_->read((offset_ + offset), buffer, size, buffer_offset);
}

std::size_t cminus::memory::offset_block::write(std::size_t offset, const std::byte *buffer, std::size_t size){
	return target_->write((offset_ + offset), buffer, size);
}

std::size_t cminus::memory::offset_block::write(std::size_t offset, const io::binary_reader &buffer, std::size_t size){
	return target_->write((offset_ + offset), buffer, size);
}

std::size_t cminus::memory::offset_block::write(std::size_t offset, const block &buffer, std::size_t size, std::size_t buffer_offset){
	return target_->write((offset_ + offset), buffer, size, buffer_offset);
}

std::size_t cminus::memory::offset_block::write(managed_object &object){
	throw exception(error_code::block_misaligned, address_);
	return 0u;
}

std::size_t cminus::memory::offset_block::set(std::size_t offset, std::byte value, std::size_t size){
	return target_->set((offset_ + offset), value, size);
}

std::shared_ptr<cminus::memory::block> cminus::memory::offset_block::get_target() const{
	return target_;
}

std::size_t cminus::memory::offset_block::get_offset() const{
	return offset_;
}

cminus::memory::exclusive_block::exclusive_block() = default;

cminus::memory::exclusive_block::exclusive_block(std::size_t address, std::size_t size, unsigned int attributes)
	: block(address, size, attributes){
	if (0u < size_)
		data_ = std::make_unique<std::byte[]>(size_);
}

cminus::memory::exclusive_block::~exclusive_block(){
	if (has_attributes(attribute_has_managed_object)){
		delete read_scalar<managed_object *>(0u);
		attributes_ &= ~attribute_has_managed_object;
	}
}

std::byte *cminus::memory::exclusive_block::get_data(std::size_t offset) const{
	return ((data_.get() == nullptr || size_ <= offset) ? nullptr : (data_.get() + offset));
}

std::size_t cminus::memory::exclusive_block::compare(int &result, std::size_t offset, const std::byte *buffer, std::size_t size) const{
	if (size == 0u || size_ <= offset){
		result = 0;
		return 0u;//Out of bounds
	}

	if (is_access_protected())//Write protected
		throw exception(error_code::access_protected, (address_ + offset));

	if ((size_ - offset) < size)//Restrict size
		size = (size_ - offset);

	if (0u < size && size <= size_)
		result = memcmp((data_.get() + offset), buffer, size);

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::exclusive_block::compare(int &result, std::size_t offset, const block &buffer, std::size_t size, std::size_t buffer_offset) const{
	if (size == 0u || size_ <= offset){
		result = 0;
		return 0u;//Out of bounds
	}

	if (is_access_protected())//Write protected
		throw exception(error_code::access_protected, (address_ + offset));

	if ((size_ - offset) < size)//Restrict size
		size = (size_ - offset);

	if (0u < size && size <= size_){
		size = buffer.compare(result, buffer_offset, (data_.get() + offset), size);
		switch (result){
		case -1:
		case 1:
			result = -result;
		default:
			break;
		}
	}

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::exclusive_block::read(std::size_t offset, std::byte *buffer, std::size_t size) const{
	if (size == static_cast<std::size_t>(-1) || (offset < size_ && (size_ - offset) < size))//Restrict size
		size = ((size_ < offset) ? 0u : (size_ - offset));

	if (size == 0u || size_ <= offset)
		return 0u;//Out of bounds

	if (is_access_protected())//Access protected
		throw exception(error_code::access_protected, (address_ + offset));

	if (0u < size && size <= size_)
		memcpy(buffer, (data_.get() + offset), size);

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::exclusive_block::read(std::size_t offset, io::binary_writer &buffer, std::size_t size) const{
	if (size == static_cast<std::size_t>(-1) || (offset < size_ && (size_ - offset) < size))//Restrict size
		size = ((size_ < offset) ? 0u : (size_ - offset));

	if (size == 0u || size_ <= offset)
		return 0u;//Out of bounds

	if (is_write_protected())//Write protected
		throw exception(error_code::write_protected, (address_ + offset));

	if (0u < size && size <= size_)
		size = buffer.write((data_.get() + offset), size);

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::exclusive_block::read(std::size_t offset, block &buffer, std::size_t size, std::size_t buffer_offset) const{
	if (size == static_cast<std::size_t>(-1) || (offset < size_ && (size_ - offset) < size))//Restrict size
		size = ((size_ < offset) ? 0u : (size_ - offset));

	if (size == 0u || size_ <= offset)
		return 0u;//Out of bounds

	if (is_access_protected())//Access protected
		throw exception(error_code::access_protected, (address_ + offset));

	if (0u < size && size <= size_)
		size = buffer.write(buffer_offset, (data_.get() + offset), size);

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::exclusive_block::write(std::size_t offset, const std::byte *buffer, std::size_t size){
	if (size == static_cast<std::size_t>(-1) || (offset < size_ && (size_ - offset) < size))//Restrict size
		size = ((size_ < offset) ? 0u : (size_ - offset));

	if (size == 0u || size_ <= offset)
		return 0u;//Out of bounds

	if (is_write_protected())//Write protected
		throw exception(error_code::write_protected, (address_ + offset));

	if (0u < size && size <= size_){
		if (has_attributes(attribute_has_managed_object)){
			delete read_scalar<managed_object *>(0u);
			attributes_ &= ~attribute_has_managed_object;
		}
		memcpy((data_.get() + offset), buffer, size);
	}

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::exclusive_block::write(std::size_t offset, const io::binary_reader &buffer, std::size_t size){
	if (size == static_cast<std::size_t>(-1) || (offset < size_ && (size_ - offset) < size))//Restrict size
		size = ((size_ < offset) ? 0u : (size_ - offset));

	if (size == 0u || size_ <= offset)
		return 0u;//Out of bounds

	if (is_write_protected())//Write protected
		throw exception(error_code::write_protected, (address_ + offset));

	if (0u < size && size <= size_){
		if (has_attributes(attribute_has_managed_object)){
			delete read_scalar<managed_object *>(0u);
			attributes_ &= ~attribute_has_managed_object;
		}
		size = buffer.read((data_.get() + offset), size);
	}

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::exclusive_block::write(std::size_t offset, const block &buffer, std::size_t size, std::size_t buffer_offset){
	if (size == static_cast<std::size_t>(-1) || (offset < size_ && (size_ - offset) < size))//Restrict size
		size = ((size_ < offset) ? 0u : (size_ - offset));

	if (size == 0u || size_ <= offset)
		return 0u;//Out of bounds

	if (offset == 0u && buffer_offset == 0u && size == sizeof(void *) && size <= size_ && size <= buffer.get_size() && buffer.has_attributes(attribute_has_managed_object)){//Copy managed
		auto object = buffer.read_scalar<managed_object *>(0u);
		if (auto object_clone = object->clone(); object_clone != nullptr){
			try{
				if (object_clone != object)
					return write(*object_clone);
			}
			catch (...){
				delete object_clone;
				throw;//Forward exception
			}
		}
	}

	if (is_write_protected())//Write protected
		throw exception(error_code::write_protected, (address_ + offset));

	if (0u < size && size <= size_){
		if (has_attributes(attribute_has_managed_object)){
			delete read_scalar<managed_object *>(0u);
			attributes_ &= ~attribute_has_managed_object;
		}

		size = buffer.read(buffer_offset, (data_.get() + offset), size);
	}

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::exclusive_block::write(managed_object &object){
	if (size_ != sizeof(void *) || !write_scalar(0u, &object))
		throw exception(error_code::block_misaligned, address_);

	attributes_ |= attribute_has_managed_object;
	return sizeof(void *);
}

std::size_t cminus::memory::exclusive_block::set(std::size_t offset, std::byte value, std::size_t size){
	if (size == 0u || size_ <= offset)
		return 0u;//Out of bounds

	if (is_write_protected())//Write protected
		throw exception(error_code::write_protected, (address_ + offset));

	if ((size_ - offset) < size)//Restrict size
		size = (size_ - offset);

	if (0u < size && size <= size_){
		if (has_attributes(attribute_has_managed_object)){
			delete reinterpret_cast<managed_object *>(read_scalar<std::size_t>(0u));
			attributes_ &= ~attribute_has_managed_object;
		}
		memset((data_.get() + offset), static_cast<int>(value), size);
	}

	return ((size <= size_) ? size : 0u);
}
