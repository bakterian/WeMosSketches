
#ifndef LMIC_APP_UTILS_H
#define LMIC_APP_UTILS_H

#include <cstdint>
#include <array>
#include <algorithm>

    
namespace lmicAppUtils
{

/* 
 *   TypeAsArray
 *   Class to be used when packing a type value in to an array of bytes 
 */
template<typename T, size_t sizeofT = sizeof(T)>
class TypeAsArray
{
    public:
    TypeAsArray(T value)
    {
        initArray(value);
    }

    /*
    *   Returns the value type that was stored and presenting it in big endian
    */
    std::array<uint8_t, sizeofT> getBigEndianArray() const
    {
        std::array<uint8_t, sizeofT> resArray = uint8Array;
        
        if(isLittleEndianArch())
        {
            std::reverse(std::begin(resArray), std::end(resArray));
        }
        
        return resArray;
    }
    
    /*
    *   Returns the value type that was stored and presenting it in little endian
    */
    std::array<uint8_t, sizeofT> getLittleEndianArray() const
    {
        std::array<uint8_t, sizeofT> resArray = uint8Array;
        
        if(!isLittleEndianArch())
        {
            std::reverse(std::begin(resArray), std::end(resArray));
        }
        
        return resArray;
    }

    /*
    *   Returns the byte array converted to the LittlEndian uin32_t format 
    */
    uint32_t getTypeAsBigEndianUint32() const
    {
        std::array<uint8_t, sizeofT> resArray = getBigEndianArray();
        return (uint32_t)(*((uint32_t*)resArray.data()));
    }
    
    /*
    *   Returns the byte array converted to the BigEndian uin32_t format 
    */
    uint32_t getTypeAsLittleEndianUint32() const
    {
        std::array<uint8_t, sizeofT> resArray = getLittleEndianArray();
        return (uint32_t)(*((uint32_t*)resArray.data()));
    }
    
    /*
    *   Returns the byte array converted to the uin32_t format 
    *   (may not be a meaningful number - depends on the original type). 
    *   No endianess conversions done
    */
    uint32_t getTypeAsUint32() const
    {
        return (uint32_t)(*((uint32_t*)uint8Array.data()));
    }
    
    /*
    *   Returns the original type
    */
    T getOriginalType() const
    {
        return (T)(*((T*)(uint8Array.data())));
    }

    private:
    void initArray(T value)
    {
        const uint8_t* pValue = (const uint8_t *) &value;
        
        auto i = 0U;
    
        for(auto& arrElement : uint8Array)
        {
            arrElement = *(pValue + i);
            ++i;
        }
    }

    bool isLittleEndianArch() const
    {
        unsigned num = 1U; /* is this num stored as 01 00 00 00 (Little Endian) or 00 00 00 01 (Big Endian) */
        bool isLeastSigByteFirst = (*(char *)&num == 1);
        return isLeastSigByteFirst;
        /* since c++ 20 this staticexpres bool can be used (std::endian::native == std::endian::big) #include <bit> */
    }

    std::array<uint8_t, sizeofT> uint8Array{};
};

/* USAGE EXAMPLE
int main()
{
  TypeAsArray<float> arrayFloat(22.4f);
  std::cout << "Float value for conversion: " << arrayFloat.getOriginalType() << std::endl;
  std::cout << "Float value in big endian uint32_t: " << (unsigned)arrayFloat.getTypeAsBigEndianUint32() << std::endl;
  
  std::cout << "Float value as byte array:";
  std::string addStr = " ";
  
  for(uint8_t& floatByte : arrayFloat.getBigEndianArray())
  {
      std::cout << addStr;
      std::cout << "0x" << std::setfill('0') << std::setw(2) << std::hex << (unsigned)floatByte;
      addStr = ", ";
  }
  std::cout << std::endl << std::endl << std::endl;
  
  uint32_t valueToCheck = 98569U;
  TypeAsArray<uint32_t> uintArray(valueToCheck);
  std::cout << "Unsigend value for conversion: " << std::dec << (unsigned)(uintArray.getOriginalType()) << std::endl;
  std::cout << "Unsigend value as LittleEndian uint32_t: " << std::dec << (unsigned)(uintArray.getTypeAsLittleEndianUint32())<< std::endl;
  std::cout << "Unsigend value as byte array:";
  addStr = " ";
  
  for(uint8_t& uintByte : uintArray.getBigEndianArray())
  {
      std::cout << addStr;
      std::cout << "0x" << std::setfill('0') << std::setw(2) << std::hex << (unsigned)uintByte;
      addStr = ", ";
  }
  std::cout << std::endl;
}
*/

} /* namespace lmicAppUtils */
#endif /* LMIC_APP_UTILS_H */
