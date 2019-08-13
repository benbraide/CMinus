#include "memory_block.h"

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
	return target_->get_attributes();
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

cminus::memory::exclusive_block::~exclusive_block() = default;

std::byte *cminus::memory::exclusive_block::get_data(std::size_t offset) const{
	return ((data_.get() == nullptr || size_ <= offset) ? nullptr : (data_.get() + offset));
}

std::size_t cminus::memory::exclusive_block::read(std::size_t offset, std::byte *buffer, std::size_t size) const{
	if (size == static_cast<std::size_t>(-1))
		size = ((size_ < offset) ? 0u : (size_ - offset));

	if (size == 0u || size_ <= offset)
		return 0u;//Out of bounds

	if (is_access_protected())//Access protected
		throw exception(error_code::access_protected, (address_ + offset));

	if ((size_ - offset) < size)//Restrict size
		size = (size_ - offset);

	if (0u < size && size <= size_)
		memcpy(buffer, (data_.get() + offset), size);

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::exclusive_block::read(std::size_t offset, io::binary_writer &buffer, std::size_t size) const{
	if (size == 0u || size_ <= offset)
		return 0u;//Out of bounds

	if (is_write_protected())//Write protected
		throw exception(error_code::write_protected, (address_ + offset));

	if ((size_ - offset) < size)//Restrict size
		size = (size_ - offset);

	if (0u < size && size <= size_)
		size = buffer.write((data_.get() + offset), size);

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::exclusive_block::read(std::size_t offset, block &buffer, std::size_t size, std::size_t buffer_offset) const{
	if (size == static_cast<std::size_t>(-1))
		size = ((size_ < offset) ? 0u : (size_ - offset));

	if (size == 0u || size_ <= offset)
		return 0u;//Out of bounds

	if (is_access_protected())//Access protected
		throw exception(error_code::access_protected, (address_ + offset));

	if ((size_ - offset) < size)//Restrict size
		size = (size_ - offset);

	if (0u < size && size <= size_)
		buffer.write(buffer_offset, (data_.get() + offset), size);

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::exclusive_block::write(std::size_t offset, const std::byte *buffer, std::size_t size){
	if (size == 0u || size_ <= offset)
		return 0u;//Out of bounds

	if (is_write_protected())//Write protected
		throw exception(error_code::write_protected, (address_ + offset));

	if ((size_ - offset) < size)//Restrict size
		size = (size_ - offset);

	if (0u < size && size <= size_)
		memcpy((data_.get() + offset), buffer, size);

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::exclusive_block::write(std::size_t offset, const io::binary_reader &buffer, std::size_t size){
	if (size == 0u || size_ <= offset)
		return 0u;//Out of bounds

	if (is_write_protected())//Write protected
		throw exception(error_code::write_protected, (address_ + offset));

	if ((size_ - offset) < size)//Restrict size
		size = (size_ - offset);

	if (0u < size && size <= size_)
		size = buffer.read((data_.get() + offset), size);

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::exclusive_block::write(std::size_t offset, const block &buffer, std::size_t size, std::size_t buffer_offset){
	if (size == 0u || size_ <= offset)
		return 0u;//Out of bounds

	if (is_write_protected())//Write protected
		throw exception(error_code::write_protected, (address_ + offset));

	if ((size_ - offset) < size)//Restrict size
		size = (size_ - offset);

	if (0u < size && size <= size_)
		buffer.read(buffer_offset, (data_.get() + offset), size);

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::exclusive_block::set(std::size_t offset, std::byte value, std::size_t size){
	if (size == 0u || size_ <= offset)
		return 0u;//Out of bounds

	if (is_write_protected())//Write protected
		throw exception(error_code::write_protected, (address_ + offset));

	if ((size_ - offset) < size)//Restrict size
		size = (size_ - offset);

	if (0u < size && size <= size_)
		memset((data_.get() + offset), static_cast<int>(value), size);

	return ((size <= size_) ? size : 0u);
}

cminus::memory::inclusive_block::inclusive_block() = default;

cminus::memory::inclusive_block::inclusive_block(composite_block *owner, std::size_t address, std::size_t size, unsigned int attributes)
	: block(address, size, attributes), owner_(owner){}

cminus::memory::inclusive_block::~inclusive_block() = default;

std::byte *cminus::memory::inclusive_block::get_data(std::size_t offset) const{
	return ((owner_->data_.get() == nullptr || size_ <= offset) ? nullptr : (owner_->data_.get() + (address_ - owner_->address_) + offset));
}

unsigned int cminus::memory::inclusive_block::get_attributes() const{
	return (owner_->get_attributes() | block::get_attributes());
}

std::size_t cminus::memory::inclusive_block::read(std::size_t offset, std::byte *buffer, std::size_t size) const{
	if (size == static_cast<std::size_t>(-1))
		size = ((size_ < offset) ? 0u : (size_ - offset));

	if (size == 0u || size_ <= offset)
		return 0u;//Out of bounds

	if (is_access_protected())//Access protected
		throw exception(error_code::access_protected, (address_ + offset));

	if ((size_ - offset) < size)//Restrict size
		size = (size_ - offset);

	if (0u < size && size <= size_)
		memcpy(buffer, (owner_->data_.get() + (address_ - owner_->address_) + offset), size);

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::inclusive_block::read(std::size_t offset, io::binary_writer &buffer, std::size_t size) const{
	if (size == 0u || size_ <= offset)
		return 0u;//Out of bounds

	if (is_write_protected())//Write protected
		throw exception(error_code::write_protected, (address_ + offset));

	if ((size_ - offset) < size)//Restrict size
		size = (size_ - offset);

	if (0u < size && size <= size_)
		size = buffer.write((owner_->data_.get() + offset), size);

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::inclusive_block::read(std::size_t offset, block &buffer, std::size_t size, std::size_t buffer_offset) const{
	if (size == static_cast<std::size_t>(-1))
		size = ((size_ < offset) ? 0u : (size_ - offset));

	if (size == 0u || size_ <= offset)
		return 0u;//Out of bounds

	if (is_access_protected())//Access protected
		throw exception(error_code::access_protected, (address_ + offset));

	if ((size_ - offset) < size)//Restrict size
		size = (size_ - offset);

	if (0u < size && size <= size_)
		buffer.write(buffer_offset, (owner_->data_.get() + (address_ - owner_->address_) + offset), size);

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::inclusive_block::write(std::size_t offset, const std::byte *buffer, std::size_t size){
	if (size == 0u || size_ <= offset)
		return 0u;//Out of bounds

	if (is_write_protected())//Write protected
		throw exception(error_code::write_protected, (address_ + offset));

	if ((size_ - offset) < size)//Restrict size
		size = (size_ - offset);

	if (0u < size && size <= size_)
		memcpy((owner_->data_.get() + (address_ - owner_->address_) + offset), buffer, size);

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::inclusive_block::write(std::size_t offset, const io::binary_reader &buffer, std::size_t size){
	if (size == 0u || size_ <= offset)
		return 0u;//Out of bounds

	if (is_write_protected())//Write protected
		throw exception(error_code::write_protected, (address_ + offset));

	if ((size_ - offset) < size)//Restrict size
		size = (size_ - offset);

	if (0u < size && size <= size_)
		size = buffer.read((owner_->data_.get() + offset), size);

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::inclusive_block::write(std::size_t offset, const block &buffer, std::size_t size, std::size_t buffer_offset){
	if (size == 0u || size_ <= offset)
		return 0u;//Out of bounds

	if (is_write_protected())//Write protected
		throw exception(error_code::write_protected, (address_ + offset));

	if ((size_ - offset) < size)//Restrict size
		size = (size_ - offset);

	if (0u < size && size <= size_)
		buffer.read(buffer_offset, (owner_->data_.get() + (address_ - owner_->address_) + offset), size);

	return ((size <= size_) ? size : 0u);
}

std::size_t cminus::memory::inclusive_block::set(std::size_t offset, std::byte value, std::size_t size){
	if (size == 0u || size_ <= offset)
		return 0u;//Out of bounds

	if (is_write_protected())//Write protected
		throw exception(error_code::write_protected, (address_ + offset));

	if ((size_ - offset) < size)//Restrict size
		size = (size_ - offset);

	if (0u < size && size <= size_)
		memset((owner_->data_.get() + (address_ - owner_->address_) + offset), static_cast<int>(value), size);

	return ((size <= size_) ? size : 0u);
}

cminus::memory::composite_block::composite_block(std::size_t reserve_size){
	if (reserve_size != 0u)
		blocks_.reserve(reserve_size);
}

cminus::memory::composite_block::composite_block(std::size_t address, std::size_t size, unsigned int attributes, std::size_t reserve_size)
	: block(address, size, attributes){
	if (reserve_size != 0u)
		blocks_.reserve(reserve_size);

	if (0u < size_)
		data_ = std::make_unique<std::byte[]>(size_);
}

cminus::memory::composite_block::~composite_block() = default;

std::byte *cminus::memory::composite_block::get_data(std::size_t offset) const{
	return ((data_.get() == nullptr || size_ <= offset) ? nullptr : (data_.get() + offset));
}

std::size_t cminus::memory::composite_block::read(std::size_t offset, std::byte *buffer, std::size_t size) const{
	if (size == static_cast<std::size_t>(-1))
		size = ((size_ < offset) ? 0u : (size_ - offset));

	if (size == 0u || size_ <= offset || blocks_.empty())
		return 0u;//Out of bounds

	if (is_access_protected())//Access protected
		throw exception(error_code::access_protected, (address_ + offset));

	std::size_t read_size = 0u;
	for (auto block : blocks_){
		if (size <= (read_size += block->read(offset, (buffer + read_size), (size - read_size))))
			break;//Read complete
		offset = 0u;
	}

	return read_size;
}

std::size_t cminus::memory::composite_block::read(std::size_t offset, io::binary_writer &buffer, std::size_t size) const{
	if (size == 0u || size_ <= offset || blocks_.empty())
		return 0u;//Out of bounds

	if (is_write_protected())//Write protected
		throw exception(error_code::write_protected, (address_ + offset));

	std::size_t write_size = 0u;
	for (auto block : blocks_){
		if (size <= (write_size += block->read(offset, buffer, (size - write_size))) || buffer.get_size() <= buffer.get_offset())
			break;//Write complete
		offset = 0u;
	}

	return write_size;
}

std::size_t cminus::memory::composite_block::read(std::size_t offset, block &buffer, std::size_t size, std::size_t buffer_offset) const{
	if (size == static_cast<std::size_t>(-1))
		size = ((size_ < offset) ? 0u : (size_ - offset));

	if (size == 0u || size_ <= offset || blocks_.empty())
		return 0u;//Out of bounds

	if (is_access_protected())//Access protected
		throw exception(error_code::access_protected, (address_ + offset));

	std::size_t read_size = 0u;
	for (auto block : blocks_){
		if (size <= (read_size += buffer.write(offset, *block, (size - read_size), (buffer_offset + read_size))))
			break;//Read complete
		offset = 0u;
	}

	return read_size;
}

std::size_t cminus::memory::composite_block::write(std::size_t offset, const std::byte *buffer, std::size_t size){
	if (size == 0u || size_ <= offset || blocks_.empty())
		return 0u;//Out of bounds

	if (is_write_protected())//Write protected
		throw exception(error_code::write_protected, (address_ + offset));

	std::size_t write_size = 0u;
	for (auto block : blocks_){
		if (size <= (write_size += block->write(offset, (buffer + write_size), (size - write_size))))
			break;//Write complete
		offset = 0u;
	}

	return write_size;
}

std::size_t cminus::memory::composite_block::write(std::size_t offset, const io::binary_reader &buffer, std::size_t size){
	if (size == 0u || size_ <= offset || blocks_.empty())
		return 0u;//Out of bounds

	if (is_write_protected())//Write protected
		throw exception(error_code::write_protected, (address_ + offset));

	std::size_t write_size = 0u;
	for (auto block : blocks_){
		if (size <= (write_size += block->write(offset, buffer, (size - write_size))) || buffer.get_bytes_remaining() == 0u)
			break;//Write complete
		offset = 0u;
	}

	return write_size;
}

std::size_t cminus::memory::composite_block::write(std::size_t offset, const block &buffer, std::size_t size, std::size_t buffer_offset){
	if (size == 0u || size_ <= offset || blocks_.empty())
		return 0u;//Out of bounds

	if (is_write_protected())//Write protected
		throw exception(error_code::write_protected, (address_ + offset));

	std::size_t write_size = 0u;
	for (auto block : blocks_){
		if (size <= (write_size += buffer.read(offset, *block, (size - write_size), (buffer_offset + write_size))))
			break;//Write complete
		offset = 0u;
	}

	return write_size;
}

std::size_t cminus::memory::composite_block::set(std::size_t offset, std::byte value, std::size_t size){
	if (size == 0u || size_ <= offset || blocks_.empty())
		return 0u;//Out of bounds

	if (is_write_protected())//Write protected
		throw exception(error_code::write_protected, (address_ + offset));

	std::size_t set_size = 0u;
	for (auto block : blocks_){
		if (size <= (set_size += block->set(offset, value, (size - set_size))))
			break;//Write complete
		offset = 0u;
	}

	return set_size;
}

cminus::memory::block *cminus::memory::composite_block::append_block_(std::size_t size, unsigned int attributes){
	if (!hit_test(next_block_offset_, size))
		throw exception(error_code::allocation_failure, next_block_offset_);

	auto block = std::make_shared<inclusive_block>(this, (address_ + next_block_offset_), size, attributes);
	if (block != nullptr){
		next_block_offset_ += block->size_;
		blocks_.push_back(block);
	}

	return block.get();
}

cminus::memory::block *cminus::memory::composite_block::get_block_(std::size_t offset) const{
	if (blocks_.empty())
		return nullptr;

	for (auto block : blocks_){
		if (hit_test(address_ + offset))
			return block.get();
	}

	return nullptr;
}
