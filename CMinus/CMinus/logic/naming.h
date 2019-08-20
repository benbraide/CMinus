#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include "../io/io_writer.h"

namespace cminus::logic{
	struct runtime;
}

namespace cminus::logic::naming{
	class parent;

	class object{
	public:
		virtual ~object() = default;

		virtual parent *get_naming_parent() const = 0;

		virtual const std::string &get_naming_value() const = 0;

		virtual std::string get_qualified_naming_value() const = 0;

		virtual void print(logic::runtime &runtime, bool is_qualified) const = 0;

		virtual bool is_same(const object &target) const = 0;

		static std::shared_ptr<object> build_name(const std::vector<std::string> &branches);

		static std::shared_ptr<object> build_name(const std::string &value);
	};

	class single : public object{
	public:
		explicit single(const std::string &value, naming::parent *parent = nullptr);

		virtual ~single();

		virtual parent *get_naming_parent() const override;

		virtual const std::string &get_naming_value() const override;

		virtual std::string get_qualified_naming_value() const override;

		virtual void print(logic::runtime &runtime, bool is_qualified) const override;

		virtual bool is_same(const object &target) const override;

	protected:
		friend class parent;

		parent *parent_ = nullptr;
		std::string value_;
	};

	class parent : public single{
	public:
		explicit parent(const std::string &value, naming::parent *parent = nullptr);

		virtual ~parent();

		virtual std::shared_ptr<object> add_object(const std::string &value, bool is_parent);

		virtual bool remove_object(const std::string &value);

		virtual std::shared_ptr<object> find_object(const std::string &value) const;

	protected:
		std::unordered_map<std::string, std::shared_ptr<object>> objects_;
	};
}
