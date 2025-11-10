#ifndef GLU_GIL_INSTMACROS_HPP
#define GLU_GIL_INSTMACROS_HPP

#define GLU_GIL_GEN_OPERAND(Name, Child, _name) \
    Child _name;                                \
                                                \
public:                                         \
    Child get##Name() const                     \
    {                                           \
        return _name;                           \
    }                                           \
    void set##Name(Child child)                 \
    {                                           \
        _name = child;                          \
    }                                           \
                                                \
private:

#define GLU_GIL_GEN_OPERAND_LIST_TRAILING_OBJECTS(Self, num, Child, Name) \
    using Name##TrailingParams = llvm::TrailingObjects<Self, Child>;      \
    friend Name##TrailingParams;                                          \
                                                                          \
    unsigned num;                                                         \
    size_t numTrailingObjects(                                            \
        typename Name##TrailingParams::template OverloadToken<Child>      \
    ) const                                                               \
    {                                                                     \
        return num;                                                       \
    }                                                                     \
    llvm::MutableArrayRef<Child> get##Name##Mutable()                     \
    {                                                                     \
        return { this->template getTrailingObjects<Child>(), num };       \
    }                                                                     \
                                                                          \
public:                                                                   \
    llvm::ArrayRef<Child> get##Name() const                               \
    {                                                                     \
        return { this->template getTrailingObjects<Child>(), num };       \
    }                                                                     \
    void set##Name(llvm::ArrayRef<Child> children)                        \
    {                                                                     \
        std::copy(                                                        \
            children.begin(), children.end(),                             \
            this->template getTrailingObjects<Child>()                    \
        );                                                                \
    }                                                                     \
                                                                          \
private:                                                                  \
    void init##Name(llvm::ArrayRef<Child> children)                       \
    {                                                                     \
        num = children.size();                                            \
        std::uninitialized_copy(                                          \
            children.begin(), children.end(),                             \
            this->template getTrailingObjects<Child>()                    \
        );                                                                \
    }

#endif // GLU_GIL_INSTMACROS_HPP
