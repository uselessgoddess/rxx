Immerse yourself in the delights of Rust without leaving C++

Now there are:

* stable constexpr `unreachable` with behaviour like `never type` from rust (in standard since C++23)
* `unreachable_t` - almost `never type`
* `transmute` `transmute_cast` `as` tried has behaviour like transmutes from rust, but can't transmute pointers in constexpr
* `not_null<T>` smart non-null pointer which `assume` via `unreachable()`
* functions from rust `ptr` (`ptr::read`, `ptr::write`, `ptr::copy`, `ptr::as`) - some of them are constexpr
----
They are completely not constexpr:
* `union_t` - safe union with access to members like C and Rust
* `maybe_uninit` based on `union_t`
* `manyally_destroy`
