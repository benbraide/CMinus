#include "memory_object.h"

std::shared_ptr<cminus::memory::block> cminus::memory::object::protect_next_block(std::size_t size){
	std::lock_guard<std::shared_mutex> guard(lock_);
	return protect_next_block_(size);
}

std::shared_ptr<cminus::memory::block> cminus::memory::object::allocate_block(std::size_t size, unsigned int attributes){
	std::lock_guard<std::shared_mutex> guard(lock_);
	return allocate_block_(size, attributes);
}

std::shared_ptr<cminus::memory::block> cminus::memory::object::reallocate_block(std::size_t address, std::size_t size){
	std::lock_guard<std::shared_mutex> guard(lock_);
	return reallocate_block_(address, size);
}

void cminus::memory::object::deallocate_block(std::size_t address){
	std::lock_guard<std::shared_mutex> guard(lock_);
	deallocate_block_(address);
}

std::size_t cminus::memory::object::compare(int &result, std::size_t destination_address, const std::byte *buffer, std::size_t size) const{
	std::shared_lock<std::shared_mutex> guard(lock_);
	return compare_(result, destination_address, buffer, size);
}

std::size_t cminus::memory::object::compare(int &result, std::size_t source_address, std::size_t destination_address, std::size_t size) const{
	std::shared_lock<std::shared_mutex> guard(lock_);
	return compare_(result, source_address, destination_address, size);
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

std::size_t cminus::memory::object::write(std::size_t destination_address, managed_object &object){
	std::shared_lock<std::shared_mutex> guard(lock_);
	return write_(destination_address, object);
}

std::size_t cminus::memory::object::set(std::size_t destination_address, std::byte value, std::size_t size){
	std::shared_lock<std::shared_mutex> guard(lock_);
	return set_(destination_address, value, size);
}

std::shared_ptr<cminus::memory::block> cminus::memory::object::get_block(std::size_t address) const{
	std::shared_lock<std::shared_mutex> guard(lock_);
	return get_block_(address, nullptr);
}

std::shared_ptr<cminus::memory::block> cminus::memory::object::get_next_block(std::size_t address) const{
	std::shared_lock<std::shared_mutex> guard(lock_);
	return get_next_block_(address, nullptr);
}

std::shared_ptr<cminus::memory::block> cminus::memory::object::find_block(std::size_t address) const{
	std::shared_lock<std::shared_mutex> guard(lock_);
	return find_block_(address, nullptr);
}

std::shared_ptr<cminus::memory::block> cminus::memory::object::protect_next_block_(std::size_t size){
	auto block = std::make_shared<protected_block>(next_address_, size);
	if (block == nullptr)
		throw exception(error_code::allocation_failure, next_address_);

	blocks_.push_back(block);
	next_address_ += size;

	return block;
}

std::shared_ptr<cminus::memory::block> cminus::memory::object::allocate_block_(std::size_t size, unsigned int attributes){
	if (size == 0u)
		throw exception(error_code::invalid_size, 0u);

	if (blocks_.size() == blocks_.max_size())
		throw exception(error_code::out_of_address_space, 0u);

	auto free_it = blocks_.begin();
	for (; free_it != blocks_.end(); ++free_it){
		if (auto free_block = dynamic_cast<memory::free_block *>(free_it->get()); free_block != nullptr)
			break;
	}

	std::size_t address = 0u;
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
			blocks_.insert(free_it, block);
	}
	else//Append
		blocks_.push_back(block);

	return block;
}

std::shared_ptr<cminus::memory::block> cminus::memory::object::reallocate_block_(std::size_t address, std::size_t size){
	if (size == 0u)
		throw exception(error_code::invalid_size, address);

	auto block = get_block_(address, nullptr);
	if (block == nullptr)
		throw exception(error_code::block_not_found, address);

	if (block->size_ == size)
		return block;

	if (block->is_access_protected())
		throw exception(error_code::access_protected, address);

	if (dynamic_cast<exclusive_block *>(block.get()) == nullptr)
		throw exception(error_code::allocation_failure, address);

	if (block->has_attributes(memory::block::attribute_has_managed_object) || !block->has_attributes(memory::block::attribute_is_heap))
		throw exception(error_code::block_not_resizable, address);

	deallocate_block_(address);
	auto new_block = allocate_block_(size, block->get_attributes());
	if (new_block != nullptr && new_block->address_ != 0u)
		new_block->write(0u, *block, ((size < block->size_) ? size : block->size_));
	else
		throw exception(error_code::allocation_failure, address);

	return new_block;
}

void cminus::memory::object::deallocate_block_(std::size_t address){
	if (blocks_.empty())
		throw exception(error_code::block_not_found, address);

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

std::size_t cminus::memory::object::compare_(int &result, std::size_t destination_address, const std::byte *buffer, std::size_t size) const{
	if (size == 0u){
		result = 0;
		return size;
	}

	auto destination_it = blocks_.end();
	auto destination_block = find_block_(destination_address, &destination_it);

	if (destination_block == nullptr || destination_it == blocks_.end())
		throw exception(error_code::block_not_found, destination_address);

	std::size_t compare_size = 0u;
	while (compare_size < size){
		compare_size += destination_block->compare(result, 0, (buffer + compare_size), (size - compare_size));
		if (result == 0 && ++destination_it != blocks_.end())
			destination_block = *destination_it;
		else
			break;
	}

	return compare_size;
}

std::size_t cminus::memory::object::compare_(int &result, std::size_t source_address, std::size_t destination_address, std::size_t size) const{
	if (size == 0u){
		result = 0;
		return size;
	}

	auto source_it = blocks_.end();
	auto source_block = find_block_(source_address, &source_it);

	if (source_block == nullptr || source_it == blocks_.end())
		throw exception(error_code::block_not_found, source_address);

	auto destination_it = blocks_.end();
	auto destination_block = find_block_(destination_address, &destination_it);

	if (destination_block == nullptr || destination_it == blocks_.end())
		throw exception(error_code::block_not_found, destination_address);

	std::size_t compare_size = 0u, current_compare_size = 0u, source_offset = 0u, destination_offset = 0u;
	while (compare_size < size){
		current_compare_size = destination_block->compare(result, source_offset, *source_block, (size - compare_size), destination_offset);
		compare_size += current_compare_size;

		if (result != 0)
			break;

		if ((source_block->size_ - source_offset) <= current_compare_size){//Use next block
			source_offset = 0u;
			if (++source_it != blocks_.end())
				source_block = *source_it;
			else
				break;
		}
		else//Advance offset
			source_offset += current_compare_size;

		if ((destination_block->size_ - destination_offset) <= current_compare_size){//Use next block
			destination_offset = 0u;
			if (++destination_it != blocks_.end())
				destination_block = *destination_it;
			else
				break;
		}
		else//Advance offset
			destination_offset += current_compare_size;
	}

	return compare_size;
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
	auto block = get_block_(destination_address, nullptr);
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

std::shared_ptr<cminus::memory::block> cminus::memory::object::get_block_(std::size_t address, std::list<std::shared_ptr<block>>::const_iterator *out_it) const{
	if (out_it != nullptr)
		*out_it = blocks_.end();

	if (blocks_.empty())
		return nullptr;

	for (auto it = blocks_.begin(); it != blocks_.end(); ++it){
		if ((*it)->address_ == address){
			if (out_it != nullptr)
				*out_it = it;
			return *it;
		}
	}

	return nullptr;
}

std::shared_ptr<cminus::memory::block> cminus::memory::object::get_next_block_(std::size_t address, std::list<std::shared_ptr<block>>::const_iterator *out_it) const{
	if (out_it != nullptr)
		*out_it = blocks_.end();

	std::list<std::shared_ptr<block>>::const_iterator it;
	get_block_(address, &it);
	if (it == blocks_.end() || ++it == blocks_.end())
		return nullptr;

	if (out_it != nullptr)
		*out_it = it;

	return *it;
}

std::shared_ptr<cminus::memory::block> cminus::memory::object::find_block_(std::size_t address, std::list<std::shared_ptr<block>>::const_iterator *out_it) const{
	if (out_it != nullptr)
		*out_it = blocks_.end();

	if (blocks_.empty())
		return nullptr;

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

	return nullptr;
}
