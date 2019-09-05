#pragma once

#include "function_declaration.h"

namespace cminus::declaration::string{
	class constructor : public function{
	public:
		explicit constructor(logic::naming::parent *parent);

		virtual ~constructor();

		virtual bool is_defined() const override;

		virtual bool supports_return_statement() const override;

	protected:
		virtual void print_return_(logic::runtime &runtime) const override;
	};

	class default_constructor : public constructor{
	public:
		explicit default_constructor(logic::naming::parent *parent);

		virtual ~default_constructor();

	protected:
		virtual void evaluate_body_(logic::runtime &runtime) const override;
	};

	class copy_constructor : public constructor{
	public:
		copy_constructor(logic::runtime &runtime, logic::naming::parent *parent);

		virtual ~copy_constructor();

	protected:
		virtual void evaluate_body_(logic::runtime &runtime) const override;
	};

	class sub_constructor : public constructor{
	public:
		sub_constructor(logic::runtime &runtime, logic::naming::parent *parent);

		virtual ~sub_constructor();

	protected:
		virtual void evaluate_body_(logic::runtime &runtime) const override;
	};

	class assignment_constructor : public constructor{
	public:
		assignment_constructor(logic::runtime &runtime, logic::naming::parent *parent);

		virtual ~assignment_constructor();

	protected:
		virtual void evaluate_body_(logic::runtime &runtime) const override;
	};

	class fill_constructor : public constructor{
	public:
		fill_constructor(logic::runtime &runtime, logic::naming::parent *parent);

		virtual ~fill_constructor();

	protected:
		virtual void evaluate_body_(logic::runtime &runtime) const override;
	};

	class destructor : public function{
	public:
		explicit destructor(logic::naming::parent *parent);

		virtual ~destructor();

		virtual bool is_defined() const override;

		virtual bool supports_return_statement() const override;

	protected:
		virtual void print_return_(logic::runtime &runtime) const override;

		virtual void evaluate_body_(logic::runtime &runtime) const override;
	};

	class empty : public function{
	public:
		empty(logic::runtime &runtime, logic::naming::parent *parent);

		virtual ~empty();

		virtual bool is_defined() const override;

	protected:
		virtual void evaluate_body_(logic::runtime &runtime) const override;
	};

	class size : public function{
	public:
		explicit size(logic::runtime &runtime, logic::naming::parent *parent);

		virtual ~size();

		virtual bool is_defined() const override;

	protected:
		virtual void evaluate_body_(logic::runtime &runtime) const override;
	};

	class data : public function{
	public:
		data(logic::runtime &runtime, bool read_only, logic::naming::parent *parent);

		virtual ~data();

		virtual bool is_defined() const override;

	protected:
		virtual void evaluate_body_(logic::runtime &runtime) const override;
	};

	class at : public function{
	public:
		at(logic::runtime &runtime, bool read_only, logic::naming::parent *parent);

		virtual ~at();

		virtual bool is_defined() const override;

	protected:
		virtual void evaluate_body_(logic::runtime &runtime) const override;
	};

	class resize : public function{
	public:
		resize(logic::runtime &runtime, logic::naming::parent *parent);

		virtual ~resize();

		virtual bool is_defined() const override;

	protected:
		virtual void evaluate_body_(logic::runtime &runtime) const override;
	};

	class clear : public function{
	public:
		clear(logic::runtime &runtime, logic::naming::parent *parent);

		virtual ~clear();

		virtual bool is_defined() const override;

	protected:
		virtual void evaluate_body_(logic::runtime &runtime) const override;
	};
}
