#pragma once

#include "global_storage.h"

namespace cminus::logic{
	struct runtime{
		memory::object &memory_object;
		io::writer &writer;

		std::shared_ptr<storage::global> global_storage;
		std::shared_ptr<storage::object> current_storage;

		std::list<storage::object *> look_up_storages;
		std::shared_ptr<memory::reference> self;
	};

	class exception : public std::exception{
	public:
		using base_type = std::exception;

		exception(const std::string &message, std::size_t line, std::size_t column);

		virtual ~exception();

		virtual char const *what() const override;

		virtual std::size_t get_line() const;

		virtual std::size_t get_column() const;

	private:
		std::string message_;
		std::size_t line_;
		std::size_t column_;
	};
}
