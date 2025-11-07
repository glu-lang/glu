#ifndef GLU_AST_ASTNODE_MACROS_HPP
#define GLU_AST_ASTNODE_MACROS_HPP

#define GLU_AST_GEN_CHILD(Self, Child, _name, Name)     \
    Child _name;                                        \
                                                        \
public:                                                 \
    Child get##Name()                                   \
    {                                                   \
        return _name;                                   \
    }                                                   \
    void set##Name(Child child)                         \
    {                                                   \
        _name = child;                                  \
        if (_name != nullptr)                           \
            _name->setParent(this);                     \
    }                                                   \
                                                        \
private:                                                \
    void init##Name(Child child, bool nullable = false) \
    {                                                   \
        if (!nullable)                                  \
            assert(child && #Name " cannot be null");   \
        _name = child;                                  \
        if (_name != nullptr)                           \
            _name->setParent(this);                     \
    }

#define GLU_AST_GEN_CHILDREN_TRAILING_OBJECTS(Self, num, Child, Name) \
    using Name##TrailingParams = llvm::TrailingObjects<Self, Child>;  \
    friend Name##TrailingParams;                                      \
                                                                      \
    unsigned num;                                                     \
    size_t numTrailingObjects(                                        \
        typename Name##TrailingParams::template OverloadToken<Child>  \
    ) const                                                           \
    {                                                                 \
        return num;                                                   \
    }                                                                 \
    llvm::MutableArrayRef<Child> get##Name##Mutable()                 \
    {                                                                 \
        return { this->template getTrailingObjects<Child>(), num };   \
    }                                                                 \
                                                                      \
public:                                                               \
    llvm::ArrayRef<Child> get##Name() const                           \
    {                                                                 \
        return { this->template getTrailingObjects<Child>(), num };   \
    }                                                                 \
    void set##Name(llvm::ArrayRef<Child> children)                    \
    {                                                                 \
        std::copy(                                                    \
            children.begin(), children.end(),                         \
            this->template getTrailingObjects<Child>()                \
        );                                                            \
        for (Child &child : get##Name##Mutable()) {                   \
            child->setParent(this);                                   \
        }                                                             \
    }                                                                 \
                                                                      \
private:                                                              \
    void init##Name(llvm::ArrayRef<Child> children)                   \
    {                                                                 \
        num = children.size();                                        \
        std::uninitialized_copy(                                      \
            children.begin(), children.end(),                         \
            this->template getTrailingObjects<Child>()                \
        );                                                            \
        for (Child &child : get##Name##Mutable()) {                   \
            child->setParent(this);                                   \
        }                                                             \
    }

#endif // GLU_AST_ASTNODE_MACROS_HPP
