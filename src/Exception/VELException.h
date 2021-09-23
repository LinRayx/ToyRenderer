#include <exception>
#include "Utils/VELCommand.h"

// RELEASE_EXCEPTIONS mode
#if VEL_ASSERT_MODE == 1
#   if VEL_DEBUG_MODE
#       define VELAssert( a, b ) assert( (a) && (b) )
#   else
#       define VELAssert( a, b ) if( !(a) ) VEL_EXCEPT_2( VEL::Exception::ERR_RT_ASSERTION_FAILED, (#a " failed. " b) )
#   endif

// EXCEPTIONS mode
#elif VEL_ASSERT_MODE == 2
#   define VELAssert( a, b ) if( !(a) ) VEL_EXCEPT_2( VEL::Exception::ERR_RT_ASSERTION_FAILED, (#a " failed. " b) )
// STANDARD mode
#else
/** Checks a condition at runtime and throws exception/ aborts if it fails.
 *
 * The macros VELAssert (and VELAssertDbg) evaluate the specified expression.
 * If it is 0, VELAssert raises an error (see VEL::RuntimeAssertionException) in Release configuration
 * and aborts in Debug configuration.
 * The macro VELAssert checks the condition in both Debug and Release configurations
 * while VELAssertDbg is only retained in the Debug configuration.
 *
 * To always abort instead of throwing an exception or disabling VELAssert in Release configuration altogether,
 * set VEL_ASSERT_MODE in CMake accordingly.
 */
#   define VELAssert( expr, mesg ) assert( (expr) && (mesg) )
#endif

namespace VEL
{
    class Exception : public std::exception
    {
    public:
        enum ExceptionCodes {
            ERR_CANNOT_WRITE_TO_FILE,
            ERR_INVALID_STATE,
            ERR_INVALIDPARAMS,
            ERR_RENDERINGAPI_ERROR,
            ERR_DUPLICATE_ITEM,
            ERR_ITEM_NOT_FOUND = ERR_DUPLICATE_ITEM,
            ERR_FILE_NOT_FOUND,
            ERR_INTERNAL_ERROR,
            ERR_RT_ASSERTION_FAILED,
            ERR_NOT_IMPLEMENTED,
            ERR_INVALID_CALL
        };
        /** Default constructor.
        */
        Exception(int number, const String& description, const String& source);

        /** Advanced constructor.
        */
        Exception(int number, const String& description, const String& source, const char* type, const char* file, long line);
    };

    /** Template struct which creates a distinct type for each exception code.
@note
This is useful because it allows us to create an overloaded method
for returning different exception types by value without ambiguity.
From 'Modern C++ Design' (Alexandrescu 2001).
*/
    class UnimplementedException : public Exception
    {
    public:
        UnimplementedException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
            : Exception(inNumber, inDescription, inSource, __FUNCTION__, inFile, inLine) {}
    };
    class FileNotFoundException : public Exception
    {
    public:
        FileNotFoundException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
            : Exception(inNumber, inDescription, inSource, __FUNCTION__, inFile, inLine) {}
    };
    class IOException : public Exception
    {
    public:
        IOException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
            : Exception(inNumber, inDescription, inSource, __FUNCTION__, inFile, inLine) {}
    };
    class InvalidStateException : public Exception
    {
    public:
        InvalidStateException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
            : Exception(inNumber, inDescription, inSource, __FUNCTION__, inFile, inLine) {}
    };
    class InvalidParametersException : public Exception
    {
    public:
        InvalidParametersException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
            : Exception(inNumber, inDescription, inSource, __FUNCTION__, inFile, inLine) {}
    };
    class ItemIdentityException : public Exception
    {
    public:
        ItemIdentityException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
            : Exception(inNumber, inDescription, inSource, __FUNCTION__, inFile, inLine) {}
    };
    class InternalErrorException : public Exception
    {
    public:
        InternalErrorException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
            : Exception(inNumber, inDescription, inSource, __FUNCTION__, inFile, inLine) {}
    };
    class RenderingAPIException : public Exception
    {
    public:
        RenderingAPIException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
            : Exception(inNumber, inDescription, inSource, __FUNCTION__, inFile, inLine) {}
    };
    class RuntimeAssertionException : public Exception
    {
    public:
        RuntimeAssertionException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
            : Exception(inNumber, inDescription, inSource, __FUNCTION__, inFile, inLine) {}
    };
    class InvalidCallException : public Exception
    {
    public:
        InvalidCallException(int inNumber, const String& inDescription, const String& inSource, const char* inFile, long inLine)
            : Exception(inNumber, inDescription, inSource, __FUNCTION__, inFile, inLine) {}
    };

    class ExceptionFactory
    {
    private:
        /// Private constructor, no construction
        ExceptionFactory() {}
        static VEL_NORETURN void _throwException(
            Exception::ExceptionCodes code, int number,
            const String& desc,
            const String& src, const char* file, long line)
        {
            switch (code)
            {
            case Exception::ERR_CANNOT_WRITE_TO_FILE:   throw IOException(number, desc, src, file, line);
            case Exception::ERR_INVALID_STATE:          throw InvalidStateException(number, desc, src, file, line);
            case Exception::ERR_INVALIDPARAMS:          throw InvalidParametersException(number, desc, src, file, line);
            case Exception::ERR_RENDERINGAPI_ERROR:     throw RenderingAPIException(number, desc, src, file, line);
            case Exception::ERR_DUPLICATE_ITEM:         throw ItemIdentityException(number, desc, src, file, line);
            case Exception::ERR_FILE_NOT_FOUND:         throw FileNotFoundException(number, desc, src, file, line);
            case Exception::ERR_INTERNAL_ERROR:         throw InternalErrorException(number, desc, src, file, line);
            case Exception::ERR_RT_ASSERTION_FAILED:    throw RuntimeAssertionException(number, desc, src, file, line);
            case Exception::ERR_NOT_IMPLEMENTED:        throw UnimplementedException(number, desc, src, file, line);
            case Exception::ERR_INVALID_CALL:           throw InvalidCallException(number, desc, src, file, line);
            default:                                    throw Exception(number, desc, src, "Exception", file, line);
            }
        }
    public:
        static VEL_NORETURN void throwException(
            Exception::ExceptionCodes code,
            const String& desc,
            const String& src, const char* file, long line)
        {
            _throwException(code, code, desc, src, file, line);
        }
    };
#ifndef VEL_EXCEPT
#define VEL_EXCEPT_3(code, desc, src)  VEL::ExceptionFactory::throwException(code, desc, src, __FILE__, __LINE__)
#define VEL_EXCEPT_2(code, desc)       VEL::ExceptionFactory::throwException(code, desc, __FUNCTION__, __FILE__, __LINE__)
#define VEL_EXCEPT_CHOOSER(arg1, arg2, arg3, arg4, ...) arg4
#define VEL_EXPAND(x) x // MSVC workaround
#define VEL_EXCEPT(...) VEL_EXPAND(VEL_EXCEPT_CHOOSER(__VA_ARGS__, VEL_EXCEPT_3, VEL_EXCEPT_2)(__VA_ARGS__))
#endif
}