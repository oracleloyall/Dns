#ifndef _DNS_BUFFER_H
#define	_DNS_BUFFER_H

#include <string>
#include <vector>

#include "dns.h"

namespace dns
{
/**
 * Buffer for DNS protocol parsing and serialization
 *
 * <domain-name> is a domain name represented as a series of labels, and
 * terminated by a label with zero length.
 *
 * <character-string> is a single length octet followed by that number
 * of characters.  <character-string> is treated as binary information,
 * and can be up to 256 characters in length (including the length octet).
 *
 */
class Buffer
{
    public:
        Buffer(char* buffer, uint bufferSize) : mBuffer(buffer), mBufferSize(bufferSize), mBufferPtr(buffer) { }

        // get current position in buffer
        uint getPos() { return mBufferPtr - mBuffer; }

        // set current position in buffer
        void setPos(const uint pos);

        // get buffer size in bytes
        uint getSize() { return mBufferSize; }

        // Helper function that get 8  bits from the buffer and keeps it an int.
        uchar get8bits();
        void put8bits(const uchar value);

        // Helper function that get 16 bits from the buffer and keeps it an int.
        uint get16bits();
        void put16bits(const uint value);

        // Helper function that get 32 bits from the buffer and keeps it an int.
        uint get32bits();
        void put32bits(const uint value);

        // Helper function that gets number of bytes from the buffer
        char* getBytes(uint count);
        void putBytes(const char* data, uint count);

        // Helper function that gets <character-string> (according to RFC 1035) from buffer
        std::string getDnsCharacterString();
        void putDnsCharacterString(const std::string& value);

        // Helper function that gets <domain> (according to RFC 1035) from buffer
        std::string getDnsDomainName(const bool compressionAllowed = true);

        // Helper function that puts <domain> (according to RFC 1035) to buffer
        void putDnsDomainName(const std::string& value, const bool compressionAllowed = true);

        // Check if there is enough space in buffer  
        void checkAvailableSpace(const uint additionalSpace);

        // Function that dumps the whole buffer
        void dump(const uint count = 0);

    private: 
        // buffer content
        char* mBuffer;
        // buffer content size
        const uint mBufferSize;
        // current position in buffer
        char* mBufferPtr;
        // list of link positions visited when decoding domain name
        std::vector<uint> mLinkPos;
};

} // namespace
#endif	/* _DNS_BUFFER_H */

