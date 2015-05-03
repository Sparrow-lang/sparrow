#pragma once


namespace Nest { namespace Common
{
    /**
     * \brief   Helper class template for saving the value of a variable
     *
     * \param   T   The type of the variable to save the value for
     *
     * This can be used to save the value of an object, and restore it at a later point. The value
     * is stored in the constructor and restored in the destructor. It can act as a scope guard for
     * restoring initial states of the objects.
     *
     * The assignment operator of the given type must not throw.
     *
     * \see     saveValue()
     */
    template <typename T>
    class SaveValue
    {
    public:
        /**
         * \brief   Constructor
         *
         * Stores the value of the given object, by copying the state of the object into another
         * object. The copy constructor will be invoked.
         *
         * Throws whatever the copy constrcutor will throw.
         */
        SaveValue(T& obj)
            : obj_(&obj)
            , value_(obj)
        {
        }

         /**
          * \brief  Restores the value of the object
          *
          * This will invoke the assignment operator of the T type. This operator must not throw
          * anyhting.
          */
        ~SaveValue()
        {
            // Restore the initial value
            *obj_ = value_;
        }

        /// Getter for the original value
        const T& originalValue() const { return value_; }

    private:
        /// Pointer to the variable for which we store the state
        T* obj_;
        
        /// The old state of the variable that will be restored on destructor
        T value_;
    };
}}
