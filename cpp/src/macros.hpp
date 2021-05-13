#ifndef RDSS_MACROS_H_
#define RDSS_MACROS_H_

#define CONCAT(a, b) CONCAT_INNER(a, b)
#define CONCAT_INNER(a, b) a ## b

#define ASSIGN_OR_RETURN_HELPER(lhs, rhs, temp) \
    auto temp = (rhs);                          \
    if(!temp.ok()) { return temp.status(); }    \
    lhs = *temp;

#define ASSIGN_OR_RETURN(lhs, rhs)                                      \
    ASSIGN_OR_RETURN_HELPER(lhs, rhs, CONCAT(temporary, __COUNTER__))

#define RETURN_IF_ERROR_HELPER(value, temp)     \
    auto temp = (value);                        \
    if(!temp.ok()) { return temp; }

#define RETURN_IF_ERROR(value)                                      \
    RETURN_IF_ERROR_HELPER(value, CONCAT(temporary, __COUNTER__))

#endif  // RDSS_MACROS_H_
