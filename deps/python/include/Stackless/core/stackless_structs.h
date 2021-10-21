#ifndef STACKLESS_STRUCTS_H
#define STACKLESS_STRUCTS_H

#ifdef __cplusplus
extern "C" {
#endif

/* platform specific constants (mainly SEH stuff to store )*/
#include "platf/slp_platformselect.h"



/*** important structures: tasklet ***/


/***************************************************************************

    Tasklet Flag Definition
    -----------------------

    blocked:        The tasklet is either waiting in a channel for
                    writing (1) or reading (-1) or not blocked (0).
                    Maintained by the channel logic. Do not change.

    atomic:         If true, schedulers will never switch. Driven by
                    the code object or dynamically, see below.

    ignore_nesting: Allows auto-scheduling, even if nesting_level
                    is not zero.

    autoschedule:   The tasklet likes to be auto-scheduled. User driven.

    block_trap:     Debugging aid. Whenever the tasklet would be
                    blocked by a channel, an exception is raised.

    is_zombie:      This tasklet is almost dead, its deallocation has
                    started. The tasklet *must* die at some time, or the
                    process can never end.

    pending_irq:    If set, an interrupt was issued during an atomic
                    operation, and should be handled when possible.


    Policy for atomic/autoschedule and switching:
    ---------------------------------------------
    A tasklet switch can always be done explicitly by calling schedule().
    Atomic and schedule are concerned with automatic features.

    atomic  autoschedule

    1           any     Neither a scheduler nor a watchdog will
                    try to switch this tasklet.

    0           0       The tasklet can be stopped on desire, or it
                    can be killed by an exception.

    0           1       Like above, plus auto-scheduling is enabled.

    Default settings:
    -----------------
    All flags are zero by default.

 ***************************************************************************/

#define SLP_TASKLET_FLAGS_BITS_blocked 2
#define SLP_TASKLET_FLAGS_BITS_atomic 1
#define SLP_TASKLET_FLAGS_BITS_ignore_nesting 1
#define SLP_TASKLET_FLAGS_BITS_autoschedule 1
#define SLP_TASKLET_FLAGS_BITS_block_trap 1
#define SLP_TASKLET_FLAGS_BITS_is_zombie 1
#define SLP_TASKLET_FLAGS_BITS_pending_irq 1

#define SLP_TASKLET_FLAGS_OFFSET_blocked 0
#define SLP_TASKLET_FLAGS_OFFSET_atomic \
    (SLP_TASKLET_FLAGS_OFFSET_blocked + SLP_TASKLET_FLAGS_BITS_blocked)
#define SLP_TASKLET_FLAGS_OFFSET_ignore_nesting \
    (SLP_TASKLET_FLAGS_OFFSET_atomic + SLP_TASKLET_FLAGS_BITS_atomic)
#define SLP_TASKLET_FLAGS_OFFSET_autoschedule \
    (SLP_TASKLET_FLAGS_OFFSET_ignore_nesting + SLP_TASKLET_FLAGS_BITS_ignore_nesting)
#define SLP_TASKLET_FLAGS_OFFSET_block_trap \
    (SLP_TASKLET_FLAGS_OFFSET_autoschedule + SLP_TASKLET_FLAGS_BITS_autoschedule)
#define SLP_TASKLET_FLAGS_OFFSET_is_zombie \
    (SLP_TASKLET_FLAGS_OFFSET_block_trap + SLP_TASKLET_FLAGS_BITS_block_trap)
#define SLP_TASKLET_FLAGS_OFFSET_pending_irq \
    (SLP_TASKLET_FLAGS_OFFSET_is_zombie + SLP_TASKLET_FLAGS_BITS_is_zombie)

typedef struct _tasklet_flags {
    signed int blocked: SLP_TASKLET_FLAGS_BITS_blocked;
    unsigned int atomic: SLP_TASKLET_FLAGS_BITS_atomic;
    unsigned int ignore_nesting: SLP_TASKLET_FLAGS_BITS_ignore_nesting;
    unsigned int autoschedule: SLP_TASKLET_FLAGS_BITS_autoschedule;
    unsigned int block_trap: SLP_TASKLET_FLAGS_BITS_block_trap;
    unsigned int is_zombie: SLP_TASKLET_FLAGS_BITS_is_zombie;
    unsigned int pending_irq: SLP_TASKLET_FLAGS_BITS_pending_irq;
} PyTaskletFlagStruc;

/* a partial copy of PyThreadState. Used to preserve 
   the state of a hard switched tasklet */
typedef struct _tasklet_tstate {
    int tracing;
    Py_tracefunc c_profilefunc;
    Py_tracefunc c_tracefunc;
    PyObject *c_profileobj;
    PyObject *c_traceobj;

    PyObject *exc_type;
    PyObject *exc_value;
    PyObject *exc_traceback;
} PyTaskletTStateStruc;

typedef struct _tasklet {
    PyObject_HEAD
    struct _tasklet *next;
    struct _tasklet *prev;
    union {
        struct _frame *frame;
        struct _cframe *cframe;
    } f;
    PyObject *tempval;
    /* bits stuff */
    struct _tasklet_flags flags;
    int recursion_depth;
    struct _cstack *cstate;
    PyObject *def_globals;
    PyObject *tsk_weakreflist;
} PyTaskletObject;


/*** important structures: cstack ***/

typedef struct _cstack {
    PyObject_VAR_HEAD
    struct _cstack *next;
    struct _cstack *prev;
#ifdef have_long_long
    long_long serial;
#else
    long serial;
#endif
    struct _tasklet *task;
    int nesting_level;
    PyThreadState *tstate;
#ifdef _SEH32
    DWORD exception_list; /* SEH handler on Win32 */
#endif
    intptr_t *startaddr;
    intptr_t stack[1];
} PyCStackObject;


/*** important structures: bomb ***/

typedef struct _bomb {
    PyObject_HEAD
    PyObject *curexc_type;
    PyObject *curexc_value;
    PyObject *curexc_traceback;
} PyBombObject;

/*** important structures: channel ***/

/***************************************************************************

    Channel Flag Definition
    -----------------------


    closing:        When the closing flag is set, the channel does not
                    accept to be extended. The computed attribute
                    'closed' is true when closing is set and the
                    channel is empty.

    preference:     0    no preference, caller will continue
                    1    sender will be inserted after receiver and run
                    -1   receiver will be inserted after sender and run

    schedule_all:   ignore preference and always schedule the next task

    Default settings:
    -----------------
    All flags are zero by default.

 ***************************************************************************/

#define SLP_CHANNEL_FLAGS_BITS_closing 1
#define SLP_CHANNEL_FLAGS_BITS_preference 2
#define SLP_CHANNEL_FLAGS_BITS_schedule_all 1

#define SLP_CHANNEL_FLAGS_OFFSET_closing 0
#define SLP_CHANNEL_FLAGS_OFFSET_preference \
    (SLP_CHANNEL_FLAGS_OFFSET_closing + SLP_CHANNEL_FLAGS_BITS_closing)
#define SLP_CHANNEL_FLAGS_OFFSET_schedule_all \
    (SLP_CHANNEL_FLAGS_OFFSET_preference + SLP_CHANNEL_FLAGS_BITS_preference)

typedef struct _channel_flags {
    unsigned int closing: SLP_CHANNEL_FLAGS_BITS_closing;
    signed int preference: SLP_CHANNEL_FLAGS_BITS_preference;
    unsigned int schedule_all: SLP_CHANNEL_FLAGS_BITS_schedule_all;
} PyChannelFlagStruc;

typedef struct _channel {
    PyObject_HEAD
    /* make sure that these fit tasklet's next/prev */
    struct _tasklet *head;
    struct _tasklet *tail;
    int balance;
    struct _channel_flags flags;
    PyObject *chan_weakreflist;
} PyChannelObject;


/*** important stuctures: cframe ***/

typedef struct _cframe {
    PyObject_VAR_HEAD
    struct _frame *f_back;      /* previous frame, or NULL */
    PyFrame_ExecFunc *f_execute;

    /*
     * the above part is compatible with frames.
     * Note that I have re-arranged some fields in the frames
     * to keep cframes as small as possible.
     */

    /* these can be used as the CFrame likes to */

    PyObject *ob1;
    PyObject *ob2;
    PyObject *ob3;
    long i, n;
    void *any1;
    void *any2;
} PyCFrameObject;



/*** associated type objects ***/

PyAPI_DATA(PyTypeObject) PyCFrame_Type;
#define PyCFrame_Check(op) ((op)->ob_type == &PyCFrame_Type)

PyAPI_DATA(PyTypeObject) PyCStack_Type;
#define PyCStack_Check(op) ((op)->ob_type == &PyCStack_Type)

PyAPI_DATA(PyTypeObject) PyBomb_Type;
#define PyBomb_Check(op) ((op)->ob_type == &PyBomb_Type)

PyAPI_DATA(PyTypeObject) PyTasklet_Type;
#define PyTasklet_Check(op) PyObject_TypeCheck(op, &PyTasklet_Type)
#define PyTasklet_CheckExact(op) ((op)->ob_type == &PyTasklet_Type)

PyAPI_DATA(PyTypeObject) PyChannel_Type;
#define PyChannel_Check(op) PyObject_TypeCheck(op, &PyChannel_Type)
#define PyChannel_CheckExact(op) ((op)->ob_type == &PyChannel_Type)

/*** these are in other bits of C-Python(r) ***/
PyAPI_DATA(PyTypeObject) PyDictIterKey_Type;
PyAPI_DATA(PyTypeObject) PyDictIterValue_Type;
PyAPI_DATA(PyTypeObject) PyDictIterItem_Type;
PyAPI_DATA(PyTypeObject) PyListIter_Type;
PyAPI_DATA(PyTypeObject) PySetIter_Type;
PyAPI_DATA(PyTypeObject) PyRangeIter_Type;
PyAPI_DATA(PyTypeObject) PyTupleIter_Type;
PyAPI_DATA(PyTypeObject) PyEnum_Type;

#ifdef __cplusplus
}
#endif

#endif
