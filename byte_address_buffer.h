#include <span>

//=====================================================================================================================
class ByteAddressBuffer
{
public:

    explicit ByteAddressBuffer()
        :
        m_bytes(nullptr)
    {

    }

    virtual ~ByteAddressBuffer() {
        if (m_bytes) {
            free(m_bytes);
        }
    }
    
    void Allocate(uint32_t size)
    {
        m_bytes = static_cast<char*>(malloc(size));
    }

    template<typename T>
    T Load(uint32_t offset)
    {
        return *reinterpret_cast<T*>(m_bytes + offset);
    }

protected:

    char* m_bytes;
};

//=====================================================================================================================
class RWByteAddressBuffer : public ByteAddressBuffer
{
public:
    virtual ~RWByteAddressBuffer() {
    }

    template<typename T>
    void Store(uint32_t offset, const T& value)
    {
        *reinterpret_cast<T*>(m_bytes + offset) = value;
    }
};