#ifndef _DNS_EXCEPTION_H
#define	_DNS_EXCEPTION_H

#include <exception>
#include <string>

namespace dns {

/**
 *  Exception class extends standard exception funtionality and adds it the text
 *  message to inform about the reason of the exception thrown.
 */
class Exception : public std::exception {
public:
    // Constructor
    // @param text Information text to be filled with the reasons of the exception
    Exception(const std::string& text) : m_text(text) { }
    Exception(const char *text) : m_text(text) { }
    virtual ~Exception() throw() { }

    // Returns the information text string
    virtual const char* what() const throw()
    {
        return m_text.data();
    }

private:
    std::string m_text;
};
}
#endif	/* _DNS_EXCEPTION_H */

