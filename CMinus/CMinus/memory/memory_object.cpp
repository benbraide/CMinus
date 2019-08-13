#include "memory_object.h"

std::shared_ptr<cminus::memory::block> cminus::memory::object::protect_next_block(std::size_t size){
	std::lock_guard<std::shared_mutex> guard(lock_);
	return protect_next_block_(size);
}

std::shared_ptr<cminus::memory::block> cminus::memory::object::allocate_exclusive_block(std::size_t size, unsigned int attributes){
	std::lock_guard<std::shared_mutex> guard(lock_);
	return allocate_block_(size, attributes, allocation_state_use_free);
}

std::shared_ptr<cminus::memory::block> cminus::memory::object::allocate_composite_block(std::size_t size, unsigned int attributes, std::size_t reserve_size){
	std::lock_guard<std::shared_mutex> guard(lock_);
	return allocate_block_(size, attributes, (allocation_state_use_free | allocation_state_composite), reserve_size);
}

std::shared_ptr<cminus::memory::block> cminus::memory::object::reallocate_block(std::size_t address, std::size_t size){
	std::lock_guard<std::shared_mutex> guard(lock_);
	return reallocate_block_(address, size);
}

void cminus::memory::object::deallocate_block(std::size_t address){
	std::lock_guard<std::shared_mutex> guard(lock_);
	deallocate_block_(address);
}

std::size_t cminus::memory::object::read(std::size_t source_address, std::byte *buffer, std::size_t size) const{
	std::shared_lock<std::shared_mutex> guard(lock_);
	return read_(source_address, buffer, size);
}

std::size_t cminus::memory::object::read(std::size_t source_address, io::binary_writer &buffer, std::size_t size) const{
	std::shared_lock<std::shared_mutex> guard(lock_);
	return read_(source_address, buffer, size);
}

std::size_t cminus::memory::object::read(std::size_t source_address, std::size_t destination_address, std::size_t size) const{
	std::shared_lock<std::shared_mutex> guard(lock_);
	return read_(source_address, destination_address, size);
}

std::size_t cminus::memory::object::write(std::size_t destination_address, const std::byte *buffer, std::size_t size){
	std::shared_lock<std::shared_mutex> guard(lock_);
	return write_(destination_address, buffer, size);
}

std::size_t cminus::memory::object::write(std::size_t destination_address, const io::binary_reader &buffer, std::size_t size){
	std::shared_lock<std::shared_mutex> guard(lock_);
	return write_(destination_address, buffer, size);
}

std::size_t cminus::memory::object::write(std::size_t source_address, std::size_t destination_address, std::size_t size){
	std::shared_lock<std::shared_mutex> guard(lock_);
	return write_(source_address, destination_address, size);
}

std::size_t cminus::memory::object::set(std::size_t destination_address, std::byte value, std::size_t size){
	std::shared_lock<std::shared_mutex> guard(lock_);
	return set_(destination_address, value, size);
}

std::shared_ptr<cminus::memory::block> cminus::memory::object::get_block(std::size_t address) const{
	std::shared_lock<std::shared_mutex> guard(lock_);
	return get_block_(address);
}

std::shared_ptr<cminus::memory::block> cminus::memory::object::get_next_block(std::size_t address) const{
	std::shared_lock<std::shared_mutex> guard(lock_);
	return get_next_block_(address);
}

std::shared_ptr<cminus::memory::block> cminus::memory::object::find_block(std::size_t address) const{
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

std::shared_ptr<cminus::memory::block> cminus::memory::object::allocate_block_(std::size_t size, unsigned int attributes, unsigned int state, std::size_t reserve_size){
	if (size == 0u)//Do nothing
		return nullptr;

	if (blocks_.size() == blocks_.max_size())
		throw exception(error_code::out_of_address_space, 0u);

	std::size_t address = 0u;
	auto free_it = blocks_.end();

	if (!blocks_.empty() && (state & allocation_state_use_free) != 0u){//Search for free block
		for (auto it = blocks_.begin(); it != blocks_.end(); ++it){
			if (auto free_block = dynamic_cast<memory::free_block *>(it->get()); free_block != nullptr && size <= free_block->size_){
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

	std::shared_ptr<block> block;
	if ((state & allocation_state_composite) == 0u)
		block = std::make_shared<exclusive_block>(address, size, attributes);
	else//Composite
		block = std::make_shared<composite_block>(address, size, attributes, reserve_size);

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
	if (size == 0u || blocks_.empty())//Do nothing
		return nullptr;

	std::shared_ptr<cminus::memory::block> block;
	for (auto it = blocks_.begin(); it != blocks_.end(); ++it){
		if ((*it)->address_ != address)
			continue;

		if ((*it)->is_access_protected())
			throw exception(error_code::access_protected, address);

		if (dynamic_cast<exclusive_block *>(it->get()) == nullptr)
			throw exception(error_code::allocation_failure, address);

		block = *it;
		if (size < block->size_){//Shrink
			auto new_block = std::make_shared<exclusive_block>(address, size, block->get_attributes());
			if (new_block == nullptr)
				throw exception(error_code::allocation_failure, address);

			blocks_.insert(it, new_block);
			new_block->write(0, *block, size);

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
			allocate_block_(size, block->get_attributes(), allocation_state_use_free);
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
