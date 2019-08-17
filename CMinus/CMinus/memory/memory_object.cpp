#include "memory_object.h"

std::size_t cminus::memory::object::lock(){
	if (lock_count_ == 0u)
		lock_.lock();
	return ++lock_count_;
}

std::size_t cminus::memory::object::unlock(){
	if (lock_count_ == 0u)
		return static_cast<std::size_t>(-1);

	if (--lock_count_ == 0u)
		lock_.unlock();

	return lock_count_;
}

bool cminus::memory::object::is_locked() const{
	return (0u < lock_count_);
}

std::shared_ptr<cminus::memory::block> cminus::memory::object::protect_next_block(std::size_t size){
	if (is_locked())
		return protect_next_block_(size);

	std::lock_guard<std::shared_mutex> guard(lock_);
	return protect_next_block_(size);
}

std::shared_ptr<cminus::memory::block> cminus::memory::object::allocate_block(std::size_t size, unsigned int attributes, std::size_t min_free_size){
	if (is_locked())
		return allocate_block_(size, attributes, min_free_size);

	std::lock_guard<std::shared_mutex> guard(lock_);
	return allocate_block_(size, attributes, min_free_size);
}

/*
std::shared_ptr<cminus::memory::block> cminus::memory::object::allocate_composite_block(std::size_t size, unsigned int attributes, std::size_t reserve_size){
	std::lock_guard<std::shared_mutex> guard(lock_);
	return allocate_block_(size, attributes, (allocation_state_use_free | allocation_state_composite), reserve_size);
}*/

std::shared_ptr<cminus::memory::block> cminus::memory::object::reallocate_block(std::size_t address, std::size_t size){
	if (is_locked())
		return reallocate_block_(address, size);

	std::lock_guard<std::shared_mutex> guard(lock_);
	return reallocate_block_(address, size);
}

void cminus::memory::object::deallocate_block(std::size_t address){
	if (!is_locked()){
		std::lock_guard<std::shared_mutex> guard(lock_);
		deallocate_block_(address);
	}
	else
		deallocate_block_(address);
}

std::size_t cminus::memory::object::read(std::size_t source_address, std::byte *buffer, std::size_t size) const{
	if (is_locked())
		return read_(source_address, buffer, size);

	std::shared_lock<std::shared_mutex> guard(lock_);
	return read_(source_address, buffer, size);
}

std::size_t cminus::memory::object::read(std::size_t source_address, io::binary_writer &buffer, std::size_t size) const{
	if (is_locked())
		return read_(source_address, buffer, size);

	std::shared_lock<std::shared_mutex> guard(lock_);
	return read_(source_address, buffer, size);
}

std::size_t cminus::memory::object::read(std::size_t source_address, std::size_t destination_address, std::size_t size) const{
	if (is_locked())
		return read_(source_address, destination_address, size);

	std::shared_lock<std::shared_mutex> guard(lock_);
	return read_(source_address, destination_address, size);
}

std::size_t cminus::memory::object::write(std::size_t destination_address, const std::byte *buffer, std::size_t size){
	if (is_locked())
		return write_(destination_address, buffer, size);

	std::shared_lock<std::shared_mutex> guard(lock_);
	return write_(destination_address, buffer, size);
}

std::size_t cminus::memory::object::write(std::size_t destination_address, const io::binary_reader &buffer, std::size_t size){
	if (is_locked())
		return write_(destination_address, buffer, size);

	std::shared_lock<std::shared_mutex> guard(lock_);
	return write_(destination_address, buffer, size);
}

std::size_t cminus::memory::object::write(std::size_t source_address, std::size_t destination_address, std::size_t size){
	if (is_locked())
		return write_(source_address, destination_address, size);

	std::shared_lock<std::shared_mutex> guard(lock_);
	return write_(source_address, destination_address, size);
}

std::size_t cminus::memory::object::write(std::size_t destination_address, managed_object &object){
	if (is_locked())
		return write_(destination_address, object);

	std::shared_lock<std::shared_mutex> guard(lock_);
	return write_(destination_address, object);
}

std::size_t cminus::memory::object::set(std::size_t destination_address, std::byte value, std::size_t size){
	if (is_locked())
		return set_(destination_address, value, size);

	std::shared_lock<std::shared_mutex> guard(lock_);
	return set_(destination_address, value, size);
}

std::shared_ptr<cminus::memory::block> cminus::memory::object::get_block(std::size_t address) const{
	if (is_locked())
		return get_block_(address);

	std::shared_lock<std::shared_mutex> guard(lock_);
	return get_block_(address);
}

std::shared_ptr<cminus::memory::block> cminus::memory::object::get_next_block(std::size_t address) const{
	if (is_locked())
		return get_next_block_(address);

	std::shared_lock<std::shared_mutex> guard(lock_);
	return get_next_block_(address);
}

std::shared_ptr<cminus::memory::block> cminus::memory::object::find_block(std::size_t address) const{
	if (is_locked())
		return find_block_(address);

	std::shared_lock<std::shared_mutex> guard(lock_);
	return find_block_(address);
}

std::shared_ptr<cminus::memory::block> cminus::memory::object::protect_next_block_(std::size_t size){
	auto block = std::make_shared<protected_block>(next_address_, size);
	if (block == nullptr)
		throw exception(error_code::allocation_failure, next_address_);

	blocks_.push_back(block);
	next_address_ += size;

	return block;
}

std::shared_ptr<cminus::memory::block> cminus::memory::object::allocate_block_(std::size_t size, unsigned int attributes, std::size_t min_free_size){
	if (size == 0u)
		throw exception(error_code::invalid_size, 0u);

	if (blocks_.size() == blocks_.max_size())
		throw exception(error_code::out_of_address_space, 0u);

	std::size_t address = 0u;
	auto free_it = blocks_.end();

	if (min_free_size != static_cast<std::size_t>(-1)){//Search for free block
		if (min_free_size < size)
			min_free_size = size;

		for (auto it = blocks_.begin(); it != blocks_.end(); ++it){
			if (auto free_block = dynamic_cast<memory::free_block *>(it->get()); free_block != nullptr && min_free_size <= free_block->size_){
				free_it = it;
				break;
			}
		}
	}

	if (free_it != blocks_.end()){
		address = (*free_it)->address_;
		if ((std::numeric_limits<std::size_t>::max() - size) < address)
			throw exception(error_code::out_of_address_space, 0u);

		(*free_it)->size_ -= size;
		(*free_it)->address_ += size;
	}
	else{//Use next address
		address = next_address_;
		if ((std::numeric_limits<std::size_t>::max() - size) < address)
			throw exception(error_code::out_of_address_space, 0u);
		next_address_ += size;
	}

	auto block = std::make_shared<exclusive_block>(address, size, attributes);
	if (block == nullptr)
		throw exception(error_code::allocation_failure, address);

	if (free_it != blocks_.end()){//Insert
		if ((*free_it)->size_ == 0u)
			*free_it = block;
		else
			blocks_.erase(free_it);
	}
	else//Append
		blocks_.push_back(block);

	return block;
}

std::shared_ptr<cminus::memory::block> cminus::memory::object::reallocate_block_(std::size_t address, std::size_t size){
	if (size == 0u)
		throw exception(error_code::invalid_size, address);

	if (blocks_.empty())
		throw exception(error_code::block_not_found, address);

	std::shared_ptr<cminus::memory::block> block;
	for (auto it = (blocks_.begin()); it != blocks_.end(); ++it){
		if ((*it)->address_ != address)
			continue;

		if ((*it)->is_access_protected())
			throw exception(error_code::access_protected, address);

		if (dynamic_cast<exclusive_block *>(it->get()) == nullptr)
			throw exception(error_code::allocation_failure, address);

		block = *it;
		if (block->has_attributes(memory::block::attribute_has_managed_object) || !block->has_attributes(memory::block::attribute_is_heap))
			throw exception(error_code::block_not_resizable, address);

		if (size < block->size_){//Shrink
			auto new_block = std::make_shared<exclusive_block>(address, size, block->get_attributes());
			if (new_block == nullptr)
				throw exception(error_code::allocation_failure, address);

			blocks_.insert(it, new_block);
			new_block->write(0u, *block, size);

			if (auto next_it = std::next(it); next_it != blocks_.end() && dynamic_cast<free_block *>(next_it->get()) != nullptr){//Merge with next
				(*next_it)->address_ -= (block->size_ - size);
				(*next_it)->size_ += (block->size_ - size);
				blocks_.erase(it);
			}
			else//Overwrite block
				*it = std::make_shared<free_block>(address, (block->size_ - size));
		}
		else if (size != block->size_){//Expand
			deallocate_block_(address);
			auto new_block = allocate_block_(size, block->get_attributes(), 0u);
			if (new_block == nullptr)
				throw exception(error_code::allocation_failure, address);

			new_block->write(0u, *block, block->size_);
			block = new_block;
		}
	}

	if (block == nullptr)
		throw exception(error_code::block_not_found, address);

	return block;
}

void cminus::memory::object::deallocate_block_(std::size_t address){
	if (blocks_.empty())
		return;

	for (auto it = blocks_.begin(); it != blocks_.end(); ++it){
		if ((*it)->address_ != address)
			continue;

		if ((*it)->is_access_protected())
			throw exception(error_code::access_protected, address);

		auto next_it = std::next(it);
		if (next_it != blocks_.end() && dynamic_cast<free_block *>(next_it->get()) != nullptr){//Merge with next
			(*it)->size_ += (*next_it)->size_;
			blocks_.erase(next_it);
		}

		if (it != blocks_.begin() && dynamic_cast<free_block *>((next_it = std::prev(it))->get()) != nullptr){//Merge with previous
			(*next_it)->size_ += (*it)->size_;
			blocks_.erase(it);
		}
		else if (next_address_ <= (address + (*it)->size_)){//Shrink
			next_address_ = address;
			blocks_.erase(it);
		}
		else//Overwrite block
			*it = std::make_shared<free_block>(address, (*it)->size_);

		return;
	}

	throw exception(error_code::block_not_found, address);
}

std::size_t cminus::memory::object::read_(std::size_t source_address, std::byte *buffer, std::size_t size) const{
	if (size == 0u)
		return size;

	auto source_it = blocks_.end();
	auto source_block = find_block_(source_address, &source_it);

	if (source_block == nullptr || source_it == blocks_.end())
		throw exception(error_code::block_not_found, source_address);

	std::size_t read_size = 0u;
	while (read_size < size){
		read_size += source_block->read(0, (buffer + read_size), (size - read_size));
		if (++source_it != blocks_.end())
			source_block = *source_it;
		else
			break;
	}

	return read_size;
}

std::size_t cminus::memory::object::read_(std::size_t source_address, io::binary_writer &buffer, std::size_t size) const{
	if (size == 0u)
		return size;

	auto source_it = blocks_.end();
	auto source_block = find_block_(source_address, &source_it);

	if (source_block == nullptr || source_it == blocks_.end())
		throw exception(error_code::block_not_found, source_address);

	std::size_t read_size = 0u;
	while (read_size < size){
		read_size += source_block->read(0, buffer, (size - read_size));
		if (buffer.get_offset() < buffer.get_size() && ++source_it != blocks_.end())
			source_block = *source_it;
		else
			break;
	}

	return read_size;
}

std::size_t cminus::memory::object::read_(std::size_t source_address, std::size_t destination_address, std::size_t size) const{
	if (size == 0u)
		return size;

	auto source_it = blocks_.end();
	auto source_block = find_block_(source_address, &source_it);

	if (source_block == nullptr || source_it == blocks_.end())
		throw exception(error_code::block_not_found, source_address);

	auto destination_it = blocks_.end();
	auto destination_block = find_block_(destination_address, &destination_it);

	if (destination_block == nullptr || destination_it == blocks_.end())
		throw exception(error_code::block_not_found, destination_address);

	std::size_t read_size = 0u, current_read_size = 0u, source_offset = 0u, destination_offset = 0u;
	while (read_size < size){
		current_read_size = source_block->read(source_offset, *destination_block, (size - read_size), destination_offset);
		read_size += current_read_size;

		if ((source_block->size_ - source_offset) <= current_read_size){//Use next block
			source_offset = 0u;
			if (++source_it != blocks_.end())
				source_block = *source_it;
			else
				break;
		}
		else//Advance offset
			source_offset += current_read_size;

		if ((destination_block->size_ - destination_offset) <= current_read_size){//Use next block
			destination_offset = 0u;
			if (++destination_it != blocks_.end())
				destination_block = *destination_it;
			else
				break;
		}
		else//Advance offset
			destination_offset += current_read_size;
	}

	return read_size;
}

std::size_t cminus::memory::object::write_(std::size_t destination_address, const std::byte *buffer, std::size_t size){
	if (size == 0u)
		return size;

	auto destination_it = blocks_.end();
	auto destination_block = find_block_(destination_address, &destination_it);

	if (destination_block == nullptr || destination_it == blocks_.end())
		throw exception(error_code::block_not_found, destination_address);

	std::size_t write_size = 0u;
	while (write_size < size){
		write_size += destination_block->write(0, (buffer + write_size), (size - write_size));
		if (++destination_it != blocks_.end())
			destination_block = *destination_it;
		else
			break;
	}

	return write_size;
}

std::size_t cminus::memory::object::write_(std::size_t destination_address, const io::binary_reader &buffer, std::size_t size){
	if (size == 0u)
		return size;

	auto destination_it = blocks_.end();
	auto destination_block = find_block_(destination_address, &destination_it);

	if (destination_block == nullptr || destination_it == blocks_.end())
		throw exception(error_code::block_not_found, destination_address);

	std::size_t write_size = 0u;
	while (write_size < size){
		write_size += destination_block->write(0, buffer, (size - write_size));
		if (0u < buffer.get_bytes_remaining() && ++destination_it != blocks_.end())
			destination_block = *destination_it;
		else
			break;
	}

	return write_size;
}

std::size_t cminus::memory::object::write_(std::size_t source_address, std::size_t destination_address, std::size_t size){
	if (size == 0u)
		return size;

	auto source_it = blocks_.end();
	auto source_block = find_block_(source_address, &source_it);

	if (source_block == nullptr || source_it == blocks_.end())
		throw exception(error_code::block_not_found, source_address);

	auto destination_it = blocks_.end();
	auto destination_block = find_block_(destination_address, &destination_it);

	if (destination_block == nullptr || destination_it == blocks_.end())
		throw exception(error_code::block_not_found, destination_address);

	std::size_t write_size = 0u, current_write_size = 0u, source_offset = 0u, destination_offset = 0u;
	while (write_size < size){
		current_write_size = destination_block->write(source_offset, *source_block, (size - write_size), destination_offset);
		write_size += current_write_size;

		if ((source_block->size_ - source_offset) <= current_write_size){//Use next block
			source_offset = 0u;
			if (++source_it != blocks_.end())
				source_block = *source_it;
			else
				break;
		}
		else//Advance offset
			source_offset += current_write_size;

		if ((destination_block->size_ - destination_offset) <= current_write_size){//Use next block
			destination_offset = 0u;
			if (++destination_it != blocks_.end())
				destination_block = *destination_it;
			else
				break;
		}
		else//Advance offset
			destination_offset += current_write_size;
	}

	return write_size;
}

std::size_t cminus::memory::object::write_(std::size_t destination_address, managed_object &object){
	auto block = get_block_(destination_address);
	if (block == nullptr)
		throw exception(error_code::block_not_found, destination_address);
	return block->write(object);
}

std::size_t cminus::memory::object::set_(std::size_t destination_address, std::byte value, std::size_t size){
	if (size == 0u)
		return size;

	auto destination_it = blocks_.end();
	auto destination_block = find_block_(destination_address, &destination_it);

	if (destination_block == nullptr || destination_it == blocks_.end())
		throw exception(error_code::block_not_found, destination_address);

	std::size_t set_size = 0u;
	while (set_size < size){
		set_size += destination_block->set(0, value, (size - set_size));
		if (++destination_it != blocks_.end())
			destination_block = *destination_it;
		else
			break;
	}

	return set_size;
}

std::shared_ptr<cminus::memory::block> cminus::memory::object::get_block_(std::size_t address) const{
	if (blocks_.empty())
		return nullptr;

	for (auto block : blocks_){
		if (block->address_ == address)
			return block;
	}

	return nullptr;
}

std::shared_ptr<cminus::memory::block> cminus::memory::object::get_next_block_(std::size_t address) const{
	if (blocks_.empty())
		return nullptr;

	auto is_before = true;
	for (auto block : blocks_){
		if (is_before && block->address_ == address)
			is_before = false;
		else if (!is_before)
			return block;
	}

	return nullptr;
}

std::shared_ptr<cminus::memory::block> cminus::memory::object::find_block_(std::size_t address, std::list<std::shared_ptr<block>>::const_iterator *out_it) const{
	if (blocks_.empty()){
		if (out_it != nullptr)
			*out_it = blocks_.end();
		return nullptr;
	}

	for (auto it = blocks_.begin(); it != blocks_.end(); ++it){
		if ((*it)->address_ == address){
			if (out_it != nullptr)
				*out_it = it;
			return *it;
		}

		if ((*it)->hit_test(address)){
			if (out_it != nullptr)
				*out_it = it;
			return std::make_shared<offset_block>(*it, (address - (*it)->address_));
		}
	}

	if (out_it != nullptr)
		*out_it = blocks_.end();

	return nullptr;
}
