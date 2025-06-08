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

/**
 * @brief Safely delete a pointer and set it to nullptr.
 * @tparam T Type of the pointer to delete.
 * @param ptr A reference to the pointer to delete.
 */
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
 * @brief Verify that a value is equal to an expected value.
 * @tparam T Type of the value to verify.
 * @param value A reference to the value to verify.
 * @param expected A reference to the expected value.
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

/**
 * @brief Verify that a pointer is equal to an expected pointer.
 * @tparam T Type of the pointer to verify.
 * @param value A pointer to the value to verify.
 * @param expected A pointer to the expected value.
 */
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

/**
 * @brief Verify that a pointer is nullptr.
 * @tparam T Type of the pointer to verify.
 * @param value A pointer to the value to verify.
 * @param expected A nullptr to compare against.
 */
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

/// @brief Utility class for pointer operations, including null checks and pointer verification \class PtrUtils
class PtrUtils
{
public:

    /**
     * @brief Verify if given pointer is nullptr.
     * @tparam T A type of the pointer to verify.
     * @param ptr The pointer to verify.
     * @return A boolean indicating whether the pointer is nullptr.
     */
    template <typename T>
    static inline bool IsNullPtr(T* ptr)
    {
        Ex_Verify(ptr, nullptr);
        return ptr == nullptr;
    }

    /**
     * @brief Verify if given pointer is valid (not nullptr).
     * @tparam T A type of the pointer to verify.
     * @param ptr The pointer to verify.
     * @return A boolean indicating whether the pointer is valid (not nullptr).
     */
    template <typename T>
    static inline bool IsValidPtr(T* ptr)
    {
        Ex_Verify(ptr, nullptr);
        return ptr != nullptr;
    }
};

/**
 * @brief Safely release a unique_ptr or shared_ptr.
 * @tparam T The type of the pointer to release.
 * @param ptr A reference to the pointer to release.
 */
template <typename T>
static inline void SafeRelease(std::unique_ptr<T>& ptr)
{
    ptr.reset(); // Releases the resource and sets the pointer to nullptr.
}


/**
 * @brief Safely release a shared_ptr.
 * @tparam T The type of the pointer to release.
 * @param ptr A reference to the shared_ptr to release.
 */
template <typename T>
static inline void SafeRelease(std::shared_ptr<T>& ptr)
{
    ptr.reset(); // Decrements the reference count and releases the resource if it was the last owner.
}

/**
 * @brief Verify shared pointers for equality or nullptr.
 * @tparam T The type of the shared pointer to verify.
 * @param ptr1 The first shared pointer to verify.
 * @param ptr2 The second shared pointer to verify.
 */
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

/**
 * @brief Verify two shared pointers for equality.
 * @tparam T The type of the shared pointers to verify.
 * @param ptr1 The first shared pointer to verify.
 * @param ptr2 The second shared pointer to verify.
 */
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

/**
 * @brief Allocate memory for an array of objects and verify the allocation.
 * @tparam T The type of the objects to allocate.
 * @param count The number of objects to allocate.
 * @return A pointer to the allocated memory.
 * @throws std::bad_alloc if the allocation fails.
 */
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

/**
 * @brief Clear an array of objects by resetting each element to its default state.
 * @tparam T The type of the objects in the array.
 * @param array A pointer to the array to clear.
 * @param size The number of elements in the array.
 */
template <typename T>
static inline void ClearArray(T* array, std::size_t size)
{
    for (std::size_t i = 0; i < size; ++i)
    {
        array[i] = T(); // Reset each element to default state.
    }
}

/**
 * @brief Print information about a pointer, including its address and whether it is nullptr.
 * @tparam T The type of the pointer to print.
 * @param ptr The pointer to print.
 * @param ptrName The name of the pointer for display purposes.
 */
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

/// @brief Scoped pointer class that automatically manages the lifetime of a pointer \class ScopedPointer
template <typename T>
class ScopedPointer
{
private:
    T* ptr;
public:

    /**
     * @brief Construct a ScopedPointer with an optional pointer.
     * @param p A pointer to initialize the ScopedPointer with. Defaults to nullptr.
     */
    explicit ScopedPointer(T* p = nullptr) : ptr(p) {}

    /**
     * @brief Destructor that safely deletes the pointer.
     * This ensures that the pointer is deleted when the ScopedPointer goes out of scope.
     */
    ~ScopedPointer() { SafeDelete(ptr); }

    /**
     * @brief Get the pointer value.
     * @return A pointer to the managed object.
     */
    T* get() const { return ptr; }
    
    /**
     * @brief Release the pointer and return it.
     * @return A pointer to the managed object, and sets the internal pointer to nullptr.
     */
    T* release()
    {
        T* temp = ptr;
        ptr = nullptr;
        return temp;
    }

    /**
     * @brief Reset the pointer to a new value.
     * @param p A pointer to set the ScopedPointer to. Defaults to nullptr.
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

/// @brief Pointer wrapper class that manages the lifetime of a pointer and provides utility functions \class PointerWrapper
template <typename T>
class PointerWrapper
{
private:
    T* ptr;

public:

    /**
     * @brief Construct a PointerWrapper with an optional pointer.
     * @param p A pointer to initialize the PointerWrapper with. Defaults to nullptr.
     */
    explicit PointerWrapper(T* p = nullptr) : ptr(p) {}

    /**
     * @brief Destructor that safely deletes the pointer.
     * This ensures that the pointer is deleted when the PointerWrapper goes out of scope.
     */
    ~PointerWrapper() { SafeDelete(ptr); }

    /**
     * @brief Get the pointer value.
     * @return A pointer to the managed object.
     */
    T* get() const { return ptr; }

    /**
     * @brief Release the pointer and return it.
     * @return A pointer to the managed object, and sets the internal pointer to nullptr.
     */
    T* release()
    {
        T* temp = ptr;
        ptr = nullptr;
        return temp;
    }

    /**
     * @brief Set the pointer to a new value.
     * @param p A pointer to set the PointerWrapper to.
     */
    void reset(T* p = nullptr)
    {
        SafeDelete(ptr);
        ptr = p;
    }

    T& operator*() { return *ptr; }
    T* operator->() { return ptr; }
};

/// @brief Thread-safe pointer wrapper class that manages the lifetime of a pointer and provides thread-safe operations \class ThreadSafePointer
template <typename T>
class ThreadSafePointer
{
private:
    T* ptr;
    mutable std::mutex mtx;

public:

    /**
     * @brief Construct a ThreadSafePointer with an optional pointer.
     * @param p A pointer to initialize the ThreadSafePointer with. Defaults to nullptr.
     */
    explicit ThreadSafePointer(T* p = nullptr) : ptr(p) {}

    /**
     * @brief Destructor that safely deletes the pointer.
     * This ensures that the pointer is deleted when the ThreadSafePointer goes out of scope.
     */
    ~ThreadSafePointer()
    {
        std::lock_guard<std::mutex> lock(mtx);
        SafeDelete(ptr);
    }

    /**
     * @brief Set the pointer to a new value in a thread-safe manner.
     * @param p A pointer to set the ThreadSafePointer to.
     */
    void set(T* p)
    {
        std::lock_guard<std::mutex> lock(mtx);
        SafeDelete(ptr);
        ptr = p;
    }

    /**
     * @brief Get the pointer value in a thread-safe manner.
     * @return A pointer to the managed object.
     */
    T* get() const
    {
        std::lock_guard<std::mutex> lock(mtx);
        return ptr;
    }
};

#endif // PTRUTILS_H