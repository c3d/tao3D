#ifndef GC_H
#define GC_H
// ****************************************************************************
//  gc.h                                                            Tao project
// ****************************************************************************
//
//   File Description:
//
//     Garbage collector managing memory for us
//
//
//
//
//
//
//
//
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "base.h"
#include <vector>
#include <map>
#include <cassert>
#include <stdint.h>

XL_BEGIN

// ============================================================================
//
//   Class declarations
//
// ============================================================================

struct AllocatorBase
// ----------------------------------------------------------------------------
//   Structure allocating data for a single data type
// ----------------------------------------------------------------------------
{
    union Chunk
    {
        Chunk *         next;           // Next in free list
        AllocatorBase * allocator;      // Allocator for this object
        uintptr_t       bits;           // Allocation bits
    };

public:
    AllocatorBase(uint objectSize);
    virtual ~AllocatorBase();

    void *              Allocate();
    void                Delete(void *);
    virtual void        Finalize(void *);

    void                MarkRoots();
    void                Mark(Chunk *inUse);
    void                Sweep();

    static AllocatorBase *ValidPointer(AllocatorBase *ptr);

    uint &              Roots(void *ptr) { return roots[ptr]; }

public:
    enum ChunkBits
    {
        PTR_MASK        = 15,           // Special bits we take out of the ptr
        USE_MASK        = 7,            // Bits used as reference counter
        ALLOCATED       = 0,            // Just allocated
        LOCKED_ROOT     = 7,            // Too many references to fit in mask
        IN_USE          = 8             // Set if already marked this time
    };

protected:
    std::vector<Chunk*> chunks;
    std::vector<uint>   offsets; // Offset for pointers
    std::map<void*,uint>roots;
    Chunk *             freeList;
    uint                chunkSize;
    uint                objectSize;
    uint                alignedSize;
    uint                available;
};


template <class Object> struct GCPtr;


template <class Object>
struct Allocator : AllocatorBase
// ----------------------------------------------------------------------------
//    Allocate objects for a given object type
// ----------------------------------------------------------------------------
{
    typedef Object  object_t;
    typedef Object *ptr_t;

public:
    Allocator();

    static Allocator *  Singleton();
    static Object *     Allocate(size_t size);
    static void         Delete(Object *);
    virtual void        Finalize(void *object);
    void                RegisterPointers();

public:
    struct PointerRegistry
    {
        PointerRegistry(Object *obj, Allocator *alloc)
            : object(obj), allocator(alloc) {}
        Object    *object;
        Allocator *allocator;

        template<class Ptr>
        PointerRegistry &operator+=(Ptr *&ptr)
        {
            allocator->offsets.push_back((char *) &ptr - (char *) object);
            return *this;
        }

        template<class Ptr>
        PointerRegistry &operator,(Ptr *&ptr)
        {
            allocator->offsets.push_back((char *) &ptr - (char *) object);
            return *this;
        }

        template<class O>
        PointerRegistry &operator+=(GCPtr<O> &ptr)
        {
            allocator->offsets.push_back((char *) &ptr - (char *) object);
            return *this;
        }

        template<class O>
        PointerRegistry &operator,(GCPtr<O> &ptr)
        {
            allocator->offsets.push_back((char *) &ptr - (char *) object);
            return *this;
        }
    };
};


template<class Object>
struct GCPtr
// ----------------------------------------------------------------------------
//   A root pointer to an object in a garbage-collected pool
// ----------------------------------------------------------------------------
{
    typedef AllocatorBase       Base;
    typedef Allocator<Object>   Alloc;

public:
    GCPtr(Object *ptr): pointer(ptr)            { Acquire(ptr); }
    template<class U>
    GCPtr(GCPtr<U> *ptr): pointer(ptr->pointer) { Acquire(pointer); }
    ~GCPtr()                                    { Release(pointer); }

    operator Object* ()                         { return pointer; }
    Object *operator->()                        { return pointer; }

    template<class U>
    GCPtr& operator=(const GCPtr<U> &o)
    {
        if (o.pointer != pointer)
        {
            Release(pointer);
            pointer = o.operator U*();
            Acquire(pointer);
        }
        return *this;
    }

    template<class U>
    bool operator==(const GCPtr<U> &o)
    {
        return pointer == o.operator U*();
    }

    template<class U>
    bool operator<(const GCPtr<U> &o)
    {
        return pointer < o.operator U*();
    }

private:
    void        Acquire(Object *ptr);
    void        Release(Object *ptr);

    Object *    pointer;
};


struct GarbageCollector
// ----------------------------------------------------------------------------
//    Structure registering all allocators
// ----------------------------------------------------------------------------
{
    GarbageCollector();
    ~GarbageCollector();

    void        Register(AllocatorBase *a);
    void        RunCollection();
    void        MustRun()    { mustRun = true; }

    static GarbageCollector *   Singleton();
    static void                 Collect();
    static void                 CollectionNeeded() { Singleton()->MustRun(); }

protected:
    std::vector<AllocatorBase *> allocators;
    bool mustRun;
};


inline void GC() { GarbageCollector::Singleton()->Collect(); }



// ============================================================================
//
//    Macros used to declare a garbage-collected type
//
// ============================================================================

// Define a garbage collected tree
// Intended usage : GARBAGE_COLLECTED(Tree) { pointers = x, y, z; }

#define GARBAGE_COLLECTED(type)                                         \
    void *operator new(size_t size)                                     \
    {                                                                   \
        return XL::Allocator<type>::Allocate(size);                     \
    }                                                                   \
                                                                        \
    void operator delete(void *ptr)                                     \
    {                                                                   \
        XL::Allocator<type>::Delete((type *) ptr);                      \
    }                                                                   \
                                                                        \
    void RegisterPointers(XL::Allocator<type>::PointerRegistry &pointers)




// ============================================================================
//
//   Inline functions for base allocator
//
// ============================================================================

inline AllocatorBase *AllocatorBase::ValidPointer(AllocatorBase *ptr)
// ----------------------------------------------------------------------------
//   Return a valid pointer from a possibly marked pointer
// ----------------------------------------------------------------------------
{
    return (AllocatorBase *) (((uintptr_t) ptr) & ~PTR_MASK);
}



// ============================================================================
//
//   Definitions for template Allocator
//
// ============================================================================

template<class Object> inline
Allocator<Object>::Allocator()
// ----------------------------------------------------------------------------
//   Create an allocator for the given size
// ----------------------------------------------------------------------------
    : AllocatorBase(sizeof (Object))
{}


template<class Object> inline
Allocator<Object> * Allocator<Object>::Singleton()
// ----------------------------------------------------------------------------
//    Return a singleton for the allocation class
// ----------------------------------------------------------------------------
{
    static Allocator *allocator = NULL;
    if (!allocator)
    {
        // Create the singleton
        allocator = new Allocator;

        // Register all pointer offsets
        Object *nullObj = NULL;
        PointerRegistry registry(nullObj, allocator);
        nullObj->RegisterPointers(registry);

        // Register the allocator with the garbage collector
        GarbageCollector::Singleton()->Register(allocator);
    }
    return allocator;
}


template<class Object> inline
Object *Allocator<Object>::Allocate(size_t size)
// ----------------------------------------------------------------------------
//   Allocate an object (invoked by operator new)
// ----------------------------------------------------------------------------
{
    assert(size == Singleton()->AllocatorBase::objectSize);
    return (Object *) Singleton()->AllocatorBase::Allocate();
}


template<class Object> inline
void Allocator<Object>::Delete(Object *obj)
// ----------------------------------------------------------------------------
//   Allocate an object (invoked by operator new)
// ----------------------------------------------------------------------------
{
    Singleton()->AllocatorBase::Delete(obj);
}


template<class Object> inline
void Allocator<Object>::Finalize(void *obj)
// ----------------------------------------------------------------------------
//   Make sure that we properly call the destructor for the object
// ----------------------------------------------------------------------------
{
    Object *object = (Object *) obj;
    delete object;
}



// ============================================================================
// 
//     GCPtr inline functions
//
// ============================================================================

template<class Object> inline
void GCPtr<Object>::Acquire(Object *ptr)
// ----------------------------------------------------------------------------
//   Increment the reference counter for the given pointer
// ----------------------------------------------------------------------------
{
    if (ptr)
    {
        Base::Chunk *chunk = ((Base::Chunk *) ptr) - 1;
        uint count = chunk->bits & Alloc::USE_MASK;
        if (count < Alloc::LOCKED_ROOT)
            chunk->bits = (chunk->bits & ~Alloc::USE_MASK) | ++count;
        if (count >= Alloc::LOCKED_ROOT)
        {
            Base *allocator = Base::ValidPointer(chunk->allocator);
            count = Alloc::LOCKED_ROOT + allocator->Roots(ptr)++;
        }
    }
}


template<class Object> inline
void GCPtr<Object>::Release(Object *ptr)
// ----------------------------------------------------------------------------
//   Decrement the reference counter for the given pointer
// ----------------------------------------------------------------------------
{
    if (pointer)
    {
        Base::Chunk *chunk = ((Base::Chunk *) ptr) - 1;
        uint count = chunk->bits & Alloc::USE_MASK;
        Base *allocator = Base::ValidPointer(chunk->allocator);
        if (count < Alloc::LOCKED_ROOT)
        {
            chunk->bits = (chunk->bits & ~Alloc::USE_MASK) | --count;
            if (!count)
                allocator->Finalize(ptr);
        }
        else if (--allocator->Roots(ptr) == 0)
        {
            chunk->bits = (chunk->bits & ~Alloc::USE_MASK) | --count;
        }
    }
}

XL_END

#endif // GC_H
