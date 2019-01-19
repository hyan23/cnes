// error.h
// Author: hyan23
// Date: 2017.08.02

#ifndef __ERROR_H__
#define __ERROR_H__

#define NOTNULL(expr)       ((expr) != NULL)
#define NUL(expr)           ((expr) == NULL)

#define SUCCEED(expr)       ((expr) == SUCCESS)
#define FAILED(expr)        (!SUCCEED(expr))

typedef enum ERROR {
    SUCCESS,    FAILURE, 
    ARGUMENTS, 
    EXISTS, NOTEXIST, 
    MEMORY_OVERFLOW, 
    BUFFER_FULL, 
    OPEN_FILE_ERROR,    READ_FILE_ERROR,    INVALID_FILE_ERROR, 
    BAD_OPCODE, ACCESS_VIOLATION, 
    SDL_ERROR
}
error_t;

#endif /* __ERROR_H__ */