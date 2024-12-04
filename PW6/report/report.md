# 实验报告

> PB22081571 薄震宇
>
> PB22000257 阎昶澍
>
> PB22020514 郭东昊

## 问题回答

### 1-1

`while`语句对应的代码布局主要可以分为4个基本块：

第1个基本块完成`while`语句前需要进行的初始化等操作，然后无条件进入第2个基本块，可以理解为`while`语句的进入基本块；第2个基本块计算条件表达式的真假；第3个基本块执行循环体即`while`语句的`true`分支；第4个基本块执行循环后面的内容即`while`语句的`false`分支。

以我的`while_hand.ll`中`while`语句的相关代码为例：

```asm
; 全局main函数定义
define dso_local i32 @main() #0 {
    ; 初始化
    store i32 0, i32* @b, align 4
    store i32 3, i32* @a, align 4
    br label %1

1:                                                      ; preds = %0, %4
    ; 加载a的值
    %2 = load i32, i32* @a, align 4
    ; 判断a是否大于0
    %3 = icmp sgt i32 %2, 0
    ; 如果a大于0，跳转到5
    br i1 %3, label %4, label %9

4:                                                      ; preds = %1
    ; 加载a, b的值
    %5 = load i32, i32* @a, align 4
    %6 = load i32, i32* @b, align 4
    ; 计算并存储
    %7 = add nsw i32 %6, %5
    store i32 %7, i32* @b, align 4
    %8 = sub nsw i32 %5, 1
    store i32 %8, i32* @a, align 4
    br label %1

9:                                                      ; preds = %1
    ; 返回b的值
    %10 = load i32, i32* @b, align 4
    ret i32 %10
}
```

第一个`br`指令为无条件跳转，用于进入`while`语句。

第二个`br`指令是根据条件表达式的值选择跳转位置。以我的语句`br i1 %3, label %4, label %9`，第一个寄存器%3是条件表达式的值，类型为`i1`，为真时跳转到`label %4`即true分支，为假时跳转到`%9`即false分支。

第三个`br`指令也是无条件跳转，跳转至条件判断的基本块，用于在执行完一次循环体后判断是否还要继续循环。

### 1-2

调用者：如果函数需要传参，在进行函数调用前先计算要传递的参数，然后使用`call`语句调用函数，如果函数有返回值，需要为返回值分配存储空间并存储。

被调用者：如果有参数，首先需要为所有参数分配空间并存储，在需要使用时再加载。

### 2-1

1. `%2 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 %0` 用于访问数组中的元素，需要提供数组的类型和基地址。

   [10 x i32] 表示一个包含10个i32元素的数组类型，[10 x i32]* %1 表示一个指向该数组的指针，i32 0 表示数组的基地址，i32 %0 表示数组中第%0个元素的偏移量。

   这种用法的原因是，GEP指令需要知道数组的基类型和数组的大小，以便正确计算元素的地址。

2. `%2 = getelementptr i32, i32* %1, i32 %0`用于计算指针的偏移量，只需要提供指针的基本类型和偏移量。i32 表示指针指向的基本类型，i32* %1 表示一个指向i32类型的指针，i32 %0 表示偏移量，即从指针%1开始偏移%0个i32元素的位置。

   这种用法的原因是，GEP指令只需要知道指针指向的基本类型和偏移量，以便正确计算新的指针位置。

### 3-1

1. 在`scope`内单独处理`func`可以将函数的定义和处理放在单独的作用域，这样可以更好地管理变量和函数的作用域，避免变量名冲突等问题。
2. 可以更好地管理资源的分配和释放，例如在函数开始时分配资源，结束时释放资源，确保资源的正确使用和释放。
3. 在`scope`内单独处理`func`，如果函数内发生错误，可以更容易地定位和处理错误，而不会影响到其他部分的代码。
4. 可以更好地支持函数的嵌套和递归调用，每次调用都会创建一个新的作用域，确保函数调用的正确性和安全性。

## 实验设计

本次实验的设计主要可以分为三个部分：变量相关，函数相关，控制流语句相关。

### 变量相关

变量相关的函数主要有 `VarDef` 和 `LVal` 两部分，其中 `VarDef` 部分用于定义变量，而 `LVal` 部分用于获取变量的值。

在变量定义的 `VarDef` 部分，需要分别讨论是常量还是变量。对于一个常变量，为了节省时间和空间以及避免该变量被非法篡改，不应将这个常变量作为一个变量存储进程序中，而是应该先记录下来这个常变量的值，在以后使用到该常变量的时候直接用这个值替代。如果常变量定义的初始值与常变量的类型不一致，则还需要先进行一次类型转换，但是这个类型转换也不是写入程序中的，而是在编译时期直接完成。

``` cpp
Ptr<Value> var;
// Calculate the initializer first
node.initializers->accept(*this);
if (cur_type == INT32_T) {
    // Is an integer variable
    // If the initializer is a constant float
    // A type cast is needed
    auto const_float = this->visitee_val->as<ConstantFloat>();
    if (const_float) {
        this->visitee_val = CONST_INT(int(const_float->get_value()));
    }
    // Initialize the variable
    auto const_int = this->visitee_val->as<ConstantInt>();
    auto initializer = const_int->get_value();
    var = ConstantInt::create(initializer, module);
} else {
    // Is a float variable
    // If the initializer is a constant integer
    // A type cast is needed
    auto const_int = this->visitee_val->as<ConstantInt>();
    if (const_int) {
        this->visitee_val = CONST_FLOAT(float(const_int->get_value()));
    }
    // Initialize the variable
    auto const_float = this->visitee_val->as<ConstantFloat>();
    auto initializer = const_float->get_value();
    var = ConstantFloat::create(initializer, module);
}
scope.push(node.name, var);
```

对于一个常数组，由于访问这个数组的时候可能会以一个变量作为下标（此时对应的值还是不可直接确定的），所以我们不得不将这个数组存储进程序里，但是，对于访问这个数组并以常量作为下标的情况，还是应该直接将数组对应下标的值直接替代进程序，不过这一部分是后面 `LVal` 所要处理的事情。

如果这个数组在全局，则只需要先获取初始化的内容，然后在剩余的空间补 0，放入全局变量空间里即可。

如果这个数组在局部，那么它还需要存进函数的 `entry_block` 中（对后面的局部变量和局部数组也是如此），而正常情况下 `entry_block` 是已经结束了的，直接进行 `create_alloca` 会放入当前的代码块中。所以，为了能够将数组放进 `entry_block` 中，我们需要将 `entry_block` 的结束标志转存下来并删掉，然后在进行 `create_alloca` 之后，将这条指令从当前代码块中删除，并插入到 `entry_block` 的末尾，同时将这条指令的所在块也设置成 `entry_block`。最后，再将结束标志插入回去。在初始化完数组之后，为了能够在之后以常数下标访问数组时快速获取到对应的值，需要将这个数组的初始化值在全局变量列表中也存储一份，所以此时还需要设置一个该数组在全局变量列表中对应的名字，这里采用的是 `UUID@func_name.var_name` 的命名方法，其中 `UUID` 为一段临时随机生成的字符串，以防两个名字重复。

``` cpp
// Fetch the length of the array
node.array_length[0]->accept(*this);
auto length = this->visitee_val->as<ConstantInt>()->get_value();
// Initialize the array
auto array_type = ArrayType::get(cur_type, length);
init_val.clear();
node.initializers->accept(*this);
if (scope.in_global()) {
    // If it is in the global scope
    // Create a global array
    int initialized_size = init_val.size();
    while (initialized_size < length) {
        // Append empty values to the array
        // Until the array is fully initialized
        if (cur_type == INT32_T) {
            init_val.push_back(CONST_INT(0));
        } else {
            init_val.push_back(CONST_FLOAT(0.0));
        }
        initialized_size++;
    }
    auto initializer = ConstantArray::create(array_type, init_val, module);
    auto global_var = GlobalVariable::create(node.name, module, array_type, true, initializer);
    scope.push(node.name, global_var);
} else {
    // If it is not in the global scope
    // Create a local variable
    // The space of the local variable should be allocated in the entry block
    // If the entry block is terminated
    // The value should be inserted before the terminator
    auto entry_block_terminator = entry_block->get_terminator();
    if (entry_block_terminator) {
        // If the entry block is terminated
        // Remove the terminator first
        entry_block->get_instructions().pop_back();
    }
    // Allocate the space for the array
    auto var = builder->create_alloca(array_type);
    // The create_alloca defaults to the current block
    // It should be moved to the entry block
    cur_block->get_instructions().pop_back();
    entry_block->add_instruction(std::dynamic_pointer_cast<Instruction>(var));
    std::dynamic_pointer_cast<Instruction>(var)->set_parent(entry_block);
    // Re-place the terminator if it is removed before
    if (entry_block_terminator) {
        entry_block->add_instruction(entry_block_terminator);
    }
    // Initialize the array
    int id = 0;
    for (auto initializer : init_val) {
        builder->create_store(initializer, builder->create_gep(var, {CONST_INT(0), CONST_INT(id)}));
        id++;
    }
    // Fill the remaining elements with 0
    while (id < length) {
        if (cur_type == INT32_T) {
            builder->create_store(CONST_INT(0), builder->create_gep(var, {CONST_INT(0), CONST_INT(id)}));
        } else {
            builder->create_store(CONST_FLOAT(0.), builder->create_gep(var, {CONST_INT(0), CONST_INT(id)}));
        }
        id++;
    }
    scope.push(node.name, var);
    // Add to a global const array
    int initialized_size = init_val.size();
    while (initialized_size < length) {
        // Append empty values to the array
        // Until the array is fully initialized
        if (cur_type == INT32_T) {
            init_val.push_back(CONST_INT(0));
        } else {
            init_val.push_back(CONST_FLOAT(0.0));
        }
        initialized_size++;
    }
    // Create a global array
    // And store the local array to the global array
    // The name of the global array should be unique
    auto initializer = ConstantArray::create(array_type, init_val, module);
    std::string global_name = generate_uuid() + '@' + cur_func->get_name() + '.' + node.name;
    auto global_var = GlobalVariable::create(global_name, module, array_type, true, initializer);
    scope.push(global_name, global_var);
    const_local_values[var] = global_var;
}
```

对于一个变量，同样也需要考虑到在全局和在局部两种情况，同时变量还存在是否被初始化的情况需要考虑。如果该变量在全局，被初始化了，那就获取初始化的值（全局变量初始化的值一定是常量），如果类型不同则进行一次编译期类型转换，如果没被初始化，则默认初始化为 0。

如果该变量在局部，被初始化了，那情况就稍微复杂一点了。首先和之前的局部常数组一样，也需要将变量存入 `entry_block`。因为初始化的值有可能是变量，所以这里对于类型不一致的情况需要使用变量的类型转换，即 `create_fptosi` 或者 `create_sitofp`。

``` cpp
if (scope.in_global()) {
    // If it is in the global scope
    if (node.is_inited) {
        node.initializers->accept(*this);
        Ptr<Constant> initializer;
        if (cur_type == INT32_T) {
            // Is an integer variable
            // If the initializer is a constant float
            // A type cast is needed
            auto const_float = this->visitee_val->as<ConstantFloat>();
            if (const_float) {
                this->visitee_val = CONST_INT(int(const_float->get_value()));
            }
            // Initialize the variable
            initializer = this->visitee_val->as<ConstantInt>();
        } else {
            // Is a float variable
            // If the initializer is a constant integer
            // A type cast is needed
            auto const_int = this->visitee_val->as<ConstantInt>();
            if (const_int) {
                this->visitee_val = CONST_FLOAT(float(const_int->get_value()));
            }
            // Initialize the variable
            initializer = this->visitee_val->as<ConstantFloat>();
        }
        // Create a global variable
        auto global_var = GlobalVariable::create(node.name, module, cur_type, false, initializer);
        scope.push(node.name, global_var);
    } else {
        // Initialize the variable with 0
        auto initializer = ConstantZero::create(cur_type, module);
        // Create a global variable
        auto global_var = GlobalVariable::create(node.name, module, cur_type, false, initializer);
        scope.push(node.name, global_var);
    }
} else {
    // If it is not in the global scope
    // Create a local variable
    // The space of the local variable should be allocated in the entry block
    // If the entry block is terminated
    // The value should be inserted before the terminator
    auto entry_block_terminator = entry_block->get_terminator();
    if (entry_block_terminator) {
        // If the entry block is terminated
        // Remove the terminator first
        entry_block->get_instructions().pop_back();
    }
    // Allocate the space for the variable
    auto var = builder->create_alloca(cur_type);
    // The create_alloca defaults to the current block
    // It should be moved to the entry block
    cur_block->get_instructions().pop_back();
    auto inst = var->as<Instruction>();
    entry_block->add_instruction(inst);
    inst->set_parent(entry_block);
    // Re-place the terminator if it is removed before
    if (entry_block_terminator) {
        entry_block->add_instruction(entry_block_terminator);
    }
    // Initialize the variable
    if (node.is_inited) {
        node.initializers->accept(*this);
        auto var_pointer_type = var->get_type()->get_pointer_element_type();
        if (var_pointer_type->is_integer_type()) {
            // If the variable is an integer variable
            // and the initializer is a float type
            // A type cast is needed
            if (this->visitee_val->get_type()->is_float_type()) {
                this->visitee_val = builder->create_fptosi(this->visitee_val, INT32_T);
            }
        } else if (var_pointer_type->is_float_type()) {
            // If the variable is a float variable
            // and the initializer is an integer type
            // A type cast is needed
            if (this->visitee_val->get_type()->is_integer_type()) {
                this->visitee_val = builder->create_sitofp(this->visitee_val, FLOAT_T);
            }
        }
        builder->create_store(this->visitee_val, var);
    }
    scope.push(node.name, var);
}
```

对于一个数组，处理方法与变量类似，只是需要将针对变量的初始化方法换成针对数组的初始化方法，并为剩余的空间补 0 即可。

``` cpp
node.array_length[0]->accept(*this);
auto const_int = this->visitee_val->as<ConstantInt>();
auto length = const_int->get_value();
if (scope.in_global()) {
    // If it is in the global scope
    if (node.is_inited) {
        // If the array is initialized
        // Initialize the array
        auto array_type = ArrayType::get(cur_type, length);
        init_val.clear();
        node.initializers->accept(*this);
        // Create a global array
        int initialized_size = init_val.size();
        while (initialized_size < length) {
            // Append empty values to the array
            // Until the array is fully initialized
            if (cur_type == INT32_T) {
                init_val.push_back(CONST_INT(0));
            } else {
                init_val.push_back(CONST_FLOAT(0.));
            }
            initialized_size++;
        }
        auto initializer = ConstantArray::create(array_type, init_val, module);
        auto global_var = GlobalVariable::create(node.name, module, array_type, false, initializer);
        scope.push(node.name, global_var);
    } else {
        // If the array is not initialized
        // Initialize the array with 0
        auto array_type = ArrayType::get(cur_type, length);
        auto initializer = ConstantZero::create(array_type, module);
        auto global_var = GlobalVariable::create(node.name, module, array_type, false, initializer);
        scope.push(node.name, global_var);
    }
} else {
    // If it is not in the global scope
    // Create a local variable
    // The space of the local variable should be allocated in the entry block
    // If the entry block is terminated
    // The value should be inserted before the terminator
    auto entry_block_terminator = entry_block->get_terminator();
    if (entry_block_terminator) {
        // If the entry block is terminated
        // Remove the terminator first
        entry_block->get_instructions().pop_back();
    }
    // Allocate the space for the array
    auto array_type = ArrayType::get(cur_type, length);
    auto var = builder->create_alloca(array_type);
    // The create_alloca defaults to the current block
    // It should be moved to the entry block
    cur_block->get_instructions().pop_back();
    auto inst = var->as<Instruction>();
    entry_block->add_instruction(inst);
    inst->set_parent(entry_block);
    // Re-place the terminator if it is removed before
    if (entry_block_terminator) {
        entry_block->add_instruction(entry_block_terminator);
    }
    // Initialize the array
    if (node.is_inited) {
        init_val.clear();
        node.initializers->accept(*this);
        int id = 0;
        for (auto initializer : init_val) {
            auto ptr = builder->create_gep(var, {CONST_INT(0), CONST_INT(id)});
            builder->create_store(initializer, ptr);
            id++;
        }
        // Fill the remaining elements with 0
        while (id < length) {
            auto ptr = builder->create_gep(var, {CONST_INT(0), CONST_INT(id)});
            if (cur_type == INT32_T) {
                builder->create_store(CONST_INT(0), ptr);
            } else {
                builder->create_store(CONST_FLOAT(0.0), ptr);
            }
            id++;
        }
    }
    scope.push(node.name, var);
}
```

下面是 `LVal` 部分，这部分用于处理左值表达式。同理，还是分成表达式是变量、指针还是数组，以及是否是常量四种情况。

如果是指针，则还要分为是指向指针的指针、指向数组的指针还是指向变量的指针，如果是指向指针的指针，则需要先载入这个指针指向的值，如果是指向数组的指针，则需要先载入这个数组的起始地址，否则直接载入变量的地址。

如果是变量，分为常变量以及普通变量两种情况，如果是常变量，则直接获取该常变量的值并写入程序中，否则，则需要加载这个变量的值。

如果是数组，首先需要考虑是否是常数组和常下标，如果是常数组和常下标，则对应的是一个常数，直接从常数组初始值列表中将这个常数加载出来并写入程序中。否则，只要二者有其一不是常数，则这个值就是一个可变的值，需要动态加载。加载的情况还需要分为两种，一是数组实际为一个指针，此时需要先得到指针中存储的地址，然后 `create_gep` 计算偏移量，参数为 `{idx}`，二是数组确实是一个真正的数组，此时需要 `create_gep` 访问数组元素地址，参数为 `{0, idx}`。以及还需要判断当前加载的值是否还需要再外面被修改，如果仅仅是取一个值，则还需要使用 `create_load` 把值取出来，否则就直接把地址返回即可。

``` cpp
void IRBuilder::visit(SyntaxTree::LVal &node) {
    // get variable
    auto lval = scope.find(node.name, false)->as<Value>();
    bool ret_is_addr = need_addr;
    need_addr = false;
    if (node.array_index.empty()) {
        // ident
        // auto var = scope.find(node.name, false)->as<Value>();
        if (ret_is_addr) {
            // this->visitee_val needs to be set as an address
            auto lval_type = lval->get_type()->get_pointer_element_type();
            if (lval_type->is_pointer_type()) {
                // lval is a pointer to pointer
                this->visitee_val = builder->create_load(lval);
            } else if (lval_type->is_array_type()) {
                // lval is a pointer to an array
                this->visitee_val = builder->create_gep(lval, {CONST_INT(0), CONST_INT(0)});
            } else {
                // lval is a pointer to a simple variable
                this->visitee_val = lval;
            }
        } else {
            // we need to load the value
            // if the lval is constant, we can load the value directly
            auto const_int = lval->as<ConstantInt>();
            auto const_float = lval->as<ConstantFloat>();
            if (const_int) {
                this->visitee_val = const_int;
            } else if (const_float) {
                this->visitee_val = const_float;
            } else {
                this->visitee_val = builder->create_load(lval);
            }
            // this->visitee_val = builder->create_load(lval);
        }
    } else {
        // ident[exp]
        // get array index, array has been limited to one dimension in this experiment
        node.array_index[0]->accept(*this);
        auto idx = this->visitee_val;
        auto const_idx = std::dynamic_pointer_cast<ConstantInt>(idx);
        // Is const index?
        if (const_idx) {
            // Is global array?
            auto global_cast = std::dynamic_pointer_cast<GlobalVariable>(lval);
            if (global_cast) {
                // If the array is const and the index is const
                // Then the value is const and can be directly loaded
                if (global_cast->is_const()) {
                    auto init_arr = std::dynamic_pointer_cast<ConstantArray>(global_cast->get_init());
                    auto idx_val = const_idx->get_value();
                    this->visitee_val = init_arr->get_element_value(idx_val);
                    return;
                }
                // Otherwise the index is const but the array is not const
                // The value cannot be loaded directly
            }
            // Is local array?
            // If is local const array, it can be converted to global const array
            auto local_cast = const_local_values[lval];
            if (local_cast) {
                global_cast = local_cast->as<GlobalVariable>();
                if (global_cast) {
                    // If the array is const and the index is const
                    // Then the value is const and can be directly loaded
                    if (global_cast->is_const()) {
                        auto init_arr = global_cast->get_init()->as<ConstantArray>();
                        auto idx_val = const_idx->get_value();
                        this->visitee_val = init_arr->get_element_value(idx_val);
                        return;
                    }
                    // Otherwise the index is const but the array is not const
                    // The value cannot be loaded directly
                }
            }
        }
        // If is not const index, the value is not const
        Ptr<Value> loaded_value;
        auto lval_pointer_type = lval->get_type()->get_pointer_element_type();
        if (lval_pointer_type->is_pointer_type()) {
            // The array is actually a pointer aka. ptr[exp]
            // Create gep by calculating pointer offset
            auto ptr_load = builder->create_load(lval);
            loaded_value = builder->create_gep(ptr_load, {idx});
        } else {
            // Otherwise the array is a real array aka. arr[exp]
            // Create gep using array method
            loaded_value = builder->create_gep(lval, {CONST_INT(0), idx});
        }
        if (ret_is_addr) {
            // If the lvalue should be used as an addr
            // which will be modified in outer scope
            this->visitee_val = loaded_value;
            // need_addr = false;
        } else {
            // If the lvalue is only used as a value
            this->visitee_val = builder->create_load(loaded_value);
        }
    }
}
```

### 函数相关

因为对`func`的处理分散几个函数中。函数的处理主要在`FuncDef`的`visit`函数中进行，但是在这里面又会调用`FuncParams`，`FuncParam`，`BlockStmt`，`ReturnStmt`等的`visit`函数。所以为了方便需要使用的各种参数的传递，设计了一些全局变量如下：

```c++
// set relative variables of function analyzed currently a global variable for the convenience of access in different functions
std::vector<SyntaxTree::FuncParam> params; // parameters of current function
std::vector<Ptr<Type>> params_types; // types of parameters of current function
Ptr<Value> ret_addr; // return address of current function
Ptr<BasicBlock> retBB; // return basic block of current function
bool has_entered_scope = false; // whether a new function is analyzed currently
```

下面通过说明这些全局变量的用法来说明对于函数部分的处理。

`params`和`params_types`分别表示函数的参数列表和参数类型列表。在`FuncParams`的`visit`函数中，对于参数列表中的每个参数调用`accept`方法，进入`FuncParam`的`visit`函数中。在这里，我们将参数`push`到`params`中，并且根据参数是否是数组指针来`push`相应的类型到`params_types`中。代码如下：

```c++
void IRBuilder::visit(SyntaxTree::FuncFParamList &node) {
    for (auto &param : node.params) {
        param->accept(*this);
    }
}

void IRBuilder::visit(SyntaxTree::FuncParam &node) {
    params.push_back(node);
    if (node.array_index.empty()) {
        if (node.param_type == SyntaxTree::Type::INT) {
            params_types.push_back(INT32_T);
        } else if (node.param_type == SyntaxTree::Type::BOOL) {
            params_types.push_back(INT32_T);
        } else if (node.param_type == SyntaxTree::Type::FLOAT) {
            params_types.push_back(FLOAT_T);
        }
    } else {
        if (node.param_type == SyntaxTree::Type::INT) {
            params_types.push_back(INT32PTR_T);
        } else if (node.param_type == SyntaxTree::Type::BOOL) {
            params_types.push_back(INT32PTR_T);
        } else if (node.param_type == SyntaxTree::Type::FLOAT) {
            params_types.push_back(FLOATPTR_T);
        }
    }
}
```

这里为了便于解决进行分配空间和类型转换等问题，对于`BOOL`类型的参数，按`int32`处理。

`ret_addr`和`retBB`分别表示函数的返回值的地址和返回语句的基本块，这样我们就可以在`ReturnStmt`的`visit`函数中通过全局变量得到`load`返回值并且创建跳转语句跳转到`retBB`（对于`void`类型的函数，会在这里直接创建空返回语句）。

`has_entered_scope`是为了处理作用域问题而设计的全局变量，其原因是一般来说在`BlockStmt`的`visit`函数的开始和结束时会分别进入和退出作用域，但是对于函数的`body`，因为我们需要在`FuncDef`的`visit`函数中先进入作用域，再处理`body`，所以这时不能在`BlockStmt`中再次进入作用域，所以这时我们就设计了`has_entered_scope`，在`FuncDef`中调用`body`的`accept`方法前设置其为`true`，以表示已经进入了作用域，不需要再次进入。这一部分的函数如下：

```cpp
void IRBuilder::visit(SyntaxTree::ReturnStmt &node) {
    if (node.ret == nullptr) {
        builder->create_void_ret();
        return;
    }
    node.ret->accept(*this);
    auto cur_ret_type = this->cur_func->get_return_type();
    auto ret_val_type = this->visitee_val->get_type();
    if (cur_ret_type->is_integer_type() && ret_val_type->is_float_type()) {
        this->visitee_val = builder->create_fptosi(this->visitee_val, INT32_T);
    } else if (cur_ret_type->is_float_type() && ret_val_type->is_integer_type()) {
        if (ret_val_type == INT1_T) {
            this->visitee_val = builder->create_zext(this->visitee_val, INT32_T);
        }
        this->visitee_val = builder->create_sitofp(this->visitee_val, FLOAT_T);
    }
    builder->create_store(this->visitee_val, ret_addr);
    builder->create_br(retBB);
}
```

这里为了方便，也将类型为`BOOL`的返回值设为`int32`类型。

在`FuncDef`的`visit`函数中，基本处理思路如下：

确定返回值类型 $\rightarrow$ 得到参数列表和参数类型列表 $\rightarrow$ 根据返回类型和参数类型创建函数并设置函数入口基本块 $\rightarrow$ 为参数分配空间 $\rightarrow$ 访问函数体 $\rightarrow$ 设置函数返回基本块

其中一些细节处理这里不再列出。

还有一个与函数相关的`FuncCall`的`visit`函数没有说明，这个函数用于实现函数调用。这里的难点在于一维数组指针作为参数时的处理。在传递数组参数时，我们实际传递的数组的第一个元素的地址。这里我们设计了一个全局变量`need_addr`表示，在参数是`int`或`float`时`need_addr = false`，表示此时直接传递值即可，否则需要将`need_addr = true`，表示这时传递的是参数（数组）的地址（参数的类型在全局变量`params_type`中）。在`LVal`的`visit`函数中也会根据`need_addr`的真假来判断是否应该返回变量的地址还是变量的值，这样我们就可以正确传递一维数组指针参数。在赋值语句`Assignstmt`的处理中也有使用`need_addr`，因为赋值时我们需要得到左侧目标的地址，所以访问`node.target`前将`need_addr`设为`true`。`need_addr`的相关设置及使用如下：

```cpp
void IRBuilder::visit(SyntaxTree::LVal &node) {
    // get variable
    auto lval = scope.find(node.name, false)->as<Value>();
    bool ret_is_addr = need_addr;
    need_addr = false;
    if (node.array_index.empty()) {
        // ident
        // auto var = scope.find(node.name, false)->as<Value>();
        if (ret_is_addr) {
            // this->visitee_val needs to be set as an address
            auto lval_type = lval->get_type()->get_pointer_element_type();
            if (lval_type->is_pointer_type()) {
                // lval is a pointer to pointer
                this->visitee_val = builder->create_load(lval);
            } else if (lval_type->is_array_type()) {
                // lval is a pointer to an array
                this->visitee_val = builder->create_gep(lval, {CONST_INT(0), CONST_INT(0)});
            } else {
                // lavl is a pointer to a simple variable
                this->visitee_val = lval;
            }
        } else {
            // we need to load the value
            // if the lval is constant, we can load the value directly
            auto const_int = lval->as<ConstantInt>();
            auto const_float = lval->as<ConstantFloat>();
            if (const_int) {
                this->visitee_val = const_int;
            } else if (const_float) {
                this->visitee_val = const_float;
            } else {
                this->visitee_val = builder->create_load(lval);
            }
            // this->visitee_val = builder->create_load(lval);
        }
    } else {
        // ident[exp]
        // get array index, array has been limited to one dimension in this experiment
        node.array_index[0]->accept(*this);
        auto idx = this->visitee_val;
        auto const_idx = std::dynamic_pointer_cast<ConstantInt>(idx);
        // Is const index?
        if (const_idx) {
            // ...此处省略代码
        }
        // If is not const index, the value is not const
        // ...
        if (ret_is_addr) {
            // If the lvalue should be used as an addr
            // which will be modified in outer scope
            this->visitee_val = loaded_value;
            // need_addr = false;
        } else {
            // If the lvalue is only used as a value
            this->visitee_val = builder->create_load(loaded_value);
        }
    }
}

void IRBuilder::visit(SyntaxTree::AssignStmt &node) {
    // ...
    need_addr = true; // need variable address to store value
    node.target->accept(*this);
    auto var_addr = this->visitee_val;
    // ...
}

void IRBuilder::visit(SyntaxTree::FuncCallStmt &node) {
  	// ...
    for (std::vector<SyntaxTree::FuncParam>::size_type i = 0; i < node.params.size(); i++) {
        need_addr = false; // reset need_addr
        auto param = node.params[i];
        auto param_type = func->get_function_type()->get_param_type(i);
        // if param is an int or float, we can pass the value directly, so we set need_addr to false
        // but if param is an array, we need to pass the address, then we should set need_addr to true
        need_addr = (param_type->is_integer_type() || param_type->is_float_type()) ? false : true;
        param->accept(*this);
        // ...
    }
    // ...
}
```

### 控制流语句相关

控制流语句的处理难度主要在短路计算和基本块的设置上。为了方便，这里也设置了几个全局变量如下：

```cpp
typedef struct CondBBs {
    Ptr<BasicBlock> trueBB;
    Ptr<BasicBlock> falseBB;
} condBBs;
std::vector<condBBs> totalCondBBs;
std::vector<condBBs> andCondBBs;
std::vector<condBBs> orCondBBs;
std::vector<condBBs> whileBBs; // store condBB and falseBB of while statement
```

其中`totalBBs`的成员表示整个条件表达式的`trueBB`和`falseBB`，不考虑短路计算。而`andCondBBs`和`orCondBBs`的成员则表示考虑短路计算条件表达式的`trueBB`和`falseBB`。对于`and`条件表达式`andCond`，我们将其两侧的条件表达式也视为独立的条件表达式`lhs`和`rhs`。若`lhs`为真，则继续判断`rhs`；若`lhs`为假，则直接跳转到`andCond`的`falseBB`，所以`lhs`的`trueBB`为`rhs`的起始位置，`lhs`的`falseBB`与`andCond`的`falseBB`相同。`rhs`的`trueBB`和`falseBB`则均与`andCond`相同。同理可得`or`条件表达式`orCond`的`lhs`的`trueBB`为`orCond`的`trueBB`，`falseBB`为`orCond`的`falseBB`，`rhs`的`trueBB`和`falseBB`则均与`orCond`相同。

`whileBBs`的成员表示的`while`语句的条件表达式的`trueBB`和`falseBB`。

下面重点讨论`LAND`和`LOR`的实现，其他条件算符的实现只需要插入比较语句即可。

对于`LAND`，我们首先需要根据上面的原则设置它的`lhs`的`trueBB`和`FalseBB`，然后调用`lhs`的`accept`方法，然后创建比较语句和条件跳转语句，然后在插入`trueBB`并调用`rhs`的`accept`方法，这就实现了`lhs`的`trueBB`是`rhs`的起始位置。而`falseBB`的插入则需要`IfStmt`或`WhileStmt`中相应的条件语句访问完整体后面插入。

对于`LOR`类似。

需要注意的是，在访问完`lhs`后，可能他本身就是一个条件语句，这时已经插入了比较语句，我们就不需要再插入比较语句。为此我们设计下面的函数来实现插入比较语句和跳转语句：

```cpp
void createCondBr(Ptr<IRStmtBuilder> builder, Ptr<Value> val, Ptr<BasicBlock> trueBB, Ptr<BasicBlock> falseBB, Ptr<Module> module) {
    // val may be also a compare instruction. In this case we don't need to create a compare inst
    auto val_type = val->get_type();
	// try to convert val to Ptr<CmpInst>
    auto cmp = val->as<CmpInst>();
    auto fcmp = val->as<FCmpInst>();
    if (val_type->is_integer_type() || cmp) {
        if (cmp == nullptr) {
            // if conversion fails, we should construct CmpInst manually
            cmp = builder->create_icmp_ne(val, CONST_INT(0));
        }
        builder->create_cond_br(cmp, trueBB, falseBB);
    } else if (val_type->is_float_type() || fcmp) {
        if (fcmp == nullptr) {
            fcmp = builder->create_fcmp_ne(val, CONST_FLOAT(0));
        }
        builder->create_cond_br(fcmp, trueBB, falseBB);
    }
}
```

在实现了条件表达式的处理后，对于`IfStmt`和`WhileStmt`，则只需要依次处理每一部分（`cond`，`if_statement`，`else_statement`，`statement`等）即可。

## 实验难点及解决方案

1. 函数相关的函数处理中参数的传递问题。在处理函数相关的`visit`函数时，因为函数较为分散，所以如何传递参数是一个难点。

   我们的解决方案便是通过实验设计**函数相关**部分中所提到的几个全局变量来避免参数的传递。

2. 一维数组指针参数的处理问题。因为在传递数组时我们实际传递的是数组第一个元素的地址，也就是相当于传递了一个指针，在被调用函数中需要根据地址取得值，所以不能简单的在调用方`store`所有参数，然后在被调用方`load`参数。

   我们的解决方案是设计了**变量相关**部分中提到的`need_addr`全局变量来表示是否需要地址，如果参数是数组指针，`need_addr`就为`true`，经过参数的`accept`方法后，得到要传递的地址。在`Assignment`的`visit`函数中也通过将`need_addr`设为`true`，然后调用`target`的`accept`方法得到其地址以进行赋值。

3. 短路计算的处理问题。短路计算中条件表达式的`trueBB`和`falseBB`的设置较为复杂。

   我们的解决方案是实验设计**控制流语句相关**部分中提到的几个全局变量。在控制流语句的处理中向`totalCondBBs`中`push`整个条件表达式的`condBBs`，然后在条件表达式的处理中根据条件算符设计`LAND`和`LOR`两侧的条件表达式的`trueBB`和`falseBB`。在设计了这样的数据结构后，短路计算的实现就清晰了起来。

4. 短路计算的测试问题。如何测试是否正确实现了短路计算也是一个问题，因为我们不知道是在得到整体条件表达式的真假后及立即进行了跳转还是完成每一个条件表达式的计算再进行的跳转。

   一个解决方案是观察生成的中间文件的`trueBB`和`falseBB`的设置是否正确。

   还有一个解决方案是设置二元条件表达式的右侧表达式为非法操作，并且通过左侧表达式就可以判断整个条件表达式的真假。这样如果正确实现了短路计算，就不会执行右侧表达式，也就不会执行非法操作，不会报错。

## 实验总结

本次实验的难点在于从整体上把握整个实验框架，处理好不同函数间的对应关系。例如我们经常会在一个函数中调用节点的子结构的`accept`方法，这时我们就需要知道对子结构的访问做了什么，访问后中`this->visitee_val`变成了什么。

在理解了整个实验框架后，设计一些方便的全局变量，实验就可以较为顺利地执行。

但是因为多人协作过程中不同成员的理解可能不一致，实现也可能不一致，所以这也是一个问题。我们也只能在成员写完之后检查自己写的函数与其他涉及到这个函数的函数是否一致，以及测试后进行debug。

总体来说，这次实验在实现上较为复杂，多人协作在减轻了个人工作量的同时也带来了一定的挑战。

## 实验反馈



## 组间交流