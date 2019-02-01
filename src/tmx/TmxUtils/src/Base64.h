//
//  base64 encoding and decoding with C++.
//  Version: 1.01.00
//
//  Modified to add namespaces and convert to class
//

#ifndef BASE64_H_C0CE2A47_D10E_42C9_A27C_C883944E704A
#define BASE64_H_C0CE2A47_D10E_42C9_A27C_C883944E704A

#include <string>

namespace tmx {
namespace utils {

class Base64
{
private:
	static const std::string base64_chars;
public:
	static inline bool IsBase64(unsigned char c) {
	  return (isalnum(c) || (c == '+') || (c == '/'));
	}
	static std::string Encode(unsigned char const* , unsigned int len);
	static std::string Decode(std::string const& s);
};

} /* namespace utils */
} /* namespace tmx */

#endif /* BASE64_H_C0CE2A47_D10E_42C9_A27C_C883944E704A */
