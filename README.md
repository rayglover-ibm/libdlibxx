# Libdlibxx

Libdlibxx is a generalized C++14 wrapper library around the unix dl
functions from `<dlfcn.h>` to handle the loading and symbol lookup in
dynamic libraries.

### Dependencies

- A C++14 compiler.

### Licence

This library is licensed under the FreeBSD license.

### Usage

#### Linking to libdlibxx

To link to this library, pass the command line argument `-ldlibxx`.

#### Loading Dynamic Libraries

```cpp
// Construct a DL handle with a string.
dlibxx::handle lib("library-name.so");
```

#### Library Loading Error Handling

A call to the `error()` function will return an `optional` error, which if present will be the most recent error. Error handling code could be performed as
follows after a library has been loaded.

```cpp
if (lib.error()) {
    std::cerr << lib.error().value() << std::endl;
    return 1;
}
```

#### Symbol Lookup

Symbol lookup is performed with the `lookup` member function. You must
pass in the type of the function as the template parameter and the name
of the function in the dynamic library as the function argument.

If the function was successfully loaded, then the `std::function` when
cast to bool will return `true`, and `false` otherwise.

```cpp
// Get an function<Signature> to the function.
auto func_symbol = lib.lookup<void(int)>("function_name");
if (func_symbol) {
    func_symbol(5);
}
else {
    std::cout << "Symbol lookup failed.\n";
}
```

#### Class Instance Creation

Due to the frequency of which dynamic libraries are used to create an
instance of an abstract class satisfying some interface, a simple interface
for creating objects from factory methods is provided.

If the instance could not be created, a `shared_ptr` containing
`nullptr` will be returned.

```cpp
struct base {
    virtual void foo() = 0;
};

std::shared_ptr<base> p = lib.create<base>("factory_function");
if (p)
    p->foo();
else
    std::cout << "Unable to create instance.\n";
```

#### Library fascades

A helper class, `dlibxx::handle_fascade`, can be used to define a collection of functions defined within a single library. The fascade will load the library and search for symbols at construction, and present them for use. For example, to wrap a library implementing the [cblas](https://en.wikipedia.org/wiki/Basic_Linear_Algebra_Subprograms) interface:

```cpp
/* The library public includes */
#include <cblas.h>

class cblas final : dlibxx::handle_fascade
{
    inline cblas(const std::string& path)
        : dlibxx::handle_fascade{ path.c_str() }
    {}

  public:
    op<decltype(::cblas_dnrm2)> dnrm2 { this, "cblas_dnrm2" };
    op<decltype(::cblas_snrm2)> snrm2 { this, "cblas_snrm2" };
    op<decltype(::cblas_dgemv)> dgemv { this, "cblas_dgemv" };
    op<decltype(::cblas_sgemv)> sgemv { this, "cblas_sgemv" };
};

int main()
{
    cblas openblas("libopenblas.so");
    cblas.sgemv(...);
}
```

#### Symbol Resolution Policy

When loading the dynamic library, it can be specified whether all symbols
should be bound on opening or only when they are referenced. The
default is to bind all symbols when the library is loaded (through the
use of `dlibxx::resolve_policy::now`), but lazy binding is also available.

It can be specified in the constructor as follows:

```cpp
dlibxx::handle lib(dlibxx::resolve_policy::lazy, "library-name.so");

```

### Example Code

Run `make` and change to the `examples/` directory.

Run the demo code with `./demo` and when prompted enter:

- `./liba.so`
- `./libb.so`
- `./libc.so`

and observe the results.
