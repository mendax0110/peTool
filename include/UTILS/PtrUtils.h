#ifndef PTRUTILS_H
#define PTRUTILS_H

#include <exception>
#include <string>
#include <sstream>
#include <type_traits>
#include <stdexcept>
#include <iostream>
#include <memory>
#include <mutex>

// Delete a pointer and set it to nullptr, given ptr can be an array or a single pointer.
template <typename T>
static inline void SafeDelete(T*& ptr)
{
    if (ptr != nullptr)
    {
        delete ptr;
        ptr = nullptr;
    }
}

/**
 * Verify that a value matches an expected value, or throw an exception.
 * General template for non-pointer types.
 */
template <typename T>
static inline void Ex_Verify(const T& value, const T& expected)
{
    if (value != expected)
    {
        std::ostringstream oss;
        oss << "Verification failed: Value (" << value
            << ") is not equal to expected value (" << expected << ").";
        throw std::logic_error(oss.str());
    }
}

// Overload for comparing pointers with other pointers
template <typename T>
static inline void Ex_Verify(T* value, T* expected)
{
    if (value != expected)
    {
        std::ostringstream oss;
        oss << "Verification failed: Pointer (" << value
            << ") is not equal to expected pointer (" << expected << ").";
        throw std::logic_error(oss.str());
    }
}

// Overload for comparing pointers with nullptr
template <typename T>
static inline void Ex_Verify(T* value, std::nullptr_t expected)
{
    if (value != expected)
    {
        std::ostringstream oss;
        oss << "Verification failed: Pointer (" << value
            << ") is not nullptr as expected.";
        throw std::logic_error(oss.str());
    }
}

/**
 * Macro to delete a pointer and verify it's nullptr, for both single and array pointers.
 */
#define tryDeletePtr(ptr)       \
    SafeDelete(ptr);            \
    Ex_Verify(ptr, nullptr);

class PtrUtils
{
public:
    /**
     * Verify if given pointer is nullptr.
     *
     * \param ptr -> Pointer to verify.
     * \return -> True if ptr is nullptr, false otherwise.
     */
    template <typename T>
    static inline bool IsNullPtr(T* ptr)
    {
        Ex_Verify(ptr, nullptr);
        return ptr == nullptr;
    }

    /**
     * Verify if given pointer is not nullptr.
     *
     * \param ptr -> Pointer to verify.
     * \return -> True if ptr is not nullptr, false otherwise.
     */
    template <typename T>
    static inline bool IsValidPtr(T* ptr)
    {
        Ex_Verify(ptr, nullptr);
        return ptr != nullptr;
    }
};

// Safe Release for Smart Pointers
template <typename T>
static inline void SafeRelease(std::unique_ptr<T>& ptr)
{
    ptr.reset(); // Releases the resource and sets the pointer to nullptr.
}

template <typename T>
static inline void SafeRelease(std::shared_ptr<T>& ptr)
{
    ptr.reset(); // Decrements the reference count and releases the resource if it was the last owner.
}

// Shared Pointer Comparison Utilities
template <typename T>
static inline void Ex_Verify(const std::shared_ptr<T>& ptr, std::nullptr_t)
{
    if (ptr != nullptr)
    {
        std::ostringstream oss;
        oss << "Verification failed: Shared pointer is not nullptr.";
        throw std::logic_error(oss.str());
    }
}

template <typename T>
static inline void Ex_Verify(const std::shared_ptr<T>& ptr1, const std::shared_ptr<T>& ptr2)
{
    if (ptr1 != ptr2)
    {
        std::ostringstream oss;
        oss << "Verification failed: Shared pointers do not match.";
        throw std::logic_error(oss.str());
    }
}

// Memory Allocation Utility
template <typename T>
static inline T* AllocateAndVerify(std::size_t count)
{
    T* ptr = new (std::nothrow) T[count];
    if (ptr == nullptr)
    {
        throw std::bad_alloc();
    }
    return ptr;
}

// Pointer Array Management
template <typename T>
static inline void ClearArray(T* array, std::size_t size)
{
    for (std::size_t i = 0; i < size; ++i)
    {
        array[i] = T(); // Reset each element to default state.
    }
}

// Debugging Utilities
template <typename T>
static inline void PrintPtrInfo(T* ptr, const std::string& ptrName = "Pointer")
{
    if (ptr == nullptr)
    {
        std::cout << ptrName << " is nullptr." << std::endl;
    }
    else
    {
        std::cout << ptrName << " points to address: " << ptr << std::endl;
    }
}

// Scoped Pointer Manager
template <typename T>
class ScopedPointer
{
private:
    T* ptr;
public:
    explicit ScopedPointer(T* p = nullptr) : ptr(p) {}
    ~ScopedPointer() { SafeDelete(ptr); }

    /**
     * Get the pointer value.
     * 
     * \return 
     */
    T* get() const { return ptr; }
    
    /**
     * Release the pointer and return it.
     * 
     * \return 
     */
    T* release()
    {
        T* temp = ptr;
        ptr = nullptr;
        return temp;
    }

    /**
     * Reset the pointer to a new value.
     * 
     * \param p -> New pointer value.
     */
    void reset(T* p = nullptr)
    {
        SafeDelete(ptr);
        ptr = p;
    }

    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }
};

// Nullptr Assertions
#ifdef DEBUG
template <typename T>
static inline void AssertNullptr(T* ptr, const std::string& msg = "Pointer is not null!")
{
    if (ptr != nullptr)
    {
        throw std::logic_error(msg);
    }
}
#endif

// RAII Wrapper for Pointers
template <typename T>
class PointerWrapper
{
private:
    T* ptr;

public:
    explicit PointerWrapper(T* p = nullptr) : ptr(p) {}
    ~PointerWrapper() { SafeDelete(ptr); }

    /**
     * Get the pointer value.
     * 
     * \return 
     */
    T* get() const { return ptr; }

    /**
     * Release the pointer and return it.
     * 
     * \return
     */
    T* release()
    {
        T* temp = ptr;
        ptr = nullptr;
        return temp;
    }

    /**
     * Reset the pointer to a new value.
     * 
     * \param p -> New pointer value.
     */
    void reset(T* p = nullptr)
    {
        SafeDelete(ptr);
        ptr = p;
    }

    T& operator*() { return *ptr; }
    T* operator->() { return ptr; }
};

// Thread-Safe Pointer Operations
template <typename T>
class ThreadSafePointer
{
private:
    T* ptr;
    mutable std::mutex mtx;

public:
    explicit ThreadSafePointer(T* p = nullptr) : ptr(p) {}
    ~ThreadSafePointer()
    {
        std::lock_guard<std::mutex> lock(mtx);
        SafeDelete(ptr);
    }

    /**
     * thread-safe set operation for pointer.
     * 
     * \param p -> Pointer to set.
     */
    void set(T* p)
    {
        std::lock_guard<std::mutex> lock(mtx);
        SafeDelete(ptr);
        ptr = p;
    }

    /**
     * thread-safe get operation for pointer.
     * 
     * \return 
     */
    T* get() const
    {
        std::lock_guard<std::mutex> lock(mtx);
        return ptr;
    }
};

#endif // PTRUTILS_H