#pragma once

#include <functional>
#include <memory>
#include <string>
#include <experimental/optional>

namespace dlibxx {
namespace util
{
	template<class T>
	using optional = std::experimental::optional<T>;

	/* Trick to convert a void* to a std::function.
	   Since casting from a void* to a function pointer is undefined,
	   we have to do a workaround. I am not positive if this is completely
	   portable or not. */
	template <typename Ret, typename... Args>
	std::function<Ret(Args...)> fptr_cast(void* fptr)
	{
		using function_type = Ret (*)(Args...);
		using ptr_size_type = std::conditional<sizeof(fptr) == 4, long, long long>::type;

		return reinterpret_cast<function_type>(
			reinterpret_cast<ptr_size_type>(fptr)
		);
	}
}
}

namespace dlibxx
{
	enum class resolve_policy { lazy, now };

	/* Wrapper around the handle for a dynamic library. */
	class handle
	{
	  public:
		static bool has_library_loaded(const char* file);

		/* Load the dynamic library by the given path. */
		handle(const char* file, resolve_policy = dlibxx::resolve_policy::lazy);

		/* Open a handle for the main program. */
		handle(resolve_policy);

		/* Closes the underlying handle */
		~handle();

		/* The last error that occured for this handle, if any. */
		inline const util::optional<std::string>& error() const { return error_; }

		/* Lookup the function named FNAME in the dynamic library. */
		template <typename Prototype>
		inline util::optional< std::function<Prototype> > lookup(const char* fname) const {
			return symbol_lookup_wrapper<Prototype>::lookup(*this, fname);
		}

		/* Create an instance of a type using the FNAME factory function.
		   Pass any arguments necessary to the factory method as well. */
		template <typename T, typename... Args>
		inline std::shared_ptr<T> create(char const* fname, Args&&... args) const
		{
			auto sym = this->symbol_lookup_impl<T*, Args...>(fname);

			if (sym) return { sym.value()(std::forward<Args>(args)...), std::default_delete<T>() };
			else     return { nullptr };
		}

	  private:
		void* handle_;
		mutable util::optional<std::string> error_;

		bool set_error() const;
		void* sym_lookup(const char* fname) const;

		template <typename Ret, typename... Args>
		util::optional< std::function<Ret (Args...)> > symbol_lookup_impl(char const* fname) const
		{
			/* Lookup the symbol from the dynamic library. */
			auto fptr = util::fptr_cast<Ret, Args...>(sym_lookup(fname));

			if (set_error()) { return {}; }
			return fptr;
		}

		/* Helper class to allow for function pointer syntax in template parameter.
		   To allow a call such as lib.lookup<int(int)>("name"), a class template
		   with partial specialization must be used since function templates
		   cannot be partially specialized. */
		template <typename Prototype>
		struct symbol_lookup_wrapper;

		template <typename Ret, typename... Args>
		struct symbol_lookup_wrapper<Ret(Args...)>
		{
			static inline auto lookup(handle const& lib, char const* fname) {
				return lib.symbol_lookup_impl<Ret, Args...>(fname);
			}
		};
	};

	/* Utility base class; manage a handle to a shared library and presents
	   symbols from it as a fascade. */
	class handle_fascade
	{
	  public:
		template <typename P>
		class op final : public std::function<P>
		{
			auto get_or_abort(dlibxx::handle& h, const char* sym)
			{
				auto fn = h.lookup<P>(sym);
				if (h.error()) {
					fprintf(stderr, "%s\n", h.error().value().c_str());
					abort();
				}
				return std::move(fn.value());
			}

		  public:
			op(dlibxx::handle& h, const char* sym)
				: std::function<P>{ get_or_abort(h, sym) }
			{}
		};

	  protected:
		inline handle_fascade(
			const char* path, resolve_policy rp = dlibxx::resolve_policy::lazy)
			: h{ path, rp } {}

		handle h;
	};
}