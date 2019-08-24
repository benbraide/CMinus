#pragma once

#include "../node/node_object.h"
#include "../memory/memory_reference.h"

namespace cminus::logic::storage{
	enum class error_code{
		nil,
		duplicate_entry,
		duplicate_function,
	};

	class exception : public std::exception{
	public:
		using base_type = std::exception;

		exception(error_code code);

		error_code get_code() const;

	private:
		error_code code_;
	};

	class object : public naming::parent{
	public:
		using parent_base_type = naming::parent;

		explicit object(const std::string &value, object *parent = nullptr);

		virtual ~object();

		virtual void add(const std::string &name, std::shared_ptr<memory::reference> entry);

		virtual void remove(const std::string &name);

		virtual std::shared_ptr<memory::reference> find(const std::string &name, bool search_tree, const object **branch = nullptr) const;

		virtual std::shared_ptr<type::object> find_type(const std::string &name, bool search_tree, const object **branch = nullptr) const;

		virtual object *find_storage(const std::string &name, bool search_tree, const object **branch = nullptr) const;

		virtual std::shared_ptr<attributes::object> find_attribute(const std::string &name, bool search_tree, const object **branch = nullptr) const;

	protected:
		std::unordered_map<std::string, std::shared_ptr<memory::reference>> entries_;
		std::unordered_map<std::string, std::shared_ptr<attributes::object>> attributes_;
	};
}

