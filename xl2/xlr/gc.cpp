// ****************************************************************************
//  gc.cpp                                                          Tao project
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

#include "gc.h"
#include <iostream>

XL_BEGIN

// ============================================================================
//
//    Allocator Base class
//
// ============================================================================

AllocatorBase::AllocatorBase(kstring tn, uint os, mark_fn mark,
                             GarbageCollector *gc)
// ----------------------------------------------------------------------------
//    Setup an empty allocator
// ----------------------------------------------------------------------------
    : typeName(tn), gc(gc), chunks(), mark(mark), freeList(NULL),
      chunkSize(1022), objectSize(os), alignedSize(os), available(0)
{
    // Make sure that our allocator generates properly aligned addresses
    uint size = PTR_MASK + 1;
    while (os + sizeof(Chunk) > size)
        size <<= 1;
    alignedSize = size - sizeof(Chunk);
}


AllocatorBase::~AllocatorBase()
// ----------------------------------------------------------------------------
//   Delete all the chunks we allocated
// ----------------------------------------------------------------------------
{
    std::vector<Chunk *>::iterator c;
    for (c = chunks.begin(); c != chunks.end(); c++)
        free(*c);
}


void *AllocatorBase::Allocate()
// ----------------------------------------------------------------------------
//   Allocate a chunk of the given size
// ----------------------------------------------------------------------------
{
    Chunk *result = freeList;
    if (!result)
    {
        // Nothing free: allocate a big enough chunk
        size_t  itemSize  = alignedSize + sizeof(Chunk);
        void   *allocated = malloc((chunkSize + 1) * itemSize);
        char   *chunkBase = (char *) allocated + alignedSize;
        chunks.push_back((Chunk *) allocated);
        for (uint i = 0; i < chunkSize; i++)
        {
            Chunk *ptr = (Chunk *) (chunkBase + i * itemSize);
            ptr->next = result;
            result = ptr;
        }
        freeList = result;
        available += chunkSize;
    }

    // REVISIT: Atomic operations here
    freeList = result->next;
    result->allocator = this;
    result->bits |= IN_USE;     // In case a collection is running right now
    if (--available < chunkSize / 4)
        GarbageCollector::CollectionNeeded();

    return &result[1];
}


void AllocatorBase::Delete(void *ptr)
// ----------------------------------------------------------------------------
//   Free a chunk of the given size
// ----------------------------------------------------------------------------
{
    Chunk *chunk = (Chunk *) ptr - 1;
    assert(ValidPointer(chunk->allocator) == this);
    chunk->next = freeList;
    freeList = chunk;
    available++;

#if 1
    // Scrub all the pointers
    uint32 *base = (uint32 *) ptr;
    uint32 *last = (uint32 *) (((char *) ptr) + alignedSize);
    for (uint *p = base; p < last; p++)
        *p = 0xDeadBeef;
#endif
}


void AllocatorBase::Finalize(void *ptr)
// ----------------------------------------------------------------------------
//   We should never reach this one
// ----------------------------------------------------------------------------
{
    std::cerr << "No finalizer installed for " << ptr << "\n";
    assert(!"No finalizer installed");
}


void AllocatorBase::MarkRoots()
// ----------------------------------------------------------------------------
//    Loop on all objects that have a reference count above 1
// ----------------------------------------------------------------------------
{
    std::vector<Chunk *>::iterator chunk;
    for (chunk = chunks.begin(); chunk != chunks.end(); chunk++)
    {
        char *chunkBase = (char *) *chunk + alignedSize;
        size_t  itemSize  = alignedSize + sizeof(Chunk);
        for (uint i = 0; i < chunkSize; i++)
        {
            Chunk *ptr = (Chunk *) (chunkBase + i * itemSize);
            if (ValidPointer(ptr->allocator) == this)
                if ((ptr->bits & IN_USE) == 0)
                    if ((ptr->bits & USE_MASK) > 0)
                        Mark(ptr+1);
        }
    }
}


void AllocatorBase::Mark(void *data)
// ----------------------------------------------------------------------------
//   Loop on all allocated items and mark which ones are in use
// ----------------------------------------------------------------------------
{
    if (!data)
        return;

    // Find chunk pointer
    Chunk *inUse = ((Chunk *) data) - 1;

    // We should only look at allocated items, otherwise oops...
    if (inUse->bits & IN_USE)
        return;

    // We'd better be in the right allocator
    assert(ValidPointer(inUse->allocator) == this);

    // We had not marked that one yet, mark it now
    inUse->bits |= IN_USE;

    // Mark all pointers in this item
    mark(data);
}


void AllocatorBase::Sweep()
// ----------------------------------------------------------------------------
//   Once we have marked everything, sweep what is not in use
// ----------------------------------------------------------------------------
{
    std::vector<Chunk *>::iterator chunk;
    for (chunk = chunks.begin(); chunk != chunks.end(); chunk++)
    {
        char *chunkBase = (char *) *chunk + alignedSize;
        size_t  itemSize  = alignedSize + sizeof(Chunk);
        for (uint i = 0; i < chunkSize; i++)
        {
            Chunk *ptr = (Chunk *) (chunkBase + i * itemSize);
            if (ValidPointer(ptr->allocator) == this)
            {
                if (ptr->bits & IN_USE)
                    ptr->bits &= ~IN_USE;
                else if ((ptr->bits & USE_MASK) == 0)
                    Finalize(ptr+1);
            }
        }
    }
}



// ============================================================================
//
//   Garbage Collector class
//
// ============================================================================

GarbageCollector::GarbageCollector()
// ----------------------------------------------------------------------------
//   Create the garbage collector
// ----------------------------------------------------------------------------
    : mustRun(false)
{}


GarbageCollector::~GarbageCollector()
// ----------------------------------------------------------------------------
//    Destroy the garbage collector
// ----------------------------------------------------------------------------
{
    std::vector<AllocatorBase *>::iterator i;
    for (i = allocators.begin(); i != allocators.end(); i++)
        delete *i;
}


void GarbageCollector::Register(AllocatorBase *allocator)
// ----------------------------------------------------------------------------
//    Record each individual allocator
// ----------------------------------------------------------------------------
{
    allocators.push_back(allocator);
}


void GarbageCollector::RunCollection(bool force)
// ----------------------------------------------------------------------------
//   Run garbage collection on all the allocators we own
// ----------------------------------------------------------------------------
{
    if (mustRun || force)
    {
        std::vector<AllocatorBase *>::iterator i;
        std::set<Listener *>::iterator l;
        mustRun = false;

        // Notify all the listeners that we begin a collection
        for (l = listeners.begin(); l != listeners.end(); l++)
            (*l)->BeginCollection();

        // Mark roots in all the allocators
        for (i = allocators.begin(); i != allocators.end(); i++)
            (*i)->MarkRoots();

        // Then sweep whatever was not referenced
        for (i = allocators.begin(); i != allocators.end(); i++)
            (*i)->Sweep();

        // Notify all the listeners that we completed the collection
        for (l = listeners.begin(); l != listeners.end(); l++)
            (*l)->EndCollection();
    }
}


GarbageCollector *GarbageCollector::Singleton()
// ----------------------------------------------------------------------------
//   Return the garbage collector
// ----------------------------------------------------------------------------
{
    static GarbageCollector *gc = NULL;
    if (!gc)
        gc = new GarbageCollector;
    return gc;
}


void GarbageCollector::Collect(bool force)
// ----------------------------------------------------------------------------
//   Collect garbage in this garbage collector
// ----------------------------------------------------------------------------
{
    Singleton()->RunCollection(force);
}


bool GarbageCollector::CanDelete(void *obj)
// ----------------------------------------------------------------------------
//   Ask all the listeners if it's OK to delete the object
// ----------------------------------------------------------------------------
{
    bool result = true;
    std::set<Listener *>::iterator i;
    for (i = listeners.begin(); i != listeners.end(); i++)
        if (! (*i)->CanDelete(obj))
            result = false;
    return result;
}

XL_END
