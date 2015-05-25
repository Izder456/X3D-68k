// Header File
// Created 3/9/2015; 11:22:22 AM

//#define NDEBUG

#ifdef NDEBUG
#define x3d_error(...) ;
#define x3d_assert(...) ;
#define x3d_errorif(...) ;

#else

void x3d_error(const char* format, ...) __attribute__((noreturn));

#define x3d_errorif(_cond, _format, ...) {if(_cond) x3d_error(_format, ##__VA_ARGS__);}
#define x3d_assert(_cond) x3d_errorif(!(_cond), "Assertion failed!\nFile: %s\nLine: %d\nFunction: %s\nCond: %s\n",__FILE__,__LINE__,__FUNCTION__,#_cond)

#endif