#ifndef FF2MODELS_UTILS_H
#define FF2MODELS_UTILS_H

template<typename T>
T* RelOffsetToPtr(const void* ptr, const int offset)
{
    return (T*)((int)ptr + (int)offset);
}
#endif //FF2MODELS_UTILS_H
