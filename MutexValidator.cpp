#include <iostream>
#include <mutex>

/**
    \brief A secure mutex class

    The class provides access to the mutex inside another object. After deleting another object, all data inside this class will be valid.

    To explain this class, consider an example. There are 2 objects of classes A and B. 
    An object of class B stores a pointer to an object of class A. The thread safety of class A is provided using std::mutex. 
    To access object A from B, a mutex is used inside A. When object A is deleted, the pointer to it inside B ceases to be valid.

    This class solves this problem. 
    Objects of this class are divided into 2 types: parent and non-parent. The parent is created in the main object, A if we talk about the example above, 
    and not the parent is not created in the main ones, i.e. B from the example above.
    Memory is allocated only in parent. Each copy of the object increases the counter by 1, 
    each destruction of the object decreases the counter by 1. Memory is freed when the counter becomes 0.
*/
class MutexValidator
{
private:
    
    // Mutex for thread-safety
    std::recursive_mutex* Mtx = nullptr;

    // Counter to store ref amount
    std::size_t* Counter = nullptr;

    // Bool variable to store info about main object validity
    bool* IsValid = nullptr;

    // Bool variable to mark original object
    bool IsOriginal;

public:

    /// \brief Default constructor
    MutexValidator()
    {
        Mtx = new std::recursive_mutex;
        Counter = new std::size_t(1);
        IsValid = new bool(true);

        IsOriginal = true;
    }

    /// \brief Copy constructor
    /// \param [in] other other MutexValidator object
    MutexValidator(const MutexValidator& other)
    {
        other.Mtx->lock();

        Mtx = other.Mtx;
        Counter = other.Counter;
        IsValid = other.IsValid;

        ++*Counter;
        IsOriginal = false;

        other.Mtx->unlock();
    }

    /// \brief Assignment operator
    /// \param [in] other other MutexValidator object
    MutexValidator& operator=(const MutexValidator& other)
    {
        // Check if it is not same object
        if (this != &other)
        {
            // Clear old data
            Mtx->lock();

            --*Counter;
            if (*Counter == 0)
            {
                delete Mtx;
                delete Counter;
                delete IsValid;
            }
        
            Mtx->unlock();

            // Set new data
            other.Mtx->lock();

            Mtx = other.Mtx;
            Counter = other.Counter;
            IsValid = other.IsValid;

            ++*Counter;
            IsOriginal = false;

            other.Mtx->unlock();
        }

        return *this;
    }

    /// \brief A method for verifying the validity of the main object
    /// \warning Should only be used inside Lock and Unlock
    /// \return returns the true if the main object is valid, otherwise it is false
    bool GetIsValid() const
    {
        return *IsValid;
    }

    /// \brief Lock code section to thread-safety
    void Lock()
    {
        Mtx->lock();
    }

    /// \brief Try to lock code section to thread-safety
    /// \return true if locked, false otherwise
    bool TryLock()
    {
        return Mtx->try_lock();
    }

    /// \brief Unlock code section to thread-safety
    void Unlock()
    {
        Mtx->unlock();
    }

    /// \brief Default destructor
    ~MutexValidator()
    {
        Mtx->lock();

        if(IsOriginal)
            *IsValid = false;
        
        --*Counter;
        
        if (*Counter == 0)
        {
            delete Mtx;
            delete Counter;
            delete IsValid;
        }

        Mtx->unlock();
    }
};

int main()
{
    MutexValidator* mwvp = new MutexValidator;
    MutexValidator mwv(*mwvp);

    mwv.Lock();
    std::cout << mwv.GetIsValid() << std::endl;
    mwv.Unlock();

    delete mwvp;

    mwv.Lock();
    std::cout << mwv.GetIsValid() << std::endl;
    mwv.Unlock();
}
