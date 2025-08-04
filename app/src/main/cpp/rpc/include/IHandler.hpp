#include "vector"

class IHandler {
public:
    virtual ~IHandler() = default;

    virtual std::vector<uint8_t> handle(const uint8_t* data, size_t size) = 0;
};