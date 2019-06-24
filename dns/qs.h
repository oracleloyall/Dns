#ifndef _DNS_QS_H
#define	_DNS_QS_H

#include <string>
#include <vector>

#include "dns.h"
#include "buffer.h"

namespace dns {

/* Class represents a DNS Question Section Entry
 * 
 * The DNS Question section entry has the following format:
 * 
 *                                     1  1  1  1  1  1
 *       0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
 *     +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *     |                                               |
 *     /                     QNAME                     /
 *     /                                               /
 *     +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *     |                     QTYPE                     |
 *     +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *     |                     QCLASS                    |
 *     +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * 
 * where:
 * 
 * QNAME           a domain name represented as a sequence of labels, where
 *                 each label consists of a length octet followed by that
 *                 number of octets.  The domain name terminates with the
 *                 zero length octet for the null label of the root.  Note
 *                 that this field may be an odd number of octets; no
 *                 padding is used.
 * 
 * QTYPE           a two octet code which specifies the type of the query.
 *                 The values for this field include all codes valid for a
 *                 TYPE field, together with some more general codes which
 *                 can match more than one type of RR.
 * 
 * QCLASS          a two octet code that specifies the class of the query.
 *                 For example, the QCLASS field is IN for the Internet.
 */
class QuerySection
{
public:

    /* Constructor */
    QuerySection(const std::string& qName = "") : mQName(qName), mQType(0), mQClass(QCLASS_IN) { };

    /* Set type of the query */
    void setType(uint qType) { mQType = qType; };

    /* Set type class of the query */
    void setClass(eQClass qClass) { mQClass = qClass; };

    /* Set name field from a string */
    void setName(const std::string& qName) { mQName = qName; } ;

    /* Get name filed of the query */
    std::string getName() const;

    /* Get the type of the query */
    uint getType() const;

    /* Get the class of the query */
    eQClass getClass() const;

    void encode(Buffer &buffer);

    std::string asString();

private:

    // Name of the query
    std::string mQName;

    // Type field
    uint mQType;

    // Class of the query
    eQClass mQClass;
};

} // namespace
#endif	/* _DNS_QS_H */

